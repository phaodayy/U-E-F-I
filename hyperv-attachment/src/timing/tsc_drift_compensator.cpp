#include "tsc_drift_compensator.h"

#include "../crt/crt.h"

#include <intrin.h>

namespace
{
	struct processor_state_t
	{
		std::uint64_t active_exit_start_tsc = 0;
		std::uint64_t last_guest_tsc = 0;
		std::uint8_t active = 0;
	};

	volatile long enabled = 0;
	volatile long long sample_count = 0;
	volatile long long ignored_sample_count = 0;
	volatile long long compensation_count = 0;
	volatile long long monotonic_clamp_count = 0;
	volatile long long audit_sequence = 0;
	std::uint64_t average_exit_cycles = 0;
	std::uint64_t last_exit_cycles = 0;

	processor_state_t processor_states[timing::tsc_drift_compensator::processor_slot_count] = { };
	timing::tsc_drift_compensator::audit_record_t audit_ring[
		timing::tsc_drift_compensator::audit_ring_capacity] = { };

	constexpr std::uint64_t minimum_valid_exit_cycles = 64;
	constexpr std::uint64_t maximum_valid_exit_cycles = 100000;
	constexpr std::uint64_t maximum_adjustment_cycles = 3000;
	constexpr std::uint64_t audit_sample_interval = 1024;
	constexpr std::uint64_t ewma_weight = 7;

	std::uint32_t current_processor_slot(std::uint64_t* const tsc_out = nullptr)
	{
		unsigned int aux = 0;
		const std::uint64_t tsc = __rdtscp(&aux);

		if (tsc_out != nullptr)
		{
			*tsc_out = tsc;
		}

		return aux % timing::tsc_drift_compensator::processor_slot_count;
	}

	std::uint64_t clamp_adjustment(const std::uint64_t cycles)
	{
		return cycles > maximum_adjustment_cycles ? maximum_adjustment_cycles : cycles;
	}

	void record_audit(const std::uint64_t exit_reason, const std::uint64_t measured_cycles,
		const std::uint64_t average_cycles, const std::uint64_t adjustment_cycles,
		const std::uint32_t processor_slot, const std::uint8_t compensated)
	{
		const std::uint64_t sequence = static_cast<std::uint64_t>(_InterlockedIncrement64(&audit_sequence));
		const std::uint64_t slot = (sequence - 1) % timing::tsc_drift_compensator::audit_ring_capacity;

		timing::tsc_drift_compensator::audit_record_t record = { };
		record.sequence = sequence;
		record.timestamp = __rdtsc();
		record.exit_reason = exit_reason;
		record.measured_cycles = measured_cycles;
		record.average_cycles = average_cycles;
		record.adjustment_cycles = adjustment_cycles;
		record.processor_slot = processor_slot;
		record.compensated = compensated;

		audit_ring[slot] = record;
	}
}

void timing::tsc_drift_compensator::set_up()
{
	_InterlockedExchange(&enabled, 0);
	_InterlockedExchange64(&sample_count, 0);
	_InterlockedExchange64(&ignored_sample_count, 0);
	_InterlockedExchange64(&compensation_count, 0);
	_InterlockedExchange64(&monotonic_clamp_count, 0);
	_InterlockedExchange64(&audit_sequence, 0);

	average_exit_cycles = 0;
	last_exit_cycles = 0;

	crt::set_memory(&processor_states[0], 0, sizeof(processor_states));
	crt::set_memory(&audit_ring[0], 0, sizeof(audit_ring));
}

void timing::tsc_drift_compensator::set_enabled(const std::uint8_t enabled_in)
{
	_InterlockedExchange(&enabled, enabled_in != 0 ? 1 : 0);
}

std::uint8_t timing::tsc_drift_compensator::is_enabled()
{
	return enabled != 0;
}

void timing::tsc_drift_compensator::begin_vmexit()
{
	if (is_enabled() == 0)
	{
		return;
	}

	std::uint64_t start_tsc = 0;
	const std::uint32_t slot = current_processor_slot(&start_tsc);

	processor_states[slot].active_exit_start_tsc = start_tsc;
	processor_states[slot].active = 1;
}

