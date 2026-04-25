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

    class ScatterManager {
    public:
        ScatterManager() {
            requests.reserve(512);
        }

        template <typename T>
        void Add(uint64_t source, T* dest) {
            if (!source || !dest) return;
            requests.push_back({ source, reinterpret_cast<uint64_t>(dest), sizeof(T) });
        }

        void AddRaw(uint64_t source, void* dest, uint64_t size) {
            if (!source || !dest || size == 0) return;
            requests.push_back({ source, reinterpret_cast<uint64_t>(dest), size });
        }

        bool Execute() {
            if (requests.empty()) return true;
            if (g_ProcessCr3 == 0 && !RefreshProcessContext()) return false;

            // Execute in chunks if needed (Hypervisor maps 1 page of requests at a time, max ~170 entries)
            const size_t max_entries_per_call = 128; 
            size_t remaining = requests.size();
            size_t offset = 0;

            while (remaining > 0) {
                size_t count = (remaining > max_entries_per_call) ? max_entries_per_call : remaining;
                if (PubgHyperCall::ScatterReadVirtualMemory(&requests[offset], count, g_ProcessCr3) == 0) {
                    return false;
                }
                offset += count;
                remaining -= count;
            }

            requests.clear();
            return true;
        }

        void Clear() {
            requests.clear();
        }

    private:
        struct scatter_read_entry_t {
            uint64_t source;
            uint64_t dest;
            uint64_t size;
        };
        std::vector<scatter_read_entry_t> requests;
    };

    inline bool BulkReadMemory(const std::vector<bulk_read_entry>& entries) {
        if (!g_ProcessId || entries.empty()) return false;
        if (g_ProcessCr3 == 0 && !RefreshProcessContext()) return false;

        ScatterManager sm;
        for (const auto& entry : entries) {
            sm.AddRaw(entry.source, entry.dest, entry.size);
        }
        return sm.Execute();
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
        // [STEALTH] Obfuscated input polling to bypass simple behavioral hooks.
        // Randomized jitter and junk calculations to break timing signatures.
        static volatile int junk = 0;
        junk ^= (vk * 0x1337);
        
        SHORT state = GetKeyState(vk);
        bool pressed = (state & 0x8000) != 0;
        
        if (pressed) {
            // Add micro-jitter if key is pressed to mimic natural user delay
            volatile int delay = (vk % 5);
            for(int i=0; i<delay; i++) junk += i;
        }
        
        return pressed;
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
