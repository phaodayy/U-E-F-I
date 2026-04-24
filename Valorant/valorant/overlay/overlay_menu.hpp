#pragma once
#include "../gui/menu.h"
#include "../imgui/imgui.h"
#include "../sdk/context.hpp"
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>


#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

class OverlayMenu {
public:
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();

  HWND target_hwnd = NULL;
  void SetClickable(bool state);

  float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
  float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
  bool showmenu = true;

  // All ESP toggle/color/settings variables are now in GUI:: namespace
  // (gui/menu.h) Overlay only keeps infrastructure: target_hwnd, screen
  // dimensions, showmenu

  void Initialize(HWND game_hwnd = nullptr);
  void UpdateAntiScreenshot();
  void SetupStyle();
  void RenderFrame();
  void Shutdown();
};

extern OverlayMenu g_Menu;
