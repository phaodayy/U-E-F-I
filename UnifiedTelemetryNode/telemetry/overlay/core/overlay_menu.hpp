#pragma once
#include "../../imgui/imgui.h"
#include "../../sdk/context.hpp"
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>
#include <string>
#include <vector>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "../../sdk/Common/Constant.h"

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
    int bone = 6;
    int key = VK_RBUTTON;
    float max_dist = 400.0f;
    bool prediction = true;
};

struct TextureInfo {
    ID3D11ShaderResourceView* SRV = nullptr;
    int Width = 0;
    int Height = 0;
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
  int active_tab = 0;
  bool anti_screenshot = true;

  bool esp_toggle = true;
  bool esp_icons = true;
  bool esp_box = true;
  int esp_box_type = 0;
  bool esp_fillbox = false;
  bool esp_head_circle = false;
  bool esp_skeleton = true;
  bool esp_health = true;
  int esp_health_pos = 0;
  int esp_distance_pos = 3;
  int esp_name_pos = 2;
  int esp_rank_pos = 2;
  int esp_weapon_pos = 2;
  int esp_spectated_pos = 2;
  int esp_teamid_pos = 0;
  int esp_killcount_pos = 2;
  int esp_survival_level_pos = 2;
  int esp_health_color_mode = 0;
  bool esp_distance = true;
  bool esp_name = true;
  bool esp_killcount = false;
  bool esp_teamid = false;
  bool esp_spectators = true;
  bool esp_rank = false;
  bool esp_survival_level = false;
  bool esp_snapline = false;
  bool esp_weapon = true;
  int esp_weapon_type = 1;
  bool esp_shield = true;
  bool esp_spectated = true;
  bool esp_offscreen = true;
  int esp_offscreen_style = 0;
  int offscreen_color_mode = 1;
  float offscreen_radius = 329.0f;
  float offscreen_size = 5.0f;
  float offscreen_near_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
  float offscreen_far_color[4] = {0.0f, 1.0f, 0.4f, 1.00f};
  bool debug_actor_esp = false;
  bool debug_loot_resolver = false;
  int active_preset = 0;
  
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

  float box_visible_color[4] = {0.0f, 1.0f, 0.0f, 1.0f};
  float box_invisible_color[4] = {1.0f, 0.5f, 0.0f, 1.0f}; 
  float skeleton_visible_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float skeleton_invisible_color[4] = {1.0f, 1.0f, 1.0f, 0.7f};
  float name_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float distance_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float weapon_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float rank_color[4] = {0.78f, 0.78f, 0.78f, 1.0f};
  float teamid_color[4] = {0.0f, 0.78f, 1.0f, 1.0f};
  float kill_color[4] = {1.0f, 0.78f, 0.18f, 1.0f};
  float survival_level_color[4] = {0.35f, 0.9f, 1.0f, 1.0f};
  float spectated_color[4] = {1.0f, 0.67f, 0.0f, 1.0f};
  float health_color[4] = {0.0f, 1.0f, 0.5f, 1.0f};
  float box_fill_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
  float snapline_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float shield_color[4] = {0.25f, 0.6f, 1.0f, 1.0f};
  bool language = 0;

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
  float name_font_size = 14.0f;
  float distance_font_size = 12.6f;
  float weapon_font_size = 14.7f;
  float rank_font_size = 11.9f;
  float teamid_badge_size = 18.0f;
  float kill_font_size = 12.0f;
  float survival_level_font_size = 12.0f;
  float spectated_font_size = 15.0f;
  float weapon_icon_size = 58.0f;
  bool esp_skeleton_dots = true;
  bool esp_spectator_list = true;
  
  bool esp_items = true;
  bool esp_vehicles = true;
  int esp_items_toggle_key = 0;
  int esp_vehicles_toggle_key = 0;
  bool esp_airdrops = true;
  bool esp_deadboxes = true;
  bool loot_weapon_special = true;
  bool loot_weapon_all = false;
  int loot_max_dist = 150;
  int vehicle_max_dist = 1000;
  float item_icon_size = 24.0f;
  float item_group_icon_size = 18.0f;
  float vehicle_icon_size = 34.0f;
  float loot_distance_font_size = 12.0f;

  bool loot_armor_lv1 = false;
  bool loot_armor_lv2 = true;
  bool loot_armor_lv3 = true;
  bool loot_helmet_lv1 = false;
  bool loot_helmet_lv2 = true;
  bool loot_helmet_lv3 = true;
  bool loot_backpack_lv1 = false;
  bool loot_backpack_lv2 = true;
  bool loot_backpack_lv3 = true;
  bool loot_ghillie_all = true;
  bool loot_ghillie_arctic = true;
  bool loot_ghillie_desert = true;
  bool loot_ghillie_jungle = true;
  bool loot_ghillie_forest = true;
  bool loot_ghillie_mossy = true;
  bool loot_ghillie_brown = true;

  bool loot_meds_boosts = true;
  bool loot_meds_healing = true;
  bool loot_utility_all = true;
  bool loot_utility_drone = true;
  bool loot_utility_scope = true;
  bool loot_repair_armor = true;
  bool loot_repair_helmet = true;
  bool loot_repair_vehicle = true;
  bool loot_utility_jammer = true;
  bool loot_utility_bluechip = true;
  bool loot_utility_vtransmitter = true;
  bool loot_utility_shield = true;
  bool loot_utility = true;
  bool loot_repair = true;

