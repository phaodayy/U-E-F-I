#include "arch.h"
#include "../crt/crt.h"
#include "../memory_manager/memory_manager.h"
#include "../timing/tsc_drift_compensator.h"

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

std::uint64_t arch::vmread(const std::uint64_t field)
{
	std::uint64_t value = 0;

	__vmx_vmread(field, &value);

	return value;
}

void arch::vmwrite(const std::uint64_t field, const std::uint64_t value)
{
	__vmx_vmwrite(field, value);
}

std::uint64_t arch::get_vmexit_instruction_length()
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

void arch::enable_feature_control_shadowing()
{
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

std::uint8_t arch::is_io_instruction(const std::uint64_t vmexit_reason)
{
#ifdef _INTELMACHINE
	return (vmexit_reason & VMX_VMEXIT_REASON_BASIC_EXIT_REASON_FLAG) == VMX_EXIT_REASON_EXECUTE_IO_INSTRUCTION;
#else
	return vmexit_reason == SVM_EXIT_REASON_IOIO;
#endif
}

void arch::enable_io_intercept()
{
#ifdef _INTELMACHINE
	const std::uint64_t controls = vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS);
	vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, controls | IA32_VMX_PROCBASED_CTLS_UNCONDITIONAL_IO_EXITING_FLAG);
#else
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_misc_vector_4 |= SVM_INTERCEPT_VECTOR4_IOIO_PROT;
    vmcb->control.clean.i = 0;
#endif
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

	const std::uint64_t raw_virtual_tsc = __rdtsc() + vmread(VMCS_CTRL_TSC_OFFSET) + tsc_software_offset;
	const std::uint64_t virtual_tsc = timing::tsc_drift_compensator::compensate_tsc(raw_virtual_tsc);

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
namespace
{
	constexpr std::uint64_t cr4_vmxe_mask = CR4_VMX_ENABLE_FLAG;
	constexpr std::uint32_t cpuid_leaf_feature_information = 0x00000001;
	constexpr std::uint32_t cpuid_leaf_hypervisor_base = 0x40000000;
	constexpr std::uint32_t cpuid_leaf_hypervisor_end = 0x400000FF;
	constexpr std::uint32_t cpuid_ecx_hypervisor_present = 1U << 31;
	constexpr std::uint32_t ia32_feature_control_msr = 0x0000003A;
	constexpr std::uint64_t ia32_feature_control_locked = 0x1;

	std::uint64_t* get_amd_trap_frame_register(trap_frame_t* const trap_frame, const std::uint64_t register_index)
	{
		if (trap_frame == nullptr)
		{
			return nullptr;
		}

		switch (register_index)
		{
		case VMX_EXIT_QUALIFICATION_GENREG_RCX:
			return &trap_frame->rcx;
		case VMX_EXIT_QUALIFICATION_GENREG_RDX:
			return &trap_frame->rdx;
		case VMX_EXIT_QUALIFICATION_GENREG_RBX:
			return &trap_frame->rbx;
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

	std::uint8_t read_amd_gpr(trap_frame_t* const trap_frame, const std::uint64_t register_index, std::uint64_t* const value)
	{
		if (value == nullptr)
		{
			return 0;
		}

		vmcb_t* const vmcb = arch::get_vmcb();

		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RAX)
		{
			*value = vmcb->save_state.rax;
			return 1;
		}

		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RSP)
		{
			*value = vmcb->save_state.rsp;
			return 1;
		}

		const std::uint64_t* const register_value = get_amd_trap_frame_register(trap_frame, register_index);
		if (register_value == nullptr)
		{
			return 0;
		}

		*value = *register_value;
		return 1;
	}

	std::uint8_t write_amd_gpr(trap_frame_t* const trap_frame, const std::uint64_t register_index, const std::uint64_t value)
	{
		vmcb_t* const vmcb = arch::get_vmcb();

		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RAX)
		{
			vmcb->save_state.rax = value;
			if (trap_frame != nullptr)
			{
				trap_frame->rax = value;
			}
			return 1;
		}

		if (register_index == VMX_EXIT_QUALIFICATION_GENREG_RSP)
		{
			vmcb->save_state.rsp = value;
			if (trap_frame != nullptr)
			{
				trap_frame->rsp = value;
			}
			return 1;
		}

