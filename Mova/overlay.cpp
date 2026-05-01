// ============================================================================
// DX11 Overlay - Hijack Xbox Game Bar widget window
// + Auto-resize to follow PUBG game window
// + OBS/capture invisibility via DKOM (direct kernel write of WDA_EXCLUDEFROMCAPTURE)
// + Background exploit thread for lag-free rendering
// ============================================================================
#include "overlay.h"
#include <Windows.h>
#include <dwmapi.h>
#include <cmath>
#include <thread>
#include <atomic>
#pragma comment(lib, "winmm.lib") // for timeBeginPeriod/timeEndPeriod

#include <d3d11.h>
#include <dxgi.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include "config.h"
#include "brand_profile.h"
#include "render.h"
#include "menu.h"
#include "Vector.h"
#include "fonts/byte_array.h"
#include "defs.h"
#include "recoil_macro.h"
#include "item_icons.h"
#include "aimbot.h"
#include <obfusheader.hpp>
#include "UnrealExploit/UnrealExploit.h"
#include "anti_capture.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

// ---------------------------------------------------------------------------
// WDA_EXCLUDEFROMCAPTURE (Win10 2004+ / build 19041+)
// ---------------------------------------------------------------------------
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

// Forward declarations for SubclassWinProc (defined after DX11 namespace)
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static WNDPROC g_origWndProc = nullptr;
static LRESULT CALLBACK SubclassWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
namespace OverlayWindow {
    HWND Hwnd = nullptr;
}

static HWND g_gameHwnd = nullptr;
static bool g_overlayVisible = true;  // Track overlay visibility to skip rendering

// ---------------------------------------------------------------------------
// Background MemoryExploitLoop thread
// Runs the heavy driver-read loop off the render thread so frames never stall.
// Uses double-buffer: exploit thread writes to shadow, then atomically swaps.
// ---------------------------------------------------------------------------
static std::thread g_exploitThread;
static std::atomic<bool> g_exploitRunning{false};
std::atomic<int>  g_exploitGeneration{0};  // Non-static: extern'd by render.cpp

// Shadow buffer for double-buffered entity data
static EntityData g_shadowEntityData;

static void ExploitThreadFunc()
{
    while (g_exploitRunning.load(std::memory_order_relaxed))
    {
        __try {
            MemoryExploitLoop();
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Sleep(1000);
        }
        g_exploitGeneration.fetch_add(1, std::memory_order_release);
        Sleep(200); // ~5Hz refresh
    }
}

void StartExploitThread()
{
    g_exploitRunning.store(true);
    g_exploitThread = std::thread(ExploitThreadFunc);
}

void StopExploitThread()
{
    g_exploitRunning.store(false);
    if (g_exploitThread.joinable())
        g_exploitThread.join();
}

namespace DX11 {
    ID3D11Device*            g_pd3dDevice = nullptr;
    ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
    IDXGISwapChain*          g_pSwapChain = nullptr;
    ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
    MSG                      Message = { 0 };
}

// Fonts
ImFont* mainfont    = nullptr;
ImFont* smallfont   = nullptr;
ImFont* iconfont    = nullptr;
ImFont* logofont    = nullptr;

