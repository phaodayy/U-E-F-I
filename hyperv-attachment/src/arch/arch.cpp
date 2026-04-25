#include "arch.h"
#include "../crt/crt.h"

#include <intrin.h>
#include <structures/trap_frame.h>

#ifdef _INTELMACHINE
#include <ia32-doc/ia32.hpp>

namespace
{
	constexpr std::uint64_t cr4_vmxe_mask = CR4_VMX_ENABLE_FLAG;
	constexpr std::uint32_t cpuid_leaf_feature_information = 0x00000001;
	constexpr std::uint32_t cpuid_leaf_hypervisor_base = 0x40000000;
	constexpr std::uint32_t cpuid_leaf_hypervisor_end = 0x400000FF;
	constexpr std::uint32_t cpuid_ecx_hypervisor_present = 1U << 31;
	constexpr std::uint32_t ia32_feature_control_msr = 0x0000003A;
	constexpr std::uint64_t ia32_feature_control_vmx_bits = 0x7;
	constexpr std::uint64_t ia32_feature_control_locked = 0x1;
	constexpr std::uint64_t tsc_software_offset = 0;

	std::uint64_t* get_trap_frame_register(trap_frame_t* const trap_frame, const std::uint64_t register_index)
	{
		if (trap_frame == nullptr)
		{
			return nullptr;
		}

		switch (register_index)
		{
		case VMX_EXIT_QUALIFICATION_GENREG_RAX:
			return &trap_frame->rax;
		case VMX_EXIT_QUALIFICATION_GENREG_RCX:
			return &trap_frame->rcx;
		case VMX_EXIT_QUALIFICATION_GENREG_RDX:
			return &trap_frame->rdx;
		case VMX_EXIT_QUALIFICATION_GENREG_RBX:
			return &trap_frame->rbx;
		case VMX_EXIT_QUALIFICATION_GENREG_RSP:
			return &trap_frame->rsp;
		case VMX_EXIT_QUALIFICATION_GENREG_RBP:
			return &trap_frame->rbp;
		case VMX_EXIT_QUALIFICATION_GENREG_RSI:
			return &trap_frame->rsi;
		case VMX_EXIT_QUALIFICATION_GENREG_RDI:
			return &trap_frame->rdi;
		case VMX_EXIT_QUALIFICATION_GENREG_R8:
			return &trap_frame->r8;
		case VMX_EXIT_QUALIFICATION_GENREG_R9:
			return &trap_frame->r9;
		case VMX_EXIT_QUALIFICATION_GENREG_R10:
			return &trap_frame->r10;
		case VMX_EXIT_QUALIFICATION_GENREG_R11:
			return &trap_frame->r11;
		case VMX_EXIT_QUALIFICATION_GENREG_R12:
			return &trap_frame->r12;
		case VMX_EXIT_QUALIFICATION_GENREG_R13:
			return &trap_frame->r13;
		case VMX_EXIT_QUALIFICATION_GENREG_R14:
			return &trap_frame->r14;
		case VMX_EXIT_QUALIFICATION_GENREG_R15:
			return &trap_frame->r15;
		default:
			return nullptr;
		}
	}

	std::uint8_t read_cr_mov_source_register(trap_frame_t* const trap_frame, const std::uint64_t register_index, std::uint64_t* const value)
	{
		if (value == nullptr)
		{
			return 0;
		}

		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RSP)
		{
			*value = arch::get_guest_rsp();
			return 1;
		}

		const std::uint64_t* const register_value = get_trap_frame_register(trap_frame, register_index);
		if (register_value == nullptr)
		{
			return 0;
		}

		*value = *register_value;
		return 1;
	}

	std::uint8_t write_cr_mov_destination_register(trap_frame_t* const trap_frame, const std::uint64_t register_index, const std::uint64_t value)
	{
		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RSP)
		{
			arch::set_guest_rsp(value);
		}

		std::uint64_t* const register_value = get_trap_frame_register(trap_frame, register_index);
		if (register_value == nullptr)
		{
			return register_index == VMX_EXIT_QUALIFICATION_GENREG_RSP ? 1 : 0;
		}

		*register_value = value;
		return 1;
	}
}

std::uint64_t vmread(const std::uint64_t field)
{
	std::uint64_t value = 0;

	__vmx_vmread(field, &value);

	return value;
}

