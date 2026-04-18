#include <winsock2.h>
#include "overlay_menu.hpp"
#include "colors.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include <d3d11.h>
#include <dwmapi.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include "translation.hpp"
#include "../sdk/context.hpp"
#include "../sdk/math.hpp"
#include "../sdk/offsets.hpp"
#include "../sdk/health.hpp"
#include "../sdk/Utils/MacroEngine.h"
#include "../sdk/skCrypt.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

// DirectX 11 Globals
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

#define STB_IMAGE_IMPLEMENTATION
#include "../sdk/Utils/stb_image.h"
#include <map>

struct TextureInfo {
    ID3D11ShaderResourceView* SRV = nullptr;
    int Width = 0;
    int Height = 0;
};
static std::map<std::string, TextureInfo> WeaponImages;

static bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;   

    ID3D11Texture2D *pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    if (FAILED(g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture))) {
        stbi_image_free(image_data);
        return false;
    }

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if (FAILED(g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv))) {
        pTexture->Release();
        stbi_image_free(image_data);
        return false;
    }
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);
    return true;
}

static TextureInfo* GetWeaponImage(std::string weaponName) {
    if (WeaponImages.find(weaponName) != WeaponImages.end()) {
        return &WeaponImages[weaponName];
    }
    ID3D11ShaderResourceView* srv = nullptr;
    int w, h;
    
    std::string path1 = "Assets/Weapon/" + weaponName + ".png";
    std::string path2 = "../Assets/Weapon/" + weaponName + ".png";
    std::string path3 = "../../Assets/Weapon/" + weaponName + ".png";
    
    if (LoadTextureFromFile(path1.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(path2.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(path3.c_str(), &srv, &w, &h)) {
        WeaponImages[weaponName] = {srv, w, h};
        return &WeaponImages[weaponName];
    }
    WeaponImages[weaponName] = {nullptr, 0, 0}; // Fallback empty if file completely missing
    return &WeaponImages[weaponName];
}


// External globals from context
extern std::vector<PlayerData> G_Players;
extern std::vector<ItemData> CachedItems;

OverlayMenu g_Menu;

// --- UTILS FROM VALORANT ---

void OverlayMenu::SetClickable(bool state) {
    if (!target_hwnd) return;
    LONG_PTR flags = GetWindowLongPtr(target_hwnd, GWL_EXSTYLE);
    if (state)
        SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, flags & ~WS_EX_TRANSPARENT);
    else
        SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, flags | WS_EX_TRANSPARENT);
}

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
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, nullptr, &g_pd3dDeviceContext);
    if (res != S_OK) return false;
    CreateRenderTarget();
    return true;
}

void OverlayMenu::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void OverlayMenu::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void OverlayMenu::CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

HWND OverlayMenu::FindWindowManual(const char* className) {
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd) {
        char cls[256]; GetClassNameA(hwnd, cls, 256);
        if (strcmp(cls, className) == 0) return hwnd;
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return NULL;
}

HWND OverlayMenu::FindOverlayForGame(HWND game_hwnd) {
    // Không dùng Discord hijack nữa, tự tạo window
    return NULL; 
}

void OverlayMenu::Initialize(HWND game_hwnd) {
    // Tự động sinh tên Class và Title ngẫu nhiên (1-15 ký tự) để chống bị quét
    wchar_t randClass[16] = {0};
    wchar_t randTitle[16] = {0};
    for (int i = 0; i < 15; i++) randClass[i] = (wchar_t)((rand() % 26) + 'a');
    for (int i = 0; i < 15; i++) randTitle[i] = (wchar_t)((rand() % 26) + 'A');

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, randClass, NULL };
    RegisterClassExW(&wc);
    
    // Tạo cửa sổ siêu trong suốt và chìm trên cùng màn hình
    target_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
        randClass, randTitle, 
        WS_POPUP, 
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), 
        NULL, NULL, wc.hInstance, NULL
    );

    if (!target_hwnd) {
        std::cout << "[-] Failed to create overlay window.\n";
        return;
    }

    SetLayeredWindowAttributes(target_hwnd, 0, 255, LWA_ALPHA);

    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(target_hwnd, &margin);

    ShowWindow(target_hwnd, SW_SHOWDEFAULT);
    UpdateWindow(target_hwnd);

    UpdateAntiScreenshot();

    if (!CreateDeviceD3D(target_hwnd)) return;

    ImGui::CreateContext();
    SetupStyle();
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    for (int i = 0; i < 9; i++) {
        aim_configs[i].enabled = true;
        aim_configs[i].fov = 10.0f;
        aim_configs[i].smooth = 5.0f;
        aim_configs[i].bone = 6;
        aim_configs[i].key = VK_RBUTTON;
        aim_configs[i].max_dist = 400.0f;
        aim_configs[i].prediction = true;

        if (i == 0 || i == 3 || i == 4) aim_configs[i].max_dist = 70.0f;
        else if (i == 1) { aim_configs[i].smooth = 1.6f; aim_configs[i].fov = 8.0f; aim_configs[i].max_dist = 500.0f; }
        else if (i == 2) { aim_configs[i].smooth = 2.5f; aim_configs[i].fov = 12.0f; aim_configs[i].max_dist = 250.0f; }
        else if (i == 5 || i == 6) aim_configs[i].max_dist = 30.0f;
    }

    LoadConfig("dataMacro/Config/settings.json");
    SetClickable(showmenu);
}

void OverlayMenu::SetupStyle() {
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 0.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(10, 10);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.08f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.8f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 0.8f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 0.8f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.0f, 1.0f, 0.8f, 0.3f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 1.0f, 0.8f, 0.5f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 1.0f, 0.8f, 0.7f);
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 1.0f, 0.8f, 0.4f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 1.0f, 0.8f, 0.6f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 1.0f, 0.8f, 0.8f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 1.0f, 0.8f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.8f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
}

ImU32 GetTeamColor(int teamID) {
    return PubgColors::GetTeamColor(teamID);
}

static void DrawRadarTeamMarker(ImDrawList* draw, float x, float y, int teamID, ImU32 color, float radius) {
    draw->AddCircleFilled(ImVec2(x, y), radius + 1.2f, IM_COL32(0, 0, 0, 210), 20);
    draw->AddCircleFilled(ImVec2(x, y), radius, color, 20);

    if (teamID > 0) {
        char teamText[16];
        sprintf_s(teamText, "%d", teamID % 100);
        ImVec2 textSize = ImGui::CalcTextSize(teamText);
        draw->AddText(ImVec2(x - textSize.x * 0.5f + 1.0f, y - textSize.y * 0.5f + 1.0f), IM_COL32(0, 0, 0, 255), teamText);
        draw->AddText(ImVec2(x - textSize.x * 0.5f, y - textSize.y * 0.5f), IM_COL32(255, 255, 255, 255), teamText);
    }
}

static ImVec2 GetPaodMiniMapCenter(float screenWidth, float screenHeight, bool expanded) {
    if (!expanded) {
        if (screenWidth == 1280.0f && screenHeight == 720.0f) return ImVec2(0.9136f, 0.8546f);
        if (screenWidth == 1280.0f && screenHeight == 768.0f) return ImVec2(0.9078f, 0.8544f);
        if (screenWidth == 1280.0f && screenHeight == 800.0f) return ImVec2(0.9038f, 0.8546f);
        if (screenWidth == 1360.0f && screenHeight == 768.0f) return ImVec2(0.9136f, 0.8548f);
        if (screenWidth == 1366.0f && screenHeight == 768.0f) return ImVec2(0.9136f, 0.8548f);
        if (screenWidth == 1440.0f && screenHeight == 900.0f) return ImVec2(0.9040f, 0.8546f);
        if (screenWidth == 1600.0f && screenHeight == 900.0f) return ImVec2(0.9138f, 0.8546f);
        if (screenWidth == 1680.0f && screenHeight == 1050.0f) return ImVec2(0.9038f, 0.8549f);
        if (screenWidth == 1728.0f && screenHeight == 1080.0f) return ImVec2(0.9063f, 0.8539f);
        if (screenWidth == 1920.0f && screenHeight == 1080.0f) return ImVec2(0.9140f, 0.8550f);
        if (screenWidth == 1920.0f && screenHeight == 1200.0f) return ImVec2(0.90478f, 0.8535f);
        if (screenWidth == 2560.0f && screenHeight == 1080.0f) return ImVec2(0.936585f, 0.8519f);
        if (screenWidth == 2560.0f && screenHeight == 1440.0f) return ImVec2(0.91424f, 0.85559f);
        if (screenWidth == 2560.0f && screenHeight == 1600.0f) return ImVec2(0.915490f, 0.852890f);
        if (screenWidth == 3440.0f && screenHeight == 1440.0f) return ImVec2(0.937135f, 0.8525f);
        if (screenWidth == 3840.0f && screenHeight == 2160.0f) return ImVec2(0.914402f, 0.8554f);
        return ImVec2(0.914402f, 0.8554f);
    }

    if (screenWidth == 1280.0f && screenHeight == 720.0f) return ImVec2(0.861323f, 0.761643f);
    if (screenWidth == 1280.0f && screenHeight == 768.0f) return ImVec2(0.851852f, 0.761865f);
    if (screenWidth == 1280.0f && screenHeight == 800.0f) return ImVec2(0.8456486f, 0.761643f);
    if (screenWidth == 1360.0f && screenHeight == 768.0f) return ImVec2(0.8613228f, 0.761821f);
    if (screenWidth == 1366.0f && screenHeight == 768.0f) return ImVec2(0.8613428f, 0.761821f);
    if (screenWidth == 1440.0f && screenHeight == 900.0f) return ImVec2(0.8455022f, 0.761643f);
    if (screenWidth == 1600.0f && screenHeight == 900.0f) return ImVec2(0.8615114f, 0.761643f);
    if (screenWidth == 1680.0f && screenHeight == 1050.0f) return ImVec2(0.8469486f, 0.761910f);
    if (screenWidth == 1728.0f && screenHeight == 1080.0f) return ImVec2(0.8493f, 0.7607f);
    if (screenWidth == 1920.0f && screenHeight == 1080.0f) return ImVec2(0.8617f, 0.762f);
    if (screenWidth == 1920.0f && screenHeight == 1200.0f) return ImVec2(0.84677f, 0.762285f);
    if (screenWidth == 2560.0f && screenHeight == 1080.0f) return ImVec2(0.897178f, 0.760035f);
    if (screenWidth == 2560.0f && screenHeight == 1440.0f) return ImVec2(0.862473f, 0.762785f);
    if (screenWidth == 2560.0f && screenHeight == 1600.0f) return ImVec2(0.863273f, 0.759935f);
    if (screenWidth == 3440.0f && screenHeight == 1440.0f) return ImVec2(0.897929f, 0.759484f);
    if (screenWidth == 3840.0f && screenHeight == 2160.0f) return ImVec2(0.862393f, 0.762185f);
    return ImVec2(0.862393f, 0.762185f);
}

