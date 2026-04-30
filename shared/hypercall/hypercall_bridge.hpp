#pragma once

#include <cstdint>
#include "hypercall_def.h"

namespace HyperCall
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

    std::uint64_t InjectMouseMovement(long x, long y);

    bool SetMouseHookAddress(std::uint64_t ept_hook_address);

    std::uint64_t RegisterSignalPage(std::uint64_t signal_page_virtual_address);
    bool QuerySignalPage(std::uint64_t signal_id, signal_page_state_t* state);
    bool UnregisterSignalPage(std::uint64_t signal_id);
    bool GetInputDiagnostics(input_diagnostics_snapshot_t* snapshot);
}
