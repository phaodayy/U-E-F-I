#pragma once

#include "overlay_menu.hpp"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>

#include "../../sdk/core/app_shutdown.hpp"
#include "../../sdk/core/console_log.hpp"
#include "../../sdk/memory/hyper_process.hpp"
#include <protec/skCrypt.h>

namespace VisualizationBridgeDiscovery {

constexpr uint32_t kRegistryMagic = 0x42565455; // UTVB
constexpr uint32_t kRegistryVersion = 1;
constexpr DWORD kFlagClearBeforeRender = 1 << 0;
constexpr DWORD kFlagPresentAfterRender = 1 << 1;

struct RegistryBlock {
    uint32_t magic = 0;
    uint32_t version = 0;
    uint64_t hwnd = 0;
    uint32_t owner_pid = 0;
    uint32_t flags = 0;
};

inline bool AssignWindow(VisualizationBridgeHost& bridge, uint64_t raw_hwnd, uint32_t expected_owner_pid = 0) {
    if (raw_hwnd == 0) return false;

    HWND hwnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(raw_hwnd));
    if (!IsWindow(hwnd)) return false;

    DWORD owner_pid = 0;
    GetWindowThreadProcessId(hwnd, &owner_pid);
    if (expected_owner_pid != 0 && owner_pid != expected_owner_pid) return false;

    bridge.hwnd = hwnd;
    return true;
}

inline VisualizationBridgeHost FromEnvironment() {
    VisualizationBridgeHost bridge = {};
    char value[64] = {};
    DWORD len = GetEnvironmentVariableA(skCrypt("UTN_VISUALIZATION_HWND"), value, static_cast<DWORD>(sizeof(value)));
    if (len == 0 || len >= sizeof(value)) return bridge;

    char* end = nullptr;
    unsigned long long raw = std::strtoull(value, &end, 0);
    if (end == value || raw == 0) return bridge;

    AssignWindow(bridge, raw);
    return bridge;
}

inline VisualizationBridgeHost FromSharedRegistry() {
    VisualizationBridgeHost bridge = {};
    HANDLE mapping = OpenFileMappingA(FILE_MAP_READ, FALSE, skCrypt("Local\\UTNVisualizationBridge"));
    if (!mapping) return bridge;

    auto* mapped = static_cast<const RegistryBlock*>(
        MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(RegistryBlock)));
    if (mapped) {
        RegistryBlock block = *mapped;
        UnmapViewOfFile(mapped);

        if (block.magic == kRegistryMagic &&
            block.version == kRegistryVersion &&
            AssignWindow(bridge, block.hwnd, block.owner_pid)) {
            bridge.clear_before_render = (block.flags & kFlagClearBeforeRender) != 0;
            bridge.present_after_render = (block.flags & kFlagPresentAfterRender) != 0;
        }
    }

    CloseHandle(mapping);
    return bridge;
}

inline HWND FindGameWindow() {
    HWND hwnd = FindWindowA(nullptr, skCrypt("PUBG: BATTLEGROUNDS "));
    if (!hwnd) hwnd = FindWindowA(skCrypt("UnrealWindow"), nullptr);
    return hwnd;
}

inline bool GetGameClientRect(HWND game, RECT* rect) {
    if (!rect || !game || !IsWindow(game)) return false;

    RECT client = {};
    if (!GetClientRect(game, &client)) return false;

    POINT top_left = { 0, 0 };
    if (!ClientToScreen(game, &top_left)) return false;

    rect->left = top_left.x;
    rect->top = top_left.y;
    rect->right = top_left.x + (client.right - client.left);
    rect->bottom = top_left.y + (client.bottom - client.top);
    return rect->right > rect->left && rect->bottom > rect->top;
}

inline HWND FindMovaviWindow() {
    HWND hwnd = FindWindowA(skCrypt("Qt5152QWindowIcon"), skCrypt("Screen Recorder"));
    if (hwnd) return hwnd;

    HWND current = GetTopWindow(nullptr);
    while (current) {
        char class_name[256] = {};
        char title[256] = {};
        GetClassNameA(current, class_name, sizeof(class_name));
        GetWindowTextA(current, title, sizeof(title));

        if (strcmp(class_name, skCrypt("Qt5152QWindowIcon")) == 0 &&
            (strstr(title, skCrypt("Screen Recorder")) != nullptr || title[0] == '\0')) {
            return current;
        }

        current = GetNextWindow(current, GW_HWNDNEXT);
    }

    return nullptr;
}

