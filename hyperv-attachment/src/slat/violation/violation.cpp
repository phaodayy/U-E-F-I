#include "violation.h"
#include "../cr3/cr3.h"
#include "../hook/hook_entry.h"
#include "../signal/signal.h"

#include "../../arch/arch.h"

std::uint8_t slat::violation::process()
{
#ifdef _INTELMACHINE
	const auto qualification = arch::get_exit_qualification();

	if (!qualification.caused_by_translation)
	{
		return 0;
	}

	const std::uint64_t physical_address = arch::get_guest_physical_address();

	if (signal::process_violation(physical_address,
		static_cast<std::uint8_t>(qualification.read_access),
		static_cast<std::uint8_t>(qualification.write_access),
		static_cast<std::uint8_t>(qualification.execute_access)) == 1)
	{
		return 1;
	}

	const hook::entry_t* const hook_entry = hook::entry_t::find(physical_address >> 12);

	if (hook_entry == nullptr)
	{
		if (qualification.execute_access && get_cr3().flags != hyperv_cr3().flags)
		{
			set_cr3(hyperv_cr3());

			return 1;
		}

		return 0;
	}

	if (qualification.execute_access)
	{
		set_cr3(hyperv_cr3());

		// page is now --x, and with shadow pfn	
	}
	else
	{
		set_cr3(hook_cr3());

		// page is now rw-, and with original pfn
	}
#else
	const vmcb_t* const vmcb = arch::get_vmcb();

	const npf_exit_info_1 npf_info = { .flags = vmcb->control.first_exit_info };
	const std::uint64_t physical_address = vmcb->control.second_exit_info;
	const std::uint8_t write_access = static_cast<std::uint8_t>(npf_info.write_access);
	const std::uint8_t execute_access = static_cast<std::uint8_t>(npf_info.execute_access);
	const std::uint8_t read_access = static_cast<std::uint8_t>(write_access == 0 && execute_access == 0);

	if (signal::process_violation(physical_address, read_access, write_access, execute_access) == 1)
	{
		return 1;
	}

	if (npf_info.present == 0 || npf_info.execute_access == 0)
	{
		return 0;
	}

	const hook::entry_t* const hook_entry = hook::entry_t::find(physical_address >> 12);

	const cr3 hook_slat_cr3 = hook_cr3();

	if (hook_entry == nullptr)
	{
		if (vmcb->control.nested_cr3.flags == hook_slat_cr3.flags)
		{
			set_cr3(hyperv_cr3());

			return 1;
		}

		return 0;
	}

	set_cr3(hook_slat_cr3);
#endif

	return 1;
}
