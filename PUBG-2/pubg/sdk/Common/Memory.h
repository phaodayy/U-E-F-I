#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <array>
#include <cstdint>
#include <vector>

#include "../hyper_process.hpp"
#include "../memory.hpp"

typedef void* VMMDLL_SCATTER_HANDLE;

#define VMMDLL_FLAG_NOCACHE 0
#define MEM_READ_CACHED 0

class c_keys {
public:
    bool InitKeyboard() {
        UpdateKeys();
        return true;
    }

    void InitCursorPosition() {}

    void UpdateKeys() {
        previous_keys_ = current_keys_;
        for (int key = 0; key < 256; ++key) {
            current_keys_[key] = (GetAsyncKeyState(key) & 0x8000) != 0;
        }
    }

    bool IsKeyDown(int vk) const {
        if (vk < 0 || vk >= 256) {
            return false;
        }
        return current_keys_[vk];
    }

    bool WasKeyPressed(int vk) const {
        if (vk < 0 || vk >= 256) {
            return false;
        }
        return current_keys_[vk] && !previous_keys_[vk];
    }

    bool WasKeyReleased(int vk) const {
        if (vk < 0 || vk >= 256) {
            return false;
        }
        return !current_keys_[vk] && previous_keys_[vk];
    }

    std::uint64_t GetAddrss() const {
        return 0;
    }

private:
    std::array<bool, 256> current_keys_ = {};
    std::array<bool, 256> previous_keys_ = {};
};

class Memory {
public:
    std::uint32_t current_pid = 0;
    std::uint64_t current_cr3 = 0;
    std::uint64_t current_base = 0;
    bool DMA_INITIALIZED = false;
    bool PROCESS_INITIALIZED = false;
    void* vHandle = reinterpret_cast<void*>(1);

    bool Init(std::uint32_t pid) {
        query_process_data_packet input = {};
        if (!PubgHyperProcess::QueryProcessData(pid, &input)) {
            ClearState();
            return false;
        }

        current_pid = pid;
        current_cr3 = input.cr3;
        current_base = reinterpret_cast<std::uint64_t>(input.base_address);
        DMA_INITIALIZED = (current_cr3 != 0);
        PROCESS_INITIALIZED = DMA_INITIALIZED;
        SyncGlobals();
        return DMA_INITIALIZED;
    }

    bool Init(const char* process_name, bool, bool) {
        if (process_name == nullptr || process_name[0] == '\0') {
            ClearState();
            return false;
        }

        wchar_t wide_name[MAX_PATH] = {};
        if (MultiByteToWideChar(CP_ACP, 0, process_name, -1, wide_name, MAX_PATH) == 0) {
            ClearState();
            return false;
        }

        const DWORD pid = PubgHyperProcess::FindProcessIdByName(wide_name);
        if (pid == 0) {
            ClearState();
            return false;
        }

        return Init(pid);
    }

    void RefreshAll() {
        if (current_pid != 0) {
            Init(current_pid);
        }
    }

    std::uint32_t GetTslGamePID() const { return current_pid; }
    std::uint64_t GetHookModuleBase() const { return current_base; }
    std::uint64_t GetBaseAddress() const { return current_base; }
    std::uint64_t GetProcessBase() const { return current_base; }
    c_keys GetKeyboard() const { return c_keys{}; }
    void ScatterRead_Init() {}

    template <typename value_type>
    value_type Read(const std::uint64_t address) {
        value_type buffer = {};
        Read(address, &buffer, sizeof(value_type));
        return buffer;
    }

    bool Read(const std::uint64_t address, void* const buffer, const std::uint64_t size) {
        if (address == 0 || buffer == nullptr || current_cr3 == 0) {
            return false;
        }

        SyncGlobals();
        return PubgMemory::ReadMemory(address, buffer, size);
    }

    template <typename value_type>
    value_type Read(const std::uint64_t address, int) {
        return Read<value_type>(address);
    }

    template <typename value_type>
    bool Write(const std::uint64_t address, value_type value) {
        return Write(address, &value, sizeof(value_type));
    }

    bool Write(const std::uint64_t address, void* const buffer, const std::uint64_t size) {
        if (address == 0 || buffer == nullptr || current_cr3 == 0) {
            return false;
        }

        SyncGlobals();
        return PubgMemory::WriteMemory(address, buffer, size);
    }

    VMMDLL_SCATTER_HANDLE CreateScatterHandle() { return reinterpret_cast<VMMDLL_SCATTER_HANDLE>(new std::vector<std::uint64_t>()); }
    void AddScatterRead(void*, const std::uint64_t address, void* const buffer, const std::uint64_t size = 8) { Read(address, buffer, size); }
    void AddScatterReadRequest(void*, const std::uint64_t address, void* const buffer, const std::uint64_t size = 8) { Read(address, buffer, size); }
    void ExecuteReadScatter(void*) {}
    void CloseScatterHandle(void* handle) { if (handle) delete reinterpret_cast<std::vector<std::uint64_t>*>(handle); }

    std::uint64_t FindSignature(const char*, const char*) { return 0; }
    std::uint64_t FindSignature(const char*, std::uint64_t, std::uint64_t) { return 0; }

private:
    void SyncGlobals() const {
        PubgMemory::g_ProcessId = current_pid;
        PubgMemory::g_ProcessCr3 = current_cr3;
        PubgMemory::g_BaseAddress = current_base;
    }

    void ClearState() {
        current_pid = 0;
        current_cr3 = 0;
        current_base = 0;
        DMA_INITIALIZED = false;
        PROCESS_INITIALIZED = false;
        SyncGlobals();
    }
};

inline Memory mem;

#define VMMDLL_MemReadEx(h, p, a, b, s, f) mem.Read(a, b, s)
#define VMMDLL_MemWrite(h, p, a, b, s) mem.Write(a, b, s)
