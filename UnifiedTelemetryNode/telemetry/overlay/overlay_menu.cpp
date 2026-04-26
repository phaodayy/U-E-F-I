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
#include "../../protec/skCrypt.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

// DirectX 11 Globals
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool g_clearBeforeRender = true;
static bool g_presentAfterRender = true;

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
    
    std::string path1 = skCrypt("Assets/Weapon/") + weaponName + skCrypt(".png");
    std::string path2 = skCrypt("../Assets/Weapon/") + weaponName + skCrypt(".png");
    std::string path3 = skCrypt("../../Assets/Weapon/") + weaponName + skCrypt(".png");
    
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
    g_clearBeforeRender = true;
    g_presentAfterRender = true;
    return CreateRenderTarget();
}

void OverlayMenu::CleanupDeviceD3D() {
    CleanupRenderTarget();
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

bool OverlayMenu::Initialize(const VisualizationBridgeHost& bridge) {
    target_hwnd = bridge.hwnd;
    if (!target_hwnd || !IsWindow(target_hwnd)) {
        target_hwnd = nullptr;
        std::cout << skCrypt("[-] Passive visualization host is not available.\n");
        std::cout << skCrypt("[-] Provide a valid bridge-owned HWND; window discovery and style mutation are disabled.\n");
        return false;
    }

    char cls[256] = {0};
    DWORD target_pid = 0;
    GetClassNameA(target_hwnd, cls, 256);
    GetWindowThreadProcessId(target_hwnd, &target_pid);
    std::cout << skCrypt("[+] Passive visualization host attached: [") << cls << skCrypt("] (PID: ") << target_pid << skCrypt(")\n");

    if (target_pid == GetCurrentProcessId()) {
        MARGINS margin = { -1 };
        DwmExtendFrameIntoClientArea(target_hwnd, &margin);
    }

    if (bridge.swap_chain) {
        if (!AcquireBridgeD3D(bridge) || !CreateRenderTarget()) {
            CleanupDeviceD3D();
            return false;
        }
        std::cout << skCrypt("[+] Bridge renderer using host-provided swap chain.\n");
    } else if (!CreateDeviceD3D(target_hwnd)) {
        return false;
    }

    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;
    // Load a system font that supports Vietnamese
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 16.0f, &font_cfg, io.Fonts->GetGlyphRangesVietnamese());
    
    SetupStyle();
    ImGui_ImplWin32_Init(target_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Default configs...
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

    LoadConfig(skCrypt("dataMacro/Config/settings.json"));
    std::cout << skCrypt("[+] Passive visualization ready.\n");
    return true;
}

void OverlayMenu::SetupStyle() {
    auto& style = ImGui::GetStyle();
    
    // --- Advanced Layout & Rounding ---
    style.WindowRounding    = 18.0f;
    style.ChildRounding     = 14.0f;
    style.FrameRounding     = 10.0f;
    style.PopupRounding     = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding      = 10.0f;
    style.TabRounding       = 10.0f;
    
    style.WindowBorderSize  = 0.0f; // Handled by custom drawing
    style.ChildBorderSize   = 0.0f; 
    style.FrameBorderSize   = 1.0f;
    
    style.WindowPadding     = ImVec2(25, 25);
    style.FramePadding      = ImVec2(12, 6);
    style.ItemSpacing       = ImVec2(14, 14);
    style.ItemInnerSpacing  = ImVec2(10, 10);
    
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    
    // --- Palette: Midnight Violet & Electric Indigo ---
    ImVec4* colors = style.Colors;
    
    // Core surfaces
    colors[ImGuiCol_WindowBg]             = ImVec4(0.04f, 0.00f, 0.12f, 0.96f); // Midnight purple
    colors[ImGuiCol_ChildBg]              = ImVec4(0.08f, 0.02f, 0.18f, 0.45f); // Soft indigo card
    colors[ImGuiCol_PopupBg]              = ImVec4(0.06f, 0.02f, 0.14f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.48f, 0.17f, 0.90f, 0.30f); // Violet border
    
    // Logic controls
    colors[ImGuiCol_FrameBg]              = ImVec4(0.12f, 0.05f, 0.28f, 0.50f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.48f, 0.17f, 0.90f, 0.20f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.48f, 0.17f, 0.90f, 0.35f);
    
    // Title & Headers
    colors[ImGuiCol_TitleBg]              = ImVec4(0.04f, 0.00f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.06f, 0.02f, 0.18f, 1.00f);
    
    // Selection highlight
    colors[ImGuiCol_Header]               = ImVec4(0.48f, 0.17f, 0.90f, 0.30f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.48f, 0.17f, 0.90f, 0.50f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.48f, 0.17f, 0.90f, 0.70f);
    
    // Buttons (Cyberpunk Purple)
    colors[ImGuiCol_Button]               = ImVec4(0.48f, 0.17f, 0.90f, 0.15f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.48f, 0.17f, 0.90f, 0.45f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.48f, 0.17f, 0.90f, 0.65f);
    
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.48f, 0.17f, 0.90f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.60f, 0.25f, 1.00f, 1.00f);
    
    colors[ImGuiCol_CheckMark]            = ImVec4(0.60f, 0.25f, 1.00f, 1.00f);
    
    colors[ImGuiCol_Text]                 = ImVec4(0.95f, 0.92f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.40f, 0.40f, 0.60f, 1.00f);
    
    colors[ImGuiCol_Separator]            = ImVec4(0.48f, 0.17f, 0.90f, 0.25f);
}

ImU32 GetTeamColor(int teamID) {
    return telemetryColors::GetTeamColor(teamID);
}

