#pragma once
#include <Windows.h>
#include <cstdint>
#include <mutex>
#include <hypercall/hypercall_bridge.hpp>
#include <hypercall/hyper_process.hpp>

namespace Driver {
    inline std::mutex g_Lock;
    inline uint32_t g_Pid = 0;
    inline uint64_t g_Cr3 = 0;

    inline bool Initialize() {
        if (!HyperCall::Init()) return false;
        return HyperProcess::Initialize();
    }

    inline bool ReadMemory(uint32_t pid, uint64_t src, void* dest, uint64_t size) {
        if (!pid || !dest || !size) return false;
        
        std::lock_guard<std::mutex> lock(g_Lock);

        // Auto-refresh CR3 if PID matches but CR3 is missing
        if (pid == g_Pid && g_Cr3 == 0) {
            query_process_data_packet q = {0};
            if (HyperProcess::QueryProcessData(pid, &q)) {
                g_Cr3 = q.cr3;
            }
        }

        if (pid == g_Pid && g_Cr3 != 0) {
            return HyperCall::ReadGuestVirtualMemory(dest, src, g_Cr3, size) == size;
        }

        // Fallback or cross-process read
        query_process_data_packet q = {0};
        if (HyperProcess::QueryProcessData(pid, &q)) {
            return HyperCall::ReadGuestVirtualMemory(dest, src, q.cr3, size) == size;
        }
        return false;
    }

    template <typename T>
    inline T Read(uint32_t pid, uint64_t address) {
        T buffer = {};
        ReadMemory(pid, address, &buffer, sizeof(T));
        return buffer;
    }

    inline bool QueryProcess(uint32_t pid, query_process_data_packet* output) {
        if (HyperProcess::QueryProcessData(pid, output)) {
            if (pid == g_Pid || g_Pid == 0) {
                g_Pid = output->process_id;
                g_Cr3 = output->cr3;
            }
            return true;
        }
        return false;
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        // Shared hypercall only supports movement dx/dy
        // Button flags would need a different call or extension
        return HyperCall::InjectMouseMovement(x, y) != 0;
    }

    inline bool Click() {
        // The shared hypercall currently doesn't expose click in the same way
        // I will keep it as movement only or use SendInput if needed
        // For now, I'll just call movement with 0,0
        return MoveMouse(0, 0);
    }
}
