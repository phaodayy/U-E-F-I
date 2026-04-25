#include "signal.h"

#include "../cr3/cr3.h"
#include "../cr3/pte.h"
#include "../hook/hook_entry.h"

#include "../../arch/arch.h"
#include "../../crt/crt.h"

#include <intrin.h>

namespace
{
	constexpr std::uint64_t page_mask = ~0xFFFULL;

	struct signal_entry_t
	{
		std::uint64_t guest_page_base = 0;
		std::uint64_t original_pfn = 0;
		std::uint64_t sequence = 0;
		std::uint64_t trigger_count = 0;
		std::uint64_t last_guest_rip = 0;
		std::uint64_t last_access_flags = 0;
		std::uint64_t last_tsc = 0;
		std::uint8_t active = 0;
		std::uint8_t original_read_access = 0;
		std::uint8_t original_write_access = 0;
		std::uint8_t original_execute_access = 0;
	};

	crt::mutex_t signal_mutex = { };
	signal_entry_t signal_entries[slat::signal::max_signal_pages] = { };
	std::uint64_t global_signal_sequence = 0;

	std::uint64_t normalize_page_base(const std::uint64_t physical_address)
	{
		return physical_address & page_mask;
	}

	std::uint64_t access_flags(const std::uint8_t read_access, const std::uint8_t write_access,
		const std::uint8_t execute_access)
	{
		std::uint64_t flags = 0;

		if (read_access != 0)
		{
			flags |= slat::signal::access_read;
		}

		if (write_access != 0)
		{
			flags |= slat::signal::access_write;
		}

		if (execute_access != 0)
		{
			flags |= slat::signal::access_execute;
		}

		return flags;
	}

	signal_entry_t* find_entry_by_page(const std::uint64_t guest_page_base)
	{
		for (std::uint64_t i = 0; i < slat::signal::max_signal_pages; i++)
		{
			signal_entry_t* const entry = &signal_entries[i];

			if (entry->active != 0 && entry->guest_page_base == guest_page_base)
			{
				return entry;
			}
		}

		return nullptr;
	}

	signal_entry_t* find_entry_by_id(const std::uint64_t id)
	{
		if (id == 0 || id > slat::signal::max_signal_pages)
		{
			return nullptr;
		}

		signal_entry_t* const entry = &signal_entries[id - 1];

		return entry->active != 0 ? entry : nullptr;
	}

	std::uint64_t id_for_entry(const signal_entry_t* const entry)
	{
		if (entry == nullptr)
		{
			return 0;
		}

		return static_cast<std::uint64_t>(entry - &signal_entries[0]) + 1;
	}

	signal_entry_t* allocate_entry()
	{
		for (std::uint64_t i = 0; i < slat::signal::max_signal_pages; i++)
		{
			signal_entry_t* const entry = &signal_entries[i];

			if (entry->active == 0)
			{
				return entry;
			}
		}

		return nullptr;
	}

	std::uint8_t read_original_access(const slat_pte* const pte, signal_entry_t* const entry)
	{
		if (pte == nullptr || entry == nullptr)
		{
			return 0;
		}

		entry->original_pfn = pte->page_frame_number;

#ifdef _INTELMACHINE
		entry->original_read_access = static_cast<std::uint8_t>(pte->read_access);
		entry->original_write_access = static_cast<std::uint8_t>(pte->write_access);
		entry->original_execute_access = static_cast<std::uint8_t>(pte->execute_access);

		return (entry->original_read_access | entry->original_write_access | entry->original_execute_access) != 0;
#else
		entry->original_read_access = static_cast<std::uint8_t>(pte->present);
		entry->original_write_access = static_cast<std::uint8_t>(pte->write);
		entry->original_execute_access = static_cast<std::uint8_t>(pte->execute_disable == 0);

		return entry->original_read_access != 0;
#endif
	}

	void make_signal_pte_faulting(slat_pte* const pte)
	{
		if (pte == nullptr)
		{
			return;
		}

#ifdef _INTELMACHINE
		pte->read_access = 0;
		pte->write_access = 0;
		pte->execute_access = 0;
#else
		pte->present = 0;
		pte->write = 0;
		pte->execute_disable = 1;
#endif
	}

	void restore_signal_pte(slat_pte* const pte, const signal_entry_t* const entry)
	{
		if (pte == nullptr || entry == nullptr)
		{
			return;
		}

		pte->page_frame_number = entry->original_pfn;

#ifdef _INTELMACHINE
		pte->read_access = entry->original_read_access;
		pte->write_access = entry->original_write_access;
		pte->execute_access = entry->original_execute_access;
#else
		pte->present = entry->original_read_access;
		pte->write = entry->original_write_access;
		pte->execute_disable = entry->original_execute_access == 0;
#endif
	}

	std::uint8_t arm_signal_page_for_cr3(const cr3 slat_cr3, const virtual_address_t guest_physical_address,
		signal_entry_t* const entry, const std::uint8_t capture_original_access)
	{
		if (slat_cr3.flags == 0 || entry == nullptr)
		{
			return 0;
		}

		slat_pte* const target_pte = slat::get_pte(slat_cr3, guest_physical_address, 1);
		if (target_pte == nullptr)
		{
			return 0;
		}

		if (capture_original_access != 0 && read_original_access(target_pte, entry) == 0)
		{
			return 0;
		}

		make_signal_pte_faulting(target_pte);
		return 1;
	}

