#include "state_persistence_manager.h"

#include "../arch/arch.h"
#include "../crt/crt.h"

#include <intrin.h>

namespace
{
	volatile long long active_invariant_mask = 0;
	volatile long long enforcement_sequence = 0;
	execution::state_persistence_manager::audit_record_t audit_ring[
		execution::state_persistence_manager::audit_ring_capacity] = { };

	std::uint64_t invariant_to_mask(const execution::state_persistence_manager::invariant_t invariant)
	{
		using namespace execution::state_persistence_manager;

		switch (invariant)
		{
		case invariant_t::cr4_vmxe_hidden:
			return invariant_cr4_vmxe_hidden;
		case invariant_t::cpuid_hypervisor_hidden:
			return invariant_cpuid_hypervisor_hidden;
		case invariant_t::feature_control_locked:
			return invariant_feature_control_locked;
		case invariant_t::tsc_exiting_base:
			return invariant_tsc_exiting_base;
		default:
			return 0;
		}
	}

	void record_audit(const std::uint64_t sequence, const std::uint64_t exit_reason,
		const std::uint64_t invariant_mask, const std::uint64_t actions)
	{
		const std::uint64_t slot = (sequence - 1) % execution::state_persistence_manager::audit_ring_capacity;

		execution::state_persistence_manager::audit_record_t record = { };
		record.sequence = sequence;
		record.timestamp = __rdtsc();
		record.exit_reason = exit_reason;
		record.invariant_mask = invariant_mask;
		record.actions = actions;

		audit_ring[slot] = record;
	}
}

void execution::state_persistence_manager::set_up()
{
	_InterlockedExchange64(&active_invariant_mask, 0);
	_InterlockedExchange64(&enforcement_sequence, 0);
	crt::set_memory(&audit_ring[0], 0, sizeof(audit_ring));
}

void execution::state_persistence_manager::set_invariant_enabled(const invariant_t invariant,
	const std::uint8_t enabled)
{
	const std::uint64_t mask = invariant_to_mask(invariant);
	if (mask == 0)
	{
		return;
	}

	if (enabled != 0)
	{
		_InterlockedOr64(&active_invariant_mask, static_cast<long long>(mask));
		return;
	}

	_InterlockedAnd64(&active_invariant_mask, static_cast<long long>(~mask));
}

std::uint8_t execution::state_persistence_manager::is_invariant_enabled(const invariant_t invariant)
{
	const std::uint64_t mask = invariant_to_mask(invariant);

	return mask != 0 && (current_invariant_mask() & mask) != 0;
}

std::uint64_t execution::state_persistence_manager::current_invariant_mask()
{
	return static_cast<std::uint64_t>(active_invariant_mask);
}

std::uint64_t execution::state_persistence_manager::enforce_on_vmexit(const std::uint64_t exit_reason)
{
	const std::uint64_t invariant_mask = current_invariant_mask();
	std::uint64_t actions = action_observed;

	if ((invariant_mask & invariant_cr4_vmxe_hidden) != 0)
	{
		arch::enable_cr4_shadowing();
		actions |= action_cr4_shadow_reasserted;
	}

	if ((invariant_mask & invariant_cpuid_hypervisor_hidden) != 0)
	{
		actions |= action_cpuid_policy_active;
	}

	if ((invariant_mask & invariant_feature_control_locked) != 0)
	{
		arch::enable_feature_control_shadowing();
		actions |= action_feature_control_shadow_reasserted;
	}

	if ((invariant_mask & invariant_tsc_exiting_base) != 0)
	{
		arch::enable_tsc_exiting();
		actions |= action_tsc_exiting_reasserted;
	}

	const std::uint64_t sequence = static_cast<std::uint64_t>(_InterlockedIncrement64(&enforcement_sequence));
	record_audit(sequence, exit_reason, invariant_mask, actions);

	return actions;
}

std::uint64_t execution::state_persistence_manager::enforcement_count()
{
	return static_cast<std::uint64_t>(enforcement_sequence);
}

std::uint64_t execution::state_persistence_manager::audit_record_count()
{
	const std::uint64_t total_count = audit_record_total_count();

	return total_count < audit_ring_capacity ? total_count : audit_ring_capacity;
}

std::uint64_t execution::state_persistence_manager::audit_record_total_count()
{
	return enforcement_count();
}

execution::state_persistence_manager::audit_record_t execution::state_persistence_manager::audit_record_at(
	const std::uint64_t index)
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
