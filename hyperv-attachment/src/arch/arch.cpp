#include "arch.h"
#include "../crt/crt.h"

#include <intrin.h>

#ifdef _INTELMACHINE
#include <ia32-doc/ia32.hpp>

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
