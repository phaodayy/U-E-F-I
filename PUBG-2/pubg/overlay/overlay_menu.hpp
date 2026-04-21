#pragma once
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

#include "../sdk/Common/Constant.h"

struct AimConfig {
    bool enabled = true;
    float fov = 10.0f;
    float smooth = 5.0f;
    int bone = 6; // 6:Head, 5:Neck, 4:Chest
    int key = VK_RBUTTON;
    float max_dist = 400.0f;
    bool prediction = true;
};


class OverlayMenu {
public:
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();

  HWND target_hwnd = NULL;
  static HWND FindOverlayForGame(HWND game_hwnd);

  float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
  float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
  bool showmenu = true;
  Scene current_scene = Scene::Lobby; 
  int active_tab = 0; // 0: Visuals, 1: Aimbot, 2: Macro, 3: Radar, 4: Settings
  bool esp_toggle = true;
  bool esp_box = true;
  int esp_box_type = 0;
  bool esp_fillbox = false;
  bool esp_head_circle = false;
  bool esp_skeleton = true;
  bool esp_health = true;
  int esp_health_pos = 0;
  bool esp_distance = true;
  bool esp_name = true;
  bool esp_snapline = false;
  bool esp_weapon = true;
  int esp_weapon_type = 1; // 0: Text, 1: Icon
  bool esp_shield = true;
  bool esp_spectated = true;
  bool esp_offscreen = true;
  bool esp_items = true; // Item ESP Toggle
  bool esp_vehicles = true; 
  bool esp_airdrops = true;
  bool esp_deadboxes = true;
  int loot_max_dist = 150;
  int vehicle_max_dist = 1000;
  bool esp_show_enemies = true;
  bool esp_show_teammates = false;
  int snapline_type = 0;

  float box_visible_color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float box_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f}; 
  float skeleton_visible_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float skeleton_invisible_color[4] = {1.0f, 1.0f, 1.0f, 0.7f};
  float name_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float distance_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float weapon_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float box_fill_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
  float snapline_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float shield_color[4] = {0.25f, 0.6f, 1.0f, 1.0f};
  bool esp_skel_interp = true;
  float skel_thickness = 1.0f;
  int box_max_dist = 600; 
  int hp_max_dist = 400; 
  int distance_txt_max_dist = 500; 
  float skel_alpha = 1.0f;
  float box_alpha = 1.0f;
  float fill_alpha = 0.25f;
  int render_distance = 600;
  int render_sleep = 5;
  bool esp_distance_lod = true; // NEW
  int skeleton_max_dist = 300; // NEW
  int name_max_dist = 200; // NEW
  int weapon_max_dist = 150; // NEW
  bool language = 0; // 0: English, 1: Vietnamese
  bool macro_enabled = false;
  bool macro_humanize = true;
  bool macro_ads_only = true; // NEW
  bool show_macro_overlay = true; // Show current weapon on OSD
  float macro_overlay_color[4] = { 0.0f, 1.0f, 0.8f, 1.0f }; // Default cyan

  // Aimbot Settings - ALWAYS PER-WEAPON NOW
  bool aim_master_enabled = true;
  bool aim_visible_only = true;
  bool aim_adaptive_fov = true;

  AimConfig aim_configs[9]; // AR, SR, DMR, SMG, LMG, SG, PT, OTHER, GLOBAL
  int aim_category_idx = 8; // Current category being edited in the menu
  int* waiting_for_key = nullptr;


  float radar_offset_x = 0.0f;
  float radar_offset_y = 0.0f;
  float radar_zoom_multiplier = 1.0f;
  float radar_scale = 1.0f;
  float radar_dot_size = 8.0f;
  float radar_rotation_offset = 0.0f;
  bool radar_enabled = true;
  bool show_radar_center = false;
  bool anti_screenshot = true;

  void Initialize(HWND game_hwnd = nullptr);
  void SetClickable(bool state);
  void SetupStyle();
  void UpdateAntiScreenshot();
  void RenderFrame();
  void DoAimbot();
  void Shutdown();

  void SaveConfig(const char *path);
  void LoadConfig(const char *path);
};

extern OverlayMenu g_Menu;
