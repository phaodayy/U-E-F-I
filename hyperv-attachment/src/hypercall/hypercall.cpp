#include "hypercall.h"
#include "../memory_manager/memory_manager.h"
#include "../memory_manager/heap_manager.h"

#include "../slat/slat.h"
#include "../slat/cr3/cr3.h"
#include "../slat/hook/hook.h"
#include "../slat/signal/signal.h"

#include "../arch/arch.h"
#include "../logs/logs.h"
#include "../crt/crt.h"
#include "../input/backend_selector.h"

#include <ia32-doc/ia32.hpp>
#include <hypercall/hypercall_def.h>
#include <intrin.h>

// === EPT Mouse Hook Shared Globals ===
volatile std::int32_t g_pending_mouse_x   = 0;
volatile std::int32_t g_pending_mouse_y   = 0;
std::uint64_t         g_mouse_callback_va    = 0;
std::uint64_t         g_mouse_shadow_page_va = 0;
std::uint64_t         g_mouse_func_offset    = 0;
std::uint64_t         g_system_cr3           = 0;

// Forward declare authorized_caller_cr3 from main.cpp
extern std::uint64_t authorized_caller_cr3;


std::uint64_t operate_on_guest_physical_memory(const trap_frame_t* const trap_frame, const memory_operation_t operation)
{
    const cr3 guest_cr3 = arch::get_guest_cr3();
    const cr3 slat_cr3 = slat::hyperv_cr3();

    const std::uint64_t guest_buffer_virtual_address = trap_frame->r8;
    const std::uint64_t guest_physical_address = trap_frame->rdx;

    std::uint64_t size_left_to_copy = trap_frame->r9;

    std::uint64_t bytes_copied = 0;

    while (size_left_to_copy != 0)
    {
        std::uint64_t size_left_of_destination_slat_page = UINT64_MAX;
        std::uint64_t size_left_of_source_slat_page = UINT64_MAX;

        const std::uint64_t guest_buffer_physical_address = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = guest_buffer_virtual_address + bytes_copied });

        void* host_destination = memory_manager::map_guest_physical(slat_cr3, guest_buffer_physical_address, &size_left_of_destination_slat_page);
        void* host_source = memory_manager::map_guest_physical(slat_cr3, guest_physical_address + bytes_copied, &size_left_of_source_slat_page);

        if (size_left_of_destination_slat_page == UINT64_MAX || size_left_of_source_slat_page == UINT64_MAX)
        {
            break;
        }

        if (operation == memory_operation_t::write_operation)
        {
            crt::swap(host_source, host_destination);
        }

        const std::uint64_t size_left_of_slat_pages = crt::min(size_left_of_source_slat_page, size_left_of_destination_slat_page);

        const std::uint64_t copy_size = crt::min(size_left_to_copy, size_left_of_slat_pages);

        if (copy_size == 0)
        {
            break;
        }

        crt::copy_memory(host_destination, host_source, copy_size);

        size_left_to_copy -= copy_size;
        bytes_copied += copy_size;
    }

    return bytes_copied;
}

