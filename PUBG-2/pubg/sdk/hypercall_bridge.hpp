#pragma once

#include <cstdint>

namespace PubgHyperCall
{
    bool Init();
    
    std::uint64_t ReadGuestVirtualMemory(void* guest_destination_buffer,
                                         std::uint64_t guest_source_virtual_address,
                                         std::uint64_t source_cr3,
                                         std::uint64_t size);

    std::uint64_t WriteGuestVirtualMemory(const void* guest_source_buffer,
                                          std::uint64_t guest_destination_virtual_address,
                                          std::uint64_t destination_cr3,
                                          std::uint64_t size);

    std::uint64_t ReadGuestCr3();
}
