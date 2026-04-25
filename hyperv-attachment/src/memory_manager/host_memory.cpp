#include "host_memory.h"

#include "../crt/crt.h"

#include <intrin.h>

namespace
{
    using namespace host_memory;

    pfn_range_t registered_ranges[max_registered_ranges] = { };
    std::uint64_t registered_ranges_count = 0;
    std::uint64_t registry_generation = 0;
    std::uint64_t system_page_count = 0;
    std::uint8_t registry_initialized = 0;
    std::uint8_t fail_closed_enabled = 0;

    audit_record_t audit_ring[audit_ring_capacity] = { };
    volatile long long audit_write_sequence = 0;
    volatile long long audit_dropped_records = 0;

    constexpr std::uint64_t access_read = 1ull << 0;
    constexpr std::uint64_t access_write = 1ull << 1;
    constexpr std::uint64_t access_execute = 1ull << 2;
    constexpr std::uint64_t access_dma_read = 1ull << 3;
    constexpr std::uint64_t access_dma_write = 1ull << 4;
    constexpr std::uint64_t access_hypercall_copy_in = 1ull << 5;
    constexpr std::uint64_t access_hypercall_copy_out = 1ull << 6;
    constexpr std::uint64_t access_diagnostic_read = 1ull << 7;

    constexpr std::uint64_t guest_ram_default_access =
        access_read | access_write | access_execute |
        access_hypercall_copy_in | access_hypercall_copy_out |
        access_diagnostic_read;

    std::uint64_t access_to_mask(const memory_access_t access)
    {
        switch (access)
        {
        case memory_access_t::read:
            return access_read;
        case memory_access_t::write:
            return access_write;
        case memory_access_t::execute:
            return access_execute;
        case memory_access_t::dma_read:
            return access_dma_read;
        case memory_access_t::dma_write:
            return access_dma_write;
        case memory_access_t::hypercall_copy_in:
            return access_hypercall_copy_in;
        case memory_access_t::hypercall_copy_out:
            return access_hypercall_copy_out;
        case memory_access_t::diagnostic_read:
            return access_diagnostic_read;
        default:
            return 0;
        }
    }

    std::uint8_t add_overflows(const std::uint64_t base, const std::uint64_t size)
    {
        return size != 0 && base + size - 1 < base;
    }

    std::uint8_t physical_range_to_pfn_range(const std::uint64_t physical_address, const std::uint64_t size,
        std::uint64_t* const base_pfn, std::uint64_t* const page_count)
    {
        if (base_pfn == nullptr || page_count == nullptr || size == 0 || add_overflows(physical_address, size) == 1)
        {
            return 0;
        }

        const std::uint64_t end_physical_address = physical_address + size - 1;
        const std::uint64_t first_pfn = physical_address / page_size;
        const std::uint64_t last_pfn = end_physical_address / page_size;

        *base_pfn = first_pfn;
        *page_count = last_pfn - first_pfn + 1;

        return 1;
    }

    std::uint8_t pfn_ranges_overlap(const std::uint64_t first_base_pfn, const std::uint64_t first_page_count,
        const std::uint64_t second_base_pfn, const std::uint64_t second_page_count)
    {
        if (first_page_count == 0 || second_page_count == 0)
        {
            return 0;
        }

        const std::uint64_t first_end_pfn = first_base_pfn + first_page_count - 1;
        const std::uint64_t second_end_pfn = second_base_pfn + second_page_count - 1;

        return first_base_pfn <= second_end_pfn && second_base_pfn <= first_end_pfn;
    }

    std::uint8_t is_host_owned_owner(const memory_owner_t owner)
    {
        return owner == memory_owner_t::host_code ||
            owner == memory_owner_t::host_data ||
            owner == memory_owner_t::host_stack ||
            owner == memory_owner_t::guard;
    }

    const pfn_range_t* find_range_for_pfn(const std::uint64_t pfn)
    {
        for (std::uint64_t i = 0; i < registered_ranges_count; i++)
        {
            const pfn_range_t* const range = &registered_ranges[i];

            if (range->page_count == 0)
            {
                continue;
            }

            if (range->base_pfn <= pfn && pfn < range->base_pfn + range->page_count)
            {
                return range;
            }
        }

        return nullptr;
    }

    std::uint8_t is_access_allowed_for_range(const pfn_range_t* const range, const memory_access_t access)
    {
        if (range == nullptr)
        {
            return 0;
        }

        if (is_host_owned_owner(range->owner) == 1 ||
            range->owner == memory_owner_t::unknown ||
            range->owner == memory_owner_t::reserved ||
            range->owner == memory_owner_t::guard)
        {
            return 0;
        }

        const std::uint64_t access_mask = access_to_mask(access);

        if (access_mask == 0)
        {
            return 0;
        }

        return (range->allowed_access_mask & access_mask) == access_mask;
    }

