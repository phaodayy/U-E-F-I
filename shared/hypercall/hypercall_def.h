#pragma once
#include <cstdint>
#include <structures/memory_operation.h>

enum class hypercall_type_t : std::uint64_t
{
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
    _hc_0x200  // get_heap_free_page_count
};

struct scatter_read_entry_t
{
    std::uint64_t guest_source_virtual_address;
    std::uint64_t guest_destination_virtual_address;
    std::uint64_t size;
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
        hypercall_type_t call_type : 4;
        std::uint64_t secondary_key : 7;
        std::uint64_t call_reserved_data : 37;
    };
};

union virt_memory_op_hypercall_info_t
{
    std::uint64_t value;

    struct
    {
        std::uint64_t primary_key : 16;
        hypercall_type_t call_type : 4;
        std::uint64_t secondary_key : 7;
        memory_operation_t memory_operation : 1;
        std::uint64_t address_of_page_directory : 36; // we will construct the other cr3 (aside from the caller process) involved in the operation from this
    };
};

#pragma warning(pop)
