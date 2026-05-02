#include "overlay_menu.hpp"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"
#include "../sdk/context.hpp"
#include "../sdk/driver.hpp"
#include "../sdk/offsets.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <chrono>

OverlayMenu g_Menu;

static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;
static WNDCLASSEXW g_wc = {0};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam,
                            LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    return true;
  switch (message) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(hWnd, message, wParam, lParam);
}

void OverlayMenu::SetClickable(bool state) {
  if (!target_hwnd)
    return;
  LONG_PTR flags = GetWindowLongPtr(target_hwnd, GWL_EXSTYLE);
  if (state)
    SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, flags & ~WS_EX_TRANSPARENT);
  else
    SetWindowLongPtr(target_hwnd, GWL_EXSTYLE, flags | WS_EX_TRANSPARENT);

  SetWindowPos(target_hwnd, HWND_TOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

bool OverlayMenu::CreateDeviceD3D(HWND hWnd) {
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0,
                                                  D3D_FEATURE_LEVEL_10_0};
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
      &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  if (res != S_OK)
    return false;
  CreateRenderTarget();
  return true;
}

void OverlayMenu::CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
}

void OverlayMenu::CreateRenderTarget() {
  ID3D11Texture2D *pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void OverlayMenu::CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}

void OverlayMenu::UpdateAntiScreenshot() {
  if (!target_hwnd)
    return;
  // WDA_EXCLUDE (4) - Hides window from all capture
  // WDA_NONE (0) - Visible to all capture
  bool anti = GUI::anti_screenshot;
  DWORD affinity = anti ? 0x00000011 : 0x00000000;
  SetWindowDisplayAffinity(target_hwnd, affinity);
}

