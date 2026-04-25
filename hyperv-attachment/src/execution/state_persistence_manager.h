#pragma once
#include <cstdint>

namespace execution::state_persistence_manager
{
	constexpr std::uint64_t audit_ring_capacity = 128;

	enum class invariant_t : std::uint8_t
	{
		cr4_vmxe_hidden,
		cpuid_hypervisor_hidden,
		feature_control_locked,
		tsc_exiting_base,
	};

	enum invariant_mask_t : std::uint64_t
	{
		invariant_cr4_vmxe_hidden = 1ull << 0,
		invariant_cpuid_hypervisor_hidden = 1ull << 1,
		invariant_feature_control_locked = 1ull << 2,
		invariant_tsc_exiting_base = 1ull << 3,
	};

	enum action_flags_t : std::uint64_t
	{
		action_observed = 1ull << 0,
		action_cr4_shadow_reasserted = 1ull << 1,
		action_cpuid_policy_active = 1ull << 2,
		action_feature_control_shadow_reasserted = 1ull << 3,
		action_tsc_exiting_reasserted = 1ull << 4,
	};

	struct audit_record_t
	{
		std::uint64_t sequence = 0;
		std::uint64_t timestamp = 0;
		std::uint64_t exit_reason = 0;
		std::uint64_t invariant_mask = 0;
		std::uint64_t actions = 0;
	};

	void set_up();
	void set_invariant_enabled(invariant_t invariant, std::uint8_t enabled);
	std::uint8_t is_invariant_enabled(invariant_t invariant);
	std::uint64_t current_invariant_mask();
	std::uint64_t enforce_on_vmexit(std::uint64_t exit_reason);
	std::uint64_t enforcement_count();
	std::uint64_t audit_record_count();
	std::uint64_t audit_record_total_count();
	audit_record_t audit_record_at(std::uint64_t index);
}
