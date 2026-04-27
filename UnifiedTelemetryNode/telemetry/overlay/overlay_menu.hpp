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

struct VisualizationBridgeHost {
    HWND hwnd = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    bool clear_before_render = false;
    bool present_after_render = false;
};

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
  bool CreateRenderTarget();
  void CleanupRenderTarget();

  HWND target_hwnd = NULL;

  float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
  float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
  bool showmenu = true;
  Scene current_scene = Scene::Lobby; 
  int active_tab = 0; // 0: Visuals, 1: precision_calibration, 2: Macro, 3: Radar, 4: Settings
  bool esp_toggle = true;
  bool esp_icons = true; // NEW: Toggle between text and icons
  bool esp_box = true;
  int esp_box_type = 0;
  bool esp_fillbox = false;
  bool esp_head_circle = false;
  bool esp_skeleton = true;
  bool esp_health = true;
  int esp_health_pos = 0; // 0:Left, 1:Right, 2:Top, 3:Bottom
  int esp_health_color_mode = 0; // 0:Dynamic, 1:Static
  bool esp_distance = true;
  bool esp_name = true;
  bool esp_killcount = false;
  bool esp_teamid = false;
  bool esp_spectators = true;
  bool esp_rank = false;
  bool esp_survival_level = false;
  bool esp_snapline = false;
  bool esp_weapon = true;
  int esp_weapon_type = 1; // 0: Text, 1: Icon
  bool esp_shield = true;
  bool esp_spectated = true;
  bool esp_offscreen = true;
  int esp_offscreen_style = 0; // 0: Triangle
  int offscreen_color_mode = 1; // 1: Distance
  float offscreen_radius = 329.0f;
  float offscreen_size = 5.0f;
  float offscreen_near_color[4] = {1.0f, 0.0f, 0.0f, 1.0f}; // Sharp Red
  float offscreen_far_color[4] = {0.0f, 1.0f, 0.4f, 1.00f}; // Vibrant Neon Green
  // --- [ MASTER ESP & RADAR ] ---
  bool esp_show_enemies = true;
  bool esp_show_teammates = false;
  int snapline_type = 0;
  bool radar_enabled = true;
  bool show_radar_center = false;
  bool share_radar = false;
  char share_radar_ip[128] = "127.0.0.1";
  float radar_offset_x = 0.0f;
  float radar_offset_y = 0.0f;
  float radar_zoom_multiplier = 1.0f;
  float radar_scale = 1.0f;
  float radar_dot_size = 8.0f;
  float radar_rotation_offset = 0.0f;
  bool anti_screenshot = true;

  // --- [ AIM & MACRO ] ---
  bool aim_master_enabled = true;
  bool aim_visible_only = true;
  bool aim_adaptive_fov = true;
  float aim_smooth_rng = 0.0f;
  int aim_key2 = 0;
  bool macro_enabled = false;
  bool macro_humanize = true;
  bool macro_ads_only = true;
  bool show_macro_overlay = true;
  float macro_overlay_color[4] = { 0.0f, 1.0f, 0.8f, 1.0f };
  AimConfig aim_configs[9];
  int aim_category_idx = 8;
  int* waiting_for_key = nullptr;

  // --- [ COLORS & VISUALS ] ---
  float box_visible_color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float box_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f}; 
  float skeleton_visible_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float skeleton_invisible_color[4] = {1.0f, 1.0f, 1.0f, 0.7f};
  float name_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float distance_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float weapon_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float health_color[4] = {0.0f, 1.0f, 0.5f, 1.0f};
  float box_fill_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
  float snapline_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float shield_color[4] = {0.25f, 0.6f, 1.0f, 1.0f};
  bool language = 0; 

  // --- [ DISTANCE LIMITS & TACTICAL ] ---
  int render_distance = 600;
  int render_sleep = 5;
  int box_max_dist = 600; 
  int hp_max_dist = 400; 
  int distance_txt_max_dist = 500; 
  int skeleton_max_dist = 300;
  int name_max_dist = 200;
  int weapon_max_dist = 150;
  bool esp_distance_lod = true;
  bool esp_grenade_prediction = true;
  bool esp_projectile_tracer = false;
  bool esp_threat_warning = true;
  bool esp_skel_interp = true;
  float skel_thickness = 1.0f;
  float box_thickness = 1.0f;
  float esp_font_size = 14.0f;
  bool esp_skeleton_dots = true;
  bool esp_spectator_list = true;
  
  // --- [ LOOT TELEMETRY: MASTER & CATEGORIES ] ---
  bool esp_items = true; 
  bool esp_vehicles = true; 
  bool esp_airdrops = true;
  bool esp_deadboxes = true;
  bool loot_weapon_special = true;
  bool loot_weapon_all = false;
  int loot_max_dist = 150;
  int vehicle_max_dist = 1000;

  // --- [ LOOT TELEMETRY: GEAR ] ---
  bool loot_armor_lv1 = false;
  bool loot_armor_lv2 = true;
  bool loot_armor_lv3 = true;
  bool loot_helmet_lv1 = false;
  bool loot_helmet_lv2 = true;
  bool loot_helmet_lv3 = true;
  bool loot_backpack_lv1 = false;
  bool loot_backpack_lv2 = true;
  bool loot_backpack_lv3 = true;
  bool loot_ghillie = true;

  // --- [ LOOT TELEMETRY: MEDICINE & UTILITY ] ---
  bool loot_meds_boosts = true;
  bool loot_meds_healing = true;
  bool loot_utility = true;
  bool loot_repair = true;

  // --- [ LOOT TELEMETRY: ATTACHMENTS ] ---
  bool loot_ammo_all = false;
  bool loot_ammo_high = true;
  bool loot_scopes_all = false;
  bool loot_scopes_high = true;
  bool loot_attach_mag = true;
  bool loot_attach_muzzle = true;
  bool loot_attach_grip = true;
  bool loot_attach_stock = true;
  bool loot_attach_scope_all = false;
  bool loot_attach_scope_high = true;

  // --- [ LOOT TELEMETRY: WEAPONS - AR ] ---
  bool loot_weapon_ace32 = true;
  bool loot_weapon_ak47 = true;
  bool loot_weapon_aug = true;
  bool loot_weapon_beryl = true;
  bool loot_weapon_g36c = true;
  bool loot_weapon_groza = true;
  bool loot_weapon_hk416 = true;
  bool loot_weapon_k2 = true;
  bool loot_weapon_m16 = true;
  bool loot_weapon_mutant = true;
  bool loot_weapon_qbz = true;
  bool loot_weapon_scar = true;
  bool loot_weapon_famas = true;

  // --- [ LOOT TELEMETRY: WEAPONS - SR & DMR ] ---
  bool loot_weapon_awm = true;
  bool loot_weapon_kar98 = true;
  bool loot_weapon_m24 = true;
  bool loot_weapon_mosin = true;
  bool loot_weapon_win94 = true;
  bool loot_weapon_dragunov = true;
  bool loot_weapon_mini14 = true;
  bool loot_weapon_mk12 = true;
  bool loot_weapon_mk14 = true;
  bool loot_weapon_qbu = true;
  bool loot_weapon_sks = true;
  bool loot_weapon_vss = true;
  bool loot_weapon_slr = true;

  // --- [ LOOT TELEMETRY: WEAPONS - SMG & LMG ] ---
  bool loot_weapon_bizon = true;
  bool loot_weapon_mp5 = true;
  bool loot_weapon_mp9 = true;
  bool loot_weapon_p90 = true;
  bool loot_weapon_thompson = true;
  bool loot_weapon_ump = true;
  bool loot_weapon_uzi = true;
  bool loot_weapon_vector = true;
  bool loot_weapon_js9 = true;
  bool loot_weapon_dp28 = true;
  bool loot_weapon_m249 = true;
  bool loot_weapon_mg3 = true;

  // --- [ LOOT TELEMETRY: WEAPONS - SG & HG ] ---
  bool loot_weapon_db = true; 
  bool loot_weapon_s12k = true;
  bool loot_weapon_dp12 = true;
  bool loot_weapon_saiga = true;
  bool loot_weapon_deagle = true;
  bool loot_weapon_m1911 = true;
  bool loot_weapon_p92 = true;
  bool loot_weapon_skorpion = true;

  // --- [ LOOT TELEMETRY: WEAPONS - SPECIAL & MELEE ] ---
  bool loot_weapon_pan = true;
  bool loot_weapon_spike = true;
  bool loot_weapon_m79 = true;
  bool loot_weapon_flare = true;
  bool loot_weapon_crossbow = true;

  // --- [ LOOT TELEMETRY: VEHICLES ] ---
  bool loot_vehicle_uaz = true;
  bool loot_vehicle_dacia = true;
  bool loot_vehicle_buggy = true;
  bool loot_vehicle_bike = true;
  bool loot_vehicle_boat = true;
  bool loot_vehicle_air = true; 
  bool loot_vehicle_brdm = true;
  bool loot_vehicle_scooter = true;
  bool loot_vehicle_tuk = true;
  bool loot_vehicle_snow = true;
  bool loot_vehicle_bus = true;
  bool loot_vehicle_truck = true;
  bool loot_vehicle_train = true;
  bool loot_vehicle_mirado = true;
  bool loot_vehicle_pickup = true;
  bool loot_vehicle_rony = true;
  bool loot_vehicle_blanc = true;


  bool Initialize(const VisualizationBridgeHost& bridge);
  void SetupStyle();
  void UpdateAntiScreenshot();
  void RenderFrame();
  void Doprecision_calibration();
  void Shutdown();

  void SaveConfig(const char *path);
  void LoadConfig(const char *path);
};

extern OverlayMenu g_Menu;
