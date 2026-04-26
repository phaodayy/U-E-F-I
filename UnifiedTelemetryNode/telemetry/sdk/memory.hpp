#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include <.shared/shared.hpp>
#include <.shared/telemetry_config.hpp>

#include "hypercall_bridge.hpp"
#include "hyper_process.hpp"
#include "netease_comm.hpp"
#include "vmouse_client.hpp"

#include <thread>
#include <chrono>
#include <random>

namespace telemetryMemory {
    inline uint32_t g_ProcessId = 0;
    inline uint64_t g_ProcessCr3 = 0;
    inline uint64_t g_BaseAddress = 0;
    inline uint64_t g_LastRefreshTime = 0; 
    inline bool g_UseNetEase = false;

    inline bool ReadMemory(uint64_t src, void* dest, uint64_t size);

    inline bool AttachToGameStealthily();

    inline bool InitializeHyperInterface() {
        if (!telemetryHyperCall::Init()) return false;
        return telemetryHyperProcess::Initialize();
    }

    inline bool QueryProcessData(uint32_t pid, query_process_data_packet* output) {
        return telemetryHyperProcess::QueryProcessData(pid, output);
    }

    inline bool RefreshProcessContext() {
        if (g_ProcessId == 0) return false;

        uint64_t current_time = GetTickCount64();
        // CACHE: Only refresh if 5 seconds passed since last refresh
        if (g_ProcessCr3 != 0 && (current_time - g_LastRefreshTime < 5000)) {
            return true;
        }

        query_process_data_packet input = {};
        if (!QueryProcessData(g_ProcessId, &input)) {
            g_ProcessCr3 = 0;
            g_BaseAddress = 0;
            return false;
        }

        g_ProcessCr3 = input.cr3;
        g_BaseAddress = reinterpret_cast<uint64_t>(input.base_address);
        g_LastRefreshTime = current_time;
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
                if (telemetryHyperCall::ScatterReadVirtualMemory(&requests[offset], count, g_ProcessCr3) == 0) {
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

        return telemetryHyperCall::ReadGuestVirtualMemory(dest, src, g_ProcessCr3, size) == size;
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

        return telemetryHyperCall::WriteGuestVirtualMemory(src, dest, g_ProcessCr3, size) == size;
    }

    inline bool IsKeyDown(int vk) {
        static uint64_t gafAsyncKeyStateExport = 0;
        static bool initialized = false;
        
        if (!initialized) {
            HMODULE hMod = LoadLibraryExA(skCrypt("C:\\Windows\\System32\\win32kbase.sys"), NULL, DONT_RESOLVE_DLL_REFERENCES);
            if (hMod) {
                void* proc = GetProcAddress(hMod, skCrypt("gafAsyncKeyState"));
                if (proc) {
                    uint64_t rva = reinterpret_cast<uint64_t>(proc) - reinterpret_cast<uint64_t>(hMod);
                    uint64_t kernel_base = telemetryHyperProcess::GetKernelModuleBase(skCrypt("win32kbase.sys"));
                    if (kernel_base != 0) {
                        gafAsyncKeyStateExport = kernel_base + rva;
                    }
                }
                FreeLibrary(hMod);
            }
            initialized = true;
        }

        // [STEALTH] Obfuscated input polling to evasion_calibration simple behavioral hooks.
        static volatile int junk = 0;
        junk ^= (vk * 0x1337);

        bool pressed = false;

        // Primary Stealth: Kernel polling via Hypervisor
        if (gafAsyncKeyStateExport != 0 && g_ProcessCr3 != 0) {
            uint8_t state_bitmap[64] = {0};
            if (telemetryHyperCall::ReadGuestVirtualMemory(state_bitmap, gafAsyncKeyStateExport, g_ProcessCr3, sizeof(state_bitmap)) == sizeof(state_bitmap)) {
                pressed = (state_bitmap[(vk * 2 / 8)] & (1 << (vk % 4 * 2))) != 0;
            }
        } 
        else {
            // Fallback to User-Mode API if Kernel address isn't found
            SHORT state = GetKeyState(vk);
            pressed = (state & 0x8000) != 0;
        }
        
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
        static std::uniform_int_distribution<int> jitter_dist(-2, 5); // Micro-jitter between -2ms and +5ms
        
        int total_ms = ms + jitter_dist(rng);
        if (total_ms < 1) total_ms = 1;

        std::this_thread::sleep_for(std::chrono::milliseconds(total_ms));

        // Random spin-lock jitter (sub-millisecond)
        if (jitter_dist(rng) > 3) {
            auto start = std::chrono::high_resolution_clock::now();
            while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() < 500) {
                _mm_pause(); // Low power spin
            }
        }
    }

    inline bool MoveMouse(long x, long y, unsigned short flags = 0) {
        // [MOD] Temporarily OFF to connect mouse later
        /*
        if (x != 0 || y != 0 || flags != 0) {
            VMouseClient::Move(static_cast<int>(x), static_cast<int>(y), flags);
        }
        */
        return true;
    }

    inline bool AttachToGameStealthily() {
        // Search for TslGame.exe using the kernel scanner (PID 0 = Auto Find)
        query_process_data_packet output = {};
        if (telemetryHyperProcess::QueryProcessData(0, &output)) {
             g_ProcessId = output.process_id;
             g_ProcessCr3 = output.cr3;
             g_BaseAddress = reinterpret_cast<uint64_t>(output.base_address);
             return g_ProcessCr3 != 0;
        }
        return false;
    }
}