// ---------------------------------------------------------------------------
// Helper: Create/Cleanup Render Target
// ---------------------------------------------------------------------------
static void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer = nullptr;
    DX11::g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer)
    {
        DX11::g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &DX11::g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupRenderTarget()
{
    if (DX11::g_mainRenderTargetView) { DX11::g_mainRenderTargetView->Release(); DX11::g_mainRenderTargetView = nullptr; }
}

// ---------------------------------------------------------------------------
// SubclassWinProc — intercept messages for ImGui on hijacked Movavi window
// ---------------------------------------------------------------------------
static LRESULT CALLBACK SubclassWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_DESTROY:
    case WM_CLOSE:
    case WM_QUIT:
        return 0;
    case WM_SIZE:
        if (DX11::g_pd3dDevice && DX11::g_pSwapChain && wParam != SIZE_MINIMIZED) {
            __try {
                CleanupRenderTarget();
                DX11::g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
        return 0;
    }
    return g_origWndProc ? CallWindowProc(g_origWndProc, hWnd, msg, wParam, lParam)
                         : DefWindowProc(hWnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// DirectX11 Init (on hijacked window)
// ---------------------------------------------------------------------------
static bool DirectXInit()
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;                                        // Double buffer for proper pipelining
    sd.BufferDesc.Width = Width;
    sd.BufferDesc.Height = Height;
    sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0;   // Unlocked FPS
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = OverlayWindow::Hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;              // Must use DISCARD for WS_EX_LAYERED + DWM alpha transparency

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        0, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &DX11::g_pSwapChain,
        &DX11::g_pd3dDevice, &featureLevel, &DX11::g_pd3dDeviceContext);

    if (FAILED(hr))
        return false;

    CreateRenderTarget();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load fonts from embedded byte_array.h (Lexend + icons)
    ImFontConfig iconfontcfg;
    iconfontcfg.MergeMode = true;
    iconfontcfg.GlyphMaxAdvanceX = 13.0f;
    static const ImWchar icon_ranges[] = { 0xe900, 0xe909, 0 };

    mainfont  = io.Fonts->AddFontFromMemoryTTF(lexend, sizeof(lexend), 16.0f);
    smallfont = io.Fonts->AddFontFromMemoryTTF(lexend, sizeof(lexend), 13.0f);
    iconfont  = io.Fonts->AddFontFromMemoryTTF(iconfontmem, sizeof(iconfontmem), 17.0f, &iconfontcfg, icon_ranges);
    logofont  = io.Fonts->AddFontFromMemoryTTF(lexendsb, sizeof(lexendsb), 31.0f);

    // Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding  = 6.0f;
    style.FrameRounding   = 4.0f;
    style.GrabRounding    = 3.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize  = 0.0f;
    style.PopupBorderSize  = 0.0f;
    style.FrameBorderSize  = 0.0f;
    style.ScrollbarSize    = 8.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowShadowSize  = 0.0f;  // CRITICAL: disable shadow, it creates dark fringe on color-keyed overlay

    style.Colors[ImGuiCol_WindowBg]        = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_ChildBg]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_PopupBg]         = ImVec4(0.12f, 0.12f, 0.12f, 0.96f);
    style.Colors[ImGuiCol_Border]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_BorderShadow]    = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg]         = ImVec4(BRAND_ACCENT_Rf * 0.15f, BRAND_ACCENT_Gf * 0.15f, BRAND_ACCENT_Bf * 0.15f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered]  = ImVec4(BRAND_ACCENT_Rf * 0.25f, BRAND_ACCENT_Gf * 0.25f, BRAND_ACCENT_Bf * 0.25f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive]   = ImVec4(BRAND_ACCENT_Rf * 0.35f, BRAND_ACCENT_Gf * 0.35f, BRAND_ACCENT_Bf * 0.35f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_TitleBg]         = ImVec4(0.12f, 0.12f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]   = ImVec4(0.20f, 0.18f, 0.35f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab]      = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive]= ImVec4(BRAND_ACCENT_Rf * 1.1f, BRAND_ACCENT_Gf * 1.3f, BRAND_ACCENT_Bf * 1.2f, 1.0f);
    style.Colors[ImGuiCol_CheckMark]       = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 1.0f);
    style.Colors[ImGuiCol_Button]          = ImVec4(BRAND_ACCENT_Rf * 0.15f, BRAND_ACCENT_Gf * 0.15f, BRAND_ACCENT_Bf * 0.15f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered]   = ImVec4(BRAND_ACCENT_Rf * 0.25f, BRAND_ACCENT_Gf * 0.25f, BRAND_ACCENT_Bf * 0.25f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive]    = ImVec4(BRAND_ACCENT_Rf * 0.40f, BRAND_ACCENT_Gf * 0.30f, BRAND_ACCENT_Bf * 0.35f + 0.08f, 1.0f);
    style.Colors[ImGuiCol_Header]          = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.40f);
    style.Colors[ImGuiCol_HeaderHovered]   = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.60f);
    style.Colors[ImGuiCol_HeaderActive]    = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]     = ImVec4(0.10f, 0.10f, 0.10f, 0.3f);
    style.Colors[ImGuiCol_ScrollbarGrab]   = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.4f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.6f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(BRAND_ACCENT_Rf, BRAND_ACCENT_Gf, BRAND_ACCENT_Bf, 0.8f);

    ImGui_ImplWin32_Init(OverlayWindow::Hwnd);
    ImGui_ImplDX11_Init(DX11::g_pd3dDevice, DX11::g_pd3dDeviceContext);
    return true;
}

// ---------------------------------------------------------------------------
// Window Procedure — old SubclassWinProc removed, OverlayWndProc is in SetupOverlay
// ---------------------------------------------------------------------------
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---------------------------------------------------------------------------
// FindGameWindow — find PUBG window by title or class
// ---------------------------------------------------------------------------
static HWND FindGameWindow()
{
    HWND hwnd = FindWindowA(NULL, OBF("PUBG: BATTLEGROUNDS "));
    if (!hwnd) hwnd = FindWindowA(OBF("UnrealWindow"), NULL);
    return hwnd;
}