void vmwrite(const std::uint64_t field, const std::uint64_t value)
{
	__vmx_vmwrite(field, value);
}

std::uint64_t get_vmexit_instruction_length()
{
	return vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH);
}

vmx_exit_qualification_ept_violation arch::get_exit_qualification()
{
	return { .flags = vmread(VMCS_EXIT_QUALIFICATION) };
}

std::uint64_t arch::get_guest_physical_address()
{
	return vmread(VMCS_GUEST_PHYSICAL_ADDRESS);
}

std::uint8_t arch::is_mov_cr(const std::uint64_t vmexit_reason)
{
	const std::uint64_t basic_exit_reason = vmexit_reason & VMX_VMEXIT_REASON_BASIC_EXIT_REASON_FLAG;

	return basic_exit_reason == VMX_EXIT_REASON_MOV_CR;
}

std::uint8_t arch::is_rdmsr(const std::uint64_t vmexit_reason)
{
	const std::uint64_t basic_exit_reason = vmexit_reason & VMX_VMEXIT_REASON_BASIC_EXIT_REASON_FLAG;

	return basic_exit_reason == VMX_EXIT_REASON_EXECUTE_RDMSR;
}

std::uint8_t arch::is_tsc_exit(const std::uint64_t vmexit_reason)
{
	const std::uint64_t basic_exit_reason = vmexit_reason & VMX_VMEXIT_REASON_BASIC_EXIT_REASON_FLAG;

	return basic_exit_reason == VMX_EXIT_REASON_EXECUTE_RDTSC ||
		basic_exit_reason == VMX_EXIT_REASON_EXECUTE_RDTSCP;
}

void arch::enable_tsc_exiting()
{
	const std::uint64_t controls = vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS);
	const std::uint64_t tsc_exiting_controls = controls | IA32_VMX_PROCBASED_CTLS_RDTSC_EXITING_FLAG;

	if (controls != tsc_exiting_controls)
	{
		vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, tsc_exiting_controls);
	}
}

void arch::enable_cr4_shadowing()
{
	const std::uint64_t cr4_mask = vmread(VMCS_CTRL_CR4_GUEST_HOST_MASK);
	const std::uint64_t cr4_shadow = vmread(VMCS_CTRL_CR4_READ_SHADOW);

	const std::uint64_t shadowed_mask = cr4_mask | cr4_vmxe_mask;
	const std::uint64_t shadowed_cr4 = cr4_shadow & ~cr4_vmxe_mask;

	if (cr4_mask != shadowed_mask)
	{
		vmwrite(VMCS_CTRL_CR4_GUEST_HOST_MASK, shadowed_mask);
	}

	if (cr4_shadow != shadowed_cr4)
	{
		vmwrite(VMCS_CTRL_CR4_READ_SHADOW, shadowed_cr4);
	}
}

std::uint8_t arch::handle_cr4_mov_exit(trap_frame_t* const trap_frame)
{
	const std::uint64_t exit_qualification = vmread(VMCS_EXIT_QUALIFICATION);
	const std::uint64_t control_register = VMX_EXIT_QUALIFICATION_MOV_CR_CONTROL_REGISTER(exit_qualification);
	const std::uint64_t access_type = VMX_EXIT_QUALIFICATION_MOV_CR_ACCESS_TYPE(exit_qualification);
	const std::uint64_t register_index = VMX_EXIT_QUALIFICATION_MOV_CR_GENERAL_PURPOSE_REGISTER(exit_qualification);

	if (control_register != VMX_EXIT_QUALIFICATION_REGISTER_CR4)
	{
		return 0;
	}

	if (access_type == VMX_EXIT_QUALIFICATION_ACCESS_MOV_FROM_CR)
	{
		const std::uint64_t visible_cr4 = vmread(VMCS_GUEST_CR4) & ~cr4_vmxe_mask;

		if (write_cr_mov_destination_register(trap_frame, register_index, visible_cr4) == 0)
		{
			return 0;
		}

		advance_guest_rip();
		return 1;
	}

	if (access_type == VMX_EXIT_QUALIFICATION_ACCESS_MOV_TO_CR)
	{
		std::uint64_t guest_requested_cr4 = 0;
		if (read_cr_mov_source_register(trap_frame, register_index, &guest_requested_cr4) == 0)
		{
			return 0;
		}

		const std::uint64_t current_guest_cr4 = vmread(VMCS_GUEST_CR4);
		const std::uint64_t cr4_mask = vmread(VMCS_CTRL_CR4_GUEST_HOST_MASK);
		const std::uint64_t cr4_shadow = vmread(VMCS_CTRL_CR4_READ_SHADOW);
		const std::uint64_t shadow_mismatch = (guest_requested_cr4 ^ cr4_shadow) & cr4_mask;

		if ((shadow_mismatch & ~cr4_vmxe_mask) != 0)
		{
			return 0;
		}

		const std::uint64_t hardware_cr4 = (current_guest_cr4 & cr4_mask) | (guest_requested_cr4 & ~cr4_mask) | cr4_vmxe_mask;
		const std::uint64_t hidden_shadow = cr4_shadow & ~cr4_vmxe_mask;

		vmwrite(VMCS_GUEST_CR4, hardware_cr4);
		vmwrite(VMCS_CTRL_CR4_READ_SHADOW, hidden_shadow);
		enable_cr4_shadowing();

		advance_guest_rip();
		return 1;
	}

	return 0;
}