void timing::tsc_drift_compensator::finish_vmexit(const std::uint64_t exit_reason)
{
	if (is_enabled() == 0)
	{
		return;
	}

	std::uint64_t end_tsc = 0;
	const std::uint32_t slot = current_processor_slot(&end_tsc);
	processor_state_t* const processor_state = &processor_states[slot];

	if (processor_state->active == 0 || processor_state->active_exit_start_tsc == 0 ||
		end_tsc <= processor_state->active_exit_start_tsc)
	{
		_InterlockedIncrement64(&ignored_sample_count);
		return;
	}

	processor_state->active = 0;

	const std::uint64_t measured_cycles = end_tsc - processor_state->active_exit_start_tsc;
	if (measured_cycles < minimum_valid_exit_cycles || measured_cycles > maximum_valid_exit_cycles)
	{
		_InterlockedIncrement64(&ignored_sample_count);
		return;
	}

	const std::uint64_t clamped_cycles = clamp_adjustment(measured_cycles);
	last_exit_cycles = measured_cycles;

	if (average_exit_cycles == 0)
	{
		average_exit_cycles = clamped_cycles;
	}
	else
	{
		average_exit_cycles = ((average_exit_cycles * ewma_weight) + clamped_cycles) / (ewma_weight + 1);
	}

	const std::uint64_t current_sample_count =
		static_cast<std::uint64_t>(_InterlockedIncrement64(&sample_count));

	if ((current_sample_count % audit_sample_interval) == 0)
	{
		record_audit(exit_reason, measured_cycles, average_exit_cycles, clamp_adjustment(average_exit_cycles), slot, 0);
	}
}

std::uint64_t timing::tsc_drift_compensator::compensate_tsc(const std::uint64_t raw_virtual_tsc)
{
	if (is_enabled() == 0)
	{
		return raw_virtual_tsc;
	}

	std::uint64_t current_tsc = 0;
	const std::uint32_t slot = current_processor_slot(&current_tsc);
	(void)current_tsc;

	const std::uint64_t adjustment = clamp_adjustment(average_exit_cycles);
	std::uint64_t compensated_tsc = raw_virtual_tsc > adjustment ? raw_virtual_tsc - adjustment : raw_virtual_tsc;
	processor_state_t* const processor_state = &processor_states[slot];

	if (compensated_tsc <= processor_state->last_guest_tsc)
	{
		compensated_tsc = processor_state->last_guest_tsc + 1;
		_InterlockedIncrement64(&monotonic_clamp_count);
	}

	processor_state->last_guest_tsc = compensated_tsc;

	const std::uint64_t current_compensation_count =
		static_cast<std::uint64_t>(_InterlockedIncrement64(&compensation_count));

	if ((current_compensation_count % audit_sample_interval) == 0)
	{
		record_audit(0, 0, average_exit_cycles, adjustment, slot, 1);
	}

	return compensated_tsc;
}

timing::tsc_drift_compensator::stats_t timing::tsc_drift_compensator::stats()
{
	stats_t current_stats = { };
	current_stats.samples = static_cast<std::uint64_t>(sample_count);
	current_stats.ignored_samples = static_cast<std::uint64_t>(ignored_sample_count);
	current_stats.average_exit_cycles = average_exit_cycles;
	current_stats.last_exit_cycles = last_exit_cycles;
	current_stats.compensation_count = static_cast<std::uint64_t>(compensation_count);
	current_stats.monotonic_clamps = static_cast<std::uint64_t>(monotonic_clamp_count);
	current_stats.max_adjustment_cycles = maximum_adjustment_cycles;
	current_stats.enabled = is_enabled();

	return current_stats;
}

std::uint64_t timing::tsc_drift_compensator::audit_record_count()
{
	const std::uint64_t total_count = audit_record_total_count();

	return total_count < audit_ring_capacity ? total_count : audit_ring_capacity;
}

std::uint64_t timing::tsc_drift_compensator::audit_record_total_count()
{
	return static_cast<std::uint64_t>(audit_sequence);
}

timing::tsc_drift_compensator::audit_record_t timing::tsc_drift_compensator::audit_record_at(
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