	void restore_signal_page_for_cr3(const cr3 slat_cr3, const virtual_address_t guest_physical_address,
		const signal_entry_t* const entry)
	{
		if (slat_cr3.flags == 0 || entry == nullptr)
		{
			return;
		}

		slat_pte* const target_pte = slat::get_pte(slat_cr3, guest_physical_address, 0);
		restore_signal_pte(target_pte, entry);
	}
}

void slat::signal::set_up()
{
	signal_mutex.lock();

	crt::set_memory(&signal_entries[0], 0, sizeof(signal_entries));
	global_signal_sequence = 0;

	signal_mutex.release();
}

std::uint64_t slat::signal::register_page(const virtual_address_t guest_physical_address)
{
	const virtual_address_t guest_page = { .address = normalize_page_base(guest_physical_address.address) };

	if (guest_page.address == 0 || hook::entry_t::find(guest_page.address >> 12) != nullptr)
	{
		return 0;
	}

	signal_mutex.lock();

	signal_entry_t* existing_entry = find_entry_by_page(guest_page.address);
	if (existing_entry != nullptr)
	{
		const std::uint64_t existing_id = id_for_entry(existing_entry);
		signal_mutex.release();
		return existing_id;
	}

	signal_entry_t* const entry = allocate_entry();
	if (entry == nullptr)
	{
		signal_mutex.release();
		return 0;
	}

	crt::set_memory(entry, 0, sizeof(*entry));
	entry->guest_page_base = guest_page.address;

	if (arm_signal_page_for_cr3(hyperv_cr3(), guest_page, entry, 1) == 0)
	{
		crt::set_memory(entry, 0, sizeof(*entry));
		signal_mutex.release();
		return 0;
	}

	const cr3 current_hook_cr3 = hook_cr3();
	if (current_hook_cr3.flags != 0 &&
		arm_signal_page_for_cr3(current_hook_cr3, guest_page, entry, 0) == 0)
	{
		restore_signal_page_for_cr3(hyperv_cr3(), guest_page, entry);
		crt::set_memory(entry, 0, sizeof(*entry));
		signal_mutex.release();
		return 0;
	}

	entry->active = 1;
	const std::uint64_t id = id_for_entry(entry);

	signal_mutex.release();

	flush_all_logical_processors_cache();

	return id;
}

std::uint8_t slat::signal::unregister_page(const std::uint64_t id)
{
	signal_mutex.lock();

	signal_entry_t* const entry = find_entry_by_id(id);
	if (entry == nullptr)
	{
		signal_mutex.release();
		return 0;
	}

	const virtual_address_t guest_page = { .address = entry->guest_page_base };

	restore_signal_page_for_cr3(hyperv_cr3(), guest_page, entry);
	restore_signal_page_for_cr3(hook_cr3(), guest_page, entry);

	crt::set_memory(entry, 0, sizeof(*entry));

	signal_mutex.release();

	flush_all_logical_processors_cache();

	return 1;
}

std::uint8_t slat::signal::query_page(const std::uint64_t id, page_state_t* const state_out)
{
	if (state_out == nullptr)
	{
		return 0;
	}

	signal_mutex.lock();

	const signal_entry_t* const entry = find_entry_by_id(id);
	if (entry == nullptr)
	{
		signal_mutex.release();
		return 0;
	}

	state_out->id = id;
	state_out->guest_physical_address = entry->guest_page_base;
	state_out->sequence = entry->sequence;
	state_out->trigger_count = entry->trigger_count;
	state_out->last_guest_rip = entry->last_guest_rip;
	state_out->last_access_flags = entry->last_access_flags;
	state_out->last_tsc = entry->last_tsc;

	signal_mutex.release();

	return 1;
}

std::uint8_t slat::signal::process_violation(const std::uint64_t guest_physical_address,
	const std::uint8_t read_access, const std::uint8_t write_access, const std::uint8_t execute_access)
{
	if ((read_access == 0 && write_access == 0) || execute_access != 0)
	{
		return 0;
	}

	const std::uint64_t guest_page_base = normalize_page_base(guest_physical_address);

	signal_mutex.lock();

	signal_entry_t* const entry = find_entry_by_page(guest_page_base);
	if (entry == nullptr)
	{
		signal_mutex.release();
		return 0;
	}

#ifndef _INTELMACHINE
	if (arch::get_vmcb()->control.next_rip == 0)
	{
		signal_mutex.release();
		return 0;
	}
#endif

	const std::uint64_t old_guest_rip = arch::get_guest_rip();
	arch::advance_guest_rip();

	if (arch::get_guest_rip() == old_guest_rip)
	{
		signal_mutex.release();
		return 0;
	}

	entry->sequence = ++global_signal_sequence;
	entry->trigger_count++;
	entry->last_guest_rip = old_guest_rip;
	entry->last_access_flags = access_flags(read_access, write_access, execute_access);
	entry->last_tsc = __rdtsc();

	signal_mutex.release();

	return 1;
}