static float GetPaodMiniMapDiv(float screenWidth, float screenHeight, bool expanded) {
    if (!expanded) {
        if (screenWidth == 1728.0f && screenHeight == 1080.0f) return 122.0f;
    } else {
        if (screenWidth == 1728.0f && screenHeight == 1080.0f) return 132.0f;
    }

    if (screenWidth == 1280.0f && screenHeight == 720.0f) return 86.0f;
    if (screenWidth == 1280.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1280.0f && screenHeight == 800.0f) return 96.0f;
    if (screenWidth == 1360.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1366.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1440.0f && screenHeight == 900.0f) return 104.0f;
    if (screenWidth == 1600.0f && screenHeight == 900.0f) return 104.0f;
    if (screenWidth == 1680.0f && screenHeight == 1050.0f) return 126.0f;
    if (screenWidth == 1920.0f && screenHeight == 1080.0f) return 126.0f;
    if (screenWidth == 1920.0f && screenHeight == 1200.0f) return 146.0f;
    if (screenWidth == 2560.0f && screenHeight == 1080.0f) return 128.0f;
    if (screenWidth == 2560.0f && screenHeight == 1440.0f) return 175.0f;
    if (screenWidth == 2560.0f && screenHeight == 1600.0f) return 172.0f;
    if (screenWidth == 3440.0f && screenHeight == 1440.0f) return 158.0f;
    if (screenWidth == 3840.0f && screenHeight == 2160.0f) return 258.0f;
    return 258.0f;
}

static bool HasPaodMiniMapProfile(float screenWidth, float screenHeight) {
    return
        (screenWidth == 1280.0f && screenHeight == 720.0f) ||
        (screenWidth == 1280.0f && screenHeight == 768.0f) ||
        (screenWidth == 1280.0f && screenHeight == 800.0f) ||
        (screenWidth == 1360.0f && screenHeight == 768.0f) ||
        (screenWidth == 1366.0f && screenHeight == 768.0f) ||
        (screenWidth == 1440.0f && screenHeight == 900.0f) ||
        (screenWidth == 1600.0f && screenHeight == 900.0f) ||
        (screenWidth == 1680.0f && screenHeight == 1050.0f) ||
        (screenWidth == 1728.0f && screenHeight == 1080.0f) ||
        (screenWidth == 1920.0f && screenHeight == 1080.0f) ||
        (screenWidth == 1920.0f && screenHeight == 1200.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1080.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1440.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1600.0f) ||
        (screenWidth == 3440.0f && screenHeight == 1440.0f) ||
        (screenWidth == 3840.0f && screenHeight == 2160.0f);
}

