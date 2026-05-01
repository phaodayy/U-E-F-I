#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include <.shared/shared.hpp>

namespace telemetryHyperProcess
{
    struct process_stats_packet {
        uint64_t working_set;
        uint64_t private_usage;
        uint64_t cpu_cycles;
    };

    bool Initialize();
    bool QueryProcessData(std::uint32_t pid, query_process_data_packet* output);
    bool GetProcessStats(std::uint32_t pid, process_stats_packet* output);
    std::uint64_t GetKernelModuleBase(const char* module_name);
    std::vector<uint32_t> FindAllPidsByGhostWalk(const char* target_name);
    DWORD FindProcessIdByName(const wchar_t* process_name);
    std::uint64_t GetEProcessAddress(std::uint32_t pid);
    bool UnlinkProcessDKOM(std::uint64_t eprocess_address);
}