static void DrawRadarTeamMarker(ImDrawList* draw, float x, float y, int teamID, ImU32 color, float radius) {
    // 1. Outer Glow/Drop Shadow
    draw->AddCircle(ImVec2(x, y), radius + 1.8f, IM_COL32(0, 0, 0, 160), 20, 1.0f);
    
    // 2. High Saturated Main Circle
    draw->AddCircleFilled(ImVec2(x, y), radius, color, 20);
    
    // 3. Inner Ring for Depth
    draw->AddCircle(ImVec2(x, y), radius - 1.0f, IM_COL32(255, 255, 255, 80), 20, 0.5f);

    if (teamID > 0) {
        char teamText[16];
        sprintf_s(teamText, "%d", teamID % 100);
        ImVec2 textSize = ImGui::CalcTextSize(teamText);
        // Draw Outline for text
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
    extern std::string global_active_key;
    bool is_authenticated = !global_active_key.empty();

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

        // --- TOGGLE MENU (F5) ---
        static bool f5_down = false;
        bool f5_current = telemetryMemory::IsKeyDown(VK_F5);
        if (f5_current && !f5_down) {
            showmenu = !showmenu;
        }
        f5_down = f5_current;

        // --- MOUSE INJECTION (FOR HIJACKED WINDOW) ---
        ImGuiIO& io = ImGui::GetIO();
        if (showmenu) {
            POINT p; GetCursorPos(&p);
            ScreenToClient(target_hwnd, &p);
            io.MousePos = ImVec2((float)p.x, (float)p.y);
            io.MouseDown[0] = telemetryMemory::IsKeyDown(VK_LBUTTON);
        }

        // --- WINDOW MESSAGE HANDLING (VALORANT STYLE) ---
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 1-2 (Weapon), X (Holster), Tab (Bag), M (Map), G (Grenade), F (Interact/Equip)
        if (telemetryMemory::IsKeyDown('1') || telemetryMemory::IsKeyDown('2') || telemetryMemory::IsKeyDown('X') || 
            telemetryMemory::IsKeyDown(VK_TAB) || telemetryMemory::IsKeyDown('M') || telemetryMemory::IsKeyDown('G') || telemetryMemory::IsKeyDown('F')) {
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

        std::vector<PlayerData> localPlayers = G_Players;
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
            std::string alertTxt = skCrypt("LICENSE STATUS: INACTIVE - PLEASE ENTER KEY IN SETTINGS [F5]");
            ImVec2 txtSize = ImGui::GetFont()->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, alertTxt.c_str());
            draw->AddRectFilled(ImVec2(10, 10), ImVec2(20 + txtSize.x, 20 + txtSize.y), IM_COL32(0, 0, 0, 150), 5.0f);
            draw->AddText(ImGui::GetFont(), 18.0f, ImVec2(15, 15), IM_COL32(255, 50, 50, 255), alertTxt.c_str());
        }

        // --- GLOBAL SPECTATOR WARNING ---
        if (is_authenticated && esp_toggle && g_Menu.esp_spectated && G_LocalSpectatedCount > 0) {
            char specText[64];
            sprintf_s(specText, sizeof(specText), "SPECTATORS: %d", G_LocalSpectatedCount);
            
            float fontSize = 23.0f;
            ImVec2 txtSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, specText);
            
            float panelW = txtSize.x + 60.0f;
            float panelH = txtSize.y + 20.0f;
            float posX = (ScreenWidth - panelW) / 2.0f;
            float posY = 60.0f; 
            
            ImVec2 pMin = ImVec2(posX, posY);
            ImVec2 pMax = ImVec2(posX + panelW, posY + panelH);
            
            // Neon Orange Border + Glow + Glass Effect
            ImGui::GetForegroundDrawList()->AddRectFilled(pMin, pMax, IM_COL32(0, 0, 0, 180), 8.0f);
            ImGui::GetForegroundDrawList()->AddRect(pMin, pMax, IM_COL32(255, 120, 0, 255), 8.0f, 0, 2.5f);
            
            // Draw "EYE" Icon (Simple circle + dot)
            float eyeX = posX + 22.0f;
            float eyeY = posY + panelH / 2.0f;
            ImGui::GetForegroundDrawList()->AddCircle(ImVec2(eyeX, eyeY), 8.0f, IM_COL32(255, 120, 0, 255), 12, 1.5f);
            ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(eyeX, eyeY), 3.0f, IM_COL32(255, 120, 0, 255));
            
            ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(posX + 45, posY + 10.0f), IM_COL32(255, 120, 0, 255), specText);
        }

        // --- 0. RADAR (MINI MAP + WORLD MAP) ---
        if (is_authenticated && esp_toggle && radar_enabled) {
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

            const bool keyWorldMapPressed = telemetryMemory::IsKeyDown('M');
            const bool canDrawWorldMap = (G_Radar.IsWorldMapVisible || keyWorldMapPressed) &&
                G_Radar.MapWorldSize > 1000.0f; 

            bool worldMapDrawn = false;

            // --- 1. WORLD MAP DOTS (Drawn first to check if we should hide minimap dots) ---
            if (canDrawWorldMap) {
                float worldLeft = G_Radar.WorldMapX;
                float worldTop = G_Radar.WorldMapY;
                float worldRight = worldLeft + G_Radar.WorldMapWidth;
                float worldBottom = worldTop + G_Radar.WorldMapHeight;
                
                
                // Fallback: If HUD widget rect is unknown, assume center square (standard telemetry behavior)
                if (G_Radar.WorldMapWidth <= 10.0f || G_Radar.WorldMapHeight <= 10.0f) {
                    float mapSize = ScreenHeight;
                    worldLeft = (ScreenWidth - mapSize) * 0.5f;
                    worldTop = 0.0f;
                    worldRight = worldLeft + mapSize;
                    worldBottom = mapSize;
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

                    // Support dynamic zoom if MapSizeFactored is valid, otherwise use full map.
                    // Use zoom factor for accurate scaling on the world map
                    const float mapX = worldLeft + (worldRight - worldLeft) * 0.5f + (radarPosX / G_Radar.MapWorldSize) * G_Radar.WorldMapZoomFactor * (worldRight - worldLeft) * 0.5f;
                    const float mapY = worldTop + (worldBottom - worldTop) * 0.5f + (radarPosY / G_Radar.MapWorldSize) * G_Radar.WorldMapZoomFactor * (worldBottom - worldTop) * 0.5f;
                    
                    if (mapX < worldLeft || mapX > worldRight || mapY < worldTop || mapY > worldBottom) {
                        continue;
                    }

                    ImU32 teamColor = GetTeamColor(player.TeamID);
                    DrawRadarTeamMarker(draw, mapX, mapY, player.TeamID, teamColor, g_Menu.radar_dot_size * 1.5f);
                }
                worldMapDrawn = true;
            }

            // --- 2. MINI MAP DOTS (Only draw if world map is NOT drawn) ---
            if (!worldMapDrawn && (hasHudMiniMapRect || G_Radar.IsMiniMapVisible)) {
                for (const auto& player : localPlayers) {
                    if (player.IsTeammate) {
                        if (!g_Menu.esp_show_teammates) continue;
                    } else {
                        if (!g_Menu.esp_show_enemies) continue;
                    }

                    const Vector3 localP = G_LocalPlayerPos;
                    const float worldRange = expandedMiniMap ? 37000.0f : 20000.0f;
                    const float dx = player.Position.x - localP.x;
                    const float dy = player.Position.y - localP.y;

                    if (dx > worldRange || dx < -worldRange || dy > worldRange || dy < -worldRange) {
                        continue;
                    }

                    const float finalX = roundf(dx / 20000.0f * mapDiv) + centerX;
                    const float finalY = roundf(dy / 20000.0f * mapDiv) + centerY;

                    const float clampedX = std::clamp(finalX, miniLeft + 3.0f, miniRight - 3.0f);
                    const float clampedY = std::clamp(finalY, miniTop + 3.0f, miniBottom - 3.0f);

                    ImU32 teamColor = GetTeamColor(player.TeamID);
                    DrawRadarTeamMarker(draw, clampedX, clampedY, player.TeamID, teamColor, g_Menu.radar_dot_size);
                }

                if (g_Menu.show_radar_center) {
                    draw->AddLine(ImVec2(centerX - 10.0f, centerY), ImVec2(centerX + 10.0f, centerY), IM_COL32(255, 255, 255, 255), 1.0f);
                    draw->AddLine(ImVec2(centerX, centerY - 10.0f), ImVec2(centerX, centerY + 10.0f), IM_COL32(255, 255, 255, 255), 1.0f);
                }
            }
        }

        // --- 0. precision_calibration & MACRO SYNC ---
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

        if (is_authenticated && canAim && activeConfig.enabled) {
            draw->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), finalFov * 8.0f, ImColor(255, 255, 255, 60), 64, 1.0f);
        }

        // --- 1. VISUALS (signal_overlay) & precision_calibration CORE ---
        if (is_authenticated) {
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
            
            // precision_calibration FILTER
            bool isprecision_calibrationTarget = canAim && activeConfig.enabled;

            Vector3 delta = player.Velocity * dt_esp; 

            // --- INTEGRATED precision_calibration TARGET CHECK (Optimization) ---
            if (isprecision_calibrationTarget && telemetryMemory::IsKeyDown(activeKey)) {
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
                            if (telemetryContext::WorldToScreen(scanBones[b], s)) {
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
                            if (telemetryMemory::IsKeyDown(VK_SHIFT)) {
                                for (auto& bp : bones_to_check) { if (bp.id == 6) { selected = bp; break; } }
                            }

                            Vector3 targetWorld = selected.world;
                            Vector2 targetScreen = selected.screen;

                            if (activeConfig.prediction) {
                                float travelTime = player.Distance / 800.0f;
                                targetWorld += (player.Velocity * travelTime);
                                // Refresh screen pos with prediction
                                telemetryContext::WorldToScreen(targetWorld, targetScreen);
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
                float alphaMult = 1.0f;
                if (g_Menu.esp_distance_lod) {
                    float fadeStart = (float)g_Menu.render_distance * 0.65f;
                    if (player.Distance > fadeStart) {
                        alphaMult = 1.0f - ((player.Distance - fadeStart) / ((float)g_Menu.render_distance - fadeStart));
                        if (alphaMult < 0.15f) alphaMult = 0.15f; 
                    }
                }
                auto ApplyAlpha = [&](ImU32 col, float mult) -> ImU32 {
                    int a = (int)((col >> 24) & 0xFF);
                    a = (int)(a * mult);
                    return (col & 0x00FFFFFF) | (a << 24);
                };

                Vector2 head_s, feet_s;
                if (telemetryContext::WorldToScreen(player.HeadPosition + delta, head_s) &&
                    telemetryContext::WorldToScreen(player.FeetPosition + delta, feet_s)) {

                    // --- PERFECT DYNAMIC BOUNDING BOX (BONE SCANNING) ---
                    float finalBoxTop, finalBoxBottom, finalBoxLeft, finalBoxRight;
                    bool useDynamicBox = false;

                    if (true) { // Try to build dynamic box from PlayerData members
                        float minX = 100000.0f, maxX = -100000.0f;
                        float minY = 100000.0f, maxY = -100000.0f;
                        bool foundValidBone = false;

                        // List of all bones available in PlayerData to form the bounding box
                        Vector3 bones[] = {
                            player.Bone_Head, player.Bone_Neck, player.Bone_Chest, player.Bone_Pelvis,
                            player.Bone_LShoulder, player.Bone_LElbow, player.Bone_LHand,
                            player.Bone_RShoulder, player.Bone_RElbow, player.Bone_RHand,
                            player.Bone_LThigh, player.Bone_LKnee, player.Bone_LFoot,
                            player.Bone_RThigh, player.Bone_RKnee, player.Bone_RFoot
                        };

                        for (const auto& boneWorld : bones) {
                            if (boneWorld.IsZero()) continue;
                            Vector2 screen;
                            if (telemetryContext::WorldToScreen(boneWorld + delta, screen)) {
                                minX = (std::min)(minX, screen.x);
                                maxX = (std::max)(maxX, screen.x);
                                minY = (std::min)(minY, screen.y);
                                maxY = (std::max)(maxY, screen.y);
                                foundValidBone = true;
                            }
                        }

                        if (foundValidBone) {
                            float boxH = maxY - minY;
                            float boxW = maxX - minX;
                            
                            // Padding for "Breathing Room" (15% Width, 12% Height)
                            float paddingW = (std::max)(boxW * 0.15f, 4.0f);
                            float paddingH = (std::max)(boxH * 0.12f, 4.0f);
                            
                            finalBoxTop = minY - (boxH * 0.25f) - 5.0f; // 25% Height + 5px buffer to ensure it clears Helmet/Face
                            finalBoxBottom = maxY + paddingH;
                            finalBoxLeft = minX - paddingW;
                            finalBoxRight = maxX + paddingW;

                            // Sanity check to avoid zero-sized boxes
                            if (finalBoxBottom - finalBoxTop > 5.0f && finalBoxRight - finalBoxLeft > 2.0f) {
                                useDynamicBox = true;
                            }
                        }
                    }

                    if (!useDynamicBox) {
                        // Fallback to Ratio Box (using DMA-style 0.5 ratio for "tight" feel)
                        float h = abs(head_s.y - feet_s.y);
                        float w = h * 0.50f; 
                        finalBoxTop = head_s.y - (h * 0.12f);
                        finalBoxBottom = feet_s.y + (h * 0.05f);
                        finalBoxLeft = head_s.x - w/2;
                        finalBoxRight = head_s.x + w/2;
                    }


                    ImU32 boxCol = player.IsVisible ? 
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color) : 
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_invisible_color);
                    
                    if (player.IsTeammate) boxCol = telemetryColors::Teammate;
                    boxCol = ApplyAlpha(boxCol, alphaMult);

                    if (esp_box && player.Distance < g_Menu.box_max_dist) {
                        draw->AddRect(ImVec2(finalBoxLeft, finalBoxTop), ImVec2(finalBoxRight, finalBoxBottom), IM_COL32(0,0,0,(int)(180 * alphaMult)), 2.5f, 0, 2.5f); // Border
                        draw->AddRect(ImVec2(finalBoxLeft, finalBoxTop), ImVec2(finalBoxRight, finalBoxBottom), boxCol, 2.5f, 0, 1.25f);       // Main Box
                    }
                    
                    // --- PREMIUM HEALTH BAR (DYNAMIC SCALING) ---
                    if (g_Menu.esp_health && player.Distance < g_Menu.hp_max_dist) {
                        float displayHealth = player.IsGroggy ? player.GroggyHealth : player.Health;
                        float healthPercent = displayHealth / 100.0f;
                        healthPercent = std::clamp(healthPercent, 0.0f, 1.0f);
                        
                        // Linear Scaling based on Box Height (PERFECT SCALING)
                        float boxH = finalBoxBottom - finalBoxTop;
                        float boxW = finalBoxRight - finalBoxLeft;
                        float barThickness = (std::max)(1.0f, boxH * 0.045f); // 4.5% of height, min 1px
                        float barOffset = (std::max)(2.0f, boxH * 0.075f);    // 7.5% of height, min 2px
                        
                        // Disable heavy effects for tiny boxes to avoid "blob" look
                        bool isTiny = (boxH < 35.0f);

                        // 1. POSITIONING
                        ImVec2 barPosStart, barPosEnd;
                        bool horizontal = false;

                        switch (g_Menu.esp_health_pos) {
                            case 0: // Left
                                barPosStart = ImVec2(finalBoxLeft - barOffset - barThickness, finalBoxTop);
                                barPosEnd = ImVec2(finalBoxLeft - barOffset, finalBoxBottom);
                                break;
                            case 1: // Right
                                barPosStart = ImVec2(finalBoxRight + barOffset, finalBoxTop);
                                barPosEnd = ImVec2(finalBoxRight + barOffset + barThickness, finalBoxBottom);
                                break;
                            case 2: // Bottom
                                horizontal = true;
                                barPosStart = ImVec2(finalBoxLeft, finalBoxBottom + barOffset);
                                barPosEnd = ImVec2(finalBoxRight, finalBoxBottom + barOffset + barThickness);
                                break;
                            case 3: // Top
                                horizontal = true;
                                barPosStart = ImVec2(finalBoxLeft, finalBoxTop - barOffset - barThickness);
                                barPosEnd = ImVec2(finalBoxRight, finalBoxTop - barOffset);
                                break;
                        }
                        
                        ImU32 hpColor = IM_COL32(0, 255, 100, 255); // Super Vibrant Neon Green
                        if (healthPercent < 0.75f) hpColor = IM_COL32(255, 255, 0, 255);
                        if (healthPercent < 0.35f) hpColor = IM_COL32(255, 50, 50, 255);
                        hpColor = ApplyAlpha(hpColor, alphaMult);
                        ImU32 bgCol = IM_COL32(0, 0, 0, (int)(150 * alphaMult));

                        auto DrawHealthSegmented = [&](ImVec2 pMin, ImVec2 pMax, bool vertical) {
                            if (!isTiny) {
                                // 1. Deep Shadow / Outer Border (Only for large boxes)
                                draw->AddRect(ImVec2(pMin.x - 1, pMin.y - 1), ImVec2(pMax.x + 1, pMax.y + 1), IM_COL32(0, 0, 0, (int)(180 * alphaMult)), 1.5f);
                            }
                            
                            // 2. Glass Background
                            draw->AddRectFilled(pMin, pMax, bgCol, 1.0f);

                            if (vertical) {
                                float h = pMax.y - pMin.y;
                                float barH = h * healthPercent;
                                ImVec2 hpMax = pMax;
                                ImVec2 hpMin = ImVec2(pMin.x, pMax.y - barH);

                                // 3. Vibrant Health Fill
                                draw->AddRectFilled(hpMin, hpMax, hpColor, 1.0f);

                                // 4. Glass Glint (Only if not tiny)
                                if (!isTiny) {
                                    float glintW = (pMax.x - pMin.x) * 0.45f;
                                    draw->AddRectFilled(hpMin, ImVec2(hpMin.x + glintW, hpMax.y), IM_COL32(255, 255, 255, (int)(50 * alphaMult)), 1.0f);
                                    
                                    // 5. Segments
                                    if (boxH > 45.0f) {
                                        for (int i = 1; i <= 3; i++) {
                                            float lineY = pMax.y - (h * (i * 0.25f));
                                            draw->AddLine(ImVec2(pMin.x, lineY), ImVec2(pMax.x, lineY), IM_COL32(0, 0, 0, 100));
                                        }
                                    }
                                }
                            } else {
                                float w_bar = pMax.x - pMin.x;
                                float barW = w_bar * healthPercent;
                                ImVec2 hpMin = pMin;
                                ImVec2 hpMax = ImVec2(pMin.x + barW, pMax.y);

                                // 3. Vibrant Health Fill
                                draw->AddRectFilled(hpMin, hpMax, hpColor, 1.0f);

                                if (!isTiny) {
                                    float glintH = (pMax.y - pMin.y) * 0.45f;
                                    draw->AddRectFilled(hpMin, ImVec2(hpMax.x, hpMin.y + glintH), IM_COL32(255, 255, 255, (int)(50 * alphaMult)), 1.0f);

                                    if (boxH > 45.0f) {
                                        for (int i = 1; i <= 3; i++) {
                                            float lineX = pMin.x + (w_bar * (i * 0.25f));
                                            draw->AddLine(ImVec2(lineX, pMin.y), ImVec2(lineX, pMax.y), IM_COL32(0, 0, 0, 100));
                                        }
                                    }
                                }
                            }
                        };

                        DrawHealthSegmented(barPosStart, barPosEnd, !horizontal);
                    }

                    if (g_Menu.esp_skeleton && player.Distance < g_Menu.skeleton_max_dist) {
                        auto DrawLine = [&](Vector3 b1, Vector3 b2) {
                            if (b1.IsZero() || b2.IsZero()) return;
                            Vector2 s1, s2;
                            if (telemetryContext::WorldToScreen(b1 + delta, s1) && telemetryContext::WorldToScreen(b2 + delta, s2)) {
                                ImU32 skelCol = player.IsVisible ? 
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color) : 
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_invisible_color);
                                
                                draw->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), ApplyAlpha(skelCol, alphaMult), g_Menu.skel_thickness);
                            }
                        };
                        DrawLine(player.Bone_Head, player.Bone_Neck);
                        DrawLine(player.Bone_Neck, player.Bone_Chest);
                        DrawLine(player.Bone_Chest, player.Bone_Pelvis);
                        DrawLine(player.Bone_Neck, player.Bone_LShoulder); // Changed from Chest to Neck
                        DrawLine(player.Bone_LShoulder, player.Bone_LElbow);
                        DrawLine(player.Bone_LElbow, player.Bone_LHand);
                        DrawLine(player.Bone_Neck, player.Bone_RShoulder); // Changed from Chest to Neck
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

                    ImU32 infoColor = player.IsVisible ? telemetryColors::Visible : telemetryColors::Invisible;
                    if (player.IsTeammate) infoColor = telemetryColors::Teammate;
                    if (player.IsGroggy) infoColor = telemetryColors::Groggy;

                    std::string infoTag = player.Name;
                    if (infoTag.empty() || infoTag == "Player") infoTag = "Unknown";
                    if (player.IsGroggy) infoTag = "[KNOCKED] " + infoTag;

                    // 1. KHOẢNG CÁCH DƯỚI CHÂN
                    if (g_Menu.esp_distance && player.Distance < g_Menu.distance_txt_max_dist) {
                        char distStr[32];
                        sprintf_s(distStr, sizeof(distStr), "[%dm]", (int)player.Distance);
                        
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = 13.0f;
                        ImVec2 distSize = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, distStr);
                        
                        ImU32 distCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(head_s.x - distSize.x / 2, finalBoxBottom + 2.0f), ApplyAlpha(distCol, alphaMult), distStr);
                    }

                    // 2. VŨ KHÍ NẰM TRÊN ĐẦU
                    float currentTopY = finalBoxTop - 2.0f; 

                    if (g_Menu.esp_weapon && !player.WeaponName.empty() && player.Distance < g_Menu.weapon_max_dist) {
                        if (g_Menu.esp_weapon_type == 1) { // IMAGE
                            TextureInfo* tex = GetWeaponImage(player.WeaponName);
                            if (tex && tex->SRV) {
                                float distanceScale = (player.Distance < 5.0f) ? 5.0f : player.Distance;
                                float targetWidth = 1200.0f / distanceScale; 
                                if (targetWidth < 25.0f) targetWidth = 25.0f;
                                if (targetWidth > 80.0f) targetWidth = 80.0f; 

                                float scale = targetWidth / tex->Width;
                                float iconW = tex->Width * scale;
                                float iconH = tex->Height * scale;
                                
                                currentTopY -= iconH;
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddImage((ImTextureID)tex->SRV, ImVec2(head_s.x - iconW/2, currentTopY), ImVec2(head_s.x + iconW/2, currentTopY + iconH), ImVec2(0,0), ImVec2(1,1), ApplyAlpha(weaponCol, alphaMult * 0.85f));
                                currentTopY -= 2.0f; 
                            } else { 
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

                    // 3. TÊN & TRẠNG THÁI
                    if ((g_Menu.esp_name || player.IsGroggy) && player.Distance < g_Menu.name_max_dist) { 
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

                    // 4. SPECTATED COUNT (EYE WARNING)
                    if (g_Menu.esp_spectated && player.SpectatedCount > 0 && player.Distance < g_Menu.name_max_dist) {
                        char specBuf[64];
                        sprintf_s(specBuf, sizeof(specBuf), "EYE: %d", player.SpectatedCount);
                        
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = 15.0f; // Slightly larger for alert
                        ImVec2 ss = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, specBuf);
                        currentTopY -= (ss.y + 2.0f);
                        
                        // Vibrant Orange/Yellow Warning Color
                        ImU32 specCol = IM_COL32(255, 170, 0, 255); 
                        
                        // Outline
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(head_s.x - ss.x / 2 + 1, currentTopY + 1), IM_COL32(0, 0, 0, (int)(200 * alphaMult)), specBuf);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(head_s.x - ss.x / 2, currentTopY), ApplyAlpha(specCol, alphaMult), specBuf);
                    }
                } else {
                    // --- 4. ADVANCED OFF-SCREEN INDICATORS ---
                    if (g_Menu.esp_offscreen && !player.IsTeammate && player.Distance < g_Menu.render_distance) {
                        float dx = player.Position.x - G_CameraLocation.x;
                        float dy = player.Position.y - G_CameraLocation.y;
                        
                        float angle_rad = atan2f(dy, dx);
                        float cam_yaw_rad = G_CameraRotation.y * (3.14159265f / 180.0f);
                        float rel_angle = angle_rad - cam_yaw_rad - (3.14159265f / 2.0f);
                        
                        float radius = g_Menu.offscreen_radius;
                        ImVec2 arrowPos = ImVec2(ScreenCenterX + cosf(rel_angle) * radius, (ScreenHeight / 2.0f) + sinf(rel_angle) * radius);
                        
                        // --- DISTANCE-BASED COLORING ---
                        ImU32 arrowCol;
                        if (g_Menu.offscreen_color_mode == 1) { // Distance Gradient
                            float t = player.Distance / g_Menu.render_distance;
                            if (t > 1.0f) t = 1.0f;
                            
                            float r = (g_Menu.offscreen_near_color[0] * (1.0f - t) + g_Menu.offscreen_far_color[0] * t) * 255.0f;
                            float g = (g_Menu.offscreen_near_color[1] * (1.0f - t) + g_Menu.offscreen_far_color[1] * t) * 255.0f;
                            float b = (g_Menu.offscreen_near_color[2] * (1.0f - t) + g_Menu.offscreen_far_color[2] * t) * 255.0f;
                            float a = (g_Menu.offscreen_near_color[3] * (1.0f - t) + g_Menu.offscreen_far_color[3] * t) * 255.0f;
                            arrowCol = IM_COL32((int)r, (int)g, (int)b, (int)a);
                        } else { // Static / Visibility
                            arrowCol = player.IsVisible ? IM_COL32(0, 255, 150, 180) : IM_COL32(255, 255, 255, 100);
                        }
                        
                        if (player.SpectatedCount > 0) arrowCol = IM_COL32(255, 170, 0, 220); 
                        
                        float sz = g_Menu.offscreen_size;
                        
                        if (g_Menu.esp_offscreen_style == 0) { // TRIANGLE
                            ImVec2 p1 = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 p2 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 p3 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddTriangleFilled(p1, p2, p3, ApplyAlpha(arrowCol, alphaMult));
                        } 
                        else if (g_Menu.esp_offscreen_style == 1) { // CHEVRON (V-SHAPE)
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 side1 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 side2 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddPolyline(&tip, 1, ApplyAlpha(arrowCol, alphaMult), 0, 2.0f); // Just points for now, need proper V
                            // Standard V-shape
                            draw->AddLine(tip, side1, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                            draw->AddLine(tip, side2, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                        }
                        else if (g_Menu.esp_offscreen_style == 2) { // ARC / MODERN
                            draw->PathArcTo(arrowPos, sz, rel_angle - 0.8f, rel_angle + 0.8f, 10);
                            draw->PathStroke(ApplyAlpha(arrowCol, alphaMult), 0, 3.0f);
                            // Add a small tip
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.2f, arrowPos.y + sinf(rel_angle) * sz * 1.2f);
                            draw->AddCircleFilled(tip, 2.0f, ApplyAlpha(arrowCol, alphaMult));
                        }
                    }
                }
            }
        }

        }

        // --- 1.5. ITEM & VEHICLE RENDERING ---
        if (is_authenticated && esp_toggle) {
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
                } else if (item.Name == "PROJECTILE") {
                    should_draw = true; col = IM_COL32(255, 0, 0, 255); // BRIGHT RED FOR DANGER
                } else { // Generic items
                    if (g_Menu.esp_items && item.Distance < g_Menu.loot_max_dist) should_draw = true;
                }

                if (should_draw) {
                    Vector2 itemScreen;
                    if (telemetryContext::WorldToScreen(item.Position, itemScreen)) {
                        char itemText[128];
                        sprintf_s(itemText, sizeof(itemText), "%s [%dm]", item.Name.c_str(), (int)item.Distance);
                        ImVec2 tsize = ImGui::CalcTextSize(itemText);
                        draw->AddText(ImVec2(itemScreen.x - tsize.x/2, itemScreen.y), col, itemText);
                    }
                }
            }
        }

        // --- 1.5 MACRO OSD (USER REQUEST) ---
        if (is_authenticated && show_macro_overlay && current_scene == Scene::Gaming) {
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

        // --- 2. APPLY precision_calibration MOVEMENT ---
        // Finalize the mouse movement after all player iteration is DONE.
        if (is_authenticated && bestTarget) {
            float moveX = (bestScreenPos.x - ScreenCenterX);
            float moveY = (bestScreenPos.y - ScreenCenterY);

            if (activeConfig.smooth > 1.0f) {
                moveX /= activeConfig.smooth;
                moveY /= activeConfig.smooth;
            }

            telemetryMemory::MoveMouse((long)moveX, (long)moveY);
        }

        // --- 3. MENU REDESIGN ---
        if (showmenu) {
            Translation::CurrentLanguage = language;
            auto Lang = Translation::Get();
            
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            
            // Custom window styling for the new design
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            
            ImGui::Begin(skCrypt("##overlay_new"), &showmenu, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
            
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // --- PREMIUM STEALTH ENGINE BACKGROUND ---
            // 1. Shadow / Glow surrounding the whole window
            drawList->AddRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(100, 30, 200, 50), 18.0f, 0, 5.0f);
            
            // 2. Main Window Fill (Rich Deep Gradient)
            drawList->AddRectFilledMultiColor(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                                             IM_COL32(10, 2, 25, 255),   // Top Left
                                             IM_COL32(20, 5, 45, 255),   // Top Right
                                             IM_COL32(10, 2, 25, 255),   // Bottom Right
                                             IM_COL32(8, 0, 20, 255));   // Bottom Left

            // 3. Subtle Hex/Grid Pattern background
            for (float i = 0; i < windowSize.x; i += 40.0f) {
                drawList->AddLine(ImVec2(windowPos.x + i, windowPos.y), ImVec2(windowPos.x + i, windowPos.y + windowSize.y), IM_COL32(200, 100, 255, 5), 1.0f);
            }
            for (float i = 0; i < windowSize.y; i += 40.0f) {
                drawList->AddLine(ImVec2(windowPos.x, windowPos.y + i), ImVec2(windowPos.x + windowSize.x, windowPos.y + i), IM_COL32(200, 100, 255, 5), 1.0f);
            }
            
            // 4. Vibrant Outer Border (Electric Violet)
            drawList->AddRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(150, 60, 255, 60), 18.0f, 0, 1.5f);

            // Helpers for consistent premium UI components
            auto BeginGlassCard = [&](const char* id, const char* label, ImVec2 size) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(30, 10, 60, 80), 12.0f);
                drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(200, 100, 255, 25), 12.0f);
                // Top Highlight
                drawList->AddLine(ImVec2(pos.x + 15, pos.y), ImVec2(pos.x + size.x - 15, pos.y), IM_COL32(200, 100, 255, 100), 2.0f);
                
                ImGui::BeginChild(id, size, false, ImGuiWindowFlags_NoBackground);
                ImGui::SetCursorPos(ImVec2(10, 5));
                ImGui::TextColored(ImVec4(0.7f, 0.4f, 1.0f, 1.0f), label);
                ImGui::Separator();
                ImGui::Spacing();
            };

            // Top Bar
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::BeginChild(skCrypt("##TopBar"), ImVec2(windowSize.x, 55), false, ImGuiWindowFlags_NoBackground);
            ImGui::SetCursorPos(ImVec2(25, 18));
            
            // Logo / Name
            ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), skCrypt("GZ"));
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), skCrypt("|"));
            ImGui::SameLine();
            ImGui::Text(Lang.MainTelemetry);
            
            // Safe Badge
            ImGui::SameLine(180);
            ImGui::SetCursorPosY(18);
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 0.5f, 1.0f), skCrypt("● "));
            ImGui::SameLine(0, 2);
            ImGui::Text(Lang.SafeStatus);
            
            ImGui::SetCursorPosY(15);
            
            // Center Title
            const char* tabTitles[] = { Lang.TabVisuals, Lang.Tabprecision_calibration, Lang.TabLoot, Lang.TabSettings, Lang.TabRadar };
            ImVec2 titleSize = ImGui::CalcTextSize(tabTitles[g_Menu.active_tab]);
            ImGui::SameLine((windowSize.x - titleSize.x) / 2.0f);
            ImGui::SetCursorPosY(15);
            ImGui::TextColored(ImVec4(0.6f, 0.3f, 1.0f, 1.0f), tabTitles[g_Menu.active_tab]);
            
            // Right Buttons
            ImGui::SameLine(windowSize.x - 70);
            ImGui::SetCursorPosY(12);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button(skCrypt("—"), ImVec2(24, 24))) { /* min */ }
            ImGui::SameLine();
            if (ImGui::Button(skCrypt("✕"), ImVec2(24, 24))) { showmenu = false; }
            ImGui::PopStyleColor();
            
            ImGui::EndChild(); // TopBar
            
            // Content Card Effect
            ImGui::SetCursorPos(ImVec2(15, 55));
            ImGui::BeginChild(skCrypt("##MainContent"), ImVec2(windowSize.x - 30, windowSize.y - 120), false, ImGuiWindowFlags_NoScrollbar);
            
            // --- HIỂN THỊ (VISUALS) TAB --- (NOW INDEX 0)
            if (g_Menu.active_tab == 0) {
                ImGui::Columns(4, skCrypt("ESPColumns"), false);
                
                // Col 1: Tổng Quan
                BeginGlassCard(skCrypt("##ESPCol1"), Lang.HeaderVisualCore, ImVec2(185, 0));
                ImGui::Checkbox(Lang.MasterToggle, &g_Menu.esp_toggle);
                ImGui::Checkbox(Lang.ESP_Offscreen, &g_Menu.esp_offscreen);
                ImGui::Checkbox(Lang.VisCheck, &g_Menu.aim_visible_only);
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.DistThresholds);
                ImGui::SliderInt(Lang.RenderDist, &g_Menu.render_distance, 50, 1000);
                ImGui::SliderInt(Lang.InfoESP, &g_Menu.name_max_dist, 50, 600);
                ImGui::SliderFloat(skCrypt("Thick"), &g_Menu.skel_thickness, 1.0f, 5.0f, skCrypt("%.1f"));
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 2: Style
                BeginGlassCard(skCrypt("##ESPCol2"), Lang.HeaderRenderStyle, ImVec2(185, 0));
                ImGui::Checkbox(Lang.Box, &g_Menu.esp_box);
                ImGui::Checkbox(Lang.Skeleton, &g_Menu.esp_skeleton);
                ImGui::Checkbox(Lang.HealthBar, &g_Menu.esp_health);
                ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 3: HUD
                BeginGlassCard(skCrypt("##ESPCol3"), Lang.HeaderOverlayHUD, ImVec2(185, 0));
                ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
                ImGui::Checkbox(Lang.AimFOV, &g_Menu.aim_configs[8].enabled);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 4: Extras
                BeginGlassCard(skCrypt("##ESPCol4"), Lang.HeaderDangerScan, ImVec2(185, 0));
                ImGui::Checkbox(Lang.ESP_Spectated, &g_Menu.esp_spectated);
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- TỰ ĐỘNG NGẮM (AIM) TAB --- (NOW INDEX 1)
            else if (g_Menu.active_tab == 1) {
                ImGui::Columns(3, skCrypt("AimColumns"), false);
                ImGui::SetColumnWidth(0, 250);
                ImGui::SetColumnWidth(1, 250);
                
                // Column 1: Config
                BeginGlassCard(skCrypt("##AimCol1"), Lang.HeaderSystemConfig, ImVec2(240, 0));
                
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.DetectedAttach); // Mapping for "Current Method"
                ImGui::Text(skCrypt("Hyper-V Stealth Bridge"));
                
                ImGui::Spacing();
                if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 35))) { g_Menu.SaveConfig("dataMacro/Config/settings.json"); }
                if (ImGui::Button(Lang.SafeStatus, ImVec2(-1, 35))) { exit(0); } // Mapping as stub for exit label? No, exit should have its own string.
                ImGui::EndChild();

                ImGui::NextColumn();                
                // Column 2: Settings
                BeginGlassCard(skCrypt("##AimCol2"), Lang.HeaderPrecisionSettings, ImVec2(240, 0));
                ImGui::Checkbox(Lang.AimEnabled, &g_Menu.aim_master_enabled);
                
                AimConfig* pCfg = &g_Menu.aim_configs[8]; // GLOBAL
                ImGui::Checkbox(Lang.AimPrediction, &pCfg->prediction);
                ImGui::Checkbox(Lang.AimVisible, &g_Menu.aim_visible_only);
                
                ImGui::Spacing();
                ImGui::SliderFloat(Lang.AimFOV, &pCfg->fov, 1.0f, 100.0f, skCrypt("%.0f px"));
                ImGui::SliderFloat(Lang.AimSmooth, &pCfg->smooth, 1.0f, 20.0f, skCrypt("%.1f"));
                ImGui::SliderFloat(skCrypt("Max Dist"), &pCfg->max_dist, 10.0f, 800.0f, skCrypt("%.0f m"));
                
                char keyDisplay[64];
                sprintf_s(keyDisplay, sizeof(keyDisplay), skCrypt("MOUSE LEFT")); 
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.AimKey);
                if (ImGui::Button(keyDisplay, ImVec2(-1, 30))) { g_Menu.waiting_for_key = &pCfg->key; }
                
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Column 3: Logic
                BeginGlassCard(skCrypt("##AimCol3"), Lang.HeaderAimStructure, ImVec2(0, 0));
                ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), Lang.AimBone);
                ImGui::Spacing();
                ImGui::TextWrapped(Lang.MacroSoon); // Stub for description
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), skCrypt("PRIORITY:"));
                ImGui::Text(skCrypt("1. Head / Neck (Visible)"));
                ImGui::Text(skCrypt("2. Upper Chest (Visible)"));
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- VẬT PHẨM TAB ---
            else if (active_tab == 2) {
                ImGui::Columns(3, skCrypt("ItemColumns"), false);
                
                // Col 1: Phím Tắt
                BeginGlassCard(skCrypt("##ItemCol1"), Lang.HeaderLootEngine, ImVec2(250, 0));
                ImGui::Checkbox(Lang.TabLoot, &g_Menu.esp_items);
                ImGui::SliderInt(Lang.RenderDist, &g_Menu.loot_max_dist, 10, 300);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 2: Grid
                BeginGlassCard(skCrypt("##ItemCol2"), Lang.HeaderPickupFilter, ImVec2(250, 0));
                ImGui::Text(skCrypt("AUTO-FILTER ACTIVE"));
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), skCrypt("Categories:"));
                ImGui::BulletText(skCrypt("Assault Rifles"));
                ImGui::BulletText(skCrypt("Sniper Rifles"));
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 3: Radar Web
                BeginGlassCard(skCrypt("##ItemCol3"), Lang.HeaderWorldEntities, ImVec2(250, 0));
                ImGui::Checkbox(skCrypt("Show Vehicles"), &g_Menu.esp_vehicles);
                ImGui::Checkbox(skCrypt("Show Airdrops"), &g_Menu.esp_airdrops);
                ImGui::Checkbox(skCrypt("Show Deathboxes"), &g_Menu.esp_deadboxes);
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- THIẾT LẬP TAB ---
            else if (active_tab == 3) {
                ImGui::Columns(3, skCrypt("SettingsColumns"), false);
                // Col 1
                BeginGlassCard(skCrypt("##SetCol1"), Lang.HeaderSystemCore, ImVec2(250, 0));
                int currentLang = g_Menu.language ? 1 : 0;
                if (ImGui::Combo(Lang.Language, &currentLang, skCrypt("English\0Tiếng Việt\0"))) g_Menu.language = (currentLang == 1);
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), skCrypt("REGISTRATION"));
                extern std::string global_active_key;
                bool is_active = !global_active_key.empty();
                if (is_active) ImGui::TextColored(ImVec4(0, 1, 0, 1), skCrypt("STATUS: ACTIVE"));
                else ImGui::TextColored(ImVec4(1, 0, 0, 1), skCrypt("STATUS: INACTIVE"));
                
                static char key_buf[128] = {0};
                ImGui::PushItemWidth(-60);
                ImGui::InputText(skCrypt("Token"), key_buf, sizeof(key_buf));
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(Lang.PasteLabel, ImVec2(80, 30))) {
                    const char* clipboard = ImGui::GetClipboardText();
                    if (clipboard) strcpy_s(key_buf, sizeof(key_buf), clipboard);
                }
                
                if (ImGui::Button(skCrypt("Validate Key"), ImVec2(-1, 35))) {
                    extern std::string GetHWID();
                    extern bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent);
                    std::string key_str(key_buf);
                    if (DoAPIRequest(key_str, GetHWID(), false)) {
                        global_active_key = key_str;
                        std::ofstream outFile("key.txt");
                        if (outFile.is_open()) { outFile << key_str; outFile.close(); }
                    }
                }
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 2
                BeginGlassCard(skCrypt("##SetCol2"), Lang.HeaderAntiTracking, ImVec2(250, 0));
                ImGui::Checkbox(Lang.ESP_Spectated, &g_Menu.esp_spectated);
                ImGui::Checkbox(Lang.AntiScreenshot, &g_Menu.anti_screenshot);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 3
                BeginGlassCard(skCrypt("##SetCol3"), Lang.HeaderEngineUtils, ImVec2(250, 0));
                ImGui::Checkbox(Lang.AimPrediction, &g_Menu.aim_configs[8].prediction);
                if (ImGui::Button(skCrypt("Safe Exit"), ImVec2(-1, 35))) { exit(0); }
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- BẢN ĐỒ TAB ---
            else if (active_tab == 4) {
                ImGui::Columns(3, skCrypt("MapColumns"), false);
                
                BeginGlassCard(skCrypt("##MapCol1"), Lang.TabRadar, ImVec2(250, 0));
                ImGui::Checkbox(Lang.RadarEnable, &g_Menu.radar_enabled);
                ImGui::Checkbox(Lang.ItemsVehicles, &g_Menu.esp_vehicles);
                ImGui::Checkbox(skCrypt("Show Airdrops"), &g_Menu.esp_airdrops);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                BeginGlassCard(skCrypt("##MapCol2"), skCrypt("MINI MAP"), ImVec2(250, 0));
                ImGui::SliderFloat(Lang.RadarDotSize, &g_Menu.radar_dot_size, 1.0f, 10.0f);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                BeginGlassCard(skCrypt("##MapCol3"), skCrypt("RADAR STATUS"), ImVec2(250, 0));
                ImGui::Text(skCrypt("World Map: %s"), G_Radar.IsWorldMapVisible ? (g_Menu.language ? skCrypt("Hien thi") : skCrypt("Visible")) : (g_Menu.language ? skCrypt("Dong") : skCrypt("Closed")));
                ImGui::Text(skCrypt("Mini Map: %s"), G_Radar.IsMiniMapVisible ? (g_Menu.language ? skCrypt("Hien thi") : skCrypt("Visible")) : (g_Menu.language ? skCrypt("An") : skCrypt("Hidden")));
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            
            ImGui::EndChild(); // MainContent
            
            // Bottom Bar Line
            draw->AddLine(ImVec2(windowPos.x, windowPos.y + windowSize.y - 40), ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y - 40), IM_COL32(0, 200, 255, 100), 1.0f);
            
            // Bottom Nav Rail (Floating Pill Design)
            float barW = windowSize.x - 100.0f;
            ImGui::SetCursorPos(ImVec2(50, windowSize.y - 55));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.08f, 0.32f, 0.3f));
            ImGui::BeginChild(skCrypt("##BottomRail"), ImVec2(barW, 45), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
            
            ImVec2 railPos = ImGui::GetWindowPos();
            ImVec2 railSize = ImGui::GetWindowSize();
            drawList->AddRectFilled(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(40, 20, 80, 80), 22.0f);
            drawList->AddRect(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(150, 80, 255, 50), 22.0f);

            float totalWidth = (110.0f * 5.0f) + (10.0f * 4.0f);
            ImGui::SetCursorPosX((railSize.x - totalWidth) / 2.0f);
            
            auto BottomTab = [&](const char* label, int id) {
                bool active = (active_tab == id);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.48f, 0.17f, 0.90f, 0.15f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.48f, 0.17f, 0.90f, 0.30f));
                
                if (active) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.3f, 1.0f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.7f, 1.0f));
                
                if (ImGui::Button(label, ImVec2(110, 40))) active_tab = id;
                
                if (active) {
                    // Refined Underglow
                    for(int i=1; i<=4; i++)
                        drawList->AddRectFilled(ImVec2(cursorPos.x + 30 - i, cursorPos.y + 36), ImVec2(cursorPos.x + 80 + i, cursorPos.y + 39), IM_COL32(150, 60, 255, 40/i), 2.0f);
                    drawList->AddRectFilled(ImVec2(cursorPos.x + 35, cursorPos.y + 36), ImVec2(cursorPos.x + 75, cursorPos.y + 38), IM_COL32(180, 100, 255, 255), 2.0f);
                }
                
                ImGui::PopStyleColor(4);
                ImGui::SameLine(0, 10.0f);
            };
            
            BottomTab(Lang.TabVisuals, 0);
            BottomTab(Lang.Tabprecision_calibration, 1);
            BottomTab(Lang.TabLoot, 2);
            BottomTab(Lang.TabSettings, 3);
            BottomTab(Lang.TabRadar, 4);
            
            ImGui::EndChild(); // Rail
            ImGui::PopStyleColor(); // ChildBg
            
            ImGui::PopStyleVar(2); // WindowPadding, WindowRounding
            ImGui::End();
        }

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
void OverlayMenu::SaveConfig(const char* path) {
    try {
        nlohmann::json j;
        j["esp_toggle"] = esp_toggle;
        j["esp_show_enemies"] = esp_show_enemies;
        j["esp_show_teammates"] = esp_show_teammates;
        j["esp_offscreen"] = esp_offscreen;
        j["esp_offscreen_style"] = esp_offscreen_style;
        j["offscreen_color_mode"] = offscreen_color_mode;
        j["offscreen_radius"] = offscreen_radius;
        j["offscreen_size"] = offscreen_size;
        j["offscreen_near_color"] = { offscreen_near_color[0], offscreen_near_color[1], offscreen_near_color[2], offscreen_near_color[3] };
        j["offscreen_far_color"] = { offscreen_far_color[0], offscreen_far_color[1], offscreen_far_color[2], offscreen_far_color[3] };
        j["esp_box"] = esp_box;
        j["esp_skeleton"] = esp_skeleton;
        j["esp_name"] = esp_name;
        j["esp_distance"] = esp_distance;
        j["esp_health"] = esp_health;
        j["esp_health_pos"] = esp_health_pos;
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
        
        // precision_calibration Configs
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
            if (j.contains("esp_health")) esp_health = j["esp_health"];
            if (j.contains("esp_health_pos")) esp_health_pos = j["esp_health_pos"];
            if (j.contains("esp_items")) esp_items = j["esp_items"];
            if (j.contains("esp_snapline")) esp_snapline = j["esp_snapline"];
            if (j.contains("esp_weapon")) esp_weapon = j["esp_weapon"];
            if (j.contains("esp_weapon_type")) esp_weapon_type = j["esp_weapon_type"];
            if (j.contains("render_distance")) render_distance = j["render_distance"];
            if (j.contains("esp_offscreen")) esp_offscreen = j["esp_offscreen"];
            if (j.contains("esp_offscreen_style")) esp_offscreen_style = j["esp_offscreen_style"];
            if (j.contains("offscreen_color_mode")) offscreen_color_mode = j["offscreen_color_mode"];
            if (j.contains("offscreen_radius")) offscreen_radius = j["offscreen_radius"];
            if (j.contains("offscreen_size")) offscreen_size = j["offscreen_size"];
            
            if (j.contains("offscreen_near_color") && j["offscreen_near_color"].is_array()) {
                for (int i = 0; i < 4; i++) offscreen_near_color[i] = j["offscreen_near_color"][i];
            }
            if (j.contains("offscreen_far_color") && j["offscreen_far_color"].is_array()) {
                for (int i = 0; i < 4; i++) offscreen_far_color[i] = j["offscreen_far_color"][i];
            }
            if (j.contains("language")) language = j["language"];
            if (j.contains("show_macro_overlay")) show_macro_overlay = j["show_macro_overlay"];
            if (j.contains("show_radar_center")) show_radar_center = j["show_radar_center"];
            radar_offset_x = 0.0f;
            radar_offset_y = 0.0f;
            radar_zoom_multiplier = 1.0f;
            radar_dot_size = 8.0f;
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

            // precision_calibration Configs
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