void OverlayMenu::RenderFrame() {
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
                if (GetAsyncKeyState(i) & 0x8000) { any_down = true; break; }
            }
            if (!any_down) need_to_release = false;
        }

        // Only search for NEW key if previous click is released
        if (!need_to_release) {
            for (int i = 0x01; i <= 0xFE; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i == VK_ESCAPE) { waiting_for_key = nullptr; last_waiting = nullptr; break; }
                    
                    *waiting_for_key = i;
                    waiting_for_key = nullptr;
                    last_waiting = nullptr;
                    // Wait for release
                    while (GetAsyncKeyState(i) & 0x8000) Sleep(1); 
                    break;
                }
            }
        }
    } else {
        need_to_release = false;
    }
    try {
        if (!target_hwnd) return;

        // --- TOGGLE MENU (F5) ---
        if (GetAsyncKeyState(VK_F5) & 1) {
            showmenu = !showmenu;
            SetClickable(showmenu);
            if (showmenu) {
                SetForegroundWindow(target_hwnd);
            }
        }

        // --- MOUSE INJECTION (FOR HIJACKED WINDOW) ---
        ImGuiIO& io = ImGui::GetIO();
        if (showmenu) {
            POINT p; GetCursorPos(&p);
            ScreenToClient(target_hwnd, &p);
            io.MousePos = ImVec2((float)p.x, (float)p.y);
            io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        }

        // --- WINDOW MESSAGE HANDLING (VALORANT STYLE) ---
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 1-2 (Weapon), X (Holster), Tab (Bag), M (Map), G (Grenade), F (Interact/Equip)
        if ((GetAsyncKeyState('1') & 1) || (GetAsyncKeyState('2') & 1) || (GetAsyncKeyState('X') & 1) || 
            (GetAsyncKeyState(VK_TAB) & 1) || (GetAsyncKeyState('M') & 1) || (GetAsyncKeyState('G') & 1) || (GetAsyncKeyState('F') & 1)) {
            MacroEngine::ForceScan();
        }

        /*
        // --- MOUSE TEST KEY (F8) ---
        if (GetAsyncKeyState(VK_F8) & 1) {
            printf("[DEBUG] F8 pressed: Attempting mouse movement test...\n");
            bool success = true;
            for (int i = 0; i < 20; i++) {
                if (!PubgMemory::MoveMouse(0, 15, 0)) {
                    success = false;
                    break;
                }
                Sleep(2);
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
        
        // Sync Camera right before rendering for buttery smooth ESP
        PubgContext::UpdateCamera();

        std::vector<PlayerData> localPlayers = G_Players;
        if (esp_toggle) PubgContext::SyncLivePlayers(localPlayers);
        
        Vector2 local_feet_s;
        bool hasLocalS = PubgContext::WorldToScreen(G_LocalPlayerPos, local_feet_s);

        float ScreenCenterX = ScreenWidth / 2.0f;
        float ScreenCenterY = ScreenHeight / 2.0f;
        ImDrawList* draw = ImGui::GetBackgroundDrawList();

        // --- 0. RADAR (MINI MAP + WORLD MAP) ---
        if (esp_toggle) {
            const bool expandedMiniMap = G_Radar.SelectMinimapSizeIndex > 0;
            const bool hasHudMiniMapRect = G_Radar.ScreenPosX > 1.0f && G_Radar.ScreenPosY > 1.0f &&
                                           G_Radar.ScreenSize > 40.0f && G_Radar.ScreenSizeY > 40.0f;
            const bool hasPaodProfile = HasPaodMiniMapProfile(ScreenWidth, ScreenHeight);
            float centerX = 0.0f;
            float centerY = 0.0f;
            float mapDiv = 0.0f;
            float miniLeft = 0.0f;
            float miniTop = 0.0f;
            float miniRight = 0.0f;
            float miniBottom = 0.0f;
            if (hasHudMiniMapRect && hasPaodProfile) {
                // PAOD uses calibrated center/div per resolution (accounts for minimap inner padding).
                const ImVec2 paodMiniMapCenter = GetPaodMiniMapCenter(ScreenWidth, ScreenHeight, expandedMiniMap);
                centerX = ScreenWidth * paodMiniMapCenter.x + g_Menu.radar_offset_x;
                centerY = ScreenHeight * paodMiniMapCenter.y + g_Menu.radar_offset_y;
                mapDiv = GetPaodMiniMapDiv(ScreenWidth, ScreenHeight, expandedMiniMap) * g_Menu.radar_zoom_multiplier;
                miniLeft = centerX - mapDiv;
                miniTop = centerY - mapDiv;
                miniRight = centerX + mapDiv;
                miniBottom = centerY + mapDiv;
            } else if (hasHudMiniMapRect) {
                const float radarW = G_Radar.ScreenSize;
                const float radarH = G_Radar.ScreenSizeY;
                centerX = G_Radar.ScreenPosX + radarW * 0.5f + g_Menu.radar_offset_x;
                centerY = G_Radar.ScreenPosY + radarH * 0.5f + g_Menu.radar_offset_y;
                const float radarMin = (radarW < radarH) ? radarW : radarH;
                mapDiv = radarMin * 0.5f * g_Menu.radar_zoom_multiplier;
                miniLeft = G_Radar.ScreenPosX;
                miniTop = G_Radar.ScreenPosY;
                miniRight = G_Radar.ScreenPosX + radarW;
                miniBottom = G_Radar.ScreenPosY + radarH;
            } else {
                const ImVec2 paodMiniMapCenter = GetPaodMiniMapCenter(ScreenWidth, ScreenHeight, expandedMiniMap);
                centerX = ScreenWidth * paodMiniMapCenter.x + g_Menu.radar_offset_x;
                centerY = ScreenHeight * paodMiniMapCenter.y + g_Menu.radar_offset_y;
                mapDiv = GetPaodMiniMapDiv(ScreenWidth, ScreenHeight, expandedMiniMap) * g_Menu.radar_zoom_multiplier;
                miniLeft = centerX - mapDiv;
                miniTop = centerY - mapDiv;
                miniRight = centerX + mapDiv;
                miniBottom = centerY + mapDiv;
            }
            if (mapDiv < 50.0f) mapDiv = 50.0f;

            float dt = (GetTickCount64() - G_LastScanTime) / 1000.0f;
            if (dt > 2.0f) dt = 0.0f;

            if (hasHudMiniMapRect || G_Radar.IsMiniMapVisible) {
                const Vector3 localP = G_LocalPlayerPos;
                const float worldRange = expandedMiniMap ? 37000.0f : 20000.0f;
                for (const auto& player : localPlayers) {
                    if (player.IsTeammate) {
                        if (!g_Menu.esp_show_teammates) continue;
                    } else {
                        if (!g_Menu.esp_show_enemies) continue;
                    }

                    const Vector3 enemyP = player.Position + (player.Velocity * dt);
                    const float deltaX = enemyP.x - localP.x;
                    const float deltaY = enemyP.y - localP.y;
                    if (deltaX > worldRange || deltaX < -worldRange || deltaY > worldRange || deltaY < -worldRange) {
                        continue;
                    }

                    const float finalX = roundf(deltaX / 20000.0f * mapDiv) + centerX;
                    const float finalY = roundf(deltaY / 20000.0f * mapDiv) + centerY;

                    const float clampedX = std::clamp(finalX, miniLeft + 3.0f, miniRight - 3.0f);
                    const float clampedY = std::clamp(finalY, miniTop + 3.0f, miniBottom - 3.0f);

                    ImU32 teamColor = GetTeamColor(player.TeamID);
                    DrawRadarTeamMarker(draw, clampedX, clampedY, player.TeamID, teamColor, 4.5f);
                }

                if (g_Menu.show_radar_center) {
                    draw->AddLine(ImVec2(centerX - 10.0f, centerY), ImVec2(centerX + 10.0f, centerY), IM_COL32(255, 255, 255, 255), 1.0f);
                    draw->AddLine(ImVec2(centerX, centerY - 10.0f), ImVec2(centerX, centerY + 10.0f), IM_COL32(255, 255, 255, 255), 1.0f);
                }
            }

            const bool keyWorldMapPressed = (GetAsyncKeyState('M') & 0x8000) != 0;
            const bool canDrawWorldMap = (G_Radar.IsWorldMapVisible || keyWorldMapPressed) &&
                G_Radar.MapSizeFactored > 1.0f &&
                G_Radar.MapWorldSize > 0.0f;
            if (canDrawWorldMap) {
                float worldLeft = G_Radar.WorldMapX;
                float worldTop = G_Radar.WorldMapY;
                float worldRight = worldLeft + G_Radar.WorldMapWidth;
                float worldBottom = worldTop + G_Radar.WorldMapHeight;
                if (!G_Radar.IsWorldMapVisible || G_Radar.WorldMapWidth <= 100.0f || G_Radar.WorldMapHeight <= 100.0f) {
                    worldLeft = (ScreenWidth - ScreenHeight) * 0.5f;
                    worldTop = 0.0f;
                    worldRight = worldLeft + ScreenHeight;
                    worldBottom = ScreenHeight;
                }

                for (const auto& player : localPlayers) {
                    if (player.IsTeammate) {
                        if (!g_Menu.esp_show_teammates) continue;
                    } else {
                        if (!g_Menu.esp_show_enemies) continue;
                    }

                    const Vector3 worldLocation = {
                        player.Position.x + G_Radar.WorldOriginLocation.x,
                        player.Position.y + G_Radar.WorldOriginLocation.y,
                        0.0f
                    };

                    const float radarPosX = worldLocation.x - G_Radar.WorldCenterLocation.x;
                    const float radarPosY = worldLocation.y - G_Radar.WorldCenterLocation.y;

                    const float mapX = ScreenCenterX + (radarPosX / G_Radar.MapSizeFactored) * (ScreenHeight * 0.5f);
                    const float mapY = ScreenCenterY + (radarPosY / G_Radar.MapSizeFactored) * (ScreenHeight * 0.5f);
                    if (mapX < worldLeft || mapX > worldRight || mapY < worldTop || mapY > worldBottom) {
                        continue;
                    }

                    ImU32 teamColor = GetTeamColor(player.TeamID);
                    DrawRadarTeamMarker(draw, mapX, mapY, player.TeamID, teamColor, 5.5f);
                }
            }
        }

        // --- 0. AIMBOT & MACRO SYNC ---
        // Weapon Check: Only aim if holding a weapon
        bool isHolstered = (MacroEngine::current_weapon_name == "" || MacroEngine::current_weapon_name == "None" || MacroEngine::current_weapon_name == "Holstered");
        int category = (int)MacroEngine::current_category; // CAT_AR=0...CAT_NONE=8
        
        // Pick the active category config (Default GLOBAL=8 if not matched)
        AimConfig& activeConfig = aim_configs[category < 9 ? category : 8];
        int activeKey = activeConfig.key;

        bool canAim = aim_master_enabled && !isHolstered;

        // --- ADAPTIVE FOV (Scale based on Scope) ---
        float fovScale = 1.0f;
        if (aim_adaptive_fov) {
            int sc = MacroEngine::current_scope;
            if (sc == 3)      fovScale = 0.85f; // 2x
            else if (sc == 4) fovScale = 0.65f; // 3x
            else if (sc == 5) fovScale = 0.50f; // 4x
            else if (sc == 6) fovScale = 0.35f; // 6x
            else if (sc == 7) fovScale = 0.25f; // 8x
            else if (sc == 8) fovScale = 0.15f; // 15x
        }
        float finalFov = activeConfig.fov * fovScale;

        PlayerData* bestTarget = nullptr;
        Vector2 bestScreenPos = { 0, 0 };
        float closestDist = finalFov * 8.0f; // Reset each frame

        if (canAim && activeConfig.enabled) {
            draw->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), finalFov * 8.0f, ImColor(255, 255, 255, 60), 64, 1.0f);
        }

        // --- 1. VISUALS (ESP) & AIMBOT CORE ---
        float dt_esp = (GetTickCount64() - G_LastScanTime) / 1000.0f;
        if (!g_Menu.esp_skel_interp) dt_esp = 0.0f;
        else if (dt_esp > 0.10f) dt_esp = 0.10f;

        for (const auto& player_ref : localPlayers) {
            auto& player = const_cast<PlayerData&>(player_ref); 

            if (player.Distance > render_distance) continue;
            if (player.Distance > activeConfig.max_dist) continue; // CATEGORY DISTANCE CHECK
            if (player.IsTeammate) {
                if (!g_Menu.esp_show_teammates) continue;
            } else {
                if (!g_Menu.esp_show_enemies) continue;
            }
            
            // AIMBOT FILTER
            bool isAimbotTarget = canAim && activeConfig.enabled;

            Vector3 delta = player.Velocity * dt_esp; 

            // --- INTEGRATED AIMBOT TARGET CHECK (Optimization) ---
            if (isAimbotTarget && (GetAsyncKeyState(activeKey) & 0x8000)) {
                if (!player.IsTeammate && player.Health > 0) {
                    if (!aim_visible_only || player.IsVisible) {
                        // SMART-BONE LOGIC: Check Head, Neck, Chest on the player and pick the one closest to crosshair
                        struct BonePos { int id; Vector3 world; Vector2 screen; float dist; };
                        std::vector<BonePos> bones_to_check;
                        
                        Vector3 scanBones[] = { 
                            player.Bone_Head, player.Bone_Neck, player.Bone_Chest, player.Bone_Pelvis,
                            player.Bone_LShoulder, player.Bone_LElbow, player.Bone_LHand,
                            player.Bone_RShoulder, player.Bone_RElbow, player.Bone_RHand,
                            player.Bone_LThigh, player.Bone_LKnee, player.Bone_LFoot,
                            player.Bone_RThigh, player.Bone_RKnee, player.Bone_RFoot
                        };
                        int boneIDs[] = { 6, 5, 2, 1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 6 is HEAD

                        for (int b = 0; b < 16; b++) {
                            Vector2 s;
                            if (PubgContext::WorldToScreen(scanBones[b], s)) {
                                float dx = s.x - ScreenCenterX;
                                float dy = s.y - ScreenCenterY;
                                bones_to_check.push_back({ boneIDs[b], scanBones[b], s, sqrtf(dx*dx + dy*dy) });
                            }
                        }

                        if (!bones_to_check.empty()) {
                            // Sort by distance from crosshair
                            std::sort(bones_to_check.begin(), bones_to_check.end(), [](const BonePos& a, const BonePos& b) {
                                return a.dist < b.dist;
                            });

                            // SHIFT OVERRIDE: If holding Shift, override the choice to HEAD (ID 6)
                            BonePos selected = bones_to_check[0];
                            if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                                for (auto& bp : bones_to_check) { if (bp.id == 6) { selected = bp; break; } }
                            }

                            Vector3 targetWorld = selected.world;
                            Vector2 targetScreen = selected.screen;

                            if (activeConfig.prediction) {
                                float travelTime = player.Distance / 800.0f;
                                targetWorld += (player.Velocity * travelTime);
                                // Refresh screen pos with prediction
                                PubgContext::WorldToScreen(targetWorld, targetScreen);
                            }

                            float finalDist = selected.dist; // Use the original screen distance for filter
                            if (finalDist < closestDist) {
                                closestDist = finalDist;
                                bestTarget = &player;
                                bestScreenPos = targetScreen;
                            }
                        }
                    }
                }
            }

            if (esp_toggle) {
                Vector2 head_s, feet_s;
                if (PubgContext::WorldToScreen(player.HeadPosition + delta, head_s) &&
                    PubgContext::WorldToScreen(player.FeetPosition + delta, feet_s)) {

                    float h = abs(head_s.y - feet_s.y);
                    float w = h * 0.65f; // Tăng chiều rộng để bao phủ súng/balo
                    
                    float boxTop = head_s.y - (h * 0.18f); // Kéo cao đỉnh Box lên 18% để không cắt ngang trán / mũ 3
                    float boxBottom = feet_s.y + (h * 0.06f);
                    
                    float alphaMult = 1.0f;
                    if (g_Menu.esp_distance_lod) {
                        float fadeStart = (float)g_Menu.render_distance * 0.65f;
                        if (player.Distance > fadeStart) {
                           alphaMult = 1.0f - ((player.Distance - fadeStart) / ((float)g_Menu.render_distance - fadeStart));
                           if (alphaMult < 0.15f) alphaMult = 0.15f; // Keep a faint ghost for visibility
                        }
                    }

                    ImU32 color = player.IsVisible ? PubgColors::Visible : PubgColors::Invisible;
                    if (player.IsTeammate) color = PubgColors::Teammate;

                    // Apply Alpha Scaling
                    auto ApplyAlpha = [&](ImU32 col, float mult) -> ImU32 {
                        int a = (int)((col >> 24) & 0xFF);
                        a = (int)(a * mult);
                        return (col & 0x00FFFFFF) | (a << 24);
                    };

                    ImU32 boxCol = player.IsVisible ? 
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color) : 
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_invisible_color);
                    
                    if (player.IsTeammate) boxCol = PubgColors::Teammate;
                    boxCol = ApplyAlpha(boxCol, alphaMult);

                    if (esp_box && player.Distance < g_Menu.box_max_dist) {
                        // Drawing Box with Border & Soft Rounding
                        draw->AddRect(ImVec2(head_s.x - w/2, boxTop), ImVec2(head_s.x + w/2, boxBottom), IM_COL32(0,0,0,(int)(180 * alphaMult)), 3.5f, 0, 2.5f); // Border
                        draw->AddRect(ImVec2(head_s.x - w/2, boxTop), ImVec2(head_s.x + w/2, boxBottom), boxCol, 3.5f, 0, 1.25f);       // Main Box
                    }
                    
                    // --- HEALTH BAR (KAKA STYLE) ---
                    if (g_Menu.esp_health && player.Distance < g_Menu.hp_max_dist) {
                        float displayHealth = player.IsGroggy ? player.GroggyHealth : player.Health;
                        float healthPercent = displayHealth / 100.0f;
                        if (healthPercent > 1.0f) healthPercent = 1.0f;
                        if (healthPercent < 0.0f) healthPercent = 0.0f;
                        
                        ImU32 hpColor = IM_COL32(0, 255, 0, 255);
                        if (healthPercent < 0.7f) hpColor = IM_COL32(255, 255, 0, 255);
                        if (healthPercent < 0.3f) hpColor = IM_COL32(255, 0, 0, 255);

                        float barH = (boxBottom - boxTop) * healthPercent;
                        draw->AddRectFilled(ImVec2(head_s.x - w/2 - 6, boxTop), ImVec2(head_s.x - w/2 - 3, boxBottom), IM_COL32(0,0,0,(int)(150 * alphaMult)));
                        draw->AddRectFilled(ImVec2(head_s.x - w/2 - 6, boxBottom - barH), ImVec2(head_s.x - w/2 - 3, boxBottom), ApplyAlpha(hpColor, alphaMult));
                    }

                    if (g_Menu.esp_skeleton && player.Distance < g_Menu.skeleton_max_dist) {
                        auto DrawLine = [&](Vector3 b1, Vector3 b2) {
                            if (b1.IsZero() || b2.IsZero()) return;
                            Vector2 s1, s2;
                            if (PubgContext::WorldToScreen(b1 + delta, s1) && PubgContext::WorldToScreen(b2 + delta, s2)) {
                                ImU32 skelCol = player.IsVisible ? 
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color) : 
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_invisible_color);
                                
                                draw->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), ApplyAlpha(skelCol, alphaMult), g_Menu.skel_thickness);
                            }
                        };
                        DrawLine(player.Bone_Head, player.Bone_Neck);
                        DrawLine(player.Bone_Neck, player.Bone_Chest);
                        DrawLine(player.Bone_Chest, player.Bone_Pelvis);
                        DrawLine(player.Bone_Chest, player.Bone_LShoulder);
                        DrawLine(player.Bone_LShoulder, player.Bone_LElbow);
                        DrawLine(player.Bone_LElbow, player.Bone_LHand);
                        DrawLine(player.Bone_Chest, player.Bone_RShoulder);
                        DrawLine(player.Bone_RShoulder, player.Bone_RElbow);
                        DrawLine(player.Bone_RElbow, player.Bone_RHand);
                        DrawLine(player.Bone_Pelvis, player.Bone_LThigh);
                        DrawLine(player.Bone_LThigh, player.Bone_LKnee);
                        DrawLine(player.Bone_LKnee, player.Bone_LFoot);
                        DrawLine(player.Bone_Pelvis, player.Bone_RThigh);
                        DrawLine(player.Bone_RThigh, player.Bone_RKnee);
                        DrawLine(player.Bone_RKnee, player.Bone_RFoot);
                    }

                    if (esp_snapline) {
                        ImVec2 start = hasLocalS ? ImVec2(local_feet_s.x, local_feet_s.y) : ImVec2(ScreenCenterX, ScreenHeight);
                        draw->AddLine(start, ImVec2(feet_s.x, feet_s.y), IM_COL32(255, 255, 255, 120), 1.25f);
                    }

                    ImU32 infoColor = player.IsVisible ? PubgColors::Visible : PubgColors::Invisible;
                    if (player.IsTeammate) infoColor = PubgColors::Teammate;
                    if (player.IsGroggy) infoColor = PubgColors::Groggy;

                    std::string infoTag = player.Name;
                    if (infoTag.empty() || infoTag == "Player") infoTag = "Unknown";
                    if (player.IsGroggy) infoTag = "[KNOCKED] " + infoTag;
                    if (player.SpectatedCount > 0) infoTag += " [" + std::to_string(player.SpectatedCount) + " \xef\xbd\xa1]";

                    // 1. KHOẢNG CÁCH DƯỚI CHÂN (Distance at feet)
                    if (g_Menu.esp_distance && player.Distance < g_Menu.distance_txt_max_dist) {
                        char distStr[32];
                        sprintf_s(distStr, sizeof(distStr), "[%dm]", (int)player.Distance);
                        
                        // Adaptive scale (Smaller when far)
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = 13.0f; // Distance is secondary
                        ImVec2 distSize = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, distStr);
                        
                        ImU32 distCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(head_s.x - distSize.x / 2, boxBottom + 2.0f), ApplyAlpha(distCol, alphaMult), distStr);
                    }

                    // 2. VŨ KHÍ NẰM TRÊN ĐẦU (Weapon above head)
                    float currentTopY = boxTop - 2.0f; 

                    if (g_Menu.esp_weapon && !player.WeaponName.empty() && player.Distance < g_Menu.weapon_max_dist) {
                        if (g_Menu.esp_weapon_type == 1) { // IMAGE
                            TextureInfo* tex = GetWeaponImage(player.WeaponName);
                            if (tex && tex->SRV) {
                                float distanceScale = (player.Distance < 5.0f) ? 5.0f : player.Distance;
                                float targetWidth = 1200.0f / distanceScale; 
                                
                                if (targetWidth < 25.0f) targetWidth = 25.0f;
                                if (targetWidth > 80.0f) targetWidth = 80.0f; // Capped for close range

                                float scale = targetWidth / tex->Width;
                                float iconW = tex->Width * scale;
                                float iconH = tex->Height * scale;
                                
                                currentTopY -= iconH; // Subtract height to move upwards
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddImage((ImTextureID)tex->SRV, ImVec2(head_s.x - iconW/2, currentTopY), ImVec2(head_s.x + iconW/2, currentTopY + iconH), ImVec2(0,0), ImVec2(1,1), ApplyAlpha(weaponCol, alphaMult * 0.85f));
                                currentTopY -= 2.0f; // Add padding
                            } else { // Fallback text
                                ImVec2 ws = ImGui::CalcTextSize(player.WeaponName.c_str());
                                currentTopY -= ws.y;
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddText(ImVec2(head_s.x - ws.x/2, currentTopY), ApplyAlpha(weaponCol, alphaMult), player.WeaponName.c_str());
                                currentTopY -= 2.0f;
                            }
                        } else { // TEXT
                        ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                        float baseFontSize = 14.5f;
                        ImVec2 ws = ImGui::GetFont()->CalcTextSizeA(baseFontSize, FLT_MAX, 0.0f, player.WeaponName.c_str());
                        currentTopY -= ws.y;
                        draw->AddText(ImVec2(head_s.x - ws.x/2, currentTopY), ApplyAlpha(weaponCol, alphaMult), player.WeaponName.c_str());
                        currentTopY -= 2.0f;
                        }
                    }

                    // 3. TÊN & TRẠNG THÁI NẰM CÙNG (Name/Status above Weapon)
                    if ((g_Menu.esp_name || player.IsGroggy) && player.Distance < g_Menu.name_max_dist) { 
                        // Adaptive scale
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = 14.5f;
                        ImVec2 ns = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, infoTag.c_str());
                        currentTopY -= ns.y;
                        
                        ImU32 nameCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_color);
                        if (player.IsVisible) nameCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color);
                        if (player.IsGroggy) nameCol = IM_COL32(255, 0, 0, 255);
                        
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(head_s.x - ns.x / 2, currentTopY), ApplyAlpha(nameCol, alphaMult), infoTag.c_str());
                    }
                }
            }
        }

        // --- 1.5. ITEM & VEHICLE RENDERING ---
        if (esp_toggle) {
            for (const auto& item : CachedItems) {
                if (item.Distance <= 0) continue;

                bool should_draw = false;
                ImU32 col = IM_COL32(200, 200, 200, 255); // Default Loot

                if (item.Name == "Vehicle") {
                    if (g_Menu.esp_vehicles && item.Distance < g_Menu.vehicle_max_dist) {
                        should_draw = true; col = IM_COL32(0, 255, 255, 255);
                    }
                } else if (item.Name == "Air Drop") {
                    if (g_Menu.esp_airdrops) { should_draw = true; col = IM_COL32(255, 50, 50, 255); }
                } else if (item.Name == "Dead Box") {
                    if (g_Menu.esp_deadboxes && item.Distance < 200.0f) { should_draw = true; col = IM_COL32(255, 140, 0, 255); }
                } else { // Generic items
                    if (g_Menu.esp_items && item.Distance < g_Menu.loot_max_dist) should_draw = true;
                }

                if (should_draw) {
                    Vector2 itemScreen;
                    if (PubgContext::WorldToScreen(item.Position, itemScreen)) {
                        char itemText[128];
                        sprintf_s(itemText, sizeof(itemText), "%s [%dm]", item.Name.c_str(), (int)item.Distance);
                        ImVec2 tsize = ImGui::CalcTextSize(itemText);
                        draw->AddText(ImVec2(itemScreen.x - tsize.x/2, itemScreen.y), col, itemText);
                    }
                }
            }
        }

        // --- 1.5 MACRO OSD (USER REQUEST) ---
        if (show_macro_overlay && current_scene == Scene::Gaming) {
            char buf[256];
            std::string scopeName = Translation::GetAttachmentName(0, MacroEngine::current_scope);
            std::string muzzleName = Translation::GetAttachmentName(1, MacroEngine::current_muzzle);
            std::string gripName = Translation::GetAttachmentName(2, MacroEngine::current_grip);

            auto Lang = Translation::Get();
            if (MacroEngine::current_weapon_name.empty() || MacroEngine::current_weapon_name == "None") {
                sprintf_s(buf, sizeof(buf), "[ %s ]", Lang.NoWeapon);
            } else {
                sprintf_s(buf, sizeof(buf), "[ %s | %s | %s | %s ]", MacroEngine::current_weapon_name.c_str(), scopeName.c_str(), muzzleName.c_str(), gripName.c_str());
            }

            ImVec2 textSize = ImGui::CalcTextSize(buf);
            float hudX = (ScreenWidth - textSize.x) * 0.5f;
            float hudY = ScreenHeight * 0.85f;

            // Draw shadow for readability
            draw->AddText(ImVec2(hudX + 1, hudY + 1), ImColor(0, 0, 0, 200), buf);
            // Draw main text
            draw->AddText(ImVec2(hudX, hudY), ImColor(macro_overlay_color[0], macro_overlay_color[1], macro_overlay_color[2], macro_overlay_color[3]), buf);
        }

        // --- 1.6 STEAM PROOF STATUS INDICATOR (WARNING ONLY) ---
        if (!anti_screenshot) {
            auto Lang = Translation::Get();
            char statusBuf[128];
            sprintf_s(statusBuf, sizeof(statusBuf), "[ %s: %s ]", Lang.AntiScreenshot, (language == 1) ? "DANG TAT (KHONG AN TOAN)" : "INACTIVE (NOT SAFE)");
            draw->AddText(ImVec2(20, ScreenHeight - 40), ImColor(255, 50, 50, 200), statusBuf);
        }

        // --- 2. APPLY AIMBOT MOVEMENT ---
        // Finalize the mouse movement after all player iteration is DONE.
        if (bestTarget) {
            float moveX = (bestScreenPos.x - ScreenCenterX);
            float moveY = (bestScreenPos.y - ScreenCenterY);

            if (activeConfig.smooth > 1.0f) {
                moveX /= activeConfig.smooth;
                moveY /= activeConfig.smooth;
            }

            PubgMemory::MoveMouse((long)moveX, (long)moveY);
        }

        // --- 3. MENU REDESIGN ---
        if (showmenu) {
            Translation::CurrentLanguage = language;
            auto Lang = Translation::Get();
            
            ImGui::SetNextWindowSize(ImVec2(620, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("##overlay", &showmenu, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
            
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            draw->AddRectFilled(windowPos, ImVec2(windowPos.x + 4, windowPos.y + windowSize.y), IM_COL32(0, 255, 204, 255)); // Side Glow

            // --- LEFT SIDEBAR ---
            ImGui::BeginChild("##Sidebar", ImVec2(150, 0), true);
            {
                ImGui::Spacing();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                
                // --- EASTER EGG TOGGLE FOR STEAM PROOF (6 CLICKS / 5 SEC) ---
                static int clickCount = 0;
                static float lastClickTime = 0;
                
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.8f, 1.0f)); // Force Cyan

                if (ImGui::Selectable("  GZ-CHEAT", false, ImGuiSelectableFlags_None, ImVec2(0, 20))) {
                    float currentTime = (float)ImGui::GetTime();
                    if (currentTime - lastClickTime > 5.0f) clickCount = 0;
                    
                    clickCount++;
                    lastClickTime = currentTime;
                    
                    if (clickCount >= 6) {
                        anti_screenshot = !anti_screenshot;
                        UpdateAntiScreenshot();
                        clickCount = 0;
                    }
                }
                ImGui::PopStyleColor(3);

                if (ImGui::IsItemHovered()) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }

                ImGui::TextDisabled("   External");
                ImGui::PopFont();
                ImGui::Separator();
                ImGui::Spacing();

                auto StyledTab = [&](const char* label, int id) {
                    bool active = (active_tab == id);
                    if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.8f, 0.25f));
                    else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));

                    if (ImGui::Button(label, ImVec2(-1, 40))) active_tab = id;
                    
                    ImGui::PopStyleColor();
                };

                StyledTab(Lang.TabVisuals, 0);
                StyledTab(Lang.TabAimbot, 1);
                StyledTab(Lang.TabMacro, 2);
                StyledTab(Lang.TabRadar, 3);
                StyledTab(Lang.TabLoot, 5);
                StyledTab(Lang.TabSettings, 4);

                // Footer of sidebar
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
                if (ImGui::Button("EXIT", ImVec2(-1, 30))) exit(0);
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // --- MAIN CONTENT AREA ---
            ImGui::BeginChild("##Content", ImVec2(0, 0), false);
            {
                ImGui::Spacing();
                if (active_tab == 0) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "VISUAL PERFORMANCE");
                    ImGui::Separator();
                    ImGui::BeginChild("##VisualsContent");
                    ImGui::Checkbox(Lang.MasterToggle, &esp_toggle);
                    ImGui::Checkbox("Enemy ESP", &g_Menu.esp_show_enemies);
                    ImGui::Checkbox("Teammate ESP", &g_Menu.esp_show_teammates);
                    ImGui::Checkbox(Lang.Box, &esp_box);
                    ImGui::Checkbox(Lang.Skeleton, &esp_skeleton);
                    if (esp_skeleton) {
                        ImGui::SameLine(); ImGui::Checkbox("Interpolate", &g_Menu.esp_skel_interp);
                    }
                    ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
                    ImGui::Checkbox(Lang.Distance, &esp_distance);
                    ImGui::Checkbox("Items & Vehicles", &g_Menu.esp_items);
                    ImGui::Separator();
                    
                    if (ImGui::TreeNode("Distance Tresholds (Culling)")) {
                        ImGui::Checkbox("Enable Smart LOD", &esp_distance_lod);
                        ImGui::SliderInt("Box Max", &box_max_dist, 50, 1000);
                        ImGui::SliderInt("Health Max", &hp_max_dist, 50, 600);
                        ImGui::SliderInt("Skeleton Max", &skeleton_max_dist, 50, 600);
                        ImGui::SliderInt("Name Max", &name_max_dist, 50, 600);
                        ImGui::SliderInt("Distance Max", &distance_txt_max_dist, 50, 1000);
                        ImGui::SliderInt("Weapon Max", &weapon_max_dist, 50, 400);
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Individual Colors")) {
                        ImGui::ColorEdit4("Visible Box", box_visible_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Invisible Box", box_invisible_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Skeleton Visible", skeleton_visible_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Skeleton Invisible", skeleton_invisible_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4(language == 1 ? "Mau Ten (Name)" : "Names", name_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Distance Text", distance_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::ColorEdit4("Weapon Text", weapon_color, ImGuiColorEditFlags_NoInputs);
                        ImGui::TreePop();
                    }
                    ImGui::Separator();
                    ImGui::SliderInt(Lang.RenderDist, &render_distance, 50, 1000);
                    
                    ImGui::Separator();
                    ImGui::Checkbox("Weapon Text / Icon", &g_Menu.esp_weapon);
                    ImGui::Combo("Draw Mode", &g_Menu.esp_weapon_type, "Text Label\0Image (Assets)\0");
                    ImGui::EndChild();
                }
                else if (active_tab == 1) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), (language == 1) ? "AIM (PHAN LOAI SUNG)" : "AIM (WEAPON CATEGORIES)");
                    ImGui::Separator();
                    ImGui::BeginChild("##AimbotContent");

                    ImGui::Checkbox(language == 1 ? "BAT AIM" : "ENABLE AIM GZ", &aim_master_enabled);
                    ImGui::Separator();

                    const char* cats[] = { "AR (M416, AKM...)", "SR (Kar98, M24...)", "DMR (SKS, SLR...)", "SMG (UMP, Vector...)", "LMG (M249, DP...)", "SG (S12K, DBS...)", "PT (P92, P1911...)", "Other (Mortar...)" };
                    if (aim_category_idx > 7) aim_category_idx = 0; 
                    ImGui::Combo(language == 1 ? "Loai sung dang sua" : "Current Category", &aim_category_idx, cats, IM_ARRAYSIZE(cats));
                    ImGui::Separator();

                    AimConfig* pCfg = &aim_configs[aim_category_idx];

                    ImGui::Checkbox(language == 1 ? "Bat Aimbot cho loai nay" : "Enabled for this Class", &pCfg->enabled);
                    
                    if (pCfg->enabled) {
                        ImGui::SliderFloat(Lang.AimFOV, &pCfg->fov, 1.0f, 100.0f, "%.1f");
                        ImGui::SliderFloat(Lang.AimSmooth, &pCfg->smooth, 1.0f, 20.0f, "%.1f");
                        
                        float maxRangeLimit = 400.0f;
                        if (aim_category_idx == 0 || aim_category_idx == 3 || aim_category_idx == 4) maxRangeLimit = 70.0f; // AR, SMG, LMG
                        else if (aim_category_idx == 1) maxRangeLimit = 500.0f; // SR
                        else if (aim_category_idx == 2) maxRangeLimit = 250.0f; // DMR
                        else if (aim_category_idx == 5 || aim_category_idx == 6) maxRangeLimit = 30.0f; // SG, PT

                        if (pCfg->max_dist > maxRangeLimit) pCfg->max_dist = maxRangeLimit;
                        ImGui::SliderFloat(language == 1 ? "Khoang cach toi da (m)" : "Max Distance (m)", &pCfg->max_dist, 10.0f, maxRangeLimit, "%.0f");
                        
                        // --- HOTKEY LISTENER ---
                        char keyDisplay[64];
                        if (waiting_for_key == &pCfg->key) {
                            strcpy_s(keyDisplay, sizeof(keyDisplay), language == 1 ? "Vui long bam phim..." : "Waiting for key...");
                        } else {
                            std::string kn = "UNKNOWN";
                            int vk = pCfg->key;
                            if (vk == VK_RBUTTON) kn = "Right Click";
                            else if (vk == VK_LBUTTON) kn = "Left Click";
                            else if (vk == VK_MBUTTON) kn = "Middle Click";
                            else if (vk == VK_XBUTTON1) kn = "Mouse 4";
                            else if (vk == VK_XBUTTON2) kn = "Mouse 5";
                            else if (vk == VK_MENU) kn = "ALT";
                            else if (vk == VK_CONTROL) kn = "CTRL";
                            else if (vk == VK_SHIFT) kn = "SHIFT";
                            else if (vk == VK_CAPITAL) kn = "CAPS";
                            else if (vk >= 'A' && vk <= 'Z') kn = (char)vk;
                            else kn = "VK_" + std::to_string(vk);

                            sprintf_s(keyDisplay, sizeof(keyDisplay), language == 1 ? "Phim Aim: [%s]" : "Aim Key: [%s]", kn.c_str());
                        }

                        if (ImGui::Button(keyDisplay, ImVec2(-1, 0))) {
                            waiting_for_key = &pCfg->key;
                        }

                        ImGui::Checkbox(Lang.AimPrediction, &pCfg->prediction);
                    }
                    
                    ImGui::Separator();
                    ImGui::Checkbox(language == 1 ? "Tu thu nho FOV theo ong ngam" : "Adaptive FOV (Scale with Scope)", &aim_adaptive_fov);
                    ImGui::Checkbox(Lang.AimVisible, &aim_visible_only);

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (ImGui::TreeNode(language == 1 ? "HUONG DAN SU DUNG AIM" : "AIM USAGE GUIDE")) {
                        ImGui::TextWrapped(language == 1 ? 
                            "1. Smart-Bone: Aim tu dong khoa vao bat ky vung nao (Dau, Than, Chan, Tay) gan tam ngam nhat." :
                            "1. Smart-Bone: Aimbot snaps to any bone (Head, Torso, Limbs) nearest to crosshair.");
                        
                        ImGui::Spacing();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), language == 1 ? 
                            "2. Meo: Nhan GIU SHIFT khi dang Aim de ep muc tieu vao DAU." :
                            "2. Tip: HOLD SHIFT while aiming to force-lock onto HEAD.");
                        
                        ImGui::TreePop();
                    }

                    ImGui::EndChild();
                }
                else if (active_tab == 2) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "MACRO CONTROL");
                    ImGui::Separator();
                    ImGui::BeginChild("##MacroContent");
                    
                    if (ImGui::Checkbox(Lang.MacroEnabled, &macro_enabled)) {
                        MacroEngine::macro_enabled = macro_enabled;
                    }
                    
                    ImGui::SameLine();
                    if (ImGui::Checkbox("MASTER AutoShot", &GameData.Config.Macro.AutoShotEnabled)) {
                    }
                    
                    ImGui::Text("AutoShot Key: "); ImGui::SameLine();
                    char autoShotKeyName[64];
                    if (waiting_for_key == &GameData.Config.Macro.AutoShotKey) {
                        strcpy_s(autoShotKeyName, sizeof(autoShotKeyName), Translation::CurrentLanguage == 1 ? "Vui long bam phim..." : "Waiting for key...");
                    } else {
                        strcpy_s(autoShotKeyName, sizeof(autoShotKeyName), Utils::keyCodeToString(GameData.Config.Macro.AutoShotKey).c_str());
                    }

                    if (ImGui::Button(autoShotKeyName, ImVec2(120, 22))) {
                         waiting_for_key = &GameData.Config.Macro.AutoShotKey;
                    }
                    
                    if (macro_enabled) {
                        ImGui::SliderFloat(Lang.MacroStrength, &MacroEngine::global_multiplier, 0.1f, 3.0f, "%.2fx");
                        if (ImGui::Checkbox(Lang.MacroHumanize, &macro_humanize)) {
                            MacroEngine::macro_humanize = macro_humanize;
                        }
                        if (ImGui::Checkbox(Translation::CurrentLanguage == 1 ? "Chi ghi tam khi ADS (Ngam)" : "Only pull when ADS/Scoping", &macro_ads_only)) {
                            MacroEngine::ads_only = macro_ads_only;
                        }
                        
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "WEAPON CLASS TUNING");
                        
                        const char* macro_cats[] = { "AR", "SR", "DMR", "SG" };
                        static int macro_cat_idx = 0;
                        ImGui::Combo("Weapon Class", &macro_cat_idx, macro_cats, IM_ARRAYSIZE(macro_cats));
                        
                        if (macro_cat_idx == 0) { // AR
                            ImGui::Checkbox("AR AutoShot Enabled", &MacroEngine::ar_trigger_enabled);
                            ImGui::SliderFloat("AR Trigger FOV", &MacroEngine::ar_trigger_fov, 0.1f, 10.0f, "%.1f");
                            ImGui::SliderFloat("AR Base Smoothing", &MacroEngine::ar_base_smoothing, 1.0f, 20.0f, "%.1f");
                        } else if (macro_cat_idx == 1) { // SR
                            ImGui::Checkbox("SR AutoShot Enabled", &MacroEngine::sr_trigger_enabled);
                            ImGui::SliderFloat("SR Trigger FOV", &MacroEngine::sr_trigger_fov, 0.1f, 10.0f, "%.1f");
                            ImGui::SliderFloat("SR Base Smoothing", &MacroEngine::sr_base_smoothing, 1.0f, 20.0f, "%.1f");
                        } else if (macro_cat_idx == 2) { // DMR
                            ImGui::Checkbox("DMR AutoShot Enabled", &MacroEngine::dmr_trigger_enabled);
                            ImGui::SliderFloat("DMR Trigger FOV", &MacroEngine::dmr_trigger_fov, 0.1f, 10.0f, "%.1f");
                            ImGui::SliderFloat("DMR Base Smoothing", &MacroEngine::dmr_base_smoothing, 1.0f, 20.0f, "%.1f");
                        } else if (macro_cat_idx == 3) { // SG
                            ImGui::Checkbox("SG AutoShot Enabled", &MacroEngine::sg_trigger_enabled);
                            ImGui::SliderFloat("SG Trigger FOV", &MacroEngine::sg_trigger_fov, 0.1f, 10.0f, "%.1f");
                        }
                        
                        ImGui::SliderFloat("Max Smooth Increase", &MacroEngine::max_smooth_increase, 0.0f, 1.0f, "%.2f");
                        ImGui::SliderFloat("Smooth FOV", &MacroEngine::smooth_fov, 1.0f, 30.0f, "%.1f");
                        
                        ImGui::Separator();
                        
                        if (ImGui::Checkbox(Lang.MacroOSD, &show_macro_overlay)) {
                            // Sync or handle
                        }
                        if (show_macro_overlay) {
                            ImGui::SameLine();
                            ImGui::ColorEdit4("##OSDColor", macro_overlay_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                        }

                        int macroModeIdx = (MacroEngine::macro_mode <= 1) ? 0 : 1;
                        if (ImGui::Combo(Translation::CurrentLanguage == 1 ? "Che do Macro" : "Macro Mode", &macroModeIdx,
                            Translation::CurrentLanguage == 1 ? "Mode 1 (On dinh)\0Mode 2 (Manh)\0" : "Mode 1 (Stable)\0Mode 2 (Aggressive)\0")) {
                            MacroEngine::macro_mode = macroModeIdx + 1;
                        }

                        ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Delay keo (ms)" : "Pull Delay (ms)", &MacroEngine::pull_delay_ms, 0, 30);
                        ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Do tre random (ms)" : "Delay Jitter (ms)", &MacroEngine::pull_delay_jitter_ms, 0, 30);
                        ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Reset loat dan (ms)" : "Burst Reset (ms)", &MacroEngine::recoil_reset_ms, 80, 1200);
                        ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Lech X random" : "Random X Jitter", &MacroEngine::x_jitter_range, 0, 8);

                        const auto liveProfile = MacroEngine::GetActiveProfile();
                        ImGui::Text("%s %d | Delay: %dms | Smoothing: %.1f",
                            Translation::CurrentLanguage == 1 ? "Profile mode" : "Profile mode",
                            MacroEngine::macro_mode,
                            liveProfile.delayMs,
                            (macro_cat_idx == 0 ? MacroEngine::ar_base_smoothing : (macro_cat_idx == 1 ? MacroEngine::sr_base_smoothing : MacroEngine::dmr_base_smoothing)));
                        
                        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                        
                        // Status Section
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "LIVE STATUS:");
                        
                        std::string weapon = (MacroEngine::current_weapon_name == "" || MacroEngine::current_weapon_name == "None") ? Lang.NoWeapon : MacroEngine::current_weapon_name;
                        ImGui::Text("%s %s", Lang.CurrentWeapon, weapon.c_str());
                        
                        std::string attach = "S:" + std::to_string(MacroEngine::current_scope) + 
                                            " M:" + std::to_string(MacroEngine::current_muzzle) + 
                                            " G:" + std::to_string(MacroEngine::current_grip);
                        ImGui::Text("%s %s", Lang.DetectedAttach, attach.c_str());

                        if (ImGui::Button(Lang.RescanAttach, ImVec2(-1, 35))) {
                            MacroEngine::ForceScan();
                        }
                    } else {
                        // Removed the "MACRO IS DISABLED" message as requested
                    }

                    ImGui::EndChild();
                }
                else if (active_tab == 3) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "RADAR AUTO ALIGN");
                    ImGui::Separator();
                    ImGui::BeginChild("##RadarContent");
                    ImGui::Checkbox(Lang.ShowCrosshair, &g_Menu.show_radar_center);
                    ImGui::TextUnformatted("Auto sync from game HUD widget (no manual calibration).");
                    ImGui::Text("MiniMap: %.1f, %.1f | %.1fx%.1f", G_Radar.ScreenPosX, G_Radar.ScreenPosY, G_Radar.ScreenSize, G_Radar.ScreenSizeY);
                    ImGui::Text("WorldMap: %s | %.1fx%.1f", G_Radar.IsWorldMapVisible ? "Visible" : "Hidden", G_Radar.WorldMapWidth, G_Radar.WorldMapHeight);
                    ImGui::EndChild();
                }
                else if (active_tab == 4) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), "SYSTEM SETTINGS");
                    ImGui::Separator();
                    ImGui::BeginChild("##MiscContent");
                    int currentLang = language;
                    if (ImGui::Combo(Lang.Language, &currentLang, "English\0Vietnamese\0")) language = currentLang;
                    
                    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
                    
                    if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 40))) {
                        g_Menu.SaveConfig("dataMacro/Config/settings.json");
                    }
                    if (ImGui::Button(Lang.LoadConfig, ImVec2(-1, 40))) {
                        g_Menu.LoadConfig("dataMacro/Config/settings.json");
                    }
                    if (ImGui::Button(language == 1 ? "TEST CHUOT [5b] (KERNEL MOVE)" : "TEST MOUSE [5b] (KERNEL MOVE)", ImVec2(-1, 40))) {
                        for (int i = 0; i < 30; i++) { PubgMemory::MoveMouse(4, 4, 0); Sleep(5); }
                        for (int i = 0; i < 30; i++) { PubgMemory::MoveMouse(-4, -4, 0); Sleep(5); }
                    }

                    if (ImGui::Button(language == 1 ? "TEST CLICK [6] (KERNEL CLICK)" : "TEST CLICK [6] (KERNEL CLICK)", ImVec2(-1, 40))) {
                        Driver::Click();
                    }
                    
                    ImGui::EndChild();
                }
                if (active_tab == 5) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.8f, 1.0f), Lang.TabLoot);
                    ImGui::Separator();
                    ImGui::BeginChild("##LootContent");
                    ImGui::Checkbox(Translation::CurrentLanguage == 1 ? "Hien thi Vat pham" : "ESP Items", &esp_items);
                    ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Khoang cach Vat pham" : "Loot Max Distance", &loot_max_dist, 10, 300);
                    ImGui::Separator();
                    ImGui::Checkbox(Translation::CurrentLanguage == 1 ? "Hien thi Phuong tien" : "ESP Vehicles", &esp_vehicles);
                    ImGui::SliderInt(Translation::CurrentLanguage == 1 ? "Khoang cach Phuong tien" : "Vehicle Max Distance", &vehicle_max_dist, 10, 2000);
                    ImGui::Separator();
                    ImGui::Checkbox(Translation::CurrentLanguage == 1 ? "Hien thi Thinh (Air Drop)" : "ESP Air Drops", &esp_airdrops);
                    ImGui::Checkbox(Translation::CurrentLanguage == 1 ? "Hien thi Xac (Dead Box)" : "ESP Dead Boxes", &esp_deadboxes);
                    ImGui::EndChild();
                }
            }
            ImGui::EndChild();
            
            ImGui::End();
        }

        ImGui::Render();
        const float clear_color[4] = { 0, 0, 0, 0 };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);

    } catch (...) {}
}

void OverlayMenu::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
}

void OverlayMenu::UpdateAntiScreenshot() {
    if (!target_hwnd) return;
    
    // WDA_EXCLUDEFROMCAPTURE (0x00000011) - New Win10 builds, completely hidden/no black box
    // WDA_NONE (0) - Visible
    
    DWORD affinity = anti_screenshot ? 0x00000011 : 0x00000000;

    typedef BOOL(WINAPI* pSWDA)(HWND, DWORD);
    HMODULE user32 = GetModuleHandleA(skCrypt("user32.dll"));
    if (user32) {
        auto swda = (pSWDA)GetProcAddress(user32, skCrypt("SetWindowDisplayAffinity"));
        if (swda) {
            swda(target_hwnd, affinity);
        }
    }
}

void OverlayMenu::DoAimbot() {
    // CONTENT REMOVED - LOGIC INTEGRATED INTO RenderFrame for Performance
}
void OverlayMenu::SaveConfig(const char* path) {
    try {
        nlohmann::json j;
        j["esp_toggle"] = esp_toggle;
        j["esp_show_enemies"] = esp_show_enemies;
        j["esp_show_teammates"] = esp_show_teammates;
        j["esp_box"] = esp_box;
        j["esp_skeleton"] = esp_skeleton;
        j["esp_name"] = esp_name;
        j["esp_distance"] = esp_distance;
        j["esp_items"] = esp_items;
        j["esp_snapline"] = esp_snapline;
        j["esp_weapon"] = esp_weapon;
        j["esp_weapon_type"] = esp_weapon_type;
        j["render_distance"] = render_distance;
        j["language"] = language;
        j["show_macro_overlay"] = show_macro_overlay;
        j["show_radar_center"] = show_radar_center;
        j["anti_screenshot"] = anti_screenshot;
        j["macro_enabled"] = macro_enabled;
        j["macro_humanize"] = macro_humanize;
        j["macro_ads_only"] = macro_ads_only;
        j["macro_global_multiplier"] = MacroEngine::global_multiplier;
        j["macro_overlay_color"] = { macro_overlay_color[0], macro_overlay_color[1], macro_overlay_color[2], macro_overlay_color[3] };

        j["esp_items"] = esp_items;
        j["esp_vehicles"] = esp_vehicles;
        j["esp_airdrops"] = esp_airdrops;
        j["esp_deadboxes"] = esp_deadboxes;
        j["loot_max_dist"] = loot_max_dist;
        j["vehicle_max_dist"] = vehicle_max_dist;

        j["esp_distance_lod"] = esp_distance_lod;
        j["skeleton_max_dist"] = skeleton_max_dist;
        j["name_max_dist"] = name_max_dist;
        j["weapon_max_dist"] = weapon_max_dist;
        j["box_max_dist"] = box_max_dist;
        j["hp_max_dist"] = hp_max_dist;
        j["distance_txt_max_dist"] = distance_txt_max_dist;

        j["color_box_vis"] = { box_visible_color[0], box_visible_color[1], box_visible_color[2], box_visible_color[3] };
        j["color_box_inv"] = { box_invisible_color[0], box_invisible_color[1], box_invisible_color[2], box_invisible_color[3] };
        j["color_skel_vis"] = { skeleton_visible_color[0], skeleton_visible_color[1], skeleton_visible_color[2], skeleton_visible_color[3] };
        j["color_skel_inv"] = { skeleton_invisible_color[0], skeleton_invisible_color[1], skeleton_invisible_color[2], skeleton_invisible_color[3] };
        j["color_names"] = { name_color[0], name_color[1], name_color[2], name_color[3] };
        j["color_dist"] = { distance_color[0], distance_color[1], distance_color[2], distance_color[3] };
        j["color_weapon"] = { weapon_color[0], weapon_color[1], weapon_color[2], weapon_color[3] };
        
        // Aimbot Configs
        j["aim_master_enabled"] = aim_master_enabled;
        j["aim_adaptive_fov"] = aim_adaptive_fov;
        j["aim_visible_only"] = aim_visible_only;
        
        nlohmann::json aim_array = nlohmann::json::array();
        for (int i = 0; i < 9; i++) {
            nlohmann::json c;
            c["enabled"] = aim_configs[i].enabled;
            c["fov"] = aim_configs[i].fov;
            c["smooth"] = aim_configs[i].smooth;
            c["bone"] = aim_configs[i].bone;
            c["key"] = aim_configs[i].key;
            c["max_dist"] = aim_configs[i].max_dist;
            c["prediction"] = aim_configs[i].prediction;
            aim_array.push_back(c);
        }
        j["aim_configs"] = aim_array;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            std::cout << "[+] Saved Config to: " << path << std::endl;
        }
    } catch (...) {}
}

void OverlayMenu::LoadConfig(const char* path) {
    try {
        std::ifstream file(path);
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            file.close();

            if (j.contains("esp_toggle")) esp_toggle = j["esp_toggle"];
            if (j.contains("esp_show_enemies")) esp_show_enemies = j["esp_show_enemies"];
            if (j.contains("esp_show_teammates")) {
                esp_show_teammates = j["esp_show_teammates"];
            } else if (j.contains("esp_team_check")) {
                // Legacy key: team_check=true means hide teammates.
                esp_show_teammates = !static_cast<bool>(j["esp_team_check"]);
            }
            if (j.contains("esp_box")) esp_box = j["esp_box"];
            if (j.contains("esp_skeleton")) esp_skeleton = j["esp_skeleton"];
            if (j.contains("esp_name")) esp_name = j["esp_name"];
            if (j.contains("esp_distance")) esp_distance = j["esp_distance"];
            if (j.contains("esp_items")) esp_items = j["esp_items"];
            if (j.contains("esp_snapline")) esp_snapline = j["esp_snapline"];
            if (j.contains("esp_weapon")) esp_weapon = j["esp_weapon"];
            if (j.contains("esp_weapon_type")) esp_weapon_type = j["esp_weapon_type"];
            if (j.contains("render_distance")) render_distance = j["render_distance"];
            if (j.contains("language")) language = j["language"];
            if (j.contains("show_macro_overlay")) show_macro_overlay = j["show_macro_overlay"];
            if (j.contains("show_radar_center")) show_radar_center = j["show_radar_center"];
            radar_offset_x = 0.0f;
            radar_offset_y = 0.0f;
            radar_zoom_multiplier = 1.0f;
            radar_rotation_offset = 0.0f;
            if (j.contains("macro_enabled")) {
                macro_enabled = j["macro_enabled"];
                MacroEngine::macro_enabled = macro_enabled;
            }
            if (j.contains("macro_humanize")) {
                macro_humanize = j["macro_humanize"];
                MacroEngine::macro_humanize = macro_humanize;
            }
            if (j.contains("macro_ads_only")) {
                macro_ads_only = j["macro_ads_only"];
                MacroEngine::ads_only = macro_ads_only;
            }
            if (j.contains("macro_global_multiplier")) MacroEngine::global_multiplier = j["macro_global_multiplier"];
            
            if (j.contains("macro_overlay_color") && j["macro_overlay_color"].is_array() && j["macro_overlay_color"].size() == 4) {
                for (int i = 0; i < 4; i++) macro_overlay_color[i] = j["macro_overlay_color"][i];
            }
            
            if (j.contains("esp_distance_lod")) esp_distance_lod = j["esp_distance_lod"];
            if (j.contains("skeleton_max_dist")) skeleton_max_dist = j["skeleton_max_dist"];
            if (j.contains("name_max_dist")) name_max_dist = j["name_max_dist"];
            if (j.contains("weapon_max_dist")) weapon_max_dist = j["weapon_max_dist"];
            if (j.contains("box_max_dist")) box_max_dist = j["box_max_dist"];
            if (j.contains("hp_max_dist")) hp_max_dist = j["hp_max_dist"];
            if (j.contains("distance_txt_max_dist")) distance_txt_max_dist = j["distance_txt_max_dist"];

            if (j.contains("color_box_vis")) for (int i = 0; i < 4; i++) box_visible_color[i] = j["color_box_vis"][i];
            if (j.contains("color_box_inv")) for (int i = 0; i < 4; i++) box_invisible_color[i] = j["color_box_inv"][i];
            if (j.contains("color_skel_vis")) for (int i = 0; i < 4; i++) skeleton_visible_color[i] = j["color_skel_vis"][i];
            if (j.contains("color_skel_inv")) for (int i = 0; i < 4; i++) skeleton_invisible_color[i] = j["color_skel_inv"][i];
            if (j.contains("color_names")) for (int i = 0; i < 4; i++) name_color[i] = j["color_names"][i];
            if (j.contains("color_dist")) for (int i = 0; i < 4; i++) distance_color[i] = j["color_dist"][i];
            if (j.contains("color_weapon")) for (int i = 0; i < 4; i++) weapon_color[i] = j["color_weapon"][i];

            if (j.contains("esp_items")) esp_items = j["esp_items"];
            if (j.contains("esp_vehicles")) esp_vehicles = j["esp_vehicles"];
            if (j.contains("esp_airdrops")) esp_airdrops = j["esp_airdrops"];
            if (j.contains("esp_deadboxes")) esp_deadboxes = j["esp_deadboxes"];
            if (j.contains("loot_max_dist")) loot_max_dist = j["loot_max_dist"];
            if (j.contains("vehicle_max_dist")) vehicle_max_dist = j["vehicle_max_dist"];

            if (j.contains("esp_distance_lod")) esp_distance_lod = j["esp_distance_lod"];
            if (j.contains("skeleton_max_dist")) skeleton_max_dist = j["skeleton_max_dist"];
            if (j.contains("name_max_dist")) name_max_dist = j["name_max_dist"];
            if (j.contains("weapon_max_dist")) weapon_max_dist = j["weapon_max_dist"];
            if (j.contains("box_max_dist")) box_max_dist = j["box_max_dist"];
            if (j.contains("hp_max_dist")) hp_max_dist = j["hp_max_dist"];
            if (j.contains("distance_txt_max_dist")) distance_txt_max_dist = j["distance_txt_max_dist"];

            // Aimbot Configs
            if (j.contains("aim_master_enabled")) aim_master_enabled = j["aim_master_enabled"];
            if (j.contains("aim_adaptive_fov")) aim_adaptive_fov = j["aim_adaptive_fov"];
            if (j.contains("aim_visible_only")) aim_visible_only = j["aim_visible_only"];

            if (j.contains("aim_configs") && j["aim_configs"].is_array()) {
                auto aim_array = j["aim_configs"];
                for (int i = 0; i < (int)aim_array.size() && i < 9; i++) {
                    auto c = aim_array[i];
                    if (c.contains("enabled")) aim_configs[i].enabled = c["enabled"];
                    if (c.contains("fov")) aim_configs[i].fov = c["fov"];
                    if (c.contains("smooth")) aim_configs[i].smooth = c["smooth"];
                    if (c.contains("bone")) aim_configs[i].bone = c["bone"];
                    if (c.contains("key")) aim_configs[i].key = c["key"];
                    if (c.contains("max_dist")) aim_configs[i].max_dist = c["max_dist"];
                    if (c.contains("prediction")) aim_configs[i].prediction = c["prediction"];
                }
            }

            std::cout << "[+] Loaded Config from: " << path << std::endl;
        }
    } catch (...) {}
}
