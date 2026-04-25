#pragma once
#include <cstdint>

#include "../../structures/virtual_address.h"

namespace slat::signal
{
	constexpr std::uint64_t max_signal_pages = 16;

	enum access_flags_t : std::uint64_t
	{
		access_read = 1ull << 0,
		access_write = 1ull << 1,
		access_execute = 1ull << 2,
	};

	struct page_state_t
	{
		std::uint64_t id = 0;
		std::uint64_t guest_physical_address = 0;
		std::uint64_t sequence = 0;
		std::uint64_t trigger_count = 0;
		std::uint64_t last_guest_rip = 0;
		std::uint64_t last_access_flags = 0;
		std::uint64_t last_tsc = 0;
	};

	void set_up();
	std::uint64_t register_page(virtual_address_t guest_physical_address);
	std::uint8_t unregister_page(std::uint64_t id);
	std::uint8_t query_page(std::uint64_t id, page_state_t* state_out);
	std::uint8_t process_violation(std::uint64_t guest_physical_address,
		std::uint8_t read_access, std::uint8_t write_access, std::uint8_t execute_access);
}
