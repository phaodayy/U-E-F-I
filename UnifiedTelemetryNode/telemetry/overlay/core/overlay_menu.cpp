#include <winsock2.h>
#include "overlay_menu.hpp"
#include "colors.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_dx11.h"
#include "../../imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <dwmapi.h>
#include <fstream>
#include <string>

extern const wchar_t* LOADER_LOGIN_PATH;
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include "../translation/translation.hpp"
#include "../entities/entity_aliases.hpp"
#include "../loot/loot_debug_renderer.hpp"
#include "../loot/loot_icon_resolver.hpp"
#include "../loot/loot_cluster_renderer.hpp"
#include "../loot/loot_source_merge.hpp"
#include "overlay_config_sections.hpp"
#include "overlay_hotkeys.hpp"
#include "../../sdk/core/console_log.hpp"
#include "../player/player_esp_layout.hpp"
#include "overlay_texture_cache.hpp"
#include "../radar/radar_overlay_renderer.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include "../../sdk/core/app_shutdown.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/core/math.hpp"
#include "../../sdk/core/offsets.hpp"
#include "../../sdk/core/health.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <protec/skCrypt.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

extern void UploadActiveLoaderConfigAsync();

// DirectX 11 Globals
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool g_clearBeforeRender = true;
static bool g_presentAfterRender = true;

// External globals from context
extern std::vector<PlayerData> G_Players;
extern std::vector<ItemData> CachedItems;

OverlayMenu g_Menu;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool OverlayMenu::CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pd3dDeviceContext);
    if (res != S_OK) return false;
    g_clearBeforeRender = true;
    g_presentAfterRender = true;
    return CreateRenderTarget();
}

void OverlayMenu::CleanupDeviceD3D() {
    CleanupRenderTarget();
    OverlayTextures::SetDevice(nullptr);
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    g_clearBeforeRender = true;
    g_presentAfterRender = true;
}

bool OverlayMenu::CreateRenderTarget() {
    if (!g_pSwapChain || !g_pd3dDevice) return false;

    ID3D11Texture2D* pBackBuffer = nullptr;
    if (FAILED(g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))) || !pBackBuffer) return false;

    HRESULT hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return SUCCEEDED(hr) && g_mainRenderTargetView != nullptr;
}

void OverlayMenu::CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

static bool AcquireBridgeD3D(const VisualizationBridgeHost& bridge) {
    if (!bridge.swap_chain) return false;

    g_pSwapChain = bridge.swap_chain;
    g_pSwapChain->AddRef();

    if (bridge.device) {
        g_pd3dDevice = bridge.device;
        g_pd3dDevice->AddRef();
    } else if (FAILED(g_pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_pd3dDevice)))) {
        return false;
    }

    if (bridge.context) {
        g_pd3dDeviceContext = bridge.context;
        g_pd3dDeviceContext->AddRef();
    } else {
        g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);
    }

    g_clearBeforeRender = bridge.clear_before_render;
    g_presentAfterRender = bridge.present_after_render;
    return g_pd3dDevice && g_pd3dDeviceContext;
}

static HWND FindTrackedGameWindow() {
    HWND hwnd = FindWindowA(nullptr, skCrypt("PUBG: BATTLEGROUNDS "));
    if (!hwnd) hwnd = FindWindowA(skCrypt("UnrealWindow"), nullptr);
    return hwnd;
}

static bool GetTrackedGameClientRect(HWND game, RECT& rect) {
    if (!game || !IsWindow(game)) return false;

    RECT client = {};
    if (!GetClientRect(game, &client)) return false;

    POINT top_left = { 0, 0 };
    if (!ClientToScreen(game, &top_left)) return false;

    rect.left = top_left.x;
    rect.top = top_left.y;
    rect.right = top_left.x + (client.right - client.left);
    rect.bottom = top_left.y + (client.bottom - client.top);
    return rect.right > rect.left && rect.bottom > rect.top;
}

