#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include <.shared/shared.hpp>
#include <.shared/pubg_config.hpp>

#include "hypercall_bridge.hpp"
#include "hyper_process.hpp"
#include "netease_comm.hpp"
#include "vmouse_client.hpp"

#include <thread>
#include <chrono>
#include <random>

namespace PubgMemory {
    inline uint32_t g_ProcessId = 0;
    inline uint64_t g_ProcessCr3 = 0;
    inline uint64_t g_BaseAddress = 0;
    inline bool g_UseNetEase = false;

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size);

    inline bool AttachToGameStealthily();

    inline bool InitializeHyperInterface() {
        if (!PubgHyperCall::Init()) return false;
        return PubgHyperProcess::Initialize();
    }

    inline bool QueryProcessData(uint32_t pid, query_process_data_packet* output) {
        return PubgHyperProcess::QueryProcessData(pid, output);
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

    inline NTSTATUS issue_syscall(c_packet*) {
        return static_cast<NTSTATUS>(0xC00000BBL);
    }

    inline bool BulkReadMemory(const std::vector<bulk_read_entry>& entries) {
        if (!g_ProcessId || entries.empty()) return false;

        for (const auto& entry : entries) {
            if (!ReadMemory(entry.source, entry.dest, entry.size)) {
                return false;
            }
        }

        return true;
    }

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        if (!g_ProcessId || !dest || size == 0) return false;

        if (g_UseNetEase) {
            return NetEaseMemory::ReadMemory(src, dest, static_cast<uint32_t>(size));
        }

        if (g_ProcessCr3 == 0 && !RefreshProcessContext()) {
            return false;
        }

        return PubgHyperCall::ReadGuestVirtualMemory(dest, src, g_ProcessCr3, size) == size;
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

        return PubgHyperCall::WriteGuestVirtualMemory(src, dest, g_ProcessCr3, size) == size;
    }

    inline bool IsKeyDown(int vk) {
        // [SECURITY] Replacing GetAsyncKeyState (System-wide polling IOC)
        // with GetKeyState (Message-buffer check, safer).
        // Future: Replace this with PubgMemory::Read<BYTE>(gafAsyncKeyState + vk) for 100% stealth.
        return (GetKeyState(vk) & 0x8000) != 0;
    }

    inline void StealthSleep(int ms) {
        if (ms <= 0) return;
        
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, 3); // Add 0-3ms jitter
        
        int total_ms = ms + dist(rng);
        std::this_thread::sleep_for(std::chrono::milliseconds(total_ms));
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        // Primary: Hypervisor Ring -1 Mouse Movement
        if (x != 0 || y != 0) {
            VMouseClient::Move(static_cast<int>(x), static_cast<int>(y));
        }

        // [SECURITY] SendInput fallback for clicks is DISABLED to avoid BattlEye flags.
        // Use a kernel driver or HID injector for clicks.
        /*
        if (flags != 0) {
            INPUT input = {};
            input.type = INPUT_MOUSE;
            input.mi.dx = 0;
            input.mi.dy = 0;
            input.mi.dwFlags = 0;
            if (flags & 0x0001) input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
            if (flags & 0x0002) input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
            if (flags & 0x0004) input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
            if (flags & 0x0008) input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
            SendInput(1, &input, sizeof(input));
        }
        */
        return true;
    }

    inline bool AttachToGameStealthily() {
        // Search for TslGame.exe using the kernel scanner (PID 0 = Auto Find)
        query_process_data_packet output = {};
        if (PubgHyperProcess::QueryProcessData(0, &output)) {
             g_ProcessId = output.process_id;
             g_ProcessCr3 = output.cr3;
             g_BaseAddress = reinterpret_cast<uint64_t>(output.base_address);
             return g_ProcessCr3 != 0;
        }
        return false;
    }
}