inline void LaunchMovaviIfNeeded() {
    const char* paths[] = {
        skCrypt("C:\\Program Files\\Movavi Screen Recorder\\ScreenRecorder.exe"),
        skCrypt("C:\\Program Files (x86)\\Movavi Screen Recorder\\ScreenRecorder.exe"),
    };

    for (const char* path : paths) {
        if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
            ShellExecuteA(nullptr, skCrypt("open"), path, nullptr, nullptr, SW_SHOWMINIMIZED);
            return;
        }
    }
}

inline bool ConfigureMovaviOverlayWindow(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return false;

    RECT target_rect = {};
    HWND game = FindGameWindow();
    if (!GetGameClientRect(game, &target_rect)) {
        target_rect.left = 0;
        target_rect.top = 0;
        target_rect.right = GetSystemMetrics(SM_CXSCREEN);
        target_rect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    const int width = target_rect.right - target_rect.left;
    const int height = target_rect.bottom - target_rect.top;
    if (width <= 0 || height <= 0) return false;

    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE,
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

    MoveWindow(hwnd, target_rect.left, target_rect.top, width, height, TRUE);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd);
    SetWindowPos(hwnd, HWND_TOPMOST, target_rect.left, target_rect.top, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);

    return true;
}

inline VisualizationBridgeHost FromMovaviHost() {
    VisualizationBridgeHost bridge = {};

    HWND hwnd = FindMovaviWindow();
    if (!hwnd) {
        LaunchMovaviIfNeeded();
        for (int i = 0; i < 150 && !AppShutdown::IsRequested(); ++i) {
            hwnd = FindMovaviWindow();
            if (hwnd) break;
            Sleep(100);
        }
    }

    if (hwnd && ConfigureMovaviOverlayWindow(hwnd)) {
        bridge.hwnd = hwnd;
        bridge.clear_before_render = true;
        bridge.present_after_render = true;
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] Movavi overlay host ready: ") << hwnd << std::endl);
    }

    return bridge;
}

inline VisualizationBridgeHost FromDiscordFallback() {
    VisualizationBridgeHost bridge = {};
    std::vector<uint32_t> discord_pids = telemetryHyperProcess::FindAllPidsByGhostWalk(skCrypt("Discord"));

    if (!discord_pids.empty()) {
        HWND hwnd = GetTopWindow(nullptr);
        while (hwnd) {
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
            bool is_match = false;
            for (uint32_t d_pid : discord_pids) {
                if (pid == d_pid) {
                    is_match = true;
                    break;
                }
            }

            if (is_match) {
                char class_name[256] = {};
                char window_name[256] = {};
                GetClassNameA(hwnd, class_name, sizeof(class_name));
                GetWindowTextA(hwnd, window_name, sizeof(window_name));

                if (strcmp(class_name, skCrypt("Chrome_WidgetWin_1")) == 0 &&
                    (strstr(window_name, skCrypt("Discord Overlay")) != nullptr ||
                     (window_name[0] == '\0' && IsWindowVisible(hwnd)))) {
                    RECT rc = {};
                    GetWindowRect(hwnd, &rc);
                    if ((rc.right - rc.left) > 100 && (rc.bottom - rc.top) > 100) {
                        bridge.hwnd = hwnd;
                        break;
                    }
                }
            }
            hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
        }
    }

    if (bridge.hwnd) {
        bridge.clear_before_render = false;
        bridge.present_after_render = true;
    }
    return bridge;
}

inline VisualizationBridgeHost ResolveHost() {
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Waiting for bridge link (Movavi first, Discord fallback)...") << std::endl);

    int attempt = 0;
    while (!AppShutdown::IsRequested()) {
        attempt++;

        VisualizationBridgeHost bridge = FromEnvironment();
        if (bridge.hwnd) return bridge;

        bridge = FromSharedRegistry();
        if (bridge.hwnd) return bridge;

        bridge = FromMovaviHost();
        if (bridge.hwnd) {
            UTN_DEV_LOG(std::cout << skCrypt("[DEV] Found passive host via Movavi.") << std::endl);
            return bridge;
        }

        bridge = FromDiscordFallback();
        if (bridge.hwnd) {
            UTN_DEV_LOG(std::cout << skCrypt("[DEV] Found passive host via Discord.") << std::endl);
            return bridge;
        }

        if (attempt % 5 == 0) {
            UTN_DEV_LOG(std::cout << skCrypt("[DEV] Still waiting for overlay host, attempt ") << attempt << std::endl);
        }
        Sleep(1000);
    }
    return {};
}

} // namespace VisualizationBridgeDiscovery
