#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include <.shared/shared.hpp>

namespace telemetryHyperProcess
{
    bool Initialize();
    bool QueryProcessData(std::uint32_t pid, query_process_data_packet* output);
    std::uint64_t GetKernelModuleBase(const char* module_name);
    std::vector<uint32_t> FindAllPidsByGhostWalk(const char* target_name);
    DWORD FindProcessIdByName(const wchar_t* process_name);
    std::uint64_t GetEProcessAddress(std::uint32_t pid);
    bool UnlinkProcessDKOM(std::uint64_t eprocess_address);
}
