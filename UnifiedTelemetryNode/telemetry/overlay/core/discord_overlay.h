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

extern LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

    // Lưu lại trạng thái gốc để khôi phục sau này
    if (g_Menu.original_style == 0) {
        g_Menu.original_style = GetWindowLongPtr(hwnd, GWL_STYLE);
        g_Menu.original_ex_style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        GetWindowRect(hwnd, &g_Menu.original_rect);
    }

    // 1. Chống chụp ảnh màn hình (Anti-Screenshot)
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

    SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE,
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MoveWindow(hwnd, target_rect.left, target_rect.top, width, height, TRUE);
    
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    SetWindowPos(hwnd, HWND_TOPMOST, target_rect.left, target_rect.top, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);

    return true;
}

inline void RestoreStealthOverlay(HWND hwnd) {
    if (!hwnd || !IsWindow(hwnd) || g_Menu.original_style == 0) return;

    // Trả lại Style gốc
    SetWindowLongPtr(hwnd, GWL_STYLE, g_Menu.original_style);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, g_Menu.original_ex_style);
    
    // Tắt Anti-Screenshot
    SetWindowDisplayAffinity(hwnd, 0);

    // Trả lại vị trí và kích thước gốc
    int w = g_Menu.original_rect.right - g_Menu.original_rect.left;
    int h = g_Menu.original_rect.bottom - g_Menu.original_rect.top;
    MoveWindow(hwnd, g_Menu.original_rect.left, g_Menu.original_rect.top, w, h, TRUE);

    // Đưa cửa sổ về trạng thái bình thường (không còn Topmost)
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Stealth Host restored to original state.") << std::endl);
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

    if (hwnd && ConfigureStealthOverlay(hwnd)) {
        bridge.hwnd = hwnd;
        bridge.clear_before_render = true;
        bridge.present_after_render = true;
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] Movavi Stealth Host active.") << std::endl);
    }

    return bridge;
}

inline VisualizationBridgeHost FromGhostHost() {
    VisualizationBridgeHost bridge = {};
    
    static bool class_registered = false;
    static wchar_t randClass[32] = { 0 };
    static wchar_t randTitle[32] = { 0 };

    if (!class_registered) {
        srand((unsigned int)GetTickCount64());
        for (int i = 0; i < 15; i++) {
            randClass[i] = (wchar_t)((rand() % 26) + (rand() % 2 ? 'a' : 'A'));
            randTitle[i] = (wchar_t)((rand() % 26) + (rand() % 2 ? 'a' : 'A'));
        }

        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, randClass, NULL };
        RegisterClassExW(&wc);
        class_registered = true;
    }

    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
        randClass, randTitle, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN), NULL, NULL, GetModuleHandle(NULL), NULL);

    if (hwnd) {
        SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
        MARGINS margin = { -1 };
        DwmExtendFrameIntoClientArea(hwnd, &margin);
        
        // Anti-Screenshot for Ghost window too
        #ifndef WDA_EXCLUDEFROMCAPTURE
        #define WDA_EXCLUDEFROMCAPTURE 0x00000011
        #endif
        SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);

        bridge.hwnd = hwnd;
        bridge.clear_before_render = true;
        bridge.present_after_render = true;
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] Ghost Stealth Host created: ") << (int)hwnd << std::endl);
    }
    
    return bridge;
}

inline VisualizationBridgeHost ResolveHost() {
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Negotiating Stealth Bridge Link (Mode: ") << g_Menu.overlay_mode << skCrypt(")...") << std::endl);

    if (g_Menu.overlay_mode == 1) {
        return FromGhostHost();
    }

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
