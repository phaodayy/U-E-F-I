#pragma once
#include <cstdint>
#include <structures/memory_operation.h>

enum class hypercall_type_t : std::uint64_t
{
    _hc_0x100, // version_ping
    _hc_0x140, // read_guest_cr3
    _hc_0x110, // guest_physical_memory_operation
    _hc_0x210, // init_hypercall_context
    _hc_0x130, // translate_guest_virtual_address
    _hc_0x230, // set_mouse_hook_address
    _hc_0x120, // guest_virtual_memory_operation
    _hc_0x150, // add_slat_code_hook
    _hc_0x170, // hide_guest_physical_page
    _hc_0x220, // inject_mouse_movement
    _hc_0x121, // scatter_read_virtual_memory
    _hc_0x160, // remove_slat_code_hook
    _hc_0x180, // log_current_state
    _hc_0x190, // flush_logs
    _hc_0x200, // get_heap_free_page_count
    _hc_0x240, // slat_signal_page_operation
    _hc_0x250, // toggle_process_protection
    _hc_0x260, // unlink_process_from_list (DKOM)
    _hc_0x270, // get_hardware_fingerprint (Ring -1 stable ID)
    _hc_0x280  // get_input_diagnostics
};

struct input_diagnostics_snapshot_t
{
    std::uint64_t version;
    std::uint64_t selected_backend;
    std::uint64_t available_backend_mask;
    std::uint64_t activated_backend_mask;
    std::uint64_t selector_init_count;
    std::uint64_t selector_activate_count;
    std::uint64_t backend_switch_count;
    std::uint64_t hypercall_inject_count;
    std::uint64_t hypercall_inject_success_count;
    std::uint64_t hypercall_inject_fail_count;
    std::uint64_t io_exit_count;
    std::uint64_t io_exit_handled_count;
    std::uint64_t io_exit_forwarded_count;
    std::uint64_t ps2_status_read_count;
    std::uint64_t ps2_data_read_count;
    std::uint64_t ps2_status_write_count;
    std::uint64_t ps2_data_write_count;
    std::uint64_t ps2_ack_queued_count;
    std::uint64_t ps2_movement_packet_count;
    std::uint64_t ps2_output_queued_count;
    std::uint64_t ps2_output_popped_count;
    std::uint64_t ps2_output_expired_count;
    std::uint64_t virtual_hid_inject_attempt_count;
    std::uint64_t last_error_code;
};

struct scatter_read_entry_t
{
    std::uint64_t guest_source_virtual_address;
    std::uint64_t guest_destination_virtual_address;
    std::uint64_t size;
};

enum class signal_hypercall_op_t : std::uint64_t
{
    register_page,
    query_page,
    unregister_page
};

struct signal_page_state_t
{
    std::uint64_t id;
    std::uint64_t guest_physical_address;
    std::uint64_t sequence;
    std::uint64_t trigger_count;
    std::uint64_t last_guest_rip;
    std::uint64_t last_access_flags;
    std::uint64_t last_tsc;
};

#pragma warning(push)
#pragma warning(disable: 4201)

constexpr std::uint64_t hypercall_default_primary_key = 0xD3AD;
constexpr std::uint64_t hypercall_default_secondary_key = 0x3F;

// Obfuscated seeds: calculated at compile-time to avoid static 8-byte signatures
#define SEED_XOR_KEY 0x59A2B3C4D5E6F701ULL
constexpr std::uint64_t hypercall_magic_seed_a = 0xA7C9F2B134D5E6F8ULL ^ SEED_XOR_KEY;
constexpr std::uint64_t hypercall_magic_seed_b = 0xBC8132A5F7E91D4CULL ^ SEED_XOR_KEY;

union hypercall_info_t
{
    std::uint64_t value;

    struct
    {
        std::uint64_t primary_key : 16;
        hypercall_type_t call_type : 6; // Expanded to 6 bits for > 16 types
        std::uint64_t secondary_key : 7;
        std::uint64_t call_reserved_data : 35; // Adjusted to match 64 bits
    };
};

union virt_memory_op_hypercall_info_t
{
    std::uint64_t value;

    struct
    {
        std::uint64_t primary_key : 16;
        hypercall_type_t call_type : 6; // Expanded to 6 bits
        std::uint64_t secondary_key : 7;
        memory_operation_t memory_operation : 1;
        std::uint64_t address_of_page_directory : 34; // Adjusted to match 64 bits
    };
};

#pragma warning(pop)
