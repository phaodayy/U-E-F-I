#pragma once

#include <cstdint>

namespace telemetryHyperCall
{
    bool Init();
    
    std::uint64_t ReadGuestVirtualMemory(void* guest_destination_buffer,
                                         std::uint64_t guest_source_virtual_address,
                                         std::uint64_t source_cr3,
                                         std::uint64_t size);

    std::uint64_t ScatterReadVirtualMemory(void* descriptors_array,
                                          std::uint64_t count,
                                          std::uint64_t source_cr3);

    std::uint64_t WriteGuestVirtualMemory(const void* guest_source_buffer,
                                          std::uint64_t guest_destination_virtual_address,
                                          std::uint64_t destination_cr3,
                                          std::uint64_t size);

    std::uint64_t ReadGuestCr3();

    std::uint64_t InjectMouseMovement(long x, long y);

    bool SetMouseHookAddress(std::uint64_t ept_hook_address);
}