// ---------------------------------------------------------------------------
// SetupOverlay - Hijack Screen Recorder window + DX11 + game tracking
// ---------------------------------------------------------------------------
//void SetupOverlay()
//{
//    // TÌM CỬA SỔ "SCREEN RECORDER" ĐỂ HIJACK
//    OverlayWindow::Hwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
//    if (!OverlayWindow::Hwnd) {
//        const char* recorderPath = "C:\\Users\\ADMIN\\AppData\\Roaming\\Movavi Screen Recorder\\ScreenRecorder.exe";
//        if (GetFileAttributesA(recorderPath) == INVALID_FILE_ATTRIBUTES) {
//            // File not found — show error and exit
//            MessageBoxA(NULL,
//                "Screen Recorder not found!\n\n"
//                "Make sure Movavi Screen Recorder is installed\n"
//                "and running before starting the cheat.",
//                "Error", MB_OK | MB_ICONERROR);
//            ExitProcess(-1);
//        }
//        // File exists — launch and wait for window
//        ShellExecuteA(NULL, "open", recorderPath, NULL, NULL, SW_SHOWMINIMIZED);
//        printf("[OVERLAY] Waiting for Screen Recorder...\n");
//        while (!OverlayWindow::Hwnd) {
//            OverlayWindow::Hwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
//            Sleep(100);
//        }
//        printf("[OVERLAY] Screen Recorder found!\n");
//    }
//
//    // Tìm cửa sổ game
//    g_gameHwnd = FindGameWindow();
//
//    // Lấy kích thước và vị trí CLIENT AREA (vùng render thực tế, không có title bar/border)
//    int posX = 0, posY = 0;
//    if (g_gameHwnd && IsWindow(g_gameHwnd)) {
//        RECT rc;
//        GetClientRect(g_gameHwnd, &rc);
//        POINT topLeft = { 0, 0 };
//        ClientToScreen(g_gameHwnd, &topLeft);
//        posX = topLeft.x;
//        posY = topLeft.y;
//        Width = rc.right;
//        Height = rc.bottom;
//    }
//    else {
//        Width = GetSystemMetrics(SM_CXSCREEN);
//        Height = GetSystemMetrics(SM_CYSCREEN);
//    }
//
//    // Phá vỡ toàn bộ các giới hạn Window của Qt, thiết lập thành nền trơn (POPUP)
//    SetWindowLong(OverlayWindow::Hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
//
//    // Thay đổi Giao diện mở rộng thành kính trong suốt, click-through, topmost
//    SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE,
//        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);
//
//    // Resize theo game window
//    MoveWindow(OverlayWindow::Hwnd, posX, posY, Width, Height, TRUE);
//
//    // Per-pixel alpha transparency via DWM composition
//    // (LWA_COLORKEY doesn't work well with DX11; use DWM + alpha channel instead)
//    SetLayeredWindowAttributes(OverlayWindow::Hwnd, 0, 255, LWA_ALPHA);
//
//    // Extend frame vào client area (DWM composition)
//    MARGINS margins = { -1, -1, -1, -1 };
//    DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &margins);
//
//    ShowWindow(OverlayWindow::Hwnd, SW_SHOWDEFAULT);
//    UpdateWindow(OverlayWindow::Hwnd);
//
//    // OBS/capture invisibility — best-effort, no error if unsupported
//    SetWindowDisplayAffinity(OverlayWindow::Hwnd, WDA_EXCLUDEFROMCAPTURE);
//
//    // Subclass window procedure to handle ImGui events
//    g_origWndProc = (WNDPROC)SetWindowLongPtr(OverlayWindow::Hwnd, GWLP_WNDPROC, (LONG_PTR)SubclassWinProc);
//
//    // Init DX11 + ImGui on hijacked window
//    if (!DirectXInit()) ExitProcess(-1);
//}

