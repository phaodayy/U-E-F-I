#pragma once
#include <cstdint>

namespace host_memory
{
    constexpr std::uint64_t page_size = 0x1000;
    constexpr std::uint64_t max_registered_ranges = 256;
    constexpr std::uint64_t audit_ring_capacity = 1024;

    enum class memory_owner_t : std::uint8_t
    {
        unknown,
        host_code,
        host_data,
        host_stack,
        guest_ram,
        shared_bounce,
        mmio,
        reserved,
        guard
    };

    enum class memory_access_t : std::uint8_t
    {
        unknown,
        read,
        write,
        execute,
        dma_read,
        dma_write,
        hypercall_copy_in,
        hypercall_copy_out,
        diagnostic_read
    };

    enum class audit_context_t : std::uint8_t
    {
        unknown,
        registry_update,
        registry_reject,
        map_guest_physical,
        translate_guest_physical,
        translate_guest_virtual,
        hypercall_memory,
        slat_hook_shadow,
        slat_hook_target,
        slat_audit,
        slat_merge,
    };

    enum class audit_action_t : std::uint8_t
    {
        observed,
        allowed,
        denied_would_block,
        registry_update,
        registry_reject,
        dropped
    };

    struct pfn_range_t
    {
        std::uint64_t base_pfn = 0;
        std::uint64_t page_count = 0;
        std::uint64_t allowed_access_mask = 0;
        std::uint64_t generation = 0;
        memory_owner_t owner = memory_owner_t::unknown;
        std::uint8_t immutable = 0;
        std::uint32_t tag = 0;
    };

    struct audit_record_t
    {
        std::uint64_t sequence = 0;
        std::uint64_t timestamp = 0;
        audit_context_t context = audit_context_t::unknown;
        memory_access_t access = memory_access_t::unknown;
        audit_action_t action = audit_action_t::observed;
        memory_owner_t owner = memory_owner_t::unknown;
        std::uint64_t guest_physical_address = 0;
        std::uint64_t host_physical_address = 0;
        std::uint64_t source_cr3 = 0;
        std::uint64_t guest_rip = 0;
        std::uint64_t size = 0;
        std::uint64_t policy_generation = 0;
    };

    // Core Registry Functions
    void initialize_registry(std::uint64_t total_system_ram_size);
    
    std::uint8_t register_host_owned_range(std::uint64_t physical_address, std::uint64_t size, memory_owner_t owner);
    std::uint8_t mark_guest_visible_range(std::uint64_t physical_address, std::uint64_t size);
    std::uint8_t register_mmio_exclusion_range(std::uint64_t physical_address, std::uint64_t size);

    // Policy & Audit
    memory_owner_t owner_for_physical_address(std::uint64_t physical_address);
    std::uint64_t registered_range_count();
    pfn_range_t registered_range_at(std::uint64_t index);

    std::uint8_t is_host_owned_physical_address(std::uint64_t physical_address);
    std::uint8_t is_host_owned_physical_range(std::uint64_t physical_address, std::uint64_t size);
    audit_action_t evaluate_physical_access(std::uint64_t host_physical_address, std::uint64_t size, memory_access_t access);
    
    void set_fail_closed_enabled(std::uint8_t enabled);
    std::uint8_t is_fail_closed_enabled();
    
    void record_audit_event(audit_context_t context, std::uint64_t guest_physical_address, std::uint64_t host_physical_address, std::uint64_t size);
    void record_audit_event_ex(audit_context_t context, memory_access_t access, audit_action_t action,
        std::uint64_t guest_physical_address, std::uint64_t host_physical_address, std::uint64_t size,
        std::uint64_t source_cr3 = 0, std::uint64_t guest_rip = 0);
    audit_action_t audit_physical_access(audit_context_t context, memory_access_t access,
        std::uint64_t guest_physical_address, std::uint64_t host_physical_address, std::uint64_t size,
        std::uint64_t source_cr3 = 0, std::uint64_t guest_rip = 0);
    std::uint64_t audit_record_count();
    std::uint64_t audit_record_total_count();
    std::uint64_t audit_dropped_count();
    audit_record_t audit_record_at(std::uint64_t index);
}
