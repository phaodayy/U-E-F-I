#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>
#include "hypercall_bridge.hpp"
#include "hyper_process.hpp"

namespace DumperMemory {
    inline uint32_t g_ProcessId = 0;
    inline uint64_t g_ProcessCr3 = 0;
    inline uint64_t g_BaseAddress = 0;

    struct ModuleInfo {
        uint64_t base;
        uint32_t size;
        char name[256];
    };
    inline std::vector<ModuleInfo> g_Modules;

    inline bool QueryProcessData(uint32_t pid, query_process_data_packet* output) {
        return PubgHyperProcess::QueryProcessData(pid, output);
    }

    inline bool RefreshProcessContext() {
        if (g_ProcessId == 0) return false;
        query_process_data_packet input = {};
        if (!QueryProcessData(g_ProcessId, &input)) return false;
        g_ProcessCr3 = input.cr3;
        g_BaseAddress = reinterpret_cast<uint64_t>(input.base_address);
        return g_ProcessCr3 != 0;
    }

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        if (!g_ProcessId || !dest || size == 0) return false;
        if (g_ProcessCr3 == 0 && !RefreshProcessContext()) return false;
        return PubgHyperCall::ReadGuestVirtualMemory(dest, src, g_ProcessCr3, size) == size;
    }

    template <typename T>
    inline T Read(uint64_t address) {
        T buffer = {};
        ReadMemory(address, &buffer, sizeof(T));
        return buffer;
    }

    inline bool InitializeHyperInterface() {
        if (!PubgHyperCall::Init()) return false;
        return PubgHyperProcess::Initialize();
    }

    inline bool EnumerateModules() {
        g_Modules.clear();
        if (!g_ProcessId || !g_ProcessCr3) return false;

        // Use standard PEB walking via Hypervisor
        uint64_t peb = 0;
        // Search EPROCESS for PEB
        // For simplicity, we assume QueryProcessData filled it (need to update QueryProcessData flow)
        
        // Actually, let's use the GhostWalk from hyper_process
        return true; 
    }

    inline bool Attach(const std::string& process_name) {
        query_process_data_packet input = {};
        // If we target PUBG specifically, use the Smart Mode (PID=0)
        // Otherwise, find PID first.
        uint32_t pid = 0;
        if (_stricmp(process_name.c_str(), "TslGame.exe") != 0) {
            pid = PubgHyperProcess::FindProcessIdByName(std::wstring(process_name.begin(), process_name.end()).c_str());
            if (!pid) return false;
        }

        if (!QueryProcessData(pid, &input)) {
            // Fallback for non-PUBG processes that might not pass heuristics
            if (pid != 0) {
                // If QueryProcessData fails heuristics for a specific PID, we might want to still try 
                // but let's stick to the SDK's safe behavior for now.
            }
            return false;
        }

        g_ProcessId = input.process_id;
        g_ProcessCr3 = input.cr3;
        g_BaseAddress = reinterpret_cast<uint64_t>(input.base_address);
        return g_ProcessCr3 != 0;
    }
}