std::uint64_t virtual_memory_copy_raw(const cr3 source_cr3, const std::uint64_t source_va, const cr3 dest_cr3, const std::uint64_t dest_va, std::uint64_t size, const memory_operation_t operation)
{
    const cr3 slat_cr3 = slat::hyperv_cr3();
    std::uint64_t bytes_copied = 0;

    while (size != 0)
    {
        std::uint64_t size_left_of_dest_vpage = UINT64_MAX;
        std::uint64_t size_left_of_dest_slat_page = UINT64_MAX;
        std::uint64_t size_left_of_src_vpage = UINT64_MAX;
        std::uint64_t size_left_of_src_slat_page = UINT64_MAX;

        const std::uint64_t src_pa = memory_manager::translate_guest_virtual_address(source_cr3, slat_cr3, { .address = source_va + bytes_copied }, &size_left_of_src_vpage);
        const std::uint64_t dest_pa = memory_manager::translate_guest_virtual_address(dest_cr3, slat_cr3, { .address = dest_va + bytes_copied }, &size_left_of_dest_vpage);

        if (size_left_of_dest_vpage == UINT64_MAX || size_left_of_src_vpage == UINT64_MAX) break;

        void* host_dest = memory_manager::map_guest_physical(slat_cr3, dest_pa, &size_left_of_dest_slat_page);
        void* host_src = memory_manager::map_guest_physical(slat_cr3, src_pa, &size_left_of_src_slat_page);

        if (size_left_of_dest_slat_page == UINT64_MAX || size_left_of_src_slat_page == UINT64_MAX) break;

        if (operation == memory_operation_t::write_operation) crt::swap(host_src, host_dest);

        const std::uint64_t size_left_of_slat_pages = crt::min(size_left_of_src_slat_page, size_left_of_dest_slat_page);
        const std::uint64_t size_left_of_vpages = crt::min(size_left_of_src_vpage, size_left_of_dest_vpage);
        const std::uint64_t copy_size = crt::min(size, crt::min(size_left_of_slat_pages, size_left_of_vpages));

        if (copy_size == 0) break;
        crt::copy_memory(host_dest, host_src, copy_size);

        size -= copy_size;
        bytes_copied += copy_size;
    }
    return bytes_copied;
}

std::uint64_t operate_on_guest_virtual_memory(const trap_frame_t* const trap_frame, const memory_operation_t operation, const std::uint64_t address_of_page_directory)
{
    const cr3 guest_source_cr3 = { .address_of_page_directory = address_of_page_directory };
    const cr3 guest_destination_cr3 = arch::get_guest_cr3();

    return virtual_memory_copy_raw(guest_source_cr3, trap_frame->r8, guest_destination_cr3, trap_frame->rdx, trap_frame->r9, operation);
}

std::uint64_t operate_on_guest_virtual_memory_scatter(const trap_frame_t* const trap_frame, const std::uint64_t address_of_page_directory)
{
    const cr3 source_cr3 = { .address_of_page_directory = address_of_page_directory };
    const cr3 dest_cr3 = arch::get_guest_cr3();
    const cr3 slat_cr3 = slat::hyperv_cr3();

    const std::uint64_t descriptors_va = trap_frame->rdx;
    const std::uint64_t count = trap_frame->r8;

    if (count == 0 || descriptors_va == 0) return 0;

    // Map descriptors array from guest to host
    std::uint64_t size_left_of_slat_page = 0;
    const std::uint64_t descriptors_pa = memory_manager::translate_guest_virtual_address(dest_cr3, slat_cr3, { .address = descriptors_va });
    if (descriptors_pa == 0) return 0;

    const auto descriptors = static_cast<const scatter_read_entry_t*>(memory_manager::map_guest_physical(slat_cr3, descriptors_pa, &size_left_of_slat_page));
    if (descriptors == nullptr) return 0;

    // Safety check: ensure the array doesn't cross page boundary for now to keep it simple
    // A better implementation would iterate through multiple pages if needed
    const std::uint64_t total_descriptors_size = count * sizeof(scatter_read_entry_t);
    if (total_descriptors_size > size_left_of_slat_page) return 0;

    std::uint64_t successful_ops = 0;
    for (std::uint64_t i = 0; i < count; ++i)
    {
        if (virtual_memory_copy_raw(source_cr3, descriptors[i].guest_source_virtual_address,
                                   dest_cr3, descriptors[i].guest_destination_virtual_address,
                                   descriptors[i].size, memory_operation_t::read_operation) == descriptors[i].size)
        {
            successful_ops++;
        }
    }

    return successful_ops;
}