// ---------------------------------------------------------------------------
// Remote-thread anti-capture: inject SetWindowDisplayAffinity call into
// the GameBar process so it runs from the window's owning process (no Error 5)
// ---------------------------------------------------------------------------
static bool RemoteSetWindowDisplayAffinity(DWORD targetPid, HWND hwnd)
{
    HANDLE hProc = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
        PROCESS_VM_WRITE | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
        FALSE, targetPid);
    if (!hProc) {
        printf(OBF("[!] Anti-capture: OpenProcess failed (err=%u)\n"), GetLastError());
        return false;
    }

    // user32.dll is already loaded in every GUI process (GameBar included)
    // GetProcAddress returns the same virtual address across processes for system DLLs
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    auto pSetWDA = (FARPROC)GetProcAddress(hUser32, "SetWindowDisplayAffinity");
    if (!pSetWDA) {
        printf(OBF("[!] Anti-capture: SetWindowDisplayAffinity not found\n"));
        CloseHandle(hProc);
        return false;
    }

    // Build a tiny parameter block + x64 shellcode in the target process
    // The shellcode calls: SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)
    //
    // Layout in remote allocation:
    //   [0x00] uint64_t  hwnd_value
    //   [0x08] uint64_t  affinity_value (0x11)
    //   [0x10] uint64_t  pSetWindowDisplayAffinity
    //   [0x18] shellcode bytes
    //
    // Shellcode (x64):
    //   mov rax, [rcx+0x10]    ; load pSetWDA from param block
    //   mov rdx, [rcx+0x08]    ; load affinity (2nd arg)
    //   mov rcx, [rcx+0x00]    ; load hwnd (1st arg)
    //   sub rsp, 0x28          ; shadow space + alignment
    //   call rax
    //   add rsp, 0x28
    //   xor eax, eax
    //   ret

    struct RemoteParams {
        uint64_t hwndVal;
        uint64_t affinityVal;
        uint64_t pFunc;
    };

    uint8_t shellcode[] = {
        0x48, 0x8B, 0x41, 0x10,         // mov rax, [rcx+0x10]
        0x48, 0x8B, 0x51, 0x08,         // mov rdx, [rcx+0x08]
        0x48, 0x8B, 0x09,               // mov rcx, [rcx]
        0x48, 0x83, 0xEC, 0x28,         // sub rsp, 0x28
        0xFF, 0xD0,                     // call rax
        0x48, 0x83, 0xC4, 0x28,         // add rsp, 0x28
        0x33, 0xC0,                     // xor eax, eax
        0xC3                            // ret
    };

    size_t totalSize = sizeof(RemoteParams) + sizeof(shellcode);
    void* remoteMem = VirtualAllocEx(hProc, NULL, totalSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteMem) {
        printf(OBF("[!] Anti-capture: VirtualAllocEx failed\n"));
        CloseHandle(hProc);
        return false;
    }

    // Write params
    RemoteParams params = {};
    params.hwndVal = (uint64_t)hwnd;
    params.affinityVal = 0x00000011; // WDA_EXCLUDEFROMCAPTURE
    params.pFunc = (uint64_t)pSetWDA;
    WriteProcessMemory(hProc, remoteMem, &params, sizeof(params), NULL);

    // Write shellcode after params
    void* codeAddr = (uint8_t*)remoteMem + sizeof(RemoteParams);
    WriteProcessMemory(hProc, codeAddr, shellcode, sizeof(shellcode), NULL);

    // Execute: thread entry = shellcode, parameter = start of remoteMem (params)
    HANDLE hThread = CreateRemoteThread(hProc, NULL, 0,
        (LPTHREAD_START_ROUTINE)codeAddr, remoteMem, 0, NULL);
    if (!hThread) {
        printf(OBF("[!] Anti-capture: CreateRemoteThread failed (err=%u)\n"), GetLastError());
        VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
        CloseHandle(hProc);
        return false;
    }

    WaitForSingleObject(hThread, 5000);
    CloseHandle(hThread);

    // Cleanup remote memory
    VirtualFreeEx(hProc, remoteMem, 0, MEM_RELEASE);
    CloseHandle(hProc);

    // Verify
    DWORD checkVal = 0;
    GetWindowDisplayAffinity(hwnd, &checkVal);
    if (checkVal == 0x11) {
        printf(OBF("[+] Anti-capture: WDA_EXCLUDEFROMCAPTURE active (remote inject OK)\n"));
        return true;
    } else {
        printf(OBF("[!] Anti-capture: Affinity check = 0x%X (expected 0x11)\n"), checkVal);
        return false;
    }
}