void OverlayMenu::Initialize(HWND game_hwnd) {
  // Randomize class and title for better stealth
  wchar_t randClass[16] = {0};
  wchar_t randTitle[16] = {0};
  srand((unsigned int)time(NULL));
  for (int i = 0; i < 15; i++)
    randClass[i] = (wchar_t)((rand() % 26) + 'a');
  for (int i = 0; i < 15; i++)
    randTitle[i] = (wchar_t)((rand() % 26) + 'A');

  g_wc = {sizeof(WNDCLASSEXW),
          CS_CLASSDC,
          WindowProc,
          0L,
          0L,
          GetModuleHandle(NULL),
          NULL,
          NULL,
          NULL,
          NULL,
          randClass,
          NULL};
  RegisterClassExW(&g_wc);

  target_hwnd = CreateWindowExW(
      WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
      randClass, randTitle, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN),
      GetSystemMetrics(SM_CYSCREEN), NULL, NULL, g_wc.hInstance, NULL);

  if (!target_hwnd) {
    std::cout << "[-] Failed to create standalone overlay window.\n";
    return;
  }
  std::cout << "[+] Standalone overlay window created: 0x" << std::hex
            << target_hwnd << std::dec << "\n";

  SetLayeredWindowAttributes(target_hwnd, 0, 255, LWA_ALPHA);

  MARGINS margin = {-1};
  DwmExtendFrameIntoClientArea(target_hwnd, &margin);

  ShowWindow(target_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(target_hwnd);

  UpdateAntiScreenshot();

  if (!CreateDeviceD3D(target_hwnd))
    return;

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  SetupStyle();
  ImGui_ImplWin32_Init(target_hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Poppins-Regular.ttf", 16.0f);
  SetClickable(showmenu);
  std::cout << "[+] Standalone Overlay ready.\n";
}

void OverlayMenu::RenderFrame() {
  try {
    if (!target_hwnd)
      return;

    if (GetAsyncKeyState(VK_F5) & 1) {
      showmenu = !showmenu;
      SetClickable(showmenu);
      if (showmenu) {
        SetForegroundWindow(target_hwnd);
      }
    }

    if (GetAsyncKeyState(VK_F4) & 1) {
      GUI::v_enable = !GUI::v_enable;
    }

    if (GetAsyncKeyState(VK_F12) & 1) {
      exit(0);
    }

    /* Handled by WndProc for Standalone Window
    ImGuiIO &io = ImGui::GetIO();
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(target_hwnd, &p);
    io.MousePos = ImVec2((float)p.x, (float)p.y);
    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    */

    MSG msg;
    while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // ========== VISUALS RENDERING (uses GUI:: namespace variables) ==========
    if (GUI::v_enable && G_PlayerController) {
      ImDrawList *draw = ImGui::GetBackgroundDrawList();

      static int debugFrameCount = 0;
      debugFrameCount++;

      uint64_t camera_manager = VDataContext::Read<uint64_t>(
          G_PlayerController + VDataOffsets::PlayerCameraManager());
      if (camera_manager) {
        struct DVector3 {
          double x, y, z;
        };
        struct FMinimalViewInfo {
          DVector3 Location;
          DVector3 Rotation;
          float FOV;
        };

        FMinimalViewInfo viewInfo = VDataContext::Read<FMinimalViewInfo>(
            camera_manager + VDataOffsets::CameraCachePrivate() + 0x10);

        Vector3 cam_location = {(float)viewInfo.Location.x,
                                (float)viewInfo.Location.y,
                                (float)viewInfo.Location.z};
        Vector3 cam_rotation = {(float)viewInfo.Rotation.x,
                                (float)viewInfo.Rotation.y,
                                (float)viewInfo.Rotation.z};
        float cam_fov = (viewInfo.FOV > 0.0f && viewInfo.FOV < 170.0f)
                            ? viewInfo.FOV
                            : 103.0f;

        auto WorldToScreen = [&](Vector3 world_pos,
                                 Vector2 &screen_pos) -> bool {
          const float PI = 3.14159265f;
          float radPitch = cam_rotation.x * PI / 180.0f;
          float radYaw = cam_rotation.y * PI / 180.0f;
          float radRoll = cam_rotation.z * PI / 180.0f;
          float sp = sinf(radPitch), cp = cosf(radPitch);
          float sy = sinf(radYaw), cy = cosf(radYaw);
          float sr = sinf(radRoll), cr = cosf(radRoll);

          float axis_x[3] = {cp * cy, cp * sy, sp};
          float axis_y[3] = {sr * sp * cy - cr * sy, sr * sp * sy + cr * cy,
                             -sr * cp};
          float axis_z[3] = {-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy,
                             cr * cp};

          Vector3 vDelta = world_pos - cam_location;

          float transformed_x = vDelta.x * axis_y[0] + vDelta.y * axis_y[1] +
                                vDelta.z * axis_y[2];
          float transformed_y = vDelta.x * axis_z[0] + vDelta.y * axis_z[1] +
                                vDelta.z * axis_z[2];
          float transformed_z = vDelta.x * axis_x[0] + vDelta.y * axis_x[1] +
                                vDelta.z * axis_x[2];

          if (transformed_z < 1.0f)
            return false;

          float FovAngle = (cam_fov <= 0.0f) ? 90.0f : cam_fov;
          float fov_rad = FovAngle * PI / 180.0f;
          float fov_tan = tanf(fov_rad / 2.0f);

          float ScreenCX = ScreenWidth / 2.0f;
          float ScreenCY = ScreenHeight / 2.0f;

          screen_pos.x =
              ScreenCX + (transformed_x / fov_tan) * ScreenCX / transformed_z;
          screen_pos.y =
              ScreenCY - (transformed_y / fov_tan) * ScreenCX / transformed_z;

          return (screen_pos.x > 0 && screen_pos.x < ScreenWidth &&
                  screen_pos.y > 0 && screen_pos.y < ScreenHeight);
        };

        // ========== ASSIST LOGIC ==========
        Vector2 screen_center = {ScreenWidth / 2.0f, ScreenHeight / 2.0f};
        float fov_pixels = GUI::a_fov * (ScreenWidth / 103.0f); // Approximate based on 103 standard FOV

        if (GUI::a_enable) {
            ImDrawList* background_draw = ImGui::GetBackgroundDrawList();
            background_draw->AddCircle(ImVec2(screen_center.x, screen_center.y), fov_pixels, IM_COL32(255, 255, 255, 80), 64, 1.0f);
        }

        static auto IsCursorVisible = []() -> bool {
            CURSORINFO ci = { sizeof(CURSORINFO) };
            if (GetCursorInfo(&ci)) {
                return (ci.flags & CURSOR_SHOWING);
            }
            return false;
        };

        ImGuiIO& io = ImGui::GetIO();
        bool skip_input = g_Menu.showmenu || io.WantCaptureMouse || IsCursorVisible();

        if (GUI::a_enable && !skip_input && (GetAsyncKeyState(GUI::a_key) & 0x8000)) {
          float best_dist = fov_pixels; 
          Vector2 best_target = {0, 0};
          bool found = false;

          for (const auto &player : CachedPlayers) {
            if (!player.bIsAlive || player.IsTeammate)
              continue;
            
            // Aim check - only aim if player is actually on screen and within FOV
            Vector3 target_bone_pos;
            if (GUI::a_target == 0) target_bone_pos = player.Bone_Head;
            else if (GUI::a_target == 1) target_bone_pos = player.Bone_Neck;
            else if (GUI::a_target == 2) target_bone_pos = player.Bone_Chest;
            else target_bone_pos = player.Bone_Pelvis;

            Vector2 screen_pos;
            if (WorldToScreen(target_bone_pos, screen_pos)) {
                float dx = screen_pos.x - screen_center.x;
                float dy = screen_pos.y - screen_center.y;
                float dist = sqrtf(dx * dx + dy * dy);
                
                if (dist < best_dist) {
                    if (player.IsVisible) { // Visibility check
                        best_dist = dist;
                        best_target = screen_pos;
                        found = true;
                    }
                }
            }
          }

          if (found) {
            float dx = best_target.x - screen_center.x;
            float dy = best_target.y - screen_center.y;

            if (abs(dx) > GUI::a_deadzone || abs(dy) > GUI::a_deadzone) {
                float smooth = GUI::a_smooth;
                if (smooth < 1.0f) smooth = 1.0f;
                
                dx = (dx * GUI::a_speed) / smooth;
                dy = (dy * GUI::a_speed) / smooth;

                VDataContext::MoveMouse((long)dx, (long)dy);
            }

            // Auto-fire check: only fire if VERY close to target (precision fire)
            if (GUI::a_trigger && best_dist < 4.5f) {
                // Left click simulation
                Driver::MoveMouse(0, 0, 0x0001); // DOWN
                Sleep(1);
                Driver::MoveMouse(0, 0, 0x0002); // UP
            }
          }
        }

        // ========== CUSTOM MACRO LOGIC ==========
        if (GUI::macro_toggle && !skip_input && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
            static auto last_macro_time = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_macro_time).count();

            if (elapsed >= GUI::macro_delay) {
                VDataContext::MoveMouse((long)GUI::macro_intensity_x, (long)GUI::macro_intensity_y);
                last_macro_time = now;
            }
        }

        int renderedCount = 0;
        int w2sFailCount = 0;

        auto DrawTextStroke = [draw](float x, float y, ImU32 fill, ImU32 stroke,
                                     const char *txt) {
          ImVec2 sz = ImGui::CalcTextSize(txt);
          float cx = x - sz.x / 2;
          for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
              if (dx || dy)
                draw->AddText(ImVec2(cx + dx, y + dy), stroke, txt);
          draw->AddText(ImVec2(cx, y), fill, txt);
        };
        ImU32 text_stroke = IM_COL32(0, 0, 0, 255);
        ImU32 text_fill = IM_COL32(255, 255, 255, 255);

        for (const auto &player : CachedPlayers) {
          float liveDist = cam_location.Distance(player.Position) / 100.0f;
          if (liveDist > GUI::render_distance)
            continue;

          // Determine box/line color based on visibility + per-feature
          // invisible color support
          float *vis_color = GUI::box_visible_color;
          float *invis_color = GUI::use_global_invisible_color
                                   ? GUI::invisible_color
                                   : GUI::box_invisible_color;

          ImColor line_color = player.IsVisible
                                   ? ImColor(vis_color[0], vis_color[1],
                                             vis_color[2], vis_color[3])
                                   : ImColor(invis_color[0], invis_color[1],
                                             invis_color[2], invis_color[3]);

          // Skeleton color with per-feature invisible support
          float *skel_vis = GUI::skeleton_visible_color;
          float *skel_invis = GUI::use_global_invisible_color
                                  ? GUI::invisible_color
                                  : GUI::skeleton_invisible_color;

          ImColor skel_col =
              player.IsVisible
                  ? ImColor(skel_vis[0], skel_vis[1], skel_vis[2], skel_vis[3])
                  : ImColor(skel_invis[0], skel_invis[1], skel_invis[2],
                            skel_invis[3]);

          skel_col.Value.w *= (GUI::b_alpha / 100.0f);
          ImColor box_col = line_color;
          box_col.Value.w *= (GUI::f_alpha / 100.0f);

          Vector3 box_root, box_head;
          if (player.MeshAddr && player.BoneArrayAddr) {
            box_root = VDataContext::GetBoneWorldPos(0, player.BoneArrayAddr,
                                                        player.MeshAddr);
            box_head = VDataContext::GetBoneWorldPos(8, player.BoneArrayAddr,
                                                        player.MeshAddr);
          } else {
            box_root = player.Position;
            box_head = player.HeadPosition;
          }

          Vector2 feet_screen, head_screen;
          bool w2s_success = WorldToScreen(box_root, feet_screen) &&
                             WorldToScreen(box_head, head_screen);

          if (GUI::v_frame && GUI::v_frame_type <= 1) {
            if (w2s_success) {
              float box_h = abs(head_screen.y - feet_screen.y);
              float box_w = box_h * 0.55f;
              float x0 = feet_screen.x - box_w / 2;
              float y0 = head_screen.y;

              renderedCount++;

              // Fill color with per-feature invisible support
              float *fill_vis = GUI::fill_visible_color;
              float *fill_invis = GUI::use_global_invisible_color
                                      ? GUI::invisible_color
                                      : GUI::fill_invisible_color;
              float *fill_c = player.IsVisible ? fill_vis : fill_invis;

              ImColor fill_col_rect =
                  ImColor(fill_c[0], fill_c[1], fill_c[2], fill_c[3]);
              fill_col_rect.Value.w *= (GUI::ff_alpha / 100.0f);

              if (GUI::v_frame_fill) {
                draw->AddRectFilled(ImVec2(x0, y0),
                                    ImVec2(x0 + box_w, y0 + box_h),
                                    (ImU32)fill_col_rect);
              }
              if (GUI::v_frame &&
                  (GUI::v_frame_type == 0 || GUI::v_frame_type == 1)) {
                ImU32 line_col = (ImU32)box_col;
                float th = 1.5f;
                if (GUI::v_frame_type == 0) {
                  float lw = box_w / 3, lh = box_h / 3;
                  draw->AddLine(ImVec2(x0, y0), ImVec2(x0, y0 + lh), line_col,
                                th);
                  draw->AddLine(ImVec2(x0, y0), ImVec2(x0 + lw, y0), line_col,
                                th);
                  draw->AddLine(ImVec2(x0 + box_w - lw, y0),
                                ImVec2(x0 + box_w, y0), line_col, th);
                  draw->AddLine(ImVec2(x0 + box_w, y0),
                                ImVec2(x0 + box_w, y0 + lh), line_col, th);
                  draw->AddLine(ImVec2(x0, y0 + box_h - lh),
                                ImVec2(x0, y0 + box_h), line_col, th);
                  draw->AddLine(ImVec2(x0, y0 + box_h),
                                ImVec2(x0 + lw, y0 + box_h), line_col, th);
                  draw->AddLine(ImVec2(x0 + box_w - lw, y0 + box_h),
                                ImVec2(x0 + box_w, y0 + box_h), line_col, th);
                  draw->AddLine(ImVec2(x0 + box_w, y0 + box_h - lh),
                                ImVec2(x0 + box_w, y0 + box_h), line_col, th);
                } else {
                  draw->AddRect(ImVec2(x0, y0), ImVec2(x0 + box_w, y0 + box_h),
                                line_col, 0.0f, 0, th);
                }
              }
            } else {
              w2sFailCount++;
            }
          }

          if (GUI::v_point && w2s_success) {
            // Head circle with per-feature invisible support
            float *hc_vis = GUI::head_circle_visible_color;
            float *hc_invis = GUI::use_global_invisible_color
                                  ? GUI::invisible_color
                                  : GUI::head_circle_invisible_color;
            float *hc_c = player.IsVisible ? hc_vis : hc_invis;

            ImU32 hc_col = IM_COL32((int)(hc_c[0] * 255), (int)(hc_c[1] * 255),
                                    (int)(hc_c[2] * 255), (int)(hc_c[3] * 255));
            draw->AddCircle(ImVec2(head_screen.x, head_screen.y), 5.0f, hc_col,
                            12, 2.0f);
          }

          // Health Bar - v_stat_type: 0=Top, 1=Bottom, 2=Left, 3=Right
          if (GUI::v_stat && w2s_success) {
            float health_pct = player.Health / player.MaxHealth;
            health_pct = (health_pct < 0.0f)   ? 0.0f
                         : (health_pct > 1.0f) ? 1.0f
                                               : health_pct;
            ImColor health_col = (health_pct > 0.5f)   ? ImColor(0, 255, 0)
                                 : (health_pct > 0.2f) ? ImColor(255, 255, 0)
                                                       : ImColor(255, 0, 0);

            float box_h = abs(head_screen.y - feet_screen.y);
            float box_w = box_h * 0.55f;
            float x0 = feet_screen.x - box_w / 2.0f;
            float y0 = head_screen.y;

            if (GUI::v_stat_type == 0) {
              // Top - kafanın üstünde yatay bar
              float bar_width = box_w;
              float bar_height = 4.0f;
              float bar_x = x0;
              float name_offset = GUI::v_alias ? 18.0f : 0.0f;
              float bar_y = y0 - name_offset - bar_height - 4.0f;

              draw->AddRectFilled(ImVec2(bar_x, bar_y),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  ImColor(0, 0, 0, 200));
              float fill_width = bar_width * health_pct;
              draw->AddRectFilled(
                  ImVec2(bar_x, bar_y),
                  ImVec2(bar_x + fill_width, bar_y + bar_height), health_col);
            } else if (GUI::v_stat_type == 1) {
              // Bottom - ayakların altında yatay bar
              float bar_width = box_w;
              float bar_height = 4.0f;
              float bar_x = x0;
              float bar_y = feet_screen.y + 4.0f;

              draw->AddRectFilled(ImVec2(bar_x, bar_y),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  ImColor(0, 0, 0, 200));
              float fill_width = bar_width * health_pct;
              draw->AddRectFilled(
                  ImVec2(bar_x, bar_y),
                  ImVec2(bar_x + fill_width, bar_y + bar_height), health_col);
            } else if (GUI::v_stat_type == 2) {
              // Left - kutunun solunda dikey bar
              float bar_width = 4.0f;
              float bar_height = box_h;
              float bar_x = x0 - bar_width - 3.0f;
              float bar_y = y0;

              draw->AddRectFilled(ImVec2(bar_x, bar_y),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  ImColor(0, 0, 0, 200));
              float fill_h = bar_height * health_pct;
              draw->AddRectFilled(ImVec2(bar_x, bar_y + bar_height - fill_h),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  health_col);
            } else {
              // Right - kutunun sağında dikey bar
              float bar_width = 4.0f;
              float bar_height = box_h;
              float bar_x = x0 + box_w + 3.0f;
              float bar_y = y0;

              draw->AddRectFilled(ImVec2(bar_x, bar_y),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  ImColor(0, 0, 0, 200));
              float fill_h = bar_height * health_pct;
              draw->AddRectFilled(ImVec2(bar_x, bar_y + bar_height - fill_h),
                                  ImVec2(bar_x + bar_width, bar_y + bar_height),
                                  health_col);
            }
          }

          if (GUI::v_frame && GUI::v_frame_type == 2) {
            Vector3 min = {box_root.x - 40, box_root.y - 40, box_root.z};
            Vector3 max = {box_root.x + 40, box_root.y + 40, box_head.z + 20};
            Vector3 corners[8] = {{min.x, min.y, min.z}, {max.x, min.y, min.z},
                                  {min.x, max.y, min.z}, {max.x, max.y, min.z},
                                  {min.x, min.y, max.z}, {max.x, min.y, max.z},
                                  {min.x, max.y, max.z}, {max.x, max.y, max.z}};
            Vector2 s[8];
            bool valid3d = true;
            for (int ci = 0; ci < 8; ci++) {
              if (!WorldToScreen(corners[ci], s[ci])) {
                valid3d = false;
                break;
              }
            }
            if (valid3d) {
              ImU32 u32_line = (ImU32)box_col;

              float *fill_vis = GUI::fill_visible_color;
              float *fill_invis = GUI::use_global_invisible_color
                                      ? GUI::invisible_color
                                      : GUI::fill_invisible_color;
              float *fill_c = player.IsVisible ? fill_vis : fill_invis;

              ImColor fill3d =
                  ImColor(fill_c[0], fill_c[1], fill_c[2], fill_c[3]);
              fill3d.Value.w *= (GUI::ff_alpha / 100.0f);
              if (GUI::v_frame_fill) {
                // Tüm 6 yüzeyi doldur
                draw->AddQuadFilled(
                    ImVec2(s[0].x, s[0].y), ImVec2(s[1].x, s[1].y),
                    ImVec2(s[3].x, s[3].y), ImVec2(s[2].x, s[2].y),
                    (ImU32)fill3d); // Alt
                draw->AddQuadFilled(
                    ImVec2(s[4].x, s[4].y), ImVec2(s[5].x, s[5].y),
                    ImVec2(s[7].x, s[7].y), ImVec2(s[6].x, s[6].y),
                    (ImU32)fill3d); // Üst
                draw->AddQuadFilled(
                    ImVec2(s[0].x, s[0].y), ImVec2(s[1].x, s[1].y),
                    ImVec2(s[5].x, s[5].y), ImVec2(s[4].x, s[4].y),
                    (ImU32)fill3d); // Ön
                draw->AddQuadFilled(
                    ImVec2(s[2].x, s[2].y), ImVec2(s[3].x, s[3].y),
                    ImVec2(s[7].x, s[7].y), ImVec2(s[6].x, s[6].y),
                    (ImU32)fill3d); // Arka
                draw->AddQuadFilled(
                    ImVec2(s[0].x, s[0].y), ImVec2(s[2].x, s[2].y),
                    ImVec2(s[6].x, s[6].y), ImVec2(s[4].x, s[4].y),
                    (ImU32)fill3d); // Sol
                draw->AddQuadFilled(
                    ImVec2(s[1].x, s[1].y), ImVec2(s[3].x, s[3].y),
                    ImVec2(s[7].x, s[7].y), ImVec2(s[5].x, s[5].y),
                    (ImU32)fill3d); // Sağ
              }
              // Tüm kenarlar ince ve şeffaf
              ImU32 thin_col =
                  (ImU32)ImColor(box_col.Value.x, box_col.Value.y,
                                 box_col.Value.z, box_col.Value.w * 0.5f);
              int edges[][2] = {{0, 1}, {0, 2}, {1, 3}, {2, 3}, {4, 5}, {4, 6},
                                {5, 7}, {6, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
              for (auto &e : edges)
                draw->AddLine(ImVec2(s[e[0]].x, s[e[0]].y),
                              ImVec2(s[e[1]].x, s[e[1]].y), thin_col, 1.0f);

              renderedCount++;
            }
          }

          if (GUI::v_bone && player.BoneCount > 0 && player.MeshAddr &&
              player.BoneArrayAddr) {
            uint64_t ba = player.BoneArrayAddr;
            uint64_t m = player.MeshAddr;
            int bc = player.BoneCount;

            Vector3 bHead = VDataContext::GetBoneWorldPos(8, ba, m);
            Vector3 bChest = VDataContext::GetBoneWorldPos(6, ba, m);
            Vector3 bPelvis = VDataContext::GetBoneWorldPos(3, ba, m);
            Vector3 bNeck, bLS, bLE, bLH, bRS, bRE, bRH, bLT, bLK, bLF, bRT,
                bRK, bRF;

            if (bc == 103) { // BOT
              bNeck = VDataContext::GetBoneWorldPos(9, ba, m);
              bLS = VDataContext::GetBoneWorldPos(33, ba, m);
              bLE = VDataContext::GetBoneWorldPos(30, ba, m);
              bLH = VDataContext::GetBoneWorldPos(32, ba, m);
              bRS = VDataContext::GetBoneWorldPos(58, ba, m);
              bRE = VDataContext::GetBoneWorldPos(55, ba, m);
              bRH = VDataContext::GetBoneWorldPos(57, ba, m);
              bLT = VDataContext::GetBoneWorldPos(63, ba, m);
              bLK = VDataContext::GetBoneWorldPos(65, ba, m);
              bLF = VDataContext::GetBoneWorldPos(69, ba, m);
              bRT = VDataContext::GetBoneWorldPos(77, ba, m);
              bRK = VDataContext::GetBoneWorldPos(79, ba, m);
              bRF = VDataContext::GetBoneWorldPos(83, ba, m);
            } else if (bc == 104) { // MALE
              bNeck = VDataContext::GetBoneWorldPos(21, ba, m);
              bLS = VDataContext::GetBoneWorldPos(23, ba, m);
              bLE = VDataContext::GetBoneWorldPos(24, ba, m);
              bLH = VDataContext::GetBoneWorldPos(25, ba, m);
              bRS = VDataContext::GetBoneWorldPos(49, ba, m);
              bRE = VDataContext::GetBoneWorldPos(50, ba, m);
              bRH = VDataContext::GetBoneWorldPos(51, ba, m);
              bLT = VDataContext::GetBoneWorldPos(77, ba, m);
              bLK = VDataContext::GetBoneWorldPos(78, ba, m);
              bLF = VDataContext::GetBoneWorldPos(80, ba, m);
              bRT = VDataContext::GetBoneWorldPos(84, ba, m);
              bRK = VDataContext::GetBoneWorldPos(85, ba, m);
              bRF = VDataContext::GetBoneWorldPos(87, ba, m);
            } else { // FEMALE (101) or similar
              bNeck = VDataContext::GetBoneWorldPos(21, ba, m);
              bLS = VDataContext::GetBoneWorldPos(23, ba, m);
              bLE = VDataContext::GetBoneWorldPos(24, ba, m);
              bLH = VDataContext::GetBoneWorldPos(25, ba, m);
              bRS = VDataContext::GetBoneWorldPos(49, ba, m);
              bRE = VDataContext::GetBoneWorldPos(50, ba, m);
              bRH = VDataContext::GetBoneWorldPos(51, ba, m);
              bLT = VDataContext::GetBoneWorldPos(75, ba, m);
              bLK = VDataContext::GetBoneWorldPos(76, ba, m);
              bLF = VDataContext::GetBoneWorldPos(78, ba, m);
              bRT = VDataContext::GetBoneWorldPos(82, ba, m);
              bRK = VDataContext::GetBoneWorldPos(83, ba, m);
              bRF = VDataContext::GetBoneWorldPos(85, ba, m);
            }

            auto DrawBoneLine = [&](Vector3 from, Vector3 to) {
              Vector2 s1, s2;
              if (WorldToScreen(from, s1) && WorldToScreen(to, s2)) {
                float thickness = GUI::b_thickness / 10.0f;
                draw->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y),
                              (ImU32)skel_col, thickness);
              }
            };
            DrawBoneLine(bHead, bNeck);
            DrawBoneLine(bNeck, bChest);
            DrawBoneLine(bChest, bPelvis);
            DrawBoneLine(bNeck, bLS);
            DrawBoneLine(bLS, bLE);
            DrawBoneLine(bLE, bLH);
            DrawBoneLine(bNeck, bRS);
            DrawBoneLine(bRS, bRE);
            DrawBoneLine(bRE, bRH);
            DrawBoneLine(bPelvis, bLT);
            DrawBoneLine(bLT, bLK);
            DrawBoneLine(bLK, bLF);
            DrawBoneLine(bPelvis, bRT);
            DrawBoneLine(bRT, bRK);
            DrawBoneLine(bRK, bRF);
          }

          if (WorldToScreen(box_head, head_screen)) {
            Vector2 feet_screen_txt;
            if (WorldToScreen(box_root, feet_screen_txt)) {
              if (GUI::v_alias) {
                std::string nameText = player.Name.empty() ? "?" : player.Name;
                DrawTextStroke(head_screen.x, head_screen.y - 18, text_fill,
                               text_stroke, nameText.c_str());
              }
              if (GUI::v_dist) {
                float realDist = cam_location.Distance(box_root) / 100.0f;
                char distBuf[32];
                sprintf_s(distBuf, sizeof(distBuf), "%.0fm", realDist);
                DrawTextStroke(feet_screen_txt.x, feet_screen_txt.y + 6,
                               text_fill, text_stroke, distBuf);
              }
            }
          }

          if (GUI::v_line) {
            Vector2 snap_target;
            if (WorldToScreen(box_root, snap_target)) {
              // Snapline color with per-feature invisible support
              float *sl_vis = GUI::snapline_visible_color;
              float *sl_invis = GUI::use_global_invisible_color
                                    ? GUI::invisible_color
                                    : GUI::snapline_invisible_color;
              float *sl_c = player.IsVisible ? sl_vis : sl_invis;

              ImColor snap_col = ImColor(sl_c[0], sl_c[1], sl_c[2], sl_c[3]);
              ImVec2 from;
              if (GUI::v_line_type == 0) // Bottom
                from = ImVec2(ScreenWidth / 2, ScreenHeight);
              else if (GUI::v_line_type == 1) // Top
                from = ImVec2(ScreenWidth / 2, 0);
              else // Center
                from = ImVec2(ScreenWidth / 2, ScreenHeight / 2);
              draw->AddLine(from, ImVec2(snap_target.x, snap_target.y),
                            (ImU32)snap_col, 1.0f);
            }
          }

          if (GUI::v_tool && !player.WeaponName.empty()) {
            Vector2 ft_scr;
            if (WorldToScreen(box_root, ft_scr)) {
              float yOff = GUI::v_dist ? 22.0f : 5.0f;
              DrawTextStroke(ft_scr.x, ft_scr.y + yOff,
                             IM_COL32(200, 200, 200, 255), text_stroke,
                             player.WeaponName.c_str());
            }
          }
        }

        if (debugFrameCount % 60 == 0) {
          printf("\r[ESP] Players: %llu | Rendered: %d | FOV: %.0f          ",
                 (unsigned long long)CachedPlayers.size(), renderedCount,
                 cam_fov);
        }
      }
    }

    // ========== GUI MENU (Neverlose-style custom drawn menu) ==========
    if (showmenu) {
      GUI::RenderMainMenu(&showmenu, 1.0f);
    }

    ImGui::Render();
    const float clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                            nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               clear_color);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);

  } catch (const std::exception &e) {
    std::cout << "\n[!] Exception in RenderFrame: " << e.what() << std::endl;
  } catch (...) {
    std::cout << "\n[!] Unknown exception in RenderFrame" << std::endl;
  }
}

