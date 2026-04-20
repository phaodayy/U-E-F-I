#pragma once

#include <Windows.h>
#include <cstdint>

#include <.shared/shared.hpp>

namespace PubgHyperProcess
{
    bool Initialize();
    bool QueryProcessData(std::uint32_t pid, query_process_data_packet* output);
    bool SetWindowStyleStealthily(HWND hwnd, LONG_PTR new_style);
    DWORD FindProcessIdByName(const wchar_t* process_name);
}