std::uint8_t arch::handle_feature_control_rdmsr(trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr)
	{
		return 0;
	}

	const std::uint32_t msr_index = static_cast<std::uint32_t>(trap_frame->rcx);
	if (msr_index != ia32_feature_control_msr)
	{
		return 0;
	}

	const std::uint64_t actual_feature_control = __readmsr(ia32_feature_control_msr);
	const std::uint64_t shadowed_feature_control =
		(actual_feature_control & ~ia32_feature_control_vmx_bits) | ia32_feature_control_locked;

	trap_frame->rax = static_cast<std::uint32_t>(shadowed_feature_control);
	trap_frame->rdx = static_cast<std::uint32_t>(shadowed_feature_control >> 32);

	advance_guest_rip();
	return 1;
}

std::uint8_t arch::handle_tsc_exit(const std::uint64_t vmexit_reason, trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr)
	{
		return 0;
	}

	const std::uint64_t basic_exit_reason = vmexit_reason & VMX_VMEXIT_REASON_BASIC_EXIT_REASON_FLAG;
	if (basic_exit_reason != VMX_EXIT_REASON_EXECUTE_RDTSC &&
		basic_exit_reason != VMX_EXIT_REASON_EXECUTE_RDTSCP)
	{
		return 0;
	}

	const std::uint64_t virtual_tsc = __rdtsc() + vmread(VMCS_CTRL_TSC_OFFSET) + tsc_software_offset;

	trap_frame->rax = static_cast<std::uint32_t>(virtual_tsc);
	trap_frame->rdx = static_cast<std::uint32_t>(virtual_tsc >> 32);

	if (basic_exit_reason == VMX_EXIT_REASON_EXECUTE_RDTSCP)
	{
		trap_frame->rcx = static_cast<std::uint32_t>(__readmsr(IA32_TSC_AUX));
	}

	advance_guest_rip();
	return 1;
}

std::uint8_t arch::handle_cpuid_spoof(trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr)
	{
		return 0;
	}

	const std::uint32_t leaf = static_cast<std::uint32_t>(trap_frame->rax);
	const std::uint32_t subleaf = static_cast<std::uint32_t>(trap_frame->rcx);

	if (leaf >= cpuid_leaf_hypervisor_base && leaf <= cpuid_leaf_hypervisor_end)
	{
		trap_frame->rax = 0;
		trap_frame->rbx = 0;
		trap_frame->rcx = 0;
		trap_frame->rdx = 0;

		advance_guest_rip();
		return 1;
	}

	if (leaf == cpuid_leaf_feature_information)
	{
		int cpu_info[4] = {};
		__cpuidex(cpu_info, static_cast<int>(leaf), static_cast<int>(subleaf));

		trap_frame->rax = static_cast<std::uint32_t>(cpu_info[0]);
		trap_frame->rbx = static_cast<std::uint32_t>(cpu_info[1]);
		trap_frame->rcx = static_cast<std::uint32_t>(cpu_info[2]) & ~cpuid_ecx_hypervisor_present;
		trap_frame->rdx = static_cast<std::uint32_t>(cpu_info[3]);

		advance_guest_rip();
		return 1;
	}

	return 0;
}

