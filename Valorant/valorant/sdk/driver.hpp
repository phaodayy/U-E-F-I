#pragma once
#include <Windows.h>
#include <winternl.h>
#include <cstdint>
#include <mutex>
#include <hypercall/hypercall_bridge.hpp>
#include <hypercall/hyper_process.hpp>

namespace Driver {
    inline uint32_t g_TargetPid = 0;
    inline uint64_t g_TargetCr3 = 0;
    inline std::mutex g_CommMutex;

    inline bool Initialize() {
        return HyperCall::Init();
    }

    inline bool ReadMemory(uint32_t pid, uint64_t src, void* dest, uint64_t size) {
        if (!size) return true;
        std::lock_guard<std::mutex> lock(g_CommMutex);
        
        // If we don't have CR3 for this PID yet, we should resolve it
        // However, usually context.cpp sets g_TargetCr3
        uint64_t cr3 = (pid == g_TargetPid) ? g_TargetCr3 : 0;
        
        if (!cr3) {
             query_process_data_packet data = { 0 };
             if (HyperProcess::QueryProcessData(pid, &data)) {
                 cr3 = data.cr3;
             }
        }

        if (!cr3) return false;

        return HyperCall::ReadGuestVirtualMemory(dest, src, cr3, size) == size;
    }

    inline bool WriteMemory(uint32_t pid, uint64_t dest, void* src, uint64_t size) {
        if (!size) return true;
        std::lock_guard<std::mutex> lock(g_CommMutex);
        
        uint64_t cr3 = (pid == g_TargetPid) ? g_TargetCr3 : 0;
        if (!cr3) {
             query_process_data_packet data = { 0 };
             if (HyperProcess::QueryProcessData(pid, &data)) {
                 cr3 = data.cr3;
             }
        }
        
        if (!cr3) return false;

        return HyperCall::WriteGuestVirtualMemory(src, dest, cr3, size) == size;
    }

    template <typename T>
    inline T Read(uint32_t pid, uint64_t address) {
        T buffer = {};
        ReadMemory(pid, address, &buffer, sizeof(T));
        return buffer;
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        std::lock_guard<std::mutex> lock(g_CommMutex);
        return HyperCall::InjectMouseMovement(x, y) == 1;
    }

    inline bool Click() {
        // HyperCall::InjectMouseMovement doesn't support flags directly in all versions
        // but it's usually enough for aimbot movement. 
        // If we need clicks, we might need a separate hypercall or use the flags in r9.
        return false; 
    }
}
