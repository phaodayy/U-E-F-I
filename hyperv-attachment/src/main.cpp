#include "arch/arch.h"
#include "hypercall/hypercall.h"
#include "hypercall/hypercall_def.h"
#include "memory_manager/memory_manager.h"
#include "memory_manager/heap_manager.h"
#include "logs/logs.h"
#include "structures/trap_frame.h"
#include <ia32-doc/ia32.hpp>
#include <cstdint>

#include "crt/crt.h"
#include "interrupts/interrupts.h"
#include "slat/slat.h"
#include "slat/cr3/cr3.h"
#include "slat/violation/violation.h"

#include <intrin.h>

#ifndef _INTELMACHINE
#include <intrin.h>
#endif

typedef std::uint64_t(*vmexit_handler_t)(std::uint64_t a1, std::uint64_t a2, std::uint64_t a3, std::uint64_t a4);

namespace
{
    std::uint8_t* original_vmexit_handler = nullptr;
    std::uint64_t uefi_boot_physical_base_address = 0;
    std::uint64_t uefi_boot_image_size = 0;
    
    std::uint64_t current_primary_key = hypercall_default_primary_key;
    std::uint64_t current_secondary_key = hypercall_default_secondary_key;
    std::uint64_t authorized_caller_cr3 = 0;
    bool is_hypercall_context_initialized = false;
}

void clean_up_uefi_boot_image()
{
    // todo: check if windows has used this reclaimed memory
    const auto mapped_uefi_boot_base = static_cast<std::uint8_t*>(memory_manager::map_host_physical(uefi_boot_physical_base_address));

    crt::set_memory(mapped_uefi_boot_base, 0, uefi_boot_image_size);
}

void process_first_vmexit()
{
    static volatile long is_first_vmexit = 1;

    if (is_first_vmexit == 1)
    {
        if (_InterlockedCompareExchange(&is_first_vmexit, 0, 1) == 1)
        {
            slat::process_first_vmexit();
            interrupts::set_up();

            clean_up_uefi_boot_image();
        }
    }

    static volatile long has_hidden_heap_pages = 0;
    static volatile long long vmexit_count = 0;

    if (has_hidden_heap_pages == 0 && 10000 <= _InterlockedIncrement64(&vmexit_count))
    {
        // hides heap from Hyper-V slat cr3. when the hook slat cr3 is initialised, the heap must also be hidden from it
        if (_InterlockedCompareExchange(&has_hidden_heap_pages, 1, 0) == 0)
        {
            long result = slat::hide_heap_pages(slat::hyperv_cr3());
            if (result == 0)
            {
                _InterlockedExchange(&has_hidden_heap_pages, 0); // Allow retry if failed
            }
        }
    }
}

std::uint64_t do_vmexit_premature_return()
{
#ifdef _INTELMACHINE
    return 0;
#else
    return __readgsqword(0);
#endif
}

