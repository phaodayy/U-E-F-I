#include "stack_normalizer.h"

#include "../arch/arch.h"
#include "../crt/crt.h"

#include <intrin.h>
#include <structures/trap_frame.h>

namespace
{
	volatile long long normalization_sequence = 0;
	volatile long long repair_sequence = 0;
	volatile long long invalid_sequence = 0;
	volatile long long blocked_sequence = 0;
	execution::stack_normalizer::policy_t active_policy =
		execution::stack_normalizer::policy_t::repair_trap_frame_rsp;
	execution::stack_normalizer::audit_record_t audit_ring[execution::stack_normalizer::audit_ring_capacity] = { };

	constexpr std::uint64_t canonical_bit = 47;
	constexpr std::uint64_t canonical_high_bits = 0xFFFFull;

	std::uint8_t is_canonical_address(const std::uint64_t address)
	{
		if (address == 0)
		{
			return 0;
		}

		const std::uint64_t high = address >> 48;
		const std::uint64_t sign = (address >> canonical_bit) & 1ull;

		return sign == 0 ? high == 0 : high == canonical_high_bits;
	}

	std::uint8_t has_standard_stack_alignment(const std::uint64_t rsp)
	{
		return (rsp & 0xFull) == 0 || (rsp & 0xFull) == 8;
	}

	std::uint64_t current_normalization_sequence()
	{
		return static_cast<std::uint64_t>(normalization_sequence);
	}

	void record_audit(const execution::stack_normalizer::context_t& context,
		const execution::stack_normalizer::result_t& result,
		const execution::stack_normalizer::policy_t policy)
	{
		const std::uint64_t slot = (result.sequence - 1) % execution::stack_normalizer::audit_ring_capacity;

		execution::stack_normalizer::audit_record_t record = { };
		record.sequence = result.sequence;
		record.timestamp = __rdtsc();
		record.exit_reason = context.exit_reason;
		record.actions = result.actions;
		record.guest_rip = result.guest_rip;
		record.original_rsp = result.original_rsp;
		record.normalized_rsp = result.normalized_rsp;
		record.event_type = result.event_type;
		record.policy = policy;

		audit_ring[slot] = record;
	}
}

void execution::stack_normalizer::set_up()
{
	_InterlockedExchange64(&normalization_sequence, 0);
	_InterlockedExchange64(&repair_sequence, 0);
	_InterlockedExchange64(&invalid_sequence, 0);
	_InterlockedExchange64(&blocked_sequence, 0);
	active_policy = policy_t::repair_trap_frame_rsp;
	crt::set_memory(&audit_ring[0], 0, sizeof(audit_ring));
}

void execution::stack_normalizer::set_policy(const policy_t policy)
{
	active_policy = policy;
}

execution::stack_normalizer::policy_t execution::stack_normalizer::current_policy()
{
	return active_policy;
}

execution::stack_normalizer::result_t execution::stack_normalizer::normalize_before_guest_resume(
	const context_t& context)
{
	const policy_t policy = active_policy;
	result_t result = { };
	result.actions = action_observed;
	result.event_type = context.event_type;
	result.sequence = static_cast<std::uint64_t>(_InterlockedIncrement64(&normalization_sequence));
	result.guest_rip = arch::get_guest_rip();

	const std::uint64_t architectural_rsp = arch::get_guest_rsp();
	const std::uint64_t candidate_rsp = context.trap_frame != nullptr ? context.trap_frame->rsp : architectural_rsp;

	result.original_rsp = candidate_rsp;
	result.normalized_rsp = candidate_rsp;

	if (has_standard_stack_alignment(candidate_rsp) == 0)
	{
		result.actions |= action_rsp_alignment_noted;
	}

	if (is_canonical_address(candidate_rsp) != 0)
	{
		result.actions |= action_rsp_was_canonical;
		record_audit(context, result, policy);
		return result;
	}

	if (context.trap_frame != nullptr &&
		context.allow_trap_frame_rsp_repair != 0 &&
		policy != policy_t::observe_only &&
		is_canonical_address(architectural_rsp) != 0)
	{
		context.trap_frame->rsp = architectural_rsp;
		result.normalized_rsp = architectural_rsp;
		result.actions |= action_rsp_repaired_from_arch;
		_InterlockedIncrement64(&repair_sequence);
		record_audit(context, result, policy);
		return result;
	}

	result.actions |= action_rsp_still_invalid;
	_InterlockedIncrement64(&invalid_sequence);

	if (policy == policy_t::observe_only)
	{
		result.actions |= action_repair_disabled_by_policy;
	}

	if (policy == policy_t::fail_closed)
	{
		result.actions |= action_resume_blocked_by_policy;
		result.can_resume = 0;
		_InterlockedIncrement64(&blocked_sequence);
	}

	record_audit(context, result, policy);
	return result;
}

std::uint64_t execution::stack_normalizer::normalization_count()
{
	return current_normalization_sequence();
}

std::uint64_t execution::stack_normalizer::repair_count()
{
	return static_cast<std::uint64_t>(repair_sequence);
}

std::uint64_t execution::stack_normalizer::invalid_count()
{
	return static_cast<std::uint64_t>(invalid_sequence);
}

std::uint64_t execution::stack_normalizer::blocked_count()
{
	return static_cast<std::uint64_t>(blocked_sequence);
}

std::uint64_t execution::stack_normalizer::audit_record_count()
{
	const std::uint64_t total_count = audit_record_total_count();

	return total_count < audit_ring_capacity ? total_count : audit_ring_capacity;
}

std::uint64_t execution::stack_normalizer::audit_record_total_count()
{
	return current_normalization_sequence();
}

execution::stack_normalizer::audit_record_t execution::stack_normalizer::audit_record_at(
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