void OverlayMenu::Shutdown() {
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
  CleanupDeviceD3D();
}

void OverlayMenu::SetupStyle() {
  ImGuiStyle &style = ImGui::GetStyle();
  style.WindowRounding = 5.0f;
  style.FrameRounding = 4.0f;
  style.GrabRounding = 4.0f;
  style.ScrollbarRounding = 0.0f;
  style.PopupRounding = 5.0f;

  ImVec4 *colors = style.Colors;
  colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.95f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.00f, 0.30f, 0.50f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.10f, 0.15f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.20f, 0.30f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.30f, 0.45f, 1.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.20f, 0.40f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.50f, 0.80f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 0.50f, 0.80f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.70f, 1.00f, 1.00f);
  colors[ImGuiCol_Button] = ImVec4(0.00f, 0.20f, 0.40f, 0.40f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.30f, 0.50f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.40f, 0.60f, 1.00f);
  colors[ImGuiCol_Header] = ImVec4(0.00f, 0.20f, 0.40f, 0.31f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.30f, 0.50f, 0.80f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.40f, 0.60f, 1.00f);
  colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.30f, 0.50f, 0.50f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.00f, 0.40f, 0.60f, 0.78f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.50f, 0.70f, 1.00f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.20f, 0.40f, 0.25f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.30f, 0.50f, 0.67f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.40f, 0.60f, 0.95f);
  colors[ImGuiCol_Tab] = ImVec4(0.05f, 0.15f, 0.25f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.30f, 0.50f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.40f, 0.60f, 1.00f);
  colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
}