		std::uint64_t* const register_value = get_amd_trap_frame_register(trap_frame, register_index);
		if (register_value == nullptr)
		{
			return 0;
		}

		*register_value = value;
		return 1;
	}

	std::uint8_t decode_amd_cr_mov_register(const vmcb_t* const vmcb, std::uint64_t* const register_index)
	{
		if (vmcb == nullptr || register_index == nullptr || vmcb->control.number_of_bytes_fetched < 3)
		{
			return 0;
		}

		const std::uint8_t* const bytes = &vmcb->control.guest_instruction_bytes[0];
		std::uint8_t rex = 0;
		std::uint8_t offset = 0;

		while (offset < vmcb->control.number_of_bytes_fetched)
		{
			const std::uint8_t byte = bytes[offset];
			if (byte == 0x66 || byte == 0x67 || byte == 0xF2 || byte == 0xF3)
			{
				++offset;
				continue;
			}

			if (byte >= 0x40 && byte <= 0x4F)
			{
				rex = byte;
				++offset;
				continue;
			}

			break;
		}

		if (offset + 2 >= vmcb->control.number_of_bytes_fetched)
		{
			return 0;
		}

		const std::uint8_t opcode_escape = bytes[offset];
		const std::uint8_t opcode = bytes[offset + 1];
		const std::uint8_t modrm = bytes[offset + 2];

		if (opcode_escape != 0x0F || (opcode != 0x20 && opcode != 0x22) || (modrm & 0xC0) != 0xC0)
		{
			return 0;
		}

		const std::uint8_t control_register = ((modrm >> 3) & 0x7) | ((rex & 0x4) != 0 ? 8 : 0);
		if (control_register != 4)
		{
			return 0;
		}

		*register_index = (modrm & 0x7) | ((rex & 0x1) != 0 ? 8 : 0);
		return 1;
	}

	void set_amd_msrpm_read_intercept(const std::uint32_t msr)
	{
		vmcb_t* const vmcb = arch::get_vmcb();
		if (vmcb->control.msrpm_base_pa == 0 || msr > 0x1FFF)
		{
			return;
		}

		auto* const msrpm = static_cast<std::uint8_t*>(memory_manager::map_host_physical(vmcb->control.msrpm_base_pa));
		const std::uint32_t byte_index = msr / 4;
		const std::uint32_t bit_index = (msr & 0x3) * 2;

		msrpm[byte_index] |= static_cast<std::uint8_t>(1U << bit_index);
	}
}

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

std::uint8_t arch::is_mov_cr(const std::uint64_t vmexit_reason)
{
	return vmexit_reason == SVM_EXIT_REASON_READ_CR4 ||
		vmexit_reason == SVM_EXIT_REASON_WRITE_CR4;
}

void arch::enable_cr4_shadowing()
{
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_cr_read |= SVM_INTERCEPT_CR4;
	vmcb->control.intercept_cr_write |= SVM_INTERCEPT_CR4;
	vmcb->control.clean.i = 0;
}

std::uint8_t arch::handle_cr4_mov_exit(trap_frame_t* const trap_frame)
{
	vmcb_t* const vmcb = get_vmcb();
	std::uint64_t register_index = 0;

	if (decode_amd_cr_mov_register(vmcb, &register_index) == 0)
	{
		return 0;
	}

	if (vmcb->control.vmexit_reason == SVM_EXIT_REASON_READ_CR4)
	{
		const std::uint64_t visible_cr4 = vmcb->save_state.cr4 & ~cr4_vmxe_mask;

		if (write_amd_gpr(trap_frame, register_index, visible_cr4) == 0)
		{
			return 0;
		}

		advance_guest_rip();
		return 1;
	}

	if (vmcb->control.vmexit_reason == SVM_EXIT_REASON_WRITE_CR4)
	{
		std::uint64_t requested_cr4 = 0;
		if (read_amd_gpr(trap_frame, register_index, &requested_cr4) == 0)
		{
			return 0;
		}

		vmcb->save_state.cr4 = requested_cr4 & ~cr4_vmxe_mask;
		vmcb->control.clean.cr = 0;

		advance_guest_rip();
		return 1;
	}

	return 0;
}

