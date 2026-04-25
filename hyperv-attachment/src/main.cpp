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
    bool is_hypercall_context_initialized = false;
#ifdef _INTELMACHINE
    volatile long is_cr4_shadowing_enabled = 0;
    volatile long is_cpuid_spoofing_enabled = 0;
    volatile long is_feature_control_shadowing_enabled = 0;
    volatile long is_tsc_offsetting_enabled = 0;
#endif
}

// Phải ở file scope (không trong anonymous namespace) để hypercall.cpp có thể extern-link
std::uint64_t authorized_caller_cr3 = 0;

// =========================================================================
// EPT Mouse Hook - INT3 Injection Handler
// Khi shadow page có INT3 được execute, bơm MOUSE_INPUT_DATA vào guest.
// MOUSE_INPUT_DATA layout (WDM):
//   +0x00: USHORT  UnitId
//   +0x02: USHORT  Reserved
//   +0x04: USHORT  Flags     (MOUSE_MOVE_RELATIVE = 0)
//   +0x06: USHORT  Reserved2
//   +0x08: ULONG   RawButtons
//   +0x0C: ULONG   ButtonFlags
//   +0x10: ULONG   ButtonData
//   +0x14: ULONG   ExtraInformation
//   +0x18: LONG    LastX  <- dx
//   +0x1C: LONG    LastY  <- dy
// =========================================================================
static void try_inject_mouse_from_bp(const trap_frame_t* const trap_frame)
{
    const std::int32_t dx = g_pending_mouse_x;
    const std::int32_t dy = g_pending_mouse_y;
    g_pending_mouse_x = 0;
    g_pending_mouse_y = 0;
    if (dx == 0 && dy == 0) return;

    // Tại thời điểm thực thi byte đầu tiên của MouseClassServiceCallback,
    // Argument 2 (RDX) chứa con trỏ tới MOUSE_INPUT_DATA đầu tiên trong chuỗi.
    const std::uint64_t input_data_gva = trap_frame->rdx;
    if (input_data_gva == 0) return;

    const cr3 guest_cr3  = arch::get_guest_cr3();
    const cr3 slat_cr3   = slat::hyperv_cr3();

    // Dịch InputDataStart (GVA) sang GPA để sửa
    const std::uint64_t input_data_gpa = memory_manager::translate_guest_virtual_address(
        guest_cr3, slat_cr3, { .address = input_data_gva });

    if (input_data_gpa == 0) return;

    // Map physical memory của guest ra host (0xFFFFFF... memory base)
    auto* const mapped_input_data = static_cast<std::uint8_t*>(
        memory_manager::map_host_physical(input_data_gpa));

    if (mapped_input_data == nullptr) return;

    // Flags (Offset +0x04)
    auto* const flags = reinterpret_cast<std::uint16_t*>(mapped_input_data + 0x04);

    // Xóa cờ MOUSE_MOVE_ABSOLUTE (1) để hệ thống nhận diện đây là Relative (0)
    *flags &= ~1;

    // LastX (Offset +0x18), LastY (Offset +0x1C)
    // Sửa trực tiếp buffer mà Windows truyền cho MouseClassServiceCallback
    auto* const last_x = reinterpret_cast<std::int32_t*>(mapped_input_data + 0x18);
    auto* const last_y = reinterpret_cast<std::int32_t*>(mapped_input_data + 0x1C);

    *last_x += dx;
    *last_y += dy;
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

#ifdef _INTELMACHINE
    if (is_cr4_shadowing_enabled == 1)
    {
        arch::enable_cr4_shadowing();
    }

    if (is_tsc_offsetting_enabled == 1)
    {
        arch::enable_tsc_exiting();
    }
#endif

    const std::uint64_t exit_reason = arch::get_vmexit_reason();

    if (arch::is_cpuid(exit_reason) == 1)
    {
#ifdef _INTELMACHINE
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a1);
#else
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a2);
#endif

        const hypercall_info_t hypercall_info = { .value = trap_frame->rcx };

        bool is_valid_key = (hypercall_info.primary_key == current_primary_key && hypercall_info.secondary_key == current_secondary_key);

        // V4: Cho phép Client tự khởi tạo lại nếu nó gửi đúng Cờ khởi tạo kèm Khóa mặc định (tránh bị Block sau lần chạy đầu).
        if (!is_valid_key && hypercall_info.call_type == hypercall_type_t::_hc_0x210) 
        {
            if (hypercall_info.primary_key == hypercall_default_primary_key && 
                hypercall_info.secondary_key == hypercall_default_secondary_key) 
            {
                is_valid_key = true;
            }
        }

        if (is_valid_key)
        {
#ifndef _INTELMACHINE
            vmcb_t* const vmcb = arch::get_vmcb();

            trap_frame->rax = vmcb->save_state.rax;
#endif

            const std::uint64_t guest_cr3 = arch::get_guest_cr3().flags;

            if (hypercall_info.call_type == hypercall_type_t::_hc_0x210)
            {
                // Obfuscated magic: use centralized seeds from hypercall_def.h
                if (trap_frame->rdx != (hypercall_magic_seed_a ^ hypercall_magic_seed_b))
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
#ifdef _INTELMACHINE
                _InterlockedExchange(&is_cr4_shadowing_enabled, 1);
                _InterlockedExchange(&is_cpuid_spoofing_enabled, 1);
                _InterlockedExchange(&is_feature_control_shadowing_enabled, 1);
                _InterlockedExchange(&is_tsc_offsetting_enabled, 1);
                arch::enable_cr4_shadowing();
                arch::enable_tsc_exiting();
#endif

                current_primary_key = (guest_cr3 ^ __rdtsc()) & 0xFFFF;
                if (current_primary_key == 0) current_primary_key = 0xD3AD;
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

#ifdef _INTELMACHINE
        if (is_cpuid_spoofing_enabled == 1 && arch::handle_cpuid_spoof(trap_frame) == 1)
        {
            return do_vmexit_premature_return();
        }
#endif
    }
#ifdef _INTELMACHINE
    else if (is_cr4_shadowing_enabled == 1 && arch::is_mov_cr(exit_reason) == 1)
    {
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a1);

        if (arch::handle_cr4_mov_exit(trap_frame) == 1)
        {
            return do_vmexit_premature_return();
        }
    }