  bool loot_ammo_556 = true;
  bool loot_ammo_762 = true;
  bool loot_ammo_9mm = true;
  bool loot_ammo_45 = true;
  bool loot_ammo_12g = true;
  bool loot_ammo_300 = true;
  bool loot_ammo_slug = true;
  bool loot_ammo_57 = true;
  bool loot_ammo_40 = true;
  bool loot_ammo_bolt = true;
  bool loot_ammo_flare = true;
  bool loot_ammo_mortar = true;

  bool loot_key_secret = true;
  bool loot_key_chimera = true;
  bool loot_key_vikendi = true;
  bool loot_key_haven = true;
  bool loot_key_security = true;
  bool loot_key_taego = true;

  bool loot_scope_reddot = true;
  bool loot_scope_holo = true;
  bool loot_scope_2x = true;
  bool loot_scope_3x = true;
  bool loot_scope_4x = true;
  bool loot_scope_6x = true;
  bool loot_scope_8x = true;
  bool loot_scope_15x = true;
  bool loot_scope_thermal = true;

  bool loot_muzzle_comp = true;
  bool loot_muzzle_flash = true;
  bool loot_muzzle_supp = true;
  bool loot_muzzle_choke = true;

  bool loot_grip_vertical = true;
  bool loot_grip_angled = true;
  bool loot_grip_half = true;
  bool loot_grip_thumb = true;
  bool loot_grip_light = true;
  bool loot_stock_heavy = true;
  bool loot_stock_cheek = true;

  bool loot_mag_ext = true;
  bool loot_mag_quick = true;
  bool loot_mag_ext_quick = true;

  bool loot_ammo_all = false;
  bool loot_ammo_high = true;
  bool loot_attach_grip = true;
  bool loot_attach_stock = true;
  bool loot_attach_mag = true;
  bool loot_attach_muzzle = true;
  bool loot_ghillie = true;
  bool loot_scopes_all = false;
  bool loot_scopes_high = true;
  bool loot_attach_scope_all = false;
  bool loot_attach_scope_high = true;

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

  bool loot_throw_frag = true;
  bool loot_throw_smoke = true;
  bool loot_throw_molotov = true;
  bool loot_throw_flash = true;
  bool loot_throw_c4 = true;
  bool loot_throw_sticky = true;
  bool loot_throw_bz = true;
  bool loot_throw_decoy = true;

  bool loot_weapon_db = true; 
  bool loot_weapon_s12k = true;
  bool loot_weapon_dp12 = true;
  bool loot_weapon_saiga = true;
  bool loot_weapon_deagle = true;
  bool loot_weapon_m1911 = true;
  bool loot_weapon_p92 = true;
  bool loot_weapon_skorpion = true;
  bool loot_weapon_nagant = true;
  bool loot_weapon_rhino = true;
  bool loot_weapon_stungun = true;

  bool loot_weapon_pan = true;
  bool loot_weapon_spike = true;
  bool loot_weapon_m79 = true;
  bool loot_weapon_flare = true;
  bool loot_weapon_crossbow = true;
  bool loot_weapon_panzer = true;

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
  void RenderLootEsp(ImDrawList* draw);
  void RenderSinglePlayerEsp(ImDrawList* draw, PlayerData& player, const Vector3& delta,
                             const Vector2& local_feet_s, bool hasLocalS,
                             float ScreenCenterX, float ScreenHeight);
  void RenderMainMenuWindow(ImDrawList* draw, float ScreenWidth, float ScreenHeight);
  void RenderPlayersAndAim(ImDrawList* draw, std::vector<PlayerData>& localPlayers,
                           const Vector2& local_feet_s, bool hasLocalS,
                           float ScreenCenterX, float ScreenCenterY,
                           float ScreenHeight, bool is_authenticated);
  void DrawLicenseWatermark(ImDrawList* draw);
  void DrawGlobalSpectatorWarning(ImDrawList* draw, float ScreenWidth);
  void RenderAdminDebugEsp(ImDrawList* draw);
  void RenderMacroOsd(ImDrawList* draw, float ScreenWidth, float ScreenHeight);
  void RenderSpectatorThreatList(ImDrawList* draw, const std::vector<PlayerData>& localPlayers,
                                 float ScreenWidth);
  void DrawAntiScreenshotWarning(ImDrawList* draw, float ScreenHeight);

  void BeginGlassCard(const char* id, const char* title, ImVec2 size = ImVec2(0,0));
  void DrawDisplayOnlyOption(const char* label);

  void RenderTabVisuals(ImVec2 windowSize);
  void RenderTabPrecision(ImVec2 windowSize);
  void RenderTabMacro(ImVec2 windowSize);
  void RenderTabLoot(ImVec2 windowSize);
  void RenderTabRadar(ImVec2 windowSize);
  void RenderTabSettings(ImVec2 windowSize);
  void RenderTabAdmin(ImVec2 windowSize);
};

extern TextureInfo PreviewInstructor;
extern TextureInfo* GetPreviewIcon(std::string folder, std::string asset);
extern OverlayMenu g_Menu;