std::uint8_t arch::handle_cpuid_spoof(trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr)
	{
		return 0;
	}

	vmcb_t* const vmcb = get_vmcb();
	const std::uint32_t leaf = static_cast<std::uint32_t>(vmcb->save_state.rax);
	const std::uint32_t subleaf = static_cast<std::uint32_t>(trap_frame->rcx);

	if (leaf >= cpuid_leaf_hypervisor_base && leaf <= cpuid_leaf_hypervisor_end)
	{
		vmcb->save_state.rax = 0;
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

		const std::uint64_t rax = static_cast<std::uint32_t>(cpu_info[0]);
		vmcb->save_state.rax = rax;
		trap_frame->rax = rax;
		trap_frame->rbx = static_cast<std::uint32_t>(cpu_info[1]);
		trap_frame->rcx = static_cast<std::uint32_t>(cpu_info[2]) & ~cpuid_ecx_hypervisor_present;
		trap_frame->rdx = static_cast<std::uint32_t>(cpu_info[3]);

		advance_guest_rip();
		return 1;
	}

	return 0;
}

std::uint8_t arch::is_rdmsr(const std::uint64_t vmexit_reason)
{
	return vmexit_reason == SVM_EXIT_REASON_MSR;
}

void arch::enable_feature_control_shadowing()
{
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_misc_vector_3 |= SVM_INTERCEPT_VECTOR3_MSR_PROT;
	vmcb->control.clean.i = 0;

	set_amd_msrpm_read_intercept(ia32_feature_control_msr);
}

std::uint8_t arch::handle_feature_control_rdmsr(trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr)
	{
		return 0;
	}

	const vmcb_t* const vmcb = get_vmcb();
	if (vmcb->control.first_exit_info != SVM_EXIT_INFO_MSR_READ ||
		static_cast<std::uint32_t>(trap_frame->rcx) != ia32_feature_control_msr)
	{
		return 0;
	}

	const std::uint64_t shadowed_feature_control = ia32_feature_control_locked;

	get_vmcb()->save_state.rax = static_cast<std::uint32_t>(shadowed_feature_control);
	trap_frame->rax = static_cast<std::uint32_t>(shadowed_feature_control);
	trap_frame->rdx = static_cast<std::uint32_t>(shadowed_feature_control >> 32);

	advance_guest_rip();
	return 1;
}

std::uint8_t arch::is_tsc_exit(const std::uint64_t vmexit_reason)
{
	return vmexit_reason == SVM_EXIT_REASON_RDTSC ||
		vmexit_reason == SVM_EXIT_REASON_RDTSCP;
}

void arch::enable_tsc_exiting()
{
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_misc_vector_3 |= SVM_INTERCEPT_VECTOR3_RDTSC;
	vmcb->control.intercept_misc_vector_4 |= SVM_INTERCEPT_VECTOR4_RDTSCP;
	vmcb->control.clean.i = 0;
}

std::uint8_t arch::handle_tsc_exit(const std::uint64_t vmexit_reason, trap_frame_t* const trap_frame)
{
	if (trap_frame == nullptr || is_tsc_exit(vmexit_reason) == 0)
	{
		return 0;
	}

	vmcb_t* const vmcb = get_vmcb();
	const std::uint64_t raw_virtual_tsc = __rdtsc() + vmcb->control.tsc_offset;
	const std::uint64_t virtual_tsc = timing::tsc_drift_compensator::compensate_tsc(raw_virtual_tsc);

	vmcb->save_state.rax = static_cast<std::uint32_t>(virtual_tsc);
	trap_frame->rax = static_cast<std::uint32_t>(virtual_tsc);
	trap_frame->rdx = static_cast<std::uint32_t>(virtual_tsc >> 32);

	if (vmexit_reason == SVM_EXIT_REASON_RDTSCP)
	{
		trap_frame->rcx = static_cast<std::uint32_t>(__readmsr(IA32_TSC_AUX));
	}

	advance_guest_rip();
	return 1;
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
	// exception-intercept bitmap is at VMCB offset 0x08.
	vmcb_t* const vmcb = get_vmcb();
	vmcb->control.intercept_exception_vector |= (1U << 3);
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

std::uint8_t arch::get_guest_cpl()
{
#ifdef _INTELMACHINE
	return static_cast<std::uint8_t>(vmread(VMCS_GUEST_SS_SELECTOR) & 3);
#else
	return static_cast<std::uint8_t>(get_vmcb()->save_state.cpl);
#endif
}
