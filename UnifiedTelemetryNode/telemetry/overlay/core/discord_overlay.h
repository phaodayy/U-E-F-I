#pragma once

#include "overlay_menu.hpp"

#include <cstring>
#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>

#include "../../sdk/core/app_shutdown.hpp"
#include "../../sdk/core/console_log.hpp"
#include <protec/skCrypt.h>

namespace VisualizationBridgeDiscovery {

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

inline bool ConfigureStealthOverlay(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd)) return false;

    // 1. Chống chụp ảnh màn hình (Anti-Screenshot)
    // Kỹ thuật này khiến cửa sổ hoàn toàn biến mất trong ảnh chụp của Game/Anti-cheat
    #ifndef WDA_EXCLUDEFROMCAPTURE
    #define WDA_EXCLUDEFROMCAPTURE 0x00000011
    #endif
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

    RECT target_rect = {};
    HWND game = FindGameWindow();
    if (!GetGameClientRect(game, &target_rect)) {
        target_rect.left = 0; target_rect.top = 0;
        target_rect.right = GetSystemMetrics(SM_CXSCREEN);
        target_rect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }

    const int width = target_rect.right - target_rect.left;
    const int height = target_rect.bottom - target_rect.top;
    if (width <= 0 || height <= 0) return false;

    // 2. Thiết lập Style một cách tinh tế
    // WS_EX_TRANSPARENT giúp click chuột xuyên qua Overlay vào Game
    // WS_EX_TOOLWINDOW giúp giấu cửa sổ khỏi Alt-Tab
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE,
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MoveWindow(hwnd, target_rect.left, target_rect.top, width, height, TRUE);
    
    // 3. Kích hoạt tính năng "Kính" (Glass effect) qua DWM
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    SetWindowPos(hwnd, HWND_TOPMOST, target_rect.left, target_rect.top, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);

    return true;
}

#if 0
inline VisualizationBridgeHost FromNvidiaHost() {
    VisualizationBridgeHost bridge = {};
    // NVIDIA Overlay cực kỳ an toàn vì nó là tiến trình Driver
    HWND hwnd = FindWindowA(skCrypt("CEF-Standard-Window"), skCrypt("NVIDIA GeForce Overlay"));
    if (hwnd && IsWindowVisible(hwnd) && ConfigureStealthOverlay(hwnd)) {
        bridge.hwnd = hwnd;
        bridge.clear_before_render = true;
        bridge.present_after_render = true;
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] NVIDIA Stealth Host active.") << std::endl);
    }
    return bridge;
}

#endif

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

    if (hwnd && ConfigureStealthOverlay(hwnd)) {
        bridge.hwnd = hwnd;
        bridge.clear_before_render = true;
        bridge.present_after_render = true;
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] Movavi Stealth Host active.") << std::endl);
    }

    return bridge;
}

#if 0
inline VisualizationBridgeHost FromDiscordFallback() {
    VisualizationBridgeHost bridge = {};
    std::vector<uint32_t> discord_pids = telemetryHyperProcess::FindAllPidsByGhostWalk(skCrypt("Discord"));

    if (!discord_pids.empty()) {
        HWND hwnd = GetTopWindow(nullptr);
        while (hwnd) {
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
            bool is_match = false;
            for (uint32_t d_pid : discord_pids) { if (pid == d_pid) { is_match = true; break; } }

            if (is_match) {
                char class_name[256] = {};
                char window_name[256] = {};
                GetClassNameA(hwnd, class_name, sizeof(class_name));
                GetWindowTextA(hwnd, window_name, sizeof(window_name));

                if (strcmp(class_name, skCrypt("Chrome_WidgetWin_1")) == 0 &&
                    (strstr(window_name, skCrypt("Discord Overlay")) != nullptr || (window_name[0] == '\0' && IsWindowVisible(hwnd)))) {
                    
                    RECT rc = {}; GetWindowRect(hwnd, &rc);
                    if ((rc.right - rc.left) > 100 && (rc.bottom - rc.top) > 100) {
                        // Với Discord chúng ta không cần ConfigureStealthOverlay vì nó đã chuẩn sẵn
                        SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
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

#endif

inline VisualizationBridgeHost ResolveHost() {
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Negotiating Stealth Bridge Link...") << std::endl);

    int attempt = 0;
    while (!AppShutdown::IsRequested()) {
        attempt++;

        VisualizationBridgeHost bridge = FromMovaviHost();
        if (bridge.hwnd) return bridge;

        if (attempt % 5 == 0) {
            UTN_DEV_LOG(std::cout << skCrypt("[DEV] Still waiting for Movavi overlay host, attempt ") << attempt << std::endl);
        }
        Sleep(1000);
    }
    return {};
}

} // namespace VisualizationBridgeDiscovery
