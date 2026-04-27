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

/**
 * @brief Host structure for the visualization bridge, managing D3D11 resources and window handles.
 */
struct VisualizationBridgeHost {
    HWND hwnd = nullptr;                 ///< Target window handle
    IDXGISwapChain* swap_chain = nullptr; ///< DirectX SwapChain for rendering
    ID3D11Device* device = nullptr;      ///< D3D11 Device
    ID3D11DeviceContext* context = nullptr; ///< D3D11 Device Context
    bool clear_before_render = false;     ///< Flag to clear render target before each frame
    bool present_after_render = false;    ///< Flag to call Present after rendering
};

/**
 * @brief Configuration settings for precision calibration/alignment logic.
 */
struct AimConfig {
    bool enabled = true;         ///< Master switch for this configuration
    float fov = 10.0f;           ///< Field of View limit for selection
    float smooth = 5.0f;        ///< Smoothing factor for movement
    int bone = 6;               ///< Target bone index (e.g., 6:Head, 5:Neck, 4:Chest)
    int key = VK_RBUTTON;       ///< Activation key code
    float max_dist = 400.0f;    ///< Maximum effective distance
    bool prediction = true;     ///< Enable trajectory prediction
};

/**
 * @brief Main Overlay Menu class managing the UI state, rendering, and feature toggles.
 */
class OverlayMenu {
public:
  // --- [ D3D11 Lifecycle Management ] ---
  bool CreateDeviceD3D(HWND hWnd);
  void CleanupDeviceD3D();
  bool CreateRenderTarget();
  void CleanupRenderTarget();

  HWND target_hwnd = NULL; ///< Handle of the window being overlaid

  // --- [ UI State & Global Settings ] ---
  float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
  float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
  bool showmenu = true;                   ///< Toggle for showing the ImGui menu
  Scene current_scene = Scene::Lobby;     ///< Current game state/scene
  int active_tab = 0;                     ///< Selected tab index (0: Visuals, 1: precision_calibration, 2: Macro, 3: Radar, 4: Settings)
  bool anti_screenshot = true;            ///< Enable protection against screenshot capture

  // --- [ VISUAL ESP (Extra Sensory Perception) ] ---
  bool esp_toggle = true;                 ///< Master toggle for Visuals
  bool esp_icons = true;                  ///< Toggle between text labels and graphical icons
  bool esp_box = true;                    ///< Draw bounding boxes around targets
  int esp_box_type = 0;                   ///< Box style index
  bool esp_fillbox = false;               ///< Fill the bounding box with a color
  bool esp_head_circle = false;           ///< Draw a circle around the head bone
  bool esp_skeleton = true;               ///< Draw skeletal lines
  bool esp_health = true;                 ///< Display health bars
  int esp_health_pos = 0;                 ///< Position of health bar (0:Left, 1:Right, 2:Top, 3:Bottom)
  int esp_health_color_mode = 0;          ///< Health color calculation (0:Dynamic, 1:Static)
  bool esp_distance = true;               ///< Show distance to target
  bool esp_name = true;                   ///< Show target name
  bool esp_killcount = false;             ///< Show target kill count
  bool esp_teamid = false;                ///< Show team ID
  bool esp_spectators = true;             ///< Show spectator count or list
  bool esp_rank = false;                  ///< Show target rank level
  bool esp_survival_level = false;        ///< Show survival level
  bool esp_snapline = false;              ///< Draw lines from screen center/bottom to target
  bool esp_weapon = true;                 ///< Show target's held weapon
  int esp_weapon_type = 1;                ///< Weapon display style (0: Text, 1: Icon)
  bool esp_shield = true;                 ///< Display shield/armor status
  bool esp_spectated = true;              ///< Alert if being spectated
  bool esp_offscreen = true;              ///< Show indicators for targets outside FOV
  int esp_offscreen_style = 0;            ///< Offscreen indicator style
  int offscreen_color_mode = 1;           ///< Offscreen color logic (1: Color by Distance)
  float offscreen_radius = 329.0f;        ///< Radius of offscreen indicator circle
  float offscreen_size = 5.0f;            ///< Size of offscreen indicators
  float offscreen_near_color[4] = {1.0f, 0.0f, 0.0f, 1.0f}; ///< Color for nearby offscreen targets
  float offscreen_far_color[4] = {0.0f, 1.0f, 0.4f, 1.00f}; ///< Color for distant offscreen targets
  
