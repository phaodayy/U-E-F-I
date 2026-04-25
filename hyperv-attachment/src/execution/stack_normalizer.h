#pragma once
#include <cstdint>

struct trap_frame_t;

namespace execution::stack_normalizer
{
	constexpr std::uint64_t audit_ring_capacity = 128;

	enum class event_type_t : std::uint8_t
	{
		unknown,
		hypercall_init,
		hypercall_dispatch,
		cpuid_spoof,
		cr4_shadow,
		feature_control_shadow,
		tsc_shadow,
		slat_violation,
		breakpoint_intercept,
		exception_reinject,
	};

	enum action_flags_t : std::uint64_t
	{
		action_observed = 1ull << 0,
		action_rsp_was_canonical = 1ull << 1,
		action_rsp_repaired_from_arch = 1ull << 2,
		action_rsp_still_invalid = 1ull << 3,
		action_rsp_alignment_noted = 1ull << 4,
		action_repair_disabled_by_policy = 1ull << 5,
		action_resume_blocked_by_policy = 1ull << 6,
		action_rsp_spoofed = 1ull << 7,
	};

	enum class policy_t : std::uint8_t
	{
		observe_only,
		repair_trap_frame_rsp,
		fail_closed,
	};

	struct context_t
	{
		event_type_t event_type = event_type_t::unknown;
		std::uint64_t exit_reason = 0;
		trap_frame_t* trap_frame = nullptr;
		std::uint8_t allow_trap_frame_rsp_repair = 1;
	};

	struct result_t
	{
		std::uint64_t actions = 0;
		std::uint64_t guest_rip = 0;
		std::uint64_t original_rsp = 0;
		std::uint64_t normalized_rsp = 0;
		std::uint64_t sequence = 0;
		event_type_t event_type = event_type_t::unknown;
		std::uint8_t can_resume = 1;
	};

	struct audit_record_t
	{
		std::uint64_t sequence = 0;
		std::uint64_t timestamp = 0;
		std::uint64_t exit_reason = 0;
		std::uint64_t actions = 0;
		std::uint64_t guest_rip = 0;
		std::uint64_t original_rsp = 0;
		std::uint64_t normalized_rsp = 0;
		event_type_t event_type = event_type_t::unknown;
		policy_t policy = policy_t::repair_trap_frame_rsp;
	};

	void set_up();
	void set_policy(policy_t policy);
	policy_t current_policy();
	result_t normalize_before_guest_resume(const context_t& context);
	std::uint64_t normalization_count();
	std::uint64_t repair_count();
	std::uint64_t invalid_count();
	std::uint64_t blocked_count();
	std::uint64_t audit_record_count();
	std::uint64_t audit_record_total_count();
	audit_record_t audit_record_at(std::uint64_t index);
}
