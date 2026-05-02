#pragma once
#include "../../imgui/imgui.h"
#include "../../sdk/core/context.hpp"
#include <d3d11.h>
#include <dwmapi.h>
#include <windows.h>
#include <string>
#include <unordered_map>
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

struct TextureInfo {
    ID3D11ShaderResourceView* SRV = nullptr;
    int Width = 0;
    int Height = 0;
    int Frames = 1;
};

class OverlayMenu {
public:
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  bool CreateRenderTarget();
  void CleanupRenderTarget();

  HWND target_hwnd = NULL;
  LONG_PTR original_style = 0;
  LONG_PTR original_ex_style = 0;
  RECT original_rect = { 0 };
  float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
  float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
  bool showmenu = true;
  Scene current_scene = Scene::Lobby;
  int active_tab = 0;
  bool anti_screenshot = true;
  double cloud_save_last_time = 0.0;
  int overlay_mode = 0; // 0: Movavi (Passive), 1: Ghost Window (Active)

  bool esp_toggle = true;
  bool esp_icons = true;
  bool esp_box = true;
  int esp_box_type = 1;
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
  int esp_damage_pos = 2;
  int esp_speed_pos = 3;
  int esp_ammo_pos = 2;

  int esp_health_row = 0;
  int esp_distance_row = 1;
  int esp_name_row = 0;
  int esp_rank_row = 0;
  int esp_weapon_row = 0;
  int esp_spectated_row = 0;
  int esp_teamid_row = 0;
  int esp_killcount_row = 0;
  int esp_survival_level_row = 0;
  int esp_damage_row = 0;
  int esp_speed_row = 0;
  int esp_ammo_row = 0;

  int esp_health_color_mode = 0;
  bool esp_distance = true;
  bool esp_name = true;
  bool esp_killcount = false;
  bool esp_damage = false;
  bool esp_speed = false;
  bool esp_ammo = true;
  int esp_health_display_mode = 0;
  int esp_health_bar_style = 0;
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
  bool esp_offscreen_text = true;
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
  bool minimap_enabled = true;
  bool bigmap_enabled = true;
  bool minimap_show_direction = true;
  bool minimap_fire_trace = true;
  float minimap_view_ray_length = 55.0f;
  float minimap_fire_ray_length = 240.0f;
  float minimap_fire_flash_ms = 420.0f;
  float minimap_ray_width = 1.1f;
  bool bigmap_show_names = true;
  bool bigmap_show_direction = true;
  bool bigmap_name_background = true;
  bool bigmap_show_legend = true;
  bool bigmap_show_vehicles = true;
  bool bigmap_show_airdrops = true;
  bool bigmap_show_deadboxes = false;
  float bigmap_marker_size = 8.0f;
  float bigmap_marker_alpha = 0.60f;
  float bigmap_icon_size = 20.0f;
  float bigmap_name_font_size = 13.0f;
  float bigmap_name_bg_alpha = 0.34f;
  float bigmap_offset_x = 0.0f;
  float bigmap_offset_y = 0.0f;
  float bigmap_screen_scale = 1.0f;