// ---------------------------------------------------------------------------
// SetupOverlay - Hijack Movavi Screen Recorder window + DX11
// Movavi is a regular Win32 Qt app — won't fight back like GameBar.
// Window is owned by a signed, legitimate process.
// ---------------------------------------------------------------------------
void SetupOverlay()
{
    // Find Movavi Screen Recorder window by Qt class name
    OverlayWindow::Hwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
    if (!OverlayWindow::Hwnd) {
        // Try to launch Movavi
        const char* recorderPaths[] = {
            "C:\\Program Files\\Movavi Screen Recorder\\ScreenRecorder.exe",
            "C:\\Program Files (x86)\\Movavi Screen Recorder\\ScreenRecorder.exe",
        };
        bool launched = false;
        for (auto path : recorderPaths) {
            if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES) {
                ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWMINIMIZED);
                launched = true;
                break;
            }
        }
        if (!launched) {
            printf(OBF("[X] Movavi Screen Recorder not found!\n"));
            printf(OBF("[X] Install Movavi or place ScreenRecorder.exe in Program Files\n"));
            getchar();
            ExitProcess(-1);
        }
        printf(OBF("[*] Waiting for Movavi Screen Recorder...\n"));
        for (int i = 0; i < 150 && !OverlayWindow::Hwnd; i++) { // 15 sec timeout
            OverlayWindow::Hwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
            Sleep(100);
        }
        if (!OverlayWindow::Hwnd) {
            printf(OBF("[X] Movavi window not found after timeout!\n"));
            getchar();
            ExitProcess(-1);
        }
    }
    printf(OBF("[+] Movavi Screen Recorder window found! HWND=%p\n"), OverlayWindow::Hwnd);

    g_gameHwnd = FindGameWindow();

    int posX = 0, posY = 0;
    if (g_gameHwnd && IsWindow(g_gameHwnd)) {
        RECT rc;
        GetClientRect(g_gameHwnd, &rc);
        POINT topLeft = { 0, 0 };
        ClientToScreen(g_gameHwnd, &topLeft);
        posX = topLeft.x;
        posY = topLeft.y;
        Width = rc.right;
        Height = rc.bottom;
    } else {
        Width = GetSystemMetrics(SM_CXSCREEN);
        Height = GetSystemMetrics(SM_CYSCREEN);
    }

    // Override Qt window styles — Movavi is Win32, won't fight back
    SetWindowLong(OverlayWindow::Hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

    // WS_EX_TOPMOST required — PUBG fullscreen borderless is itself topmost
    SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE,
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

    MoveWindow(OverlayWindow::Hwnd, posX, posY, Width, Height, TRUE);
    SetLayeredWindowAttributes(OverlayWindow::Hwnd, 0, 255, LWA_ALPHA);

    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &margins);

    ShowWindow(OverlayWindow::Hwnd, SW_SHOWDEFAULT);
    UpdateWindow(OverlayWindow::Hwnd);

    // Subclass window procedure — intercepts ImGui events on hijacked window
    g_origWndProc = (WNDPROC)SetWindowLongPtr(OverlayWindow::Hwnd, GWLP_WNDPROC, (LONG_PTR)SubclassWinProc);

    // Anti-capture via DKOM — writes WDA_EXCLUDEFROMCAPTURE to tagWND in desktop heap
    // This bypasses the cross-process SetWindowDisplayAffinity limitation
    AntiCapture::SetAffinityDKOM(OverlayWindow::Hwnd);

    if (!DirectXInit()) {
        printf(OBF("[X] DX11 init FAILED!\n"));
        ExitProcess(-1);
    }
    printf(OBF("[+] DX11 initialized OK\n"));

    try {
        ItemIcons::InitItemIcons(DX11::g_pd3dDevice);
        printf("[+] Item icons loaded: %d textures\n", ItemIcons::GetLoadedIconCount());
    } catch (...) {
        printf("[!] Icon loading failed - text-only mode\n");
    }
}