    void record_registry_audit(const audit_context_t context, const audit_action_t action,
        const std::uint64_t physical_address, const std::uint64_t size, const memory_owner_t owner,
        const std::uint64_t generation)
    {
        const std::uint64_t sequence = static_cast<std::uint64_t>(_InterlockedIncrement64(&audit_write_sequence));
        const std::uint64_t slot = (sequence - 1) % audit_ring_capacity;

        audit_record_t record = { };
        record.sequence = sequence;
        record.timestamp = __rdtsc();
        record.context = context;
        record.action = action;
        record.owner = owner;
        record.host_physical_address = physical_address;
        record.size = size;
        record.policy_generation = generation;

        audit_ring[slot] = record;
    }

    std::uint8_t register_range(const std::uint64_t physical_address, const std::uint64_t size,
        const memory_owner_t owner, const std::uint64_t allowed_access_mask, const std::uint8_t immutable,
        const std::uint32_t tag)
    {
        if (registry_initialized == 0)
        {
            initialize_registry(0);
        }

        std::uint64_t base_pfn = 0;
        std::uint64_t page_count = 0;

        if (physical_range_to_pfn_range(physical_address, size, &base_pfn, &page_count) == 0)
        {
            record_registry_audit(audit_context_t::registry_reject, audit_action_t::registry_reject,
                physical_address, size, owner, registry_generation);
            return 0;
        }

        if (system_page_count != 0 && system_page_count < base_pfn + page_count)
        {
            record_registry_audit(audit_context_t::registry_reject, audit_action_t::registry_reject,
                physical_address, size, owner, registry_generation);
            return 0;
        }

        if (registered_ranges_count >= max_registered_ranges)
        {
            _InterlockedIncrement64(&audit_dropped_records);
            record_registry_audit(audit_context_t::registry_reject, audit_action_t::dropped,
                physical_address, size, owner, registry_generation);
            return 0;
        }

        for (std::uint64_t i = 0; i < registered_ranges_count; i++)
        {
            const pfn_range_t* const current = &registered_ranges[i];

            if (pfn_ranges_overlap(base_pfn, page_count, current->base_pfn, current->page_count) == 1)
            {
                record_registry_audit(audit_context_t::registry_reject, audit_action_t::registry_reject,
                    physical_address, size, owner, current->generation);
                return 0;
            }
        }

        pfn_range_t* const range = &registered_ranges[registered_ranges_count++];

        range->base_pfn = base_pfn;
        range->page_count = page_count;
        range->allowed_access_mask = allowed_access_mask;
        range->generation = ++registry_generation;
        range->owner = owner;
        range->immutable = immutable;
        range->tag = tag;

        record_registry_audit(audit_context_t::registry_update, audit_action_t::registry_update,
            physical_address, size, owner, range->generation);

        return 1;
    }
}

void host_memory::initialize_registry(const std::uint64_t total_system_ram_size)
{
    crt::set_memory(&registered_ranges[0], 0, sizeof(registered_ranges));
    crt::set_memory(&audit_ring[0], 0, sizeof(audit_ring));

    registered_ranges_count = 0;
    registry_generation = 0;
    system_page_count = (total_system_ram_size + page_size - 1) / page_size;
    registry_initialized = 1;
    fail_closed_enabled = 0;

    _InterlockedExchange64(&audit_write_sequence, 0);
    _InterlockedExchange64(&audit_dropped_records, 0);
}

std::uint8_t host_memory::register_host_owned_range(const std::uint64_t physical_address, const std::uint64_t size,
    const memory_owner_t owner)
{
    if (is_host_owned_owner(owner) == 0 && owner != memory_owner_t::reserved)
    {
        return 0;
    }

    return register_range(physical_address, size, owner, 0, 1, 0);
}

std::uint8_t host_memory::mark_guest_visible_range(const std::uint64_t physical_address, const std::uint64_t size)
{
    return register_range(physical_address, size, memory_owner_t::guest_ram, guest_ram_default_access, 0, 0);
}

std::uint8_t host_memory::register_mmio_exclusion_range(const std::uint64_t physical_address, const std::uint64_t size)
{
    return register_range(physical_address, size, memory_owner_t::mmio, 0, 1, 0);
}

host_memory::memory_owner_t host_memory::owner_for_physical_address(const std::uint64_t physical_address)
{
    const pfn_range_t* const range = find_range_for_pfn(physical_address / page_size);

    if (range == nullptr)
    {
        return memory_owner_t::unknown;
    }

    return range->owner;
}

std::uint64_t host_memory::registered_range_count()
{
    return registered_ranges_count;
}

host_memory::pfn_range_t host_memory::registered_range_at(const std::uint64_t index)
{
    if (index >= registered_ranges_count)
    {
        return { };
    }

    return registered_ranges[index];
}

std::uint8_t host_memory::is_host_owned_physical_address(const std::uint64_t physical_address)
{
    return is_host_owned_owner(owner_for_physical_address(physical_address));
}