#else
std::uint8_t get_vmcb_routine_bytes[27];

typedef vmcb_t*(*get_vmcb_routine_t)();

vmcb_t* arch::get_vmcb()
{
	get_vmcb_routine_t get_vmcb_routine = reinterpret_cast<get_vmcb_routine_t>(&get_vmcb_routine_bytes[0]);

	return get_vmcb_routine();
}

void arch::parse_vmcb_gadget(const std::uint8_t* const get_vmcb_gadget)
{
	constexpr std::uint32_t final_needed_opcode_offset = 23;

	crt::copy_memory(&get_vmcb_routine_bytes[0], get_vmcb_gadget, final_needed_opcode_offset);

	if (get_vmcb_gadget[25] == 8) // needs to be dereffed once more
	{
		constexpr std::uint8_t return_bytes[4] = {
			0x48, 0x8B, 0x00, // mov rax, [rax]
			0xC3 // ret
		};

		crt::copy_memory(&get_vmcb_routine_bytes[final_needed_opcode_offset], &return_bytes[0], sizeof(return_bytes));
	}
	else
	{
		get_vmcb_routine_bytes[final_needed_opcode_offset] = 0xC3;
	}
}
#endif

std::uint64_t arch::get_vmexit_reason()
{
#ifdef _INTELMACHINE
	return vmread(VMCS_EXIT_REASON);
#else
	const vmcb_t* const vmcb = get_vmcb();

	return vmcb->control.vmexit_reason;
#endif
}

std::uint8_t arch::is_cpuid(const std::uint64_t vmexit_reason)
{
#ifdef _INTELMACHINE
	return vmexit_reason == VMX_EXIT_REASON_EXECUTE_CPUID;
#else
	return vmexit_reason == SVM_EXIT_REASON_CPUID;
#endif
}

std::uint8_t arch::is_slat_violation(const std::uint64_t vmexit_reason)
{
#ifdef _INTELMACHINE
	return vmexit_reason == VMX_EXIT_REASON_EPT_VIOLATION;
#else
	return vmexit_reason == SVM_EXIT_REASON_NPF;
#endif
}

std::uint8_t arch::is_non_maskable_interrupt_exit(const std::uint64_t vmexit_reason)
{
#ifdef _INTELMACHINE
	if (vmexit_reason != VMX_EXIT_REASON_EXCEPTION_OR_NMI)
	{
		return 0;
	}

	const std::uint64_t raw_interruption_information = vmread(VMCS_VMEXIT_INTERRUPTION_INFORMATION);

	const vmexit_interrupt_information interrupt_information = { .flags = static_cast<std::uint32_t>(raw_interruption_information) };

	return interrupt_information.interruption_type == interruption_type::non_maskable_interrupt;
#else
	return vmexit_reason == SVM_EXIT_REASON_PHYSICAL_NMI;
#endif
}

std::uint8_t arch::is_breakpoint_exit(const std::uint64_t vmexit_reason)
{
#ifdef _INTELMACHINE
	if (vmexit_reason != VMX_EXIT_REASON_EXCEPTION_OR_NMI)
	{
		return 0;
	}

	const std::uint64_t raw_info = vmread(VMCS_VMEXIT_INTERRUPTION_INFORMATION);
	const vmexit_interrupt_information info = { .flags = static_cast<std::uint32_t>(raw_info) };

	// Vector 3 = #BP (INT3). INT3 triggers software_exception (6). 
    // Hardware breakpoints trigger hardware_exception (3). We accept both.
	return (info.vector == 3 && 
            (info.interruption_type == interruption_type::hardware_exception || 
             info.interruption_type == interruption_type::software_exception)) ? 1 : 0;
#else
	// AMD: SVM exit code for exception #3 = 0x43 (SVM_EXIT_EXCEPTION_3)
	if (vmexit_reason != 0x43) return 0;
	return 1;
#endif
}

