#pragma once
#include <cstdint>
#include <vector>

#include <structures/trap_frame.h>

namespace hypercall
{
	bool init();
	std::uint64_t get_primary_key();
	std::uint64_t get_secondary_key();

	std::uint64_t read_guest_physical_memory(void* guest_destination_buffer,
	                                         std::uint64_t guest_source_physical_address, std::uint64_t size);
	std::uint64_t write_guest_physical_memory(const void* guest_source_buffer,
	                                          std::uint64_t guest_destination_physical_address, std::uint64_t size);

	std::uint64_t read_guest_virtual_memory(void* guest_destination_buffer, std::uint64_t guest_source_virtual_address,
	                                        std::uint64_t source_cr3, std::uint64_t size);
	std::uint64_t write_guest_virtual_memory(const void* guest_source_buffer,
	                                         std::uint64_t guest_destination_virtual_address,
	                                         std::uint64_t destination_cr3, std::uint64_t size);

	std::uint64_t translate_guest_virtual_address(std::uint64_t guest_virtual_address, std::uint64_t guest_cr3);

	std::uint64_t set_mouse_hook_address(std::uint64_t ept_hook_address);

	std::uint64_t read_guest_cr3();

	std::uint64_t add_slat_code_hook(std::uint64_t target_guest_physical_address,
	                                 std::uint64_t shadow_page_guest_physical_address);
	std::uint64_t remove_slat_code_hook(std::uint64_t target_guest_physical_address);

	std::uint64_t hide_guest_physical_page(std::uint64_t target_guest_physical_address);

	std::uint64_t flush_logs(std::vector<trap_frame_log_t>& logs);

	std::uint64_t get_heap_free_page_count();

	std::uint64_t inject_mouse_movement(long x, long y);
}