// ---------------------------------------------------------------------------
// Low-level mouse hook for mouse wheel (overlay is WS_EX_TRANSPARENT so
// WM_MOUSEWHEEL never reaches our WndProc)
// ---------------------------------------------------------------------------
static float g_mouseWheelDelta = 0.0f;
static HHOOK g_mouseHook = NULL;

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_MOUSEWHEEL) {
        auto* pData = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        // Skip injected events (from our macro/aimbot SendInput)
        if (!(pData->flags & LLMHF_INJECTED)) {
            g_mouseWheelDelta += (float)GET_WHEEL_DELTA_WPARAM(pData->mouseData) / (float)WHEEL_DELTA;
        }
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

// ---------------------------------------------------------------------------
// SafeRender — SEH-only wrapper (can't mix with try/catch in same function)
// Returns false if device is lost.
// ---------------------------------------------------------------------------
static bool SafeRender()
{
    __try {
        Render();

        HRESULT hrDevice = DX11::g_pd3dDevice->GetDeviceRemovedReason();
        if (FAILED(hrDevice)) {
            printf(OBF("[!] DX11 device removed (hr=0x%08X)\n"), hrDevice);
            return false;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        // SEH exception — try to salvage ImGui frame
        __try {
            ImGui::EndFrame();
            ImGui::Render();
            if (DX11::g_mainRenderTargetView) {
                DX11::g_pd3dDeviceContext->OMSetRenderTargets(1, &DX11::g_mainRenderTargetView, nullptr);
                const float cc[4] = { 0, 0, 0, 0 };
                DX11::g_pd3dDeviceContext->ClearRenderTargetView(DX11::g_mainRenderTargetView, cc);
            }
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            DX11::g_pSwapChain->Present(0, 0);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
        if (g_mouseHook) { UnhookWindowsHookEx(g_mouseHook); g_mouseHook = NULL; }
        menuShow = false;
        return true;
    }
}

// ---------------------------------------------------------------------------
// RunMainLoop
// ---------------------------------------------------------------------------
void RunMainLoop()
{
    g_recoilMacro.Start(); // Start macro thread
    timeBeginPeriod(1);    // Enable 1ms timer resolution (Sleep precision)

    // Start background exploit thread — removes 10-50ms stalls from render
    StartExploitThread();

    // Start aimbot thread — driver MouseMove IOCTL runs off render thread
    StartAimbotThread();

    // Create high-resolution waitable timer for frame pacing.
    // Replaces CPU-burning busy-wait spin loop.
    HANDLE hFrameTimer = NULL;
    {
        // Try high-resolution timer first (Win10 1803+)
        typedef HANDLE(WINAPI* PFN_CWTE)(LPSECURITY_ATTRIBUTES, LPCWSTR, DWORD, DWORD);
        PFN_CWTE pCreateWaitableTimerExW = (PFN_CWTE)GetProcAddress(
            GetModuleHandleA("kernel32.dll"), "CreateWaitableTimerExW");
        if (pCreateWaitableTimerExW) {
            hFrameTimer = pCreateWaitableTimerExW(
                NULL, NULL,
                0x00000002 /*CREATE_WAITABLE_TIMER_HIGH_RESOLUTION*/,
                TIMER_ALL_ACCESS);
        }
        // Fallback for older Windows
        if (!hFrameTimer)
            hFrameTimer = CreateWaitableTimerW(NULL, TRUE, NULL);
    }

    //// Global crash safety: log crashes and prevent termination
    //SetUnhandledExceptionFilter([](EXCEPTION_POINTERS* ep) -> LONG {
    //    if (g_mouseHook) { UnhookWindowsHookEx(g_mouseHook); g_mouseHook = NULL; }

    //    // Log crash to file (survives app death)
    //    FILE* dbg = fopen("C:\\drv_debug.log", "a");
    //    if (dbg) {
    //        fprintf(dbg, "CRASH: code=0x%08X addr=0x%llX rip=0x%llX\n",
    //            ep->ExceptionRecord->ExceptionCode,
    //            (uint64_t)ep->ExceptionRecord->ExceptionAddress,
    //            (uint64_t)ep->ContextRecord->Rip);
    //        fclose(dbg);
    //    }
    //    printf("[!] CRASH CAUGHT: 0x%08X at 0x%llX\n",
    //        ep->ExceptionRecord->ExceptionCode,
    //        (uint64_t)ep->ExceptionRecord->ExceptionAddress);

    //    return EXCEPTION_CONTINUE_SEARCH;
    //});

    // Device-lost flag: set when Present() returns DXGI_ERROR_DEVICE_REMOVED/RESET
    // or when the overlay HWND becomes invalid (GameBar reclaimed it)
    bool deviceLost = false;
    int deviceLostFrames = 0;

    ZeroMemory(&DX11::Message, sizeof(MSG));
    DWORD lastWindowTrackTick = 0;
    const DWORD WINDOW_TRACK_MS = 250; // Check window position at 4Hz (was every frame = 144Hz)

    // Prevent std::terminate from killing the process
    // (C++ exceptions in background threads call terminate() which kills everything)
    //std::set_terminate([]() {
    //    FILE* dbg = fopen("C:\\drv_debug.log", "a");
    //    if (dbg) { fprintf(dbg, "std::terminate called — prevented\n"); fclose(dbg); }
    //    printf("[!] std::terminate intercepted\n");
    //    // Don't actually terminate — just loop forever (thread dies, app survives)
    //    while (true) Sleep(10000);
    //});

    while (true)
    {
        if (PeekMessage(&DX11::Message, OverlayWindow::Hwnd, 0, 0, PM_REMOVE)) {
            if (DX11::Message.message == WM_QUIT || DX11::Message.message == WM_CLOSE) {
                printf(OBF("[!] WM_%s intercepted — treating as overlay loss\n"),
                    DX11::Message.message == WM_QUIT ? "QUIT" : "CLOSE");
                deviceLost = true;
                continue;
            }
            TranslateMessage(&DX11::Message);
            DispatchMessage(&DX11::Message);
        }

        // ---------------------------------------------------------------
        // Game window tracking — throttled to every 250ms
        // Previously ran every frame (144Hz = ~720 kernel calls/sec).
        // At 4Hz we get ~20 kernel calls/sec — 36x reduction.
        // ---------------------------------------------------------------
        DWORD trackNow = GetTickCount();
        if (trackNow - lastWindowTrackTick >= WINDOW_TRACK_MS)
        {
            lastWindowTrackTick = trackNow;

            // Re-find game window if lost
            if (!g_gameHwnd || !IsWindow(g_gameHwnd)) {
                HWND newGameHwnd = FindGameWindow();
                if (newGameHwnd) g_gameHwnd = newGameHwnd;
            }

            if (g_gameHwnd && IsWindow(g_gameHwnd)) {
                // Check if game is minimized
                if (IsIconic(g_gameHwnd)) {
                    if (g_overlayVisible) {
                        ShowWindow(OverlayWindow::Hwnd, SW_HIDE);
                        g_overlayVisible = false;
                    }
                } else {
                    // Get CLIENT AREA (actual render region, excludes title bar/borders)
                    RECT rc;
                    GetClientRect(g_gameHwnd, &rc);
                    POINT topLeft = { 0, 0 };
                    ClientToScreen(g_gameHwnd, &topLeft);
                    int newX = topLeft.x;
                    int newY = topLeft.y;
                    int newW = rc.right;
                    int newH = rc.bottom;

                    if (newW > 0 && newH > 0) {
                        // Check if position or size changed
                        RECT overlayRc;
                        GetWindowRect(OverlayWindow::Hwnd, &overlayRc);
                        int curW = overlayRc.right - overlayRc.left;
                        int curH = overlayRc.bottom - overlayRc.top;

                        if (overlayRc.left != newX || overlayRc.top != newY ||
                            curW != newW || curH != newH)
                        {
                            // Size changed → need to resize SwapChain too
                            if (curW != newW || curH != newH) {
                                __try {
                                    CleanupRenderTarget();
                                    HRESULT hr = DX11::g_pSwapChain->ResizeBuffers(0, newW, newH, DXGI_FORMAT_UNKNOWN, 0);
                                    if (SUCCEEDED(hr)) {
                                        CreateRenderTarget();
                                    } else {
                                        deviceLost = true;
                                    }
                                } __except(EXCEPTION_EXECUTE_HANDLER) {
                                    deviceLost = true;
                                }
                            }
                            SetWindowPos(OverlayWindow::Hwnd, HWND_TOP,
                                newX, newY, newW, newH,
                                SWP_NOACTIVATE | SWP_SHOWWINDOW);
                        }
                        Width = newW;
                        Height = newH;
                    }

                    // Focus check: hide overlay when game is not focused
                    // (keep visible when menu is open so user can interact)
                    HWND fgWnd = GetForegroundWindow();
                    bool gameIsFocused = (fgWnd == g_gameHwnd) || (fgWnd == OverlayWindow::Hwnd);
                    if (!gameIsFocused && !menuShow) {
                        if (g_overlayVisible) {
                            ShowWindow(OverlayWindow::Hwnd, SW_HIDE);
                            g_overlayVisible = false;
                        }
                    } else {
                        if (!g_overlayVisible) {
                            ShowWindow(OverlayWindow::Hwnd, SW_SHOWNOACTIVATE);
                            g_overlayVisible = true;
                        }
                    }
                }
            }
        }

        // Toggle focus + mouse hook when menu opens/closes.
        // Click-through handled via WM_NCHITTEST in WndProc.
        // Instead, we steal foreground focus from the game so it stops
        // processing mouse input while the menu is open.
        static bool s_hookActive = false;
        if (menuShow && !s_hookActive) {
            // Menu opening: steal focus from game
            SetForegroundWindow(OverlayWindow::Hwnd);
            SetFocus(OverlayWindow::Hwnd);
            g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
            s_hookActive = true;
        } else if (!menuShow && s_hookActive) {
            // Menu closing: give focus back to game
            if (g_gameHwnd && IsWindow(g_gameHwnd)) {
                SetForegroundWindow(g_gameHwnd);
                SetFocus(g_gameHwnd);
            }
            if (g_mouseHook) { UnhookWindowsHookEx(g_mouseHook); g_mouseHook = NULL; }
            s_hookActive = false;
            g_mouseWheelDelta = 0.0f;
        }

        // Skip full render when overlay is hidden (saves entire GPU frame)
        if (!g_overlayVisible && !menuShow) {
            Sleep(16); // ~60Hz idle polling when hidden
            continue;
        }

        // Periodic TOPMOST enforcement (~every 2 sec)
        static int integrityCounter = 0;
        if (++integrityCounter >= 120) {
            integrityCounter = 0;
            if (IsWindow(OverlayWindow::Hwnd) && g_overlayVisible) {
                SetWindowPos(OverlayWindow::Hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                    SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
        }

        // Check if overlay HWND is still valid
        if (!IsWindow(OverlayWindow::Hwnd)) {
            if (!deviceLost) {
                deviceLost = true;
                printf(OBF("[!] Overlay window lost\n"));
            }
        }

        // If device is lost, relaunch GameBar + suspend + reinit every ~2 sec
        if (deviceLost) {
            deviceLostFrames++;
            if (deviceLostFrames % 120 == 0) {
                printf(OBF("[*] Recovering overlay (finding Movavi)...\n"));

                // 1. Tear down old DX11 + ImGui
                __try {
                    ImGui_ImplDX11_Shutdown();
                    ImGui_ImplWin32_Shutdown();
                    ImGui::DestroyContext();
                } __except(EXCEPTION_EXECUTE_HANDLER) {}

                CleanupRenderTarget();
                if (DX11::g_pSwapChain) { DX11::g_pSwapChain->Release(); DX11::g_pSwapChain = nullptr; }
                if (DX11::g_pd3dDeviceContext) { DX11::g_pd3dDeviceContext->Release(); DX11::g_pd3dDeviceContext = nullptr; }
                if (DX11::g_pd3dDevice) { DX11::g_pd3dDevice->Release(); DX11::g_pd3dDevice = nullptr; }

                // 2. Find or relaunch Movavi
                HWND newHwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
                if (!newHwnd) {
                    printf(OBF("[*] Relaunching Movavi for recovery...\n"));
                    const char* paths[] = {
                        "C:\\Program Files\\Movavi Screen Recorder\\ScreenRecorder.exe",
                        "C:\\Program Files (x86)\\Movavi Screen Recorder\\ScreenRecorder.exe",
                    };
                    for (auto p : paths) {
                        if (GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES) {
                            ShellExecuteA(NULL, "open", p, NULL, NULL, SW_SHOWMINIMIZED);
                            break;
                        }
                    }
                    Sleep(3000);
                    newHwnd = FindWindowA("Qt5152QWindowIcon", "Screen Recorder");
                }

                if (newHwnd) {
                    OverlayWindow::Hwnd = newHwnd;
                    SetWindowLong(OverlayWindow::Hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
                    SetWindowLong(OverlayWindow::Hwnd, GWL_EXSTYLE,
                        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);

                    int px = 0, py = 0;
                    if (g_gameHwnd && IsWindow(g_gameHwnd)) {
                        RECT rc; GetClientRect(g_gameHwnd, &rc);
                        POINT tl = {0,0}; ClientToScreen(g_gameHwnd, &tl);
                        px = tl.x; py = tl.y;
                        Width = rc.right; Height = rc.bottom;
                    } else {
                        Width = GetSystemMetrics(SM_CXSCREEN);
                        Height = GetSystemMetrics(SM_CYSCREEN);
                    }
                    MoveWindow(OverlayWindow::Hwnd, px, py, Width, Height, TRUE);
                    SetLayeredWindowAttributes(OverlayWindow::Hwnd, 0, 255, LWA_ALPHA);
                    MARGINS margins = { -1, -1, -1, -1 };
                    DwmExtendFrameIntoClientArea(OverlayWindow::Hwnd, &margins);
                    ShowWindow(OverlayWindow::Hwnd, SW_SHOWDEFAULT);

                    if (DirectXInit()) {
                        printf(OBF("[+] Overlay recovered successfully\n"));
                        deviceLost = false;
                        deviceLostFrames = 0;
                        g_overlayVisible = true;
                    } else {
                        printf(OBF("[!] DX11 reinit failed — will retry\n"));
                    }
                }
            }
            Sleep(16);
            continue;
        }

        // Manual mouse input (overlay is always WS_EX_TRANSPARENT)
        // Convert screen coords to client coords (overlay may not be at 0,0)
        POINT cp; GetCursorPos(&cp);
        ScreenToClient(OverlayWindow::Hwnd, &cp);
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = (float)cp.x;
        io.MousePos.y = (float)cp.y;
        io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

        // Consume accumulated mouse wheel delta from hook
        if (g_mouseWheelDelta != 0.0f) {
            io.MouseWheel += g_mouseWheelDelta;
            g_mouseWheelDelta = 0.0f;
        }

        // Validate DX11 objects before rendering
        if (!DX11::g_pd3dDevice || !DX11::g_pSwapChain || !DX11::g_pd3dDeviceContext) {
            Sleep(16);
            continue;
        }

        if (!SafeRender()) {
            deviceLost = true;
        }

        // Frame pacing with waitable timer (~144fps target).
        // Replaces CPU-burning busy-wait spin loop that used
        // QueryPerformanceCounter in a tight for(;;) loop.
        // Waitable timer yields the CPU to the OS scheduler,
        // freeing cycles for PUBG instead of wasting them here.
        if (hFrameTimer) {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = -69444LL; // 6.9444ms in 100ns units ≈ 144fps
            SetWaitableTimer(hFrameTimer, &dueTime, 0, NULL, NULL, FALSE);
            WaitForSingleObject(hFrameTimer, 10);
        } else {
            Sleep(6); // Fallback: ~144fps via Sleep (less precise)
        }
    }

    // Stop background threads
    StopAimbotThread();
    StopExploitThread();
    timeEndPeriod(1); // Restore timer resolution
    g_recoilMacro.Stop(); // Stop macro thread

    // Cleanup
    if (hFrameTimer) CloseHandle(hFrameTimer);
    if (g_mouseHook) { UnhookWindowsHookEx(g_mouseHook); g_mouseHook = NULL; }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupRenderTarget();
    if (DX11::g_pSwapChain) { DX11::g_pSwapChain->Release(); }
    if (DX11::g_pd3dDeviceContext) { DX11::g_pd3dDeviceContext->Release(); }
    if (DX11::g_pd3dDevice) { DX11::g_pd3dDevice->Release(); }
    // Don't DestroyWindow — this is Movavi's window, not ours
    // Restore original WndProc so Movavi can continue normally
    if (g_origWndProc && OverlayWindow::Hwnd)
        SetWindowLongPtr(OverlayWindow::Hwnd, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);
}