void arch::enable_breakpoint_intercept()
{
#ifdef _INTELMACHINE
	// Đọc Exception Bitmap hiện tại từ VMCS, bật bit 3 (#BP = INT3)
	const std::uint64_t current_bitmap = vmread(VMCS_CTRL_EXCEPTION_BITMAP);
	vmwrite(VMCS_CTRL_EXCEPTION_BITMAP, current_bitmap | (1ULL << 3));
#else
	// AMD: Intercept exception #BP (vector 3) bằng cách bật bit 3 
	// trong intercept_misc_vector_3 (VMCB offset 0x0C = intercept exceptions)
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_misc_vector_3 |= (1U << 3);
	vmcb->control.clean.i = 0; // Đánh dấu dirty để SVM reload intercepts
#endif
}

void arch::reinject_exception(const std::uint8_t vector)
{
#ifdef _INTELMACHINE
	// Intel: Dùng nguyên VM-Exit Interruption-Information Field để ném lại đúng loại exception (hardware/software)
	const std::uint32_t exit_info = static_cast<std::uint32_t>(vmread(VMCS_VMEXIT_INTERRUPTION_INFORMATION));
	vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, exit_info);
	vmwrite(VMCS_CTRL_VMENTRY_INSTRUCTION_LENGTH, vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH));
#else
	// AMD: Tương tự, copy exit_info hoặc ít nhất ném lại với type = exception (3)
	vmcb_t* const vmcb = get_vmcb();
	
	const std::uint64_t eventinj = 
		(static_cast<std::uint64_t>(vector))        |
		(3ULL << 8)                                  |   // type = exception // AMD Type is 3 for ALL exceptions (hardware and software)
		(1ULL << 31);                                    // valid
	
	*reinterpret_cast<std::uint64_t*>(
		reinterpret_cast<std::uint8_t*>(&vmcb->control) + 0xA8) = eventinj;
#endif
}



cr3 arch::get_guest_cr3()
{
	cr3 guest_cr3;

#ifdef _INTELMACHINE
	guest_cr3.flags = vmread(VMCS_GUEST_CR3);
#else
	const vmcb_t* const vmcb = get_vmcb();

	guest_cr3.flags = vmcb->save_state.cr3;
#endif

	return guest_cr3;
}

cr3 arch::get_slat_cr3()
{
	cr3 slat_cr3;

#ifdef _INTELMACHINE
	slat_cr3.flags = vmread(VMCS_CTRL_EPT_POINTER);
#else
	const vmcb_t* const vmcb = arch::get_vmcb();

	slat_cr3 = vmcb->control.nested_cr3;
#endif

	return slat_cr3;
}

void arch::set_slat_cr3(const cr3 slat_cr3)
{
#ifdef _INTELMACHINE
	vmwrite(VMCS_CTRL_EPT_POINTER, slat_cr3.flags);
#else
	vmcb_t* const vmcb = arch::get_vmcb();

	vmcb->control.nested_cr3 = slat_cr3;
#endif
}

std::uint64_t arch::get_guest_rsp()
{
#ifdef _INTELMACHINE
	return vmread(VMCS_GUEST_RSP);
#else
	const vmcb_t* const vmcb = get_vmcb();

	return vmcb->save_state.rsp;
#endif
}

void arch::set_guest_rsp(const std::uint64_t guest_rsp)
{
#ifdef _INTELMACHINE
	vmwrite(VMCS_GUEST_RSP, guest_rsp);
#else
	vmcb_t* const vmcb = get_vmcb();

	vmcb->save_state.rsp = guest_rsp;
#endif
}

std::uint64_t arch::get_guest_rip()
{
#ifdef _INTELMACHINE
	return vmread(VMCS_GUEST_RIP);
#else
	const vmcb_t* const vmcb = get_vmcb();

	return vmcb->save_state.rip;
#endif
}

void arch::set_guest_rip(const std::uint64_t guest_rip)
{
#ifdef _INTELMACHINE
	vmwrite(VMCS_GUEST_RIP, guest_rip);
#else
	vmcb_t* const vmcb = get_vmcb();

	vmcb->save_state.rip = guest_rip;
#endif
}

void arch::advance_guest_rip()
{
#ifdef _INTELMACHINE
	const std::uint64_t guest_rip = get_guest_rip();
	const std::uint64_t instruction_length = get_vmexit_instruction_length();

	const std::uint64_t next_rip = guest_rip + instruction_length;
#else
	const vmcb_t* const vmcb = get_vmcb();

	const std::uint64_t next_rip = vmcb->control.next_rip;
#endif

	set_guest_rip(next_rip);
}