std::uint64_t vmexit_handler_detour(const std::uint64_t a1, const std::uint64_t a2, const std::uint64_t a3, const std::uint64_t a4)
{
    process_first_vmexit();

    const std::uint64_t exit_reason = arch::get_vmexit_reason();

    if (arch::is_cpuid(exit_reason) == 1)
    {
#ifdef _INTELMACHINE
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a1);
#else
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a2);
#endif

        const hypercall_info_t hypercall_info = { .value = trap_frame->rcx };

        if (hypercall_info.primary_key == current_primary_key && hypercall_info.secondary_key == current_secondary_key)
        {
#ifndef _INTELMACHINE
            vmcb_t* const vmcb = arch::get_vmcb();

            trap_frame->rax = vmcb->save_state.rax;
#endif

            const std::uint64_t guest_cr3 = arch::get_guest_cr3().flags;

            if (hypercall_info.call_type == hypercall_type_t::init_hypercall_context)
            {
                // Obfuscated magic: compile-time XOR to avoid static signature
                constexpr std::uint64_t magic_seed_a = 0xDEADFACE12345678ULL;
                constexpr std::uint64_t magic_seed_b = 0xE79A1423D87BECF6ULL;
                if (trap_frame->rdx != (magic_seed_a ^ magic_seed_b))
                {
                    trap_frame->rax = 0;
#ifndef _INTELMACHINE
                    vmcb->save_state.rax = trap_frame->rax;
#endif
                    arch::advance_guest_rip();
                    return do_vmexit_premature_return();
                }

                // Bỏ chặn "chỉ khởi tạo 1 lần" để cho phép restart lại Cheat (mà không cần khởi động lại PC).
                // Magic seed chính là chìa khóa định danh an toàn.
                authorized_caller_cr3 = guest_cr3;
                is_hypercall_context_initialized = true;

                current_primary_key = (guest_cr3 ^ __rdtsc()) & 0xFFFF;
                if (current_primary_key == 0) current_primary_key = 0x1337;
                current_secondary_key = (__rdtsc() >> 8) & 0x7F;

                trap_frame->rax = (current_primary_key << 16) | current_secondary_key;

#ifndef _INTELMACHINE
                vmcb->save_state.rax = trap_frame->rax;
#endif
                arch::advance_guest_rip();
                return do_vmexit_premature_return();
            }

            if (!is_hypercall_context_initialized || guest_cr3 != authorized_caller_cr3)
            {
                trap_frame->rax = 0;
                
#ifndef _INTELMACHINE
                vmcb->save_state.rax = trap_frame->rax;
#endif
                arch::advance_guest_rip();
                return do_vmexit_premature_return();
            }

            trap_frame->rsp = arch::get_guest_rsp();

            hypercall::process(hypercall_info, trap_frame);

#ifndef _INTELMACHINE
            vmcb->save_state.rax = trap_frame->rax;
#endif

            arch::set_guest_rsp(trap_frame->rsp);
            arch::advance_guest_rip();

            return do_vmexit_premature_return();
        }
    }
    else if (arch::is_slat_violation(exit_reason) == 1 && slat::violation::process() == 1)
    {
        return do_vmexit_premature_return();
    }
    else if (arch::is_non_maskable_interrupt_exit(exit_reason) == 1)
    {
        interrupts::process_nmi();
    }

    return reinterpret_cast<vmexit_handler_t>(original_vmexit_handler)(a1, a2, a3, a4);
}

void entry_point(std::uint8_t** const vmexit_handler_detour_out, std::uint8_t* const original_vmexit_handler_routine, const std::uint64_t heap_physical_base, const std::uint64_t heap_physical_usable_base, const std::uint64_t heap_total_size, const std::uint64_t _uefi_boot_physical_base_address, const std::uint32_t _uefi_boot_image_size,
#ifdef _INTELMACHINE
    const std::uint64_t reserved_one)
{
    (void)reserved_one;

#else
const std::uint8_t* const get_vmcb_gadget)
{
    arch::parse_vmcb_gadget(get_vmcb_gadget);
#endif
    original_vmexit_handler = original_vmexit_handler_routine;
    uefi_boot_physical_base_address = _uefi_boot_physical_base_address;
    uefi_boot_image_size = _uefi_boot_image_size;

    heap_manager::initial_physical_base = heap_physical_base;
    heap_manager::initial_size = heap_total_size;

    *vmexit_handler_detour_out = reinterpret_cast<std::uint8_t*>(vmexit_handler_detour);

    const std::uint64_t heap_physical_end = heap_physical_base + heap_total_size;
    const std::uint64_t heap_usable_size = heap_physical_end - heap_physical_usable_base;

    void* const mapped_heap_usable_base = memory_manager::map_host_physical(heap_physical_usable_base);

    heap_manager::set_up(mapped_heap_usable_base, heap_usable_size);

    logs::set_up();
    slat::set_up();
}
