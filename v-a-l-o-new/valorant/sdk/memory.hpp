#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include "../../.shared/shared.hpp"

#include "../../../shared/hypercall/hypercall_bridge.hpp"
#include "../../../shared/hypercall/hyper_process.hpp"

#include <thread>
#include <chrono>
#include <random>

namespace ValorantMemory {
    inline uint32_t g_ProcessId = 0;
    inline uint64_t g_ProcessCr3 = 0;
    inline uint64_t g_BaseAddress = 0;

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size);

    inline bool InitializeHyperInterface() {
        if (!HyperCall::Init()) return false;
        return HyperProcess::Initialize();
    }

    inline bool QueryProcessData(uint32_t pid, query_process_data_packet* output) {
        return HyperProcess::QueryProcessData(pid, output, "VALORANT-Win64");
    }

    inline bool RefreshProcessContext() {
        if (g_ProcessId == 0) {
            g_ProcessCr3 = 0;
            g_BaseAddress = 0;
            return false;
        }

        query_process_data_packet input = {};
        if (!QueryProcessData(g_ProcessId, &input)) {
            g_ProcessCr3 = 0;
            g_BaseAddress = 0;
            return false;
        }

        g_ProcessCr3 = input.cr3;
        g_BaseAddress = reinterpret_cast<uint64_t>(input.base_address);
        return g_ProcessCr3 != 0;
    }

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        if (!g_ProcessId || !dest || size == 0) return false;

        if (g_ProcessCr3 == 0 && !RefreshProcessContext()) {
            return false;
        }

        return HyperCall::ReadGuestVirtualMemory(dest, src, g_ProcessCr3, size) == size;
    }

    template <typename T>
    inline T Read(uint64_t address) {
        T buffer = {};
        ReadMemory(address, &buffer, sizeof(T));
        return buffer;
    }

    inline bool WriteMemory(uint64_t dest, void* src, uint64_t size) {
        if (!g_ProcessId || !src || size == 0) return false;

        if (g_ProcessCr3 == 0 && !RefreshProcessContext()) {
            return false;
        }

        return HyperCall::WriteGuestVirtualMemory(src, dest, g_ProcessCr3, size) == size;
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        if (x != 0 || y != 0) {
            HyperCall::InjectMouseMovement(x, y);
        }
        return true;
    }

    inline bool AttachToGameStealthily() {
        query_process_data_packet output = {};
        if (HyperProcess::QueryProcessData(0, &output, "VALORANT-Win64")) {
             g_ProcessId = output.process_id;
             g_ProcessCr3 = output.cr3;
             g_BaseAddress = reinterpret_cast<uint64_t>(output.base_address);
             return g_ProcessCr3 != 0;
        }
        return false;
    }
}

