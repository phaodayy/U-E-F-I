#pragma once
#include <cstdint>

namespace timing::tsc_drift_compensator
{
	constexpr std::uint64_t processor_slot_count = 256;
	constexpr std::uint64_t audit_ring_capacity = 128;

	struct stats_t
	{
		std::uint64_t samples = 0;
		std::uint64_t ignored_samples = 0;
		std::uint64_t average_exit_cycles = 0;
		std::uint64_t last_exit_cycles = 0;
		std::uint64_t compensation_count = 0;
		std::uint64_t monotonic_clamps = 0;
		std::uint64_t max_adjustment_cycles = 0;
		std::uint8_t enabled = 0;
	};

	struct audit_record_t
	{
		std::uint64_t sequence = 0;
		std::uint64_t timestamp = 0;
		std::uint64_t exit_reason = 0;
		std::uint64_t measured_cycles = 0;
		std::uint64_t average_cycles = 0;
		std::uint64_t adjustment_cycles = 0;
		std::uint32_t processor_slot = 0;
		std::uint8_t compensated = 0;
	};

	void set_up();
	void set_enabled(std::uint8_t enabled);
	std::uint8_t is_enabled();
	void begin_vmexit();
	void finish_vmexit(std::uint64_t exit_reason);
	std::uint64_t compensate_tsc(std::uint64_t raw_virtual_tsc);
	stats_t stats();
	std::uint64_t audit_record_count();
	std::uint64_t audit_record_total_count();
	audit_record_t audit_record_at(std::uint64_t index);
}