  bool flick_enabled = true;
  bool flick_visible_only = true;
  bool flick_auto_shot = false;
  bool flick_shot_hold = true;
  bool flick_return = true;
  int flick_behavior_mode = 1;
  bool flick_fov_circle = true;
  float flick_fov_circle_color[4] = { 0.0f, 0.70f, 1.0f, 0.40f };
  float flick_smoothness = 1.0f;
  bool flick_follow_auto_shot = false;
  std::unordered_map<std::string, bool> flick_weapon_enabled;
  std::unordered_map<std::string, bool> flick_category_enabled;
  std::unordered_map<std::string, bool> flick_category_visible_only;
  std::unordered_map<std::string, bool> flick_category_shot_hold;
  std::unordered_map<std::string, bool> flick_category_auto_shot;
  std::unordered_map<std::string, bool> flick_category_follow_auto_shot;
  std::unordered_map<std::string, int> flick_category_behavior_mode;
  std::unordered_map<std::string, int> flick_category_target_part;
  std::unordered_map<std::string, int> flick_category_key;
  std::unordered_map<std::string, float> flick_category_move_speed;
  std::unordered_map<std::string, float> flick_category_smoothness;
  std::unordered_map<std::string, bool> flick_category_fov_circle;
  std::unordered_map<std::string, float> flick_category_fov;
  std::unordered_map<std::string, float> flick_category_max_dist;
  int flick_selected_category = 0;
  float flick_fov = 100.0f;
  float flick_max_dist = 400.0f;
  int flick_target_part = 3;
  int flick_key = VK_RBUTTON;
  int flick_key2 = 0;
  bool flick_weapon_s686 = true;
  bool flick_weapon_s12k = true;
  bool flick_weapon_s1897 = true;
  bool flick_weapon_dbs = true;
  bool flick_weapon_o12 = true;
  bool flick_weapon_slr = true;
  bool flick_weapon_mini14 = true;
  bool flick_weapon_sks = true;
  bool flick_weapon_vss = true;
  bool flick_weapon_qbu = true;
  bool flick_weapon_kar98k = true;
  bool flick_weapon_m24 = true;
  bool flick_weapon_awm = true;
  bool flick_weapon_lynx = true;
  bool flick_weapon_win94 = true;
  bool flick_weapon_mosin = true;
  bool flick_weapon_panzerfaust = true;
  bool flick_weapon_mk12 = true;
  bool flick_weapon_mk14 = true;
  bool flick_weapon_dragunov = true;

  bool aimbot_enabled = false;
  bool aimbot_visible_only = true;
  bool aimbot_draw_fov = true;
  float aimbot_fov = 60.0f;
  float aimbot_max_dist = 350.0f;
  float aimbot_smooth = 8.0f;
  int aimbot_target_part = 3;
  int aimbot_key = VK_LMENU;
  int aimbot_key2 = 0;
  bool aimbot_weapon_m416 = true;
  bool aimbot_weapon_akm = true;
  bool aimbot_weapon_beryl = true;
  bool aimbot_weapon_ace32 = true;
  bool aimbot_weapon_aug = true;
  bool aimbot_weapon_groza = true;
  bool aimbot_weapon_scarl = true;
  bool aimbot_weapon_m16 = true;
  bool aimbot_weapon_qbz = true;
  bool aimbot_weapon_k2 = true;
  bool aimbot_weapon_famas = true;
  bool aimbot_weapon_g36c = true;
  bool aimbot_weapon_mutant = true;
  bool aimbot_weapon_slr = true;
  bool aimbot_weapon_mini14 = true;
  bool aimbot_weapon_sks = true;
  bool aimbot_weapon_vss = true;
  bool aimbot_weapon_qbu = true;
  bool aimbot_weapon_mk12 = true;
  bool aimbot_weapon_mk14 = true;
  bool aimbot_weapon_dragunov = true;
  bool aimbot_weapon_kar98k = true;
  bool aimbot_weapon_m24 = true;
  bool aimbot_weapon_awm = true;
  bool aimbot_weapon_lynx = true;
  bool aimbot_weapon_win94 = true;
  bool aimbot_weapon_mosin = true;

  bool macro_enabled = false;
  bool macro_humanize = true;
  bool macro_ads_only = true;
  bool show_macro_overlay = true;
  float macro_recoil_strength = 50.0f;
  float macro_overlay_color[4] = { 0.0f, 1.0f, 0.8f, 1.0f };
  int* waiting_for_key = nullptr;