bool OverlayMenu::Initialize(const VisualizationBridgeHost& bridge) {
    target_hwnd = bridge.hwnd;
    if (!target_hwnd || !IsWindow(target_hwnd)) {
        target_hwnd = nullptr;
        UTN_DEV_LOG(std::cout << skCrypt("[-][DEV] Passive visualization host is not available.") << std::endl);
        return false;
    }

    char cls[256] = {0};
    DWORD target_pid = 0;
    GetClassNameA(target_hwnd, cls, 256);
    GetWindowThreadProcessId(target_hwnd, &target_pid);
    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Passive visualization host attached: [") << cls << skCrypt("] (PID: ") << target_pid << skCrypt(")") << std::endl);

    if (target_pid == GetCurrentProcessId()) {
        MARGINS margin = { -1 };
        DwmExtendFrameIntoClientArea(target_hwnd, &margin);
    }

    if (bridge.swap_chain) {
        if (!AcquireBridgeD3D(bridge) || !CreateRenderTarget()) {
            CleanupDeviceD3D();
            return false;
        }
        UTN_DEV_LOG(std::cout << skCrypt("[DEV] Bridge renderer using host-provided swap chain.") << std::endl);
    } else if (!CreateDeviceD3D(target_hwnd)) {
        return false;
    }

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    // Load a system font that supports Vietnamese (Tahoma is preferred by user)
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f, &font_cfg, io.Fonts->GetGlyphRangesVietnamese());

    SetupStyle();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    LoadConfig(skCrypt("dataMacro/Config/settings.json"));

    // Load Preview Asset (Now relative to EXE for bin deployment)
    OverlayTextures::SetDevice(g_pd3dDevice);
    OverlayTextures::LoadInstructor();

    UTN_DEV_LOG(std::cout << skCrypt("[DEV] Passive visualization ready.") << std::endl);
    return true;
}

