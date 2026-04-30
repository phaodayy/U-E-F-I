#pragma once
#include <ia32-doc/ia32.hpp>
#include <cstdint>

#include "amd_def.h"

struct trap_frame_t;

namespace arch
{
	std::uint64_t vmread(std::uint64_t field);
	void vmwrite(std::uint64_t field, std::uint64_t value);
	std::uint64_t get_vmexit_reason();
	std::uint64_t get_vmexit_instruction_length();

#ifdef _INTELMACHINE
	vmx_exit_qualification_ept_violation get_exit_qualification();
	std::uint64_t get_guest_physical_address();
#endif

	std::uint8_t is_cpuid(std::uint64_t vmexit_reason);
	std::uint8_t is_slat_violation(std::uint64_t vmexit_reason);

	std::uint8_t is_non_maskable_interrupt_exit(std::uint64_t vmexit_reason);
	std::uint8_t is_breakpoint_exit(std::uint64_t vmexit_reason);
	std::uint8_t is_io_instruction(std::uint64_t vmexit_reason);

	void enable_breakpoint_intercept();
	void enable_io_intercept();
	void reinject_exception(std::uint8_t vector);

	cr3 get_guest_cr3();
	std::uint8_t get_guest_cpl();

	cr3 get_slat_cr3();
	void set_slat_cr3(cr3 slat_cr3);

	std::uint64_t get_guest_rsp();
	void set_guest_rsp(std::uint64_t guest_rsp);

	std::uint64_t get_guest_rip();
	void set_guest_rip(std::uint64_t guest_rip);

	void advance_guest_rip();

	std::uint8_t is_mov_cr(std::uint64_t vmexit_reason);
	void enable_cr4_shadowing();
	std::uint8_t handle_cr4_mov_exit(trap_frame_t* trap_frame);
	std::uint8_t handle_cpuid_spoof(trap_frame_t* trap_frame);
	std::uint8_t is_rdmsr(std::uint64_t vmexit_reason);
	void enable_feature_control_shadowing();
	std::uint8_t handle_feature_control_rdmsr(trap_frame_t* trap_frame);
	std::uint8_t is_tsc_exit(std::uint64_t vmexit_reason);
	void enable_tsc_exiting();
	std::uint8_t handle_tsc_exit(std::uint64_t vmexit_reason, trap_frame_t* trap_frame);

#ifdef _INTELMACHINE
	vmx_exit_qualification_ept_violation get_exit_qualification();

	std::uint64_t get_guest_physical_address();
#else
	vmcb_t* get_vmcb();
	void parse_vmcb_gadget(const std::uint8_t* get_vmcb_gadget);
#endif
}