#endif
#ifdef _INTELMACHINE
    else if (is_tsc_offsetting_enabled == 1 && arch::is_tsc_exit(exit_reason) == 1)
    {
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a1);

        if (arch::handle_tsc_exit(exit_reason, trap_frame) == 1)
        {
            return do_vmexit_premature_return();
        }
    }
#endif
#ifdef _INTELMACHINE
    else if (is_feature_control_shadowing_enabled == 1 && arch::is_rdmsr(exit_reason) == 1)
    {
        trap_frame_t* const trap_frame = *reinterpret_cast<trap_frame_t**>(a1);

        if (arch::handle_feature_control_rdmsr(trap_frame) == 1)
        {
            return do_vmexit_premature_return();
        }
    }
#endif
    else if (arch::is_slat_violation(exit_reason) == 1 && slat::violation::process() == 1)
    {
        return do_vmexit_premature_return();
    }
    else if (arch::is_non_maskable_interrupt_exit(exit_reason) == 1)
    {
        interrupts::process_nmi();
    }
    else if (arch::is_breakpoint_exit(exit_reason) == 1 && g_mouse_shadow_page_va != 0)
    {
        const std::uint64_t rip = arch::get_guest_rip();

        // Tính VA kỳ vọng: page base + offset hàm callback trong page
        const std::uint64_t expected_hook_rip = (g_mouse_callback_va & ~0xFFFULL) | g_mouse_func_offset;

        if (rip == expected_hook_rip)
        {
            // Đúng là hook của ta. Lấy trap_frame để đọc RDX.
            trap_frame_t* trap_frame;
#ifdef _INTELMACHINE
            trap_frame = reinterpret_cast<trap_frame_t*>(a4);
#else
            trap_frame = reinterpret_cast<trap_frame_t*>(a2);
#endif
            // Bơm tọa độ chuột vào struct MOUSE_INPUT_DATA của guest
            try_inject_mouse_from_bp(trap_frame);

            // Skip qua INT3 byte (1 byte), tiếp tục thực thi code gốc
            arch::set_guest_rip(rip + 1);

            return do_vmexit_premature_return();
        }
        else
        {
            // Breakpoint không phải của ta (debugger, anti-cheat, etc.)
            // Ném lại exception cho guest xử lý bình thường
            arch::reinject_exception(3);
            return do_vmexit_premature_return();
        }
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