std::uint8_t copy_stack_data_from_log_exit(std::uint64_t* const stack_data, const std::uint64_t stack_data_count, const cr3 guest_cr3, const std::uint64_t rsp)
{
    if (rsp == 0)
    {
        return 0;
    }

    const cr3 slat_cr3 = slat::hyperv_cr3();

    std::uint64_t bytes_read = 0;
    std::uint64_t bytes_remaining = stack_data_count * sizeof(std::uint64_t);

    while (bytes_remaining != 0)
    {
        std::uint64_t virtual_size_left = 0;

        const std::uint64_t rsp_guest_physical_address = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = rsp + bytes_read }, &virtual_size_left);

        if (rsp_guest_physical_address == 0)
        {
            return 0;
        }

        std::uint64_t physical_size_left = 0;

        // rcx has just been pushed onto stack
        const auto rsp_mapped = static_cast<const std::uint64_t*>(memory_manager::map_guest_physical(slat_cr3, rsp_guest_physical_address, &physical_size_left));

        const std::uint64_t size_left_of_page = crt::min(physical_size_left, virtual_size_left);
        const std::uint64_t size_to_read = crt::min(bytes_remaining, size_left_of_page);

        if (size_to_read == 0)
        {
            return 0;
        }

        crt::copy_memory(reinterpret_cast<std::uint8_t*>(stack_data) + bytes_read, rsp_mapped, size_to_read);

        bytes_remaining -= size_to_read;
        bytes_read += size_to_read;
    }

    return 1;
}

void do_stack_data_copy(trap_frame_log_t& trap_frame, const cr3 guest_cr3)
{
    constexpr std::uint64_t stack_data_count = trap_frame_log_stack_data_count + 1;

    std::uint64_t stack_data[stack_data_count] = { };

    copy_stack_data_from_log_exit(&stack_data[0], stack_data_count, guest_cr3, trap_frame.rsp);

    crt::copy_memory(&trap_frame.stack_data, &stack_data[1], sizeof(trap_frame.stack_data));

    trap_frame.rcx = stack_data[0];
    trap_frame.rsp += 8; // get rid of the rcx value we push onto stack ourselves
}

void log_current_state(trap_frame_log_t trap_frame)
{
    cr3 guest_cr3 = arch::get_guest_cr3();

    do_stack_data_copy(trap_frame, guest_cr3);

    trap_frame.cr3 = guest_cr3.flags;
    trap_frame.rip = arch::get_guest_rip();

    logs::add_log(trap_frame);
}

std::uint64_t flush_logs(const trap_frame_t* const trap_frame)
{
    std::uint64_t stored_logs_count = logs::stored_log_index;

    const cr3 guest_cr3 = arch::get_guest_cr3();
    const cr3 slat_cr3 = slat::hyperv_cr3();

    const std::uint64_t guest_virtual_address = trap_frame->rdx;
    const std::uint16_t count = static_cast<std::uint16_t>(trap_frame->r8);

    if (logs::flush(slat_cr3, guest_virtual_address, guest_cr3, count) == 0)
    {
        return -1;
    }

    return stored_logs_count;
}

std::uint64_t process_signal_page_hypercall(const hypercall_info_t hypercall_info, const trap_frame_t* const trap_frame)
{
    const auto operation = static_cast<signal_hypercall_op_t>(hypercall_info.call_reserved_data);

    switch (operation)
    {
    case signal_hypercall_op_t::register_page:
    {
        const std::uint64_t signal_page_va = trap_frame->rdx;
        if (signal_page_va == 0)
        {
            return 0;
        }

        const cr3 guest_cr3 = { .flags = authorized_caller_cr3 };
        const cr3 slat_cr3 = slat::hyperv_cr3();
        const std::uint64_t signal_page_pa = memory_manager::translate_guest_virtual_address(
            guest_cr3, slat_cr3, { .address = signal_page_va });

        if (signal_page_pa == 0)
        {
            return 0;
        }

        return slat::signal::register_page({ .address = signal_page_pa });
    }
    case signal_hypercall_op_t::query_page:
    {
        const std::uint64_t signal_id = trap_frame->rdx;
        const std::uint64_t state_va = trap_frame->r8;

        if (state_va == 0)
        {
            return 0;
        }

        slat::signal::page_state_t state = { };
        if (slat::signal::query_page(signal_id, &state) == 0)
        {
            return 0;
        }

        signal_page_state_t guest_state = { };
        guest_state.id = state.id;
        guest_state.guest_physical_address = state.guest_physical_address;
        guest_state.sequence = state.sequence;
        guest_state.trigger_count = state.trigger_count;
        guest_state.last_guest_rip = state.last_guest_rip;
        guest_state.last_access_flags = state.last_access_flags;
        guest_state.last_tsc = state.last_tsc;

        const cr3 guest_cr3 = { .flags = authorized_caller_cr3 };
        const std::uint64_t bytes_written = memory_manager::operate_on_guest_virtual_memory(
            slat::hyperv_cr3(), &guest_state, state_va, guest_cr3, sizeof(guest_state), memory_operation_t::write_operation);

        return bytes_written == sizeof(guest_state);
    }
    case signal_hypercall_op_t::unregister_page:
        return slat::signal::unregister_page(trap_frame->rdx);
    default:
        return 0;
    }
}

