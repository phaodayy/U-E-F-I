#pragma once
// ============================================================
// VMouseClient - Virtual HID Mouse Client
// Ported from vmouse-main (github/vmouse) for PUBG-2 integration
// Communicates with vmouse.sys virtual HID mouse driver via
// HidD_SetOutputReport (safe, no kernel hacking, no BSOD risk)
// ============================================================
#pragma comment(lib, "hid")
#pragma comment(lib, "Cfgmgr32")

#include <Windows.h>
#include <hidsdi.h>
#include <cfgmgr32.h>
#include <iostream>

namespace VMouseClient {

// Must match the INF/HW ID of the vmouse driver
// Pattern: HID#MOUSE_DEVICE is the Hardware ID defined in vmouse.inf
static constexpr WCHAR DEVICE_PREFIX[] = L"\\\\?\\HID#MOUSE_DEVICE";

// Report IDs (must match vmouse Device.c report descriptor)
static constexpr BYTE REPORT_ID_INPUT  = 0x01;
static constexpr BYTE REPORT_ID_OUTPUT = 0x02;

#pragma pack(push, 1)
struct MouseReport {
    BYTE report_id; // REPORT_ID_OUTPUT (0x02)
    BYTE buttons;   // 0 = none, 1 = left, 2 = right, 4 = middle
    CHAR x;         // Relative X movement (-127 to 127)
    CHAR y;         // Relative Y movement (-127 to 127)
};
#pragma pack(pop)

inline HANDLE g_DeviceHandle = INVALID_HANDLE_VALUE;
inline bool   g_IsReady      = false;
inline BYTE   g_Buttons      = 0;

inline bool Initialize() {
    if (g_IsReady) return true;

    GUID hid_guid;
    HidD_GetHidGuid(&hid_guid);

    ULONG list_length = 0;
    CONFIGRET cr = CM_Get_Device_Interface_List_Size(
        &list_length, &hid_guid, nullptr,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT
    );
    if (CR_SUCCESS != cr || list_length <= 1) {
        printf("[VMouseClient] Failed to get HID interface list size!\n");
        return false;
    }

    PWSTR device_list = static_cast<PWSTR>(malloc(list_length * sizeof(WCHAR)));
    if (!device_list) return false;
    ZeroMemory(device_list, list_length * sizeof(WCHAR));

    cr = CM_Get_Device_Interface_List(
        &hid_guid, nullptr,
        reinterpret_cast<PZZWSTR>(device_list),
        list_length,
        CM_GET_DEVICE_INTERFACE_LIST_PRESENT
    );
    if (CR_SUCCESS != cr) {
        free(device_list);
        return false;
    }

    const size_t prefix_len = wcslen(DEVICE_PREFIX);
    for (PWSTR iface = device_list; *iface; iface += wcslen(iface) + 1) {
        if (0 != wcsncmp(iface, DEVICE_PREFIX, prefix_len)) continue;

        HANDLE h = CreateFileW(iface, GENERIC_WRITE, FILE_SHARE_WRITE,
                               nullptr, OPEN_EXISTING, 0, nullptr);
        if (INVALID_HANDLE_VALUE == h) continue;

        g_DeviceHandle = h;
        g_IsReady = true;
        printf("[VMouseClient] Virtual HID Mouse device found and connected!\n");
        break;
    }
    free(device_list);

    if (!g_IsReady) {
        printf("[VMouseClient] vmouse.sys device NOT found. Is the driver loaded?\n");
    }
    return g_IsReady;
}

// Send a single relative movement report (x, y in range -127..127)
inline bool SendReport(CHAR x, CHAR y, BYTE buttons = 0) {
    if (!g_IsReady || g_DeviceHandle == INVALID_HANDLE_VALUE) return false;

    MouseReport report{};
    report.report_id = REPORT_ID_OUTPUT;
    report.buttons   = buttons;
    report.x         = x;
    report.y         = y;

    return HidD_SetOutputReport(g_DeviceHandle, &report, sizeof(report)) == TRUE;
}

// Move mouse by (dx, dy) pixels. Chunks into -127..127 steps automatically.
inline bool Move(int dx, int dy) {
    if (!g_IsReady) return false;

    bool ok = true;
    while (dx != 0 || dy != 0) {
        CHAR step_x = static_cast<CHAR>(max(-127, min(127, dx)));
        CHAR step_y = static_cast<CHAR>(max(-127, min(127, dy)));
        ok = ok && SendReport(step_x, step_y, g_Buttons);
        dx -= step_x;
        dy -= step_y;
    }
    return ok;
}

inline void Shutdown() {
    if (g_DeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(g_DeviceHandle);
        g_DeviceHandle = INVALID_HANDLE_VALUE;
    }
    g_IsReady = false;
}

} // namespace VMouseClient