  float box_visible_color[4] = {1.0f, 0.22f, 0.24f, 1.0f};
  float box_invisible_color[4] = {1.0f, 0.72f, 0.26f, 0.90f}; 
  float skeleton_visible_color[4] = {0.92f, 0.96f, 1.0f, 0.78f};
  float skeleton_invisible_color[4] = {1.0f, 0.82f, 0.48f, 0.58f};
  float name_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  float name_visible_color[4] = {0.92f, 0.96f, 1.0f, 0.96f};
  float name_invisible_color[4] = {1.0f, 0.82f, 0.48f, 0.88f};
  float distance_color[4] = {0.82f, 0.88f, 0.94f, 0.95f};
  float weapon_color[4] = {0.80f, 0.94f, 1.0f, 0.95f};
  float rank_color[4] = {0.78f, 0.78f, 0.78f, 1.0f};
  float teamid_color[4] = {0.0f, 0.78f, 1.0f, 1.0f};
  float kill_color[4] = {1.0f, 0.78f, 0.18f, 1.0f};
  float survival_level_color[4] = {0.35f, 0.9f, 1.0f, 1.0f};
  float damage_color[4] = {1.0f, 0.48f, 0.18f, 1.0f};
  float speed_color[4] = {0.45f, 1.0f, 0.72f, 1.0f};
  float ammo_color[4] = {0.95f, 0.95f, 0.72f, 1.0f};
  float close_warning_color[4] = {1.0f, 0.20f, 0.08f, 1.0f};
  float aim_warning_color[4] = {1.0f, 0.12f, 0.08f, 1.0f};
  float view_direction_color[4] = {0.20f, 0.82f, 1.0f, 0.85f};
  float spectated_color[4] = {1.0f, 0.67f, 0.0f, 1.0f};
  float health_color[4] = {0.27f, 0.90f, 0.52f, 1.0f};
  float box_fill_color[4] = {0.0f, 0.0f, 0.0f, 0.25f};
  float snapline_color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float shield_color[4] = {0.25f, 0.6f, 1.0f, 1.0f};
  bool team_color_custom = false;
  float team_custom_colors[4][4] = {
    {1.0f, 0.24f, 0.24f, 1.0f},
    {0.20f, 0.82f, 1.0f, 1.0f},
    {0.35f, 1.0f, 0.55f, 1.0f},
    {1.0f, 0.76f, 0.18f, 1.0f}
  };
  bool esp_multilayer_nameplate = true;
  int language = 0; // 0: English, 1: Vietnamese

  int render_distance = 400;
  int render_sleep = 5;
  int box_max_dist = 400; 
  int hp_max_dist = 400; 
  int distance_txt_max_dist = 400; 
  int skeleton_max_dist = 300;
  int name_max_dist = 200;
  int weapon_max_dist = 150;
  bool esp_distance_lod = true;
  bool esp_prediction = true;
  bool esp_grenade_prediction = true;
  bool esp_projectile_tracer = false;
  bool esp_threat_warning = true;
  bool esp_aim_warning = true;
  bool esp_view_direction = true;
  bool esp_status_badges = true;
  bool esp_health_text = true;
  bool esp_close_warning = true;
  float esp_close_warning_distance = 65.0f;
  float esp_view_direction_length = 35.0f;
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
  float damage_font_size = 12.0f;
  float speed_font_size = 12.0f;
  float ammo_font_size = 12.0f;
  float spectated_font_size = 15.0f;
  float weapon_icon_size = 58.0f;
  bool esp_text_background = true;
  float esp_text_bg_alpha = 0.32f;
  bool esp_skeleton_dots = true;
  bool esp_spectator_list = true;
  bool player_list_enabled = true;
  bool player_list_hold_required = true;
  int player_list_hold_key = VK_TAB;
  
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
  bool asset_animation_enabled = true;
  bool asset_animation_glow = false;
  bool asset_animation_shine = false;
  float asset_animation_strength = 1.0f;
  float asset_animation_speed = 1.0f;

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
                                 float ScreenWidth, float ScreenHeight);
  void DrawAntiScreenshotWarning(ImDrawList* draw, float ScreenHeight);

  void BeginGlassCard(const char* id, const char* title, ImVec2 size = ImVec2(0,0));
  void DrawDisplayOnlyOption(const char* label);

  void RenderTabVisuals(ImVec2 windowSize);
  void RenderTabPrecision(ImVec2 windowSize);
  void RenderTabAimbot(ImVec2 windowSize);
  void RenderTabMacro(ImVec2 windowSize);
  void RenderTabLoot(ImVec2 windowSize);
  void RenderTabRadar(ImVec2 windowSize);
  void RenderTabSettings(ImVec2 windowSize);
  void RenderTabAdmin(ImVec2 windowSize);
};

extern TextureInfo PreviewInstructor;
extern TextureInfo* GetPreviewIcon(std::string folder, std::string asset);
extern OverlayMenu g_Menu;