void hypercall::process(const hypercall_info_t hypercall_info, trap_frame_t* const trap_frame)
{
    switch (hypercall_info.call_type)
    {
    case hypercall_type_t::_hc_0x100:
    {
        // Version Ping: Trả về 0x2323 (v2.3) để xác nhận bản sửa lỗi System CR3 Sniffer
        trap_frame->rax = 0x2323;
        break;
    }
    case hypercall_type_t::_hc_0x110:
    {
        const auto memory_operation = static_cast<memory_operation_t>(hypercall_info.call_reserved_data);

        trap_frame->rax = operate_on_guest_physical_memory(trap_frame, memory_operation);

        break;
    }
    case hypercall_type_t::_hc_0x120:
    {
        const virt_memory_op_hypercall_info_t virt_memory_op_info = { .value = hypercall_info.value };

        const memory_operation_t memory_operation = virt_memory_op_info.memory_operation;
        const std::uint64_t address_of_page_directory = virt_memory_op_info.address_of_page_directory;

        trap_frame->rax = operate_on_guest_virtual_memory(trap_frame, memory_operation, address_of_page_directory);

        break;
    }
    case hypercall_type_t::_hc_0x121:
    {
        const virt_memory_op_hypercall_info_t virt_memory_op_info = { .value = hypercall_info.value };
        const std::uint64_t address_of_page_directory = virt_memory_op_info.address_of_page_directory;

        trap_frame->rax = operate_on_guest_virtual_memory_scatter(trap_frame, address_of_page_directory);

        break;
    }
    case hypercall_type_t::_hc_0x130:
    {
        const virtual_address_t guest_virtual_address = { .address = trap_frame->rdx };

        const cr3 target_guest_cr3 = { .flags = trap_frame->r8 };
        const cr3 slat_cr3 = slat::hyperv_cr3();

        trap_frame->rax = memory_manager::translate_guest_virtual_address(target_guest_cr3, slat_cr3, guest_virtual_address);

        break;
    }
    case hypercall_type_t::_hc_0x140:
    {
        const cr3 guest_cr3 = arch::get_guest_cr3();

        trap_frame->rax = guest_cr3.flags;

        break;
    }
    case hypercall_type_t::_hc_0x150:
    {
        const virtual_address_t target_guest_physical_address = { .address = trap_frame->rdx };
        const virtual_address_t shadow_page_guest_physical_address = { .address = trap_frame->r8 };

        trap_frame->rax = slat::hook::add(target_guest_physical_address, shadow_page_guest_physical_address);

        break;
    }
    case hypercall_type_t::_hc_0x160:
    {
        const virtual_address_t target_guest_physical_address = { .address = trap_frame->rdx };

        trap_frame->rax = slat::hook::remove(target_guest_physical_address);

        break;
    }
    case hypercall_type_t::_hc_0x170:
    {
        const virtual_address_t target_guest_physical_address = { .address = trap_frame->rdx };

        trap_frame->rax = slat::hide_physical_page_from_guest(target_guest_physical_address);

        break;
    }
    case hypercall_type_t::_hc_0x180:
    {
        trap_frame_log_t trap_frame_log;

        crt::copy_memory(&trap_frame_log, trap_frame, sizeof(trap_frame_t));

        log_current_state(trap_frame_log);

        break;
    }
    case hypercall_type_t::_hc_0x190:
    {
        trap_frame->rax = flush_logs(trap_frame);

        break;
    }
    case hypercall_type_t::_hc_0x200:
    {
        trap_frame->rax = heap_manager::get_free_page_count();

        break;
    }
    case hypercall_type_t::_hc_0x240:
    {
        trap_frame->rax = process_signal_page_hypercall(hypercall_info, trap_frame);

        break;
    }
    case hypercall_type_t::_hc_0x230:
    {
        trap_frame->rax = 0;
        break;
    }
    case hypercall_type_t::_hc_0x220:
    {
        // RDX = dx, R8 = dy, R9 = buttons (optional)
        const std::int32_t dx = static_cast<std::int32_t>(static_cast<std::int16_t>(trap_frame->rdx));
        const std::int32_t dy = static_cast<std::int32_t>(static_cast<std::int16_t>(trap_frame->r8));
        const std::uint8_t buttons = static_cast<std::uint8_t>(trap_frame->r9);

        trap_frame->rax = input::backend_selector::InjectMouse(dx, dy, buttons) ? 1 : 0;
        break;
    }
    case hypercall_type_t::_hc_0x250:
    {
        // Toggle Process Protection (PPL Bypass)
        // RDX = EPROCESS Address (GVA), R8 = 0 (Off) or 1 (On)
        const std::uint64_t eprocess_va = trap_frame->rdx;
        const bool enable = (trap_frame->r8 != 0);

        if (eprocess_va == 0) { trap_frame->rax = 0; break; }

        const cr3 guest_cr3 = { .flags = authorized_caller_cr3 };
        const cr3 slat_cr3 = slat::hyperv_cr3();

        // Offsets for Protection (Win 10/11 common)
        // 0x87A for Win 11, 0x6AA for Win 10. We will flip both to be safe.
        std::uint64_t offsets[] = { 0x6AA, 0x87A, 0x58A }; // Broad coverage

        for (auto offset : offsets) {
            std::uint64_t addr = eprocess_va + offset;
            std::uint64_t pa = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = addr });
            if (pa != 0) {
                std::uint8_t* mapped = static_cast<std::uint8_t*>(memory_manager::map_host_physical(pa));
                if (mapped) {
                    // Protection: bit 0:3 = Type, bit 4:7 = Audit
                    // We zero it to disable PPL (0 = None)
                    // We restore it to 0x31 (PPL-WinSystem) if enabling (common for LSASS)
                    *mapped = enable ? 0x31 : 0x00;
                }
            }
        }
        
        trap_frame->rax = 1; 
        break;
    }
    case hypercall_type_t::_hc_0x260:
    {
        // DKOM: Unlink process from ActiveProcessLinks
        // RDX = EPROCESS Address (GVA)
        // ActiveProcessLinks offset = 0x448 (verified in usermode code)
        const std::uint64_t eprocess_va = trap_frame->rdx;
        if (eprocess_va == 0) { trap_frame->rax = 0; break; }

        const cr3 guest_cr3 = { .flags = authorized_caller_cr3 };
        const cr3 slat_cr3 = slat::hyperv_cr3();

        constexpr std::uint64_t apl_offset = 0x448; // ActiveProcessLinks

        // Read our Flink and Blink
        std::uint64_t our_flink = 0, our_blink = 0;
        std::uint64_t flink_pa = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = eprocess_va + apl_offset });
        std::uint64_t blink_pa = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = eprocess_va + apl_offset + 8 });

        if (flink_pa == 0 || blink_pa == 0) { trap_frame->rax = 0; break; }

        auto* flink_mapped = static_cast<std::uint64_t*>(memory_manager::map_host_physical(flink_pa));
        auto* blink_mapped = static_cast<std::uint64_t*>(memory_manager::map_host_physical(blink_pa));

        if (!flink_mapped || !blink_mapped) { trap_frame->rax = 0; break; }

        our_flink = *flink_mapped;
        our_blink = *blink_mapped;

        if (our_flink == 0 || our_blink == 0) { trap_frame->rax = 0; break; }

        // Patch previous->Flink = our_flink
        // previous is at (our_blink - 0) since Flink is first member of LIST_ENTRY
        std::uint64_t prev_flink_pa = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = our_blink });
        if (prev_flink_pa != 0) {
            auto* prev_flink = static_cast<std::uint64_t*>(memory_manager::map_host_physical(prev_flink_pa));
            if (prev_flink) *prev_flink = our_flink;
        }

        // Patch next->Blink = our_blink
        // next is at (our_flink + 8) since Blink is second member of LIST_ENTRY
        std::uint64_t next_blink_pa = memory_manager::translate_guest_virtual_address(guest_cr3, slat_cr3, { .address = our_flink + 8 });
        if (next_blink_pa != 0) {
            auto* next_blink = static_cast<std::uint64_t*>(memory_manager::map_host_physical(next_blink_pa));
            if (next_blink) *next_blink = our_blink;
        }

        // Point ourselves to ourselves (safe self-loop)
        *flink_mapped = eprocess_va + apl_offset;
        *blink_mapped = eprocess_va + apl_offset;

        trap_frame->rax = 1;
        break;
    }
    case hypercall_type_t::_hc_0x270:
    {
        // [STEALTH V5] True Immutable Machine ID (SMBIOS UUID Scanner)
        // We scan the physical F-segment (0xF0000-0xFFFFF) for the SMBIOS entry point.
        // This retrieves the permanent hardware UUID assigned by the manufacturer.
        std::uint64_t final_hwid = 0;
        bool found = false;

        // 1. Search for SMBIOS Anchor "_SM_" (32-bit entry point) or "_SM3_" (64-bit)
        for (std::uint64_t addr = 0xF0000; addr < 0xFFFFF; addr += 16) {
            const auto* header = static_cast<const std::uint8_t*>(memory_manager::map_host_physical(addr));
            if (header[0] == '_' && header[1] == 'S' && header[2] == 'M' && header[3] == '_') {
                // Found SMBIOS Table Entry Point
                // Header + 0x18 is the table address (32-bit)
                std::uint32_t table_addr = *reinterpret_cast<const std::uint32_t*>(header + 0x18);
                std::uint16_t table_len = *reinterpret_cast<const std::uint16_t*>(header + 0x16);
                
                const std::uint8_t* table = static_cast<const std::uint8_t*>(memory_manager::map_host_physical(table_addr));
                if (table) {
                    // Iterate tables to find Type 1 (System Information)
                    const std::uint8_t* ptr = table;
                    while (ptr < table + table_len) {
                        std::uint8_t type = ptr[0];
                        std::uint8_t length = ptr[1];
                        if (type == 1 && length >= 0x19) {
                            // Type 1 Found - UUID starts at offset 0x08 (16 bytes)
                            const std::uint64_t* uuid_parts = reinterpret_cast<const std::uint64_t*>(ptr + 8);
                            final_hwid = uuid_parts[0] ^ uuid_parts[1]; // Combine 128-bit UUID into 64-bit
                            found = true;
                            break;
                        }
                        // Skip to next table (tables end with double null)
                        ptr += length;
                        while(ptr[0] != 0 || ptr[1] != 0) ptr++;
                        ptr += 2;
                    }
                }
                if (found) break;
            }
        }

        // Fallback to CPU traits if SMBIOS UUID is not accessible
        if (!found) {
            int brand[4];
            for (std::uint32_t i = 0; i < 3; ++i) {
                __cpuid(brand, 0x80000002 + i);
                for (int j = 0; j < 4; ++j) final_hwid ^= (static_cast<std::uint64_t>(brand[j]) << (j % 8));
            }
            final_hwid ^= __readmsr(0x17); // Platform ID
        }

        trap_frame->rax = final_hwid;
        break;
    }
    case hypercall_type_t::_hc_0x280:
    {
        const std::uint64_t snapshot_va = trap_frame->rdx;
        if (snapshot_va == 0) {
            trap_frame->rax = 0;
            break;
        }

        input_diagnostics_snapshot_t snapshot = input::backend_selector::Diagnostics();
        const cr3 guest_cr3 = { .flags = authorized_caller_cr3 };
        const std::uint64_t bytes_written = memory_manager::operate_on_guest_virtual_memory(
            slat::hyperv_cr3(), &snapshot, snapshot_va, guest_cr3, sizeof(snapshot), memory_operation_t::write_operation);

        trap_frame->rax = bytes_written == sizeof(snapshot);
        break;
    }
    default:
        break;
    }
}