void OverlayMenu::RenderFrame() {
    extern std::string global_active_key;
    extern bool HasActiveLoaderEntitlement();
    bool is_authenticated = HasActiveLoaderEntitlement();
    OverlayConfigSections::ClampAll(*this);

    static bool need_to_release = false;
    if (waiting_for_key) {
        // First time entering wait state? Set need_to_release to true
        static void* last_waiting = nullptr;
        if (last_waiting != waiting_for_key) {
            need_to_release = true;
            last_waiting = waiting_for_key;
        }

        if (need_to_release) {
            bool any_down = false;
            for (int i = 0x01; i <= 0xFE; i++) {
                if (telemetryMemory::IsKeyDown(i)) { any_down = true; break; }
            }
            if (!any_down) need_to_release = false;
        }

        // Only search for NEW key if previous click is released
        if (!need_to_release) {
            for (int i = 0x01; i <= 0xFE; i++) {
                if (telemetryMemory::IsKeyDown(i)) {
                    if (i == VK_ESCAPE) { waiting_for_key = nullptr; last_waiting = nullptr; break; }

                    *waiting_for_key = i;
                    waiting_for_key = nullptr;
                    last_waiting = nullptr;
                    // Wait for release
                    while (telemetryMemory::IsKeyDown(i)) telemetryMemory::StealthSleep(1);
                    break;
                }
            }
        }
    } else {
        need_to_release = false;
    }
    try {
        if (!target_hwnd) return;

        static DWORD last_window_track_tick = 0;
        const DWORD now_track = GetTickCount();
        if (now_track - last_window_track_tick >= 250) {
            last_window_track_tick = now_track;

            HWND game_hwnd = FindTrackedGameWindow();
            if (game_hwnd && IsWindow(game_hwnd)) {
                if (IsIconic(game_hwnd) && !showmenu) {
                    ShowWindow(target_hwnd, SW_HIDE);
                } else {
                    RECT target_rect = {};
                    if (GetTrackedGameClientRect(game_hwnd, target_rect)) {
                        RECT current_rect = {};
                        GetWindowRect(target_hwnd, &current_rect);

                        const int new_w = target_rect.right - target_rect.left;
                        const int new_h = target_rect.bottom - target_rect.top;
                        const int cur_w = current_rect.right - current_rect.left;
                        const int cur_h = current_rect.bottom - current_rect.top;

                        if (new_w > 0 && new_h > 0 &&
                            (current_rect.left != target_rect.left ||
                             current_rect.top != target_rect.top ||
                             cur_w != new_w ||
                             cur_h != new_h)) {
                            if (g_pSwapChain && (cur_w != new_w || cur_h != new_h)) {
                                CleanupRenderTarget();
                                if (SUCCEEDED(g_pSwapChain->ResizeBuffers(0, new_w, new_h, DXGI_FORMAT_UNKNOWN, 0))) {
                                    CreateRenderTarget();
                                }
                            }

                            SetWindowPos(target_hwnd, HWND_TOPMOST,
                                target_rect.left, target_rect.top, new_w, new_h,
                                SWP_NOACTIVATE | SWP_SHOWWINDOW);
                        } else {
                            SetWindowPos(target_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                        }
                    }

                    HWND foreground = GetForegroundWindow();
                    const bool should_show = showmenu || foreground == game_hwnd || foreground == target_hwnd;
                    ShowWindow(target_hwnd, should_show ? SW_SHOWNOACTIVATE : SW_HIDE);
                }
            }
        }

        // --- TOGGLE MENU (F5) ---
        static bool f5_down = false;
        bool f5_current = telemetryMemory::IsKeyDown(VK_F5);
        if (f5_current && !f5_down) {
            showmenu = !showmenu;
            
            // Toggle Window Input Transparency
            LONG_PTR exStyle = GetWindowLongPtr(target_hwnd, GWL_EXSTYLE);
            if (showmenu) {
                // Remove TRANSPARENT so we can type/click
                SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
                SetForegroundWindow(target_hwnd);
                SetFocus(target_hwnd);
            } else {
                // Re-add TRANSPARENT so clicks pass through to game
                SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
            }
        }
        f5_down = f5_current;

        static bool items_hotkey_down = false;
        static bool vehicles_hotkey_down = false;
        const bool allow_quick_toggles = is_authenticated && !showmenu && waiting_for_key == nullptr;
        const bool item_toggle_pressed = OverlayHotkeys::ConsumePressed(esp_items_toggle_key, items_hotkey_down);
        const bool vehicle_toggle_pressed = OverlayHotkeys::ConsumePressed(esp_vehicles_toggle_key, vehicles_hotkey_down);
        if (allow_quick_toggles && item_toggle_pressed) esp_items = !esp_items;
        if (allow_quick_toggles && vehicle_toggle_pressed) esp_vehicles = !esp_vehicles;

        // --- INPUT INJECTION (FOR HIJACKED WINDOW) ---
        ImGuiIO& io = ImGui::GetIO();
        if (showmenu) {
            // Mouse
            POINT p; GetCursorPos(&p);
            ScreenToClient(target_hwnd, &p);
            io.MousePos = ImVec2((float)p.x, (float)p.y);
            io.MouseDown[0] = telemetryMemory::IsKeyDown(VK_LBUTTON);
            io.MouseDown[1] = telemetryMemory::IsKeyDown(VK_RBUTTON);

            // Manual Keyboard Input Sync (Since captured window messages don't reach us for hijacked windows)
            static bool key_states[256] = { false };
            for (int i = 0x08; i <= 0x5A; i++) { // Backspace to Z (covers common input) + common chars
                bool down = (GetAsyncKeyState(i) & 0x8000);
                if (down && !key_states[i]) {
                    // 1. Priority Shortcuts (Ctrl+V, Ctrl+A)
                    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                        if (i == 'V') {
                            const char* clipboard = ImGui::GetClipboardText();
                            if (clipboard) io.AddInputCharactersUTF8(clipboard);
                            key_states[i] = down;
                            continue;
                        } else if (i == 'A') {
                            io.AddKeyEvent(ImGuiMod_Ctrl, true); io.AddKeyEvent(ImGuiKey_A, true);
                            io.AddKeyEvent(ImGuiKey_A, false); io.AddKeyEvent(ImGuiMod_Ctrl, false);
                            key_states[i] = down;
                            continue;
                        }
                    }

                    // 2. Normal Key Logic
                    if (i >= 0x30 && i <= 0x5A) { // Alpha-numeric
                        BYTE kst[256];
                        GetKeyboardState(kst);
                        wchar_t buf[2];
                        if (ToUnicode(i, MapVirtualKey(i, MAPVK_VK_TO_VSC), kst, buf, 2, 0) == 1) {
                            io.AddInputCharacter(buf[0]);
                        }
                    } else if (i == VK_BACK) { io.AddKeyEvent(ImGuiKey_Backspace, true); io.AddKeyEvent(ImGuiKey_Backspace, false); }
                    else if (i == VK_SPACE) { io.AddInputCharacter(' '); }
                    else if (i == VK_TAB) { io.AddKeyEvent(ImGuiKey_Tab, true); io.AddKeyEvent(ImGuiKey_Tab, false); }
                }
                key_states[i] = down;
            }
            // Additional symbols and NumPad
            for (int i : {VK_OEM_1, VK_OEM_2, VK_OEM_3, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_7, VK_OEM_PERIOD, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PLUS}) {
                 bool down = (GetAsyncKeyState(i) & 0x8000);
                 if (down && !key_states[i & 0xFF]) {
                     BYTE kst[256]; GetKeyboardState(kst);
                     wchar_t buf[2];
                     if (ToUnicode(i, MapVirtualKey(i, MAPVK_VK_TO_VSC), kst, buf, 2, 0) == 1) io.AddInputCharacter(buf[0]);
                 }
                 key_states[i & 0xFF] = down;
            }
        }

        // --- WINDOW MESSAGE HANDLING (VALORANT STYLE) ---
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 1-2 (Weapon), X (Holster), Tab (Bag), M (Map), G (Grenade), F (Interact/Equip)
        if (is_authenticated &&
            (telemetryMemory::IsKeyDown('1') || telemetryMemory::IsKeyDown('2') || telemetryMemory::IsKeyDown('X') ||
             telemetryMemory::IsKeyDown(VK_TAB) || telemetryMemory::IsKeyDown('M') || telemetryMemory::IsKeyDown('G') || telemetryMemory::IsKeyDown('F'))) {
            MacroEngine::ForceScan();
        }

        /*
        // --- MOUSE TEST KEY (F8) ---
        if (telemetryMemory::IsKeyDown(VK_F8)) {
            printf("[DEBUG] F8 pressed: Attempting mouse movement test...\n");
            bool success = true;
            for (int i = 0; i < 20; i++) {
                if (!telemetryMemory::MoveMouse(0, 15, 0)) {
                    success = false;
                    break;
                }
                telemetryMemory::StealthSleep(2);
            }
            if (success) {
                printf("[DEBUG] Mouse Move test successful!\n");
            } else {
                printf("[DEBUG] ERROR: Mouse Move FAILED (Driver/Logitech not ready).\n");
            }
        }
        */

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        float ScreenWidth = io.DisplaySize.x;
        float ScreenHeight = io.DisplaySize.y;

        // Sync Camera right before rendering for buttery smooth signal_overlay
        if (is_authenticated) {
            telemetryContext::UpdateCamera();
        }

        std::vector<PlayerData> localPlayers;
        {
            std::lock_guard<std::mutex> lock(G_PlayersMutex);
            localPlayers = G_Players;
        }
        if (esp_toggle && is_authenticated) telemetryContext::SyncLivePlayers(localPlayers);

        Vector2 local_feet_s;
        bool hasLocalS = false;
        if (is_authenticated) {
            hasLocalS = telemetryContext::WorldToScreen(G_LocalPlayerPos, local_feet_s);
        }

        float ScreenCenterX = ScreenWidth / 2.0f;
        float ScreenCenterY = ScreenHeight / 2.0f;
        ImDrawList* draw = ImGui::GetBackgroundDrawList();

        // --- LICENSE STATUS WATERMARK (If INACTIVE) ---
        if (!is_authenticated) {
            DrawLicenseWatermark(draw);
        }

        // --- GLOBAL SPECTATOR WARNING ---
        if (is_authenticated) {
            DrawGlobalSpectatorWarning(draw, ScreenWidth);
        }

        // --- 0. RADAR (MINI MAP + WORLD MAP) ---
        if (is_authenticated && esp_toggle && radar_enabled) {
            RadarOverlayRenderer::Draw(draw, *this, localPlayers);
        }

        // --- 0. PLAYER ESP ---
        RenderPlayersAndAim(draw, localPlayers, local_feet_s, hasLocalS, ScreenCenterX, ScreenCenterY, ScreenHeight, is_authenticated);

        // --- 1.5. ITEM & VEHICLE RENDERING (MERGED ENGINE) ---
        if (is_authenticated && esp_toggle) {
            RenderLootEsp(draw);
        }

        // --- 1.4.1 ADMIN DEBUG ACTOR ESP (Draw Class Names at World Positions) ---
        if (is_authenticated) {
            RenderAdminDebugEsp(draw);
        }

        // --- 1.5 MACRO OSD (USER REQUEST) ---
        if (is_authenticated) {
            RenderMacroOsd(draw, ScreenWidth, ScreenHeight);
        }

        // --- 1.6 VIVID SPECTATOR & THREAT LIST ---
        if (is_authenticated) {
            RenderSpectatorThreatList(draw, localPlayers, ScreenWidth, ScreenHeight);
        }

        // --- 1.6 STEAM PROOF STATUS INDICATOR (WARNING ONLY) ---
        DrawAntiScreenshotWarning(draw, ScreenHeight);

        // --- 3. MENU REDESIGN ---
        RenderMainMenuWindow(draw, ScreenWidth, ScreenHeight);

        ImGui::Render();
        const float clear_color[4] = { 0, 0, 0, 0 };
        if (!g_pd3dDeviceContext || !g_mainRenderTargetView) return;

        ID3D11RenderTargetView* previous_render_target = nullptr;
        ID3D11DepthStencilView* previous_depth_stencil = nullptr;
        g_pd3dDeviceContext->OMGetRenderTargets(1, &previous_render_target, &previous_depth_stencil);

        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        if (g_clearBeforeRender) {
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        }
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pd3dDeviceContext->OMSetRenderTargets(1, &previous_render_target, previous_depth_stencil);
        if (previous_render_target) previous_render_target->Release();
        if (previous_depth_stencil) previous_depth_stencil->Release();

        if (g_presentAfterRender && g_pSwapChain) {
            g_pSwapChain->Present(1, 0);
        }

    } catch (...) {}
}

void OverlayMenu::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
}

void OverlayMenu::UpdateAntiScreenshot() {
    // Passive visualization does not modify display-affinity or extended
    // window styles. Screenshot/privacy policy belongs to the owning bridge.
}

void OverlayMenu::Doprecision_calibration() {
    // CONTENT REMOVED - LOGIC INTEGRATED INTO RenderFrame for Performance
}
