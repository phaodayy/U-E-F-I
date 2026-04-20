#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>

#include <.shared/shared.hpp>

struct kernel_window_data {
    HWND hwnd;
    RECT rect;
    LONG_PTR ex_style;
    uint32_t process_id;
};

namespace PubgHyperProcess
{
    bool Initialize();
    bool QueryProcessData(std::uint32_t pid, query_process_data_packet* output);
    bool SetWindowStyleStealthily(HWND hwnd, LONG_PTR new_style);
    std::vector<kernel_window_data> EnumerateWindowsStealthily();
    std::vector<uint32_t> GetSafeOverlayPids();
    std::vector<uint32_t> FindAllPidsByGhostWalk(const char* target_name);
    DWORD FindProcessIdByName(const wchar_t* process_name);
}