std::uint8_t host_memory::is_host_owned_physical_range(const std::uint64_t physical_address, const std::uint64_t size)
{
    std::uint64_t base_pfn = 0;
    std::uint64_t page_count = 0;

    if (physical_range_to_pfn_range(physical_address, size, &base_pfn, &page_count) == 0)
    {
        return 0;
    }

    for (std::uint64_t i = 0; i < registered_ranges_count; i++)
    {
        const pfn_range_t* const range = &registered_ranges[i];

        if (is_host_owned_owner(range->owner) == 1 &&
            pfn_ranges_overlap(base_pfn, page_count, range->base_pfn, range->page_count) == 1)
        {
            return 1;
        }
    }

    return 0;
}

host_memory::audit_action_t host_memory::evaluate_physical_access(const std::uint64_t host_physical_address,
    const std::uint64_t size, const memory_access_t access)
{
    std::uint64_t base_pfn = 0;
    std::uint64_t page_count = 0;

    if (physical_range_to_pfn_range(host_physical_address, size, &base_pfn, &page_count) == 0)
    {
        return audit_action_t::denied_would_block;
    }

    std::uint64_t current_pfn = base_pfn;
    const std::uint64_t end_pfn = base_pfn + page_count;

    while (current_pfn < end_pfn)
    {
        const pfn_range_t* const range = find_range_for_pfn(current_pfn);

        if (is_access_allowed_for_range(range, access) == 0)
        {
            return audit_action_t::denied_would_block;
        }

        std::uint64_t next_pfn = range->base_pfn + range->page_count;

        if (next_pfn <= current_pfn || next_pfn > end_pfn)
        {
            next_pfn = end_pfn;
        }

        current_pfn = next_pfn;
    }

    return audit_action_t::allowed;
}

void host_memory::set_fail_closed_enabled(const std::uint8_t enabled)
{
    fail_closed_enabled = enabled != 0;
}

std::uint8_t host_memory::is_fail_closed_enabled()
{
    return fail_closed_enabled;
}

void host_memory::record_audit_event(const audit_context_t context, const std::uint64_t guest_physical_address,
    const std::uint64_t host_physical_address, const std::uint64_t size)
{
    record_audit_event_ex(context, memory_access_t::unknown, audit_action_t::observed,
        guest_physical_address, host_physical_address, size);
}

void host_memory::record_audit_event_ex(const audit_context_t context, const memory_access_t access,
    const audit_action_t action, const std::uint64_t guest_physical_address,
    const std::uint64_t host_physical_address, const std::uint64_t size, const std::uint64_t source_cr3,
    const std::uint64_t guest_rip)
{
    const std::uint64_t sequence = static_cast<std::uint64_t>(_InterlockedIncrement64(&audit_write_sequence));
    const std::uint64_t slot = (sequence - 1) % audit_ring_capacity;
    const std::uint64_t owner_lookup_address = host_physical_address != 0 ? host_physical_address : guest_physical_address;
    const pfn_range_t* const range = find_range_for_pfn(owner_lookup_address / page_size);

    audit_record_t record = { };
    record.sequence = sequence;
    record.timestamp = __rdtsc();
    record.context = context;
    record.access = access;
    record.action = action;
    record.owner = range != nullptr ? range->owner : memory_owner_t::unknown;
    record.guest_physical_address = guest_physical_address;
    record.host_physical_address = host_physical_address;
    record.source_cr3 = source_cr3;
    record.guest_rip = guest_rip;
    record.size = size;
    record.policy_generation = range != nullptr ? range->generation : registry_generation;

    audit_ring[slot] = record;
}

host_memory::audit_action_t host_memory::audit_physical_access(const audit_context_t context, const memory_access_t access,
    const std::uint64_t guest_physical_address, const std::uint64_t host_physical_address, const std::uint64_t size,
    const std::uint64_t source_cr3, const std::uint64_t guest_rip)
{
    const audit_action_t action = evaluate_physical_access(host_physical_address, size, access);

    record_audit_event_ex(context, access, action, guest_physical_address, host_physical_address, size,
        source_cr3, guest_rip);

    return action;
}

std::uint64_t host_memory::audit_record_count()
{
    const std::uint64_t total_count = audit_record_total_count();

    return total_count < audit_ring_capacity ? total_count : audit_ring_capacity;
}

std::uint64_t host_memory::audit_record_total_count()
{
    return static_cast<std::uint64_t>(audit_write_sequence);
}

std::uint64_t host_memory::audit_dropped_count()
{
    return static_cast<std::uint64_t>(audit_dropped_records);
}

host_memory::audit_record_t host_memory::audit_record_at(const std::uint64_t index)
{
    const std::uint64_t count = audit_record_count();

    if (index >= count)
    {
        return { };
    }

    const std::uint64_t total_count = audit_record_total_count();
    const std::uint64_t first_sequence = total_count < audit_ring_capacity ? 1 : total_count - audit_ring_capacity + 1;
    const std::uint64_t sequence = first_sequence + index;
    const std::uint64_t slot = (sequence - 1) % audit_ring_capacity;

    return audit_ring[slot];
}