  // --- [ MASTER ESP & RADAR ] ---
  bool esp_show_enemies = true;           ///< Show enemy players
  bool esp_show_teammates = false;        ///< Show teammates
  int snapline_type = 0;                  ///< Origin of snaplines (e.g., bottom, center)
  bool radar_enabled = true;              ///< Enable mini-map radar
  bool show_radar_center = false;         ///< Draw center point on radar
  bool share_radar = false;               ///< Share radar data over network
  char share_radar_ip[128] = "127.0.0.1"; ///< Target IP for radar sharing
  float radar_offset_x = 0.0f;            ///< Radar UI offset X
  float radar_offset_y = 0.0f;            ///< Radar UI offset Y
  float radar_zoom_multiplier = 1.0f;     ///< Radar zoom level
  float radar_scale = 1.0f;               ///< Radar dot scaling
  float radar_dot_size = 8.0f;            ///< Size of individual dots on radar
  float radar_rotation_offset = 0.0f;     ///< Manual rotation correction for radar

  // --- [ PRECISION & MACRO ] ---
  bool aim_master_enabled = true;         ///< Master switch for precision aids
  bool aim_visible_only = true;           ///< Only target visible entities
  bool aim_adaptive_fov = true;           ///< Adjust FOV based on distance
  float aim_smooth_rng = 0.0f;            ///< Randomness factor for smoothing
  int aim_key2 = 0;                       ///< Alternative activation key
  bool macro_enabled = false;             ///< Enable input macros (recoil control, etc.)
  bool macro_humanize = true;             ///< Add random delays/offsets to macros
  bool macro_ads_only = true;             ///< Only apply macros when Aiming Down Sights
  bool show_macro_overlay = true;         ///< Draw status for macro engine
  float macro_overlay_color[4] = { 0.0f, 1.0f, 0.8f, 1.0f };
  AimConfig aim_configs[9];               ///< Per-weapon category configurations
  int aim_category_idx = 8;               ///< Currently active config category
  int* waiting_for_key = nullptr;         ///< Pointer to variable awaiting key assignment

  // --- [ COLORS & THEMES ] ---
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
  bool language = 0;                      ///< 0: English, 1: Vietnamese

  // --- [ DISTANCE LIMITS & TACTICAL ] ---
  int render_distance = 600;              ///< Global render distance cap
  int render_sleep = 5;                   ///< Sleep interval between render loops (ms)
  int box_max_dist = 600; 
  int hp_max_dist = 400; 
  int distance_txt_max_dist = 500; 
  int skeleton_max_dist = 300;
  int name_max_dist = 200;
  int weapon_max_dist = 150;
  bool esp_distance_lod = true;           ///< Level of Detail adjustments based on distance
  bool esp_grenade_prediction = true;     ///< Draw predicted grenade trajectory
  bool esp_projectile_tracer = false;     ///< Show tracers for flying projectiles
  bool esp_threat_warning = true;         ///< Show warnings for nearby threats
  bool esp_skel_interp = true;            ///< Interpolate skeletal bone positions
  float skel_thickness = 1.0f;            ///< Thickness of skeletal lines
  float box_thickness = 1.0f;             ///< Thickness of bounding box lines
  float esp_font_size = 14.0f;            ///< Default font size for ESP text
  bool esp_skeleton_dots = true;          ///< Draw dots at bone joints
  bool esp_spectator_list = true;         ///< Show a list of people spectating
  
  // --- [ LOOT TELEMETRY: MASTER & CATEGORIES ] ---
  bool esp_items = true;                  ///< Show ground items
  bool esp_vehicles = true;               ///< Show vehicles
  bool esp_airdrops = true;               ///< Show airdrop crates
  bool esp_deadboxes = true;              ///< Show player death boxes
  bool loot_weapon_special = true;        ///< Highlight rare/valuable weapons
  bool loot_weapon_all = false;           ///< Show all weapons on ground
  int loot_max_dist = 150;                ///< Loot visibility distance cap
  int vehicle_max_dist = 1000;            ///< Vehicle visibility distance cap

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
  bool loot_ghillie_all = true;
  bool loot_ghillie_arctic = true;
  bool loot_ghillie_desert = true;
  bool loot_ghillie_jungle = true;
  bool loot_ghillie_forest = true;
  bool loot_ghillie_mossy = true;
  bool loot_ghillie_brown = true;

  // --- [ LOOT TELEMETRY: MEDICINE & UTILITY ] ---
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
  bool loot_utility = true; // Legacy group
  bool loot_repair = true;  // Legacy group

  // --- [ LOOT TELEMETRY: AMMO - INDIVIDUAL ] ---
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

  // --- [ LOOT TELEMETRY: KEYS ] ---
  bool loot_key_secret = true;
  bool loot_key_chimera = true;
  bool loot_key_vikendi = true;
  bool loot_key_haven = true;
  bool loot_key_security = true;
  bool loot_key_taego = true;

  // --- [ LOOT TELEMETRY: ATTACHMENTS - SCOPES ] ---
  bool loot_scope_reddot = true;
  bool loot_scope_holo = true;
  bool loot_scope_2x = true;
  bool loot_scope_3x = true;
  bool loot_scope_4x = true;
  bool loot_scope_6x = true;
  bool loot_scope_8x = true;
  bool loot_scope_15x = true;
  bool loot_scope_thermal = true;

  // --- [ LOOT TELEMETRY: ATTACHMENTS - MUZZLES ] ---
  bool loot_muzzle_comp = true;
  bool loot_muzzle_flash = true;
  bool loot_muzzle_supp = true;
  bool loot_muzzle_choke = true;

  // --- [ LOOT TELEMETRY: ATTACHMENTS - GRIPS & STOCKS ] ---
  bool loot_grip_vertical = true;
  bool loot_grip_angled = true;
  bool loot_grip_half = true;
  bool loot_grip_thumb = true;
  bool loot_grip_light = true;
  bool loot_stock_heavy = true;
  bool loot_stock_cheek = true;

  // --- [ LOOT TELEMETRY: ATTACHMENTS - MAGAZINES ] ---
  bool loot_mag_ext = true;
  bool loot_mag_quick = true;
  bool loot_mag_ext_quick = true;

  // --- [ LOOT TELEMETRY: COMPATIBILITY GROUPS ] ---
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

  // --- [ LOOT TELEMETRY: THROWABLES ] ---
  bool loot_throw_frag = true;
  bool loot_throw_smoke = true;
  bool loot_throw_molotov = true;
  bool loot_throw_flash = true;
  bool loot_throw_c4 = true;
  bool loot_throw_sticky = true;
  bool loot_throw_bz = true;
  bool loot_throw_decoy = true;

  // --- [ LOOT TELEMETRY: WEAPONS - SG & HG ] ---
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

  // --- [ LOOT TELEMETRY: WEAPONS - SPECIAL & MELEE ] ---
  bool loot_weapon_pan = true;
  bool loot_weapon_spike = true;
  bool loot_weapon_m79 = true;
  bool loot_weapon_flare = true;
  bool loot_weapon_crossbow = true;
  bool loot_weapon_panzer = true;

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


  /**
   * @brief Initializes the overlay with the provided rendering bridge.
   * @param bridge The bridge host containing handle and D3D resources.
   * @return True if initialization succeeded.
   */
  bool Initialize(const VisualizationBridgeHost& bridge);

  /**
   * @brief Sets up the ImGui style components (colors, rounding, sizing).
   */
  void SetupStyle();

  /**
   * @brief Updates the anti-screenshot mechanism state.
   */
  void UpdateAntiScreenshot();

  /**
   * @brief Main rendering call to be executed every frame.
   */
  void RenderFrame();

  /**
   * @brief Executes precision calibration logic (Aimbot processing).
   */
  void Doprecision_calibration();

  /**
   * @brief Cleans up resources and shuts down the overlay.
   */
  void Shutdown();

  /**
   * @brief Saves the current configuration to a JSON file.
   * @param path The filesystem path to save to.
   */
  void SaveConfig(const char *path);

  /**
   * @brief Loads configuration from a JSON file.
   * @param path The filesystem path to load from.
   */
  void LoadConfig(const char *path);
};

/// @brief Global instance of the OverlayMenu.
extern OverlayMenu g_Menu;
