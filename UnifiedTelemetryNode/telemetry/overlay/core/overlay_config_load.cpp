#include "overlay_menu.hpp"
#include "../../sdk/core/console_log.hpp"
#include "../../sdk/core/app_paths.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include "../../../nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>

namespace {

void ApplyModernVisualDefaults(OverlayMenu& menu) {
    menu.esp_box_type = 1;
    menu.box_thickness = 1.5f;
    menu.skel_thickness = 1.25f;
    menu.esp_fillbox = false;

    menu.box_visible_color[0] = 1.0f; menu.box_visible_color[1] = 0.22f; menu.box_visible_color[2] = 0.24f; menu.box_visible_color[3] = 1.0f;
    menu.box_invisible_color[0] = 1.0f; menu.box_invisible_color[1] = 0.72f; menu.box_invisible_color[2] = 0.26f; menu.box_invisible_color[3] = 0.90f;
    menu.skeleton_visible_color[0] = 0.92f; menu.skeleton_visible_color[1] = 0.96f; menu.skeleton_visible_color[2] = 1.0f; menu.skeleton_visible_color[3] = 0.78f;
    menu.skeleton_invisible_color[0] = 1.0f; menu.skeleton_invisible_color[1] = 0.82f; menu.skeleton_invisible_color[2] = 0.48f; menu.skeleton_invisible_color[3] = 0.58f;
    menu.name_visible_color[0] = 0.92f; menu.name_visible_color[1] = 0.96f; menu.name_visible_color[2] = 1.0f; menu.name_visible_color[3] = 0.96f;
    menu.name_invisible_color[0] = 1.0f; menu.name_invisible_color[1] = 0.82f; menu.name_invisible_color[2] = 0.48f; menu.name_invisible_color[3] = 0.88f;
    menu.distance_color[0] = 0.82f; menu.distance_color[1] = 0.88f; menu.distance_color[2] = 0.94f; menu.distance_color[3] = 0.95f;
    menu.weapon_color[0] = 0.80f; menu.weapon_color[1] = 0.94f; menu.weapon_color[2] = 1.0f; menu.weapon_color[3] = 0.95f;
    menu.health_color[0] = 0.27f; menu.health_color[1] = 0.90f; menu.health_color[2] = 0.52f; menu.health_color[3] = 1.0f;
    menu.damage_color[0] = 1.0f; menu.damage_color[1] = 0.48f; menu.damage_color[2] = 0.18f; menu.damage_color[3] = 1.0f;
    menu.speed_color[0] = 0.45f; menu.speed_color[1] = 1.0f; menu.speed_color[2] = 0.72f; menu.speed_color[3] = 1.0f;
    menu.ammo_color[0] = 0.95f; menu.ammo_color[1] = 0.95f; menu.ammo_color[2] = 0.72f; menu.ammo_color[3] = 1.0f;
    menu.close_warning_color[0] = 1.0f; menu.close_warning_color[1] = 0.20f; menu.close_warning_color[2] = 0.08f; menu.close_warning_color[3] = 1.0f;
    menu.aim_warning_color[0] = 1.0f; menu.aim_warning_color[1] = 0.12f; menu.aim_warning_color[2] = 0.08f; menu.aim_warning_color[3] = 1.0f;
    menu.view_direction_color[0] = 0.20f; menu.view_direction_color[1] = 0.82f; menu.view_direction_color[2] = 1.0f; menu.view_direction_color[3] = 0.85f;
    menu.esp_aim_warning = true;
    menu.esp_view_direction = true;
    menu.esp_status_badges = true;
    menu.esp_health_text = true;
    menu.esp_health_display_mode = 0;
    menu.esp_health_bar_style = 0;
    menu.esp_close_warning = true;
    menu.esp_close_warning_distance = 65.0f;
    menu.esp_offscreen_text = true;
    menu.esp_view_direction_length = 35.0f;
    menu.player_list_enabled = true;
    menu.player_list_hold_required = true;
    menu.player_list_hold_key = VK_TAB;
}

} // namespace

void OverlayMenu::LoadConfig(const char* path) {
    try {
        const std::string resolvedPath = AppPaths::RuntimePath(path ? path : "");
        std::ifstream file(resolvedPath);
        if (!file.is_open() && path) {
            file.clear();
            file.open(path);
        }
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            file.close();

            if (j.contains("esp_toggle")) esp_toggle = j["esp_toggle"];
            if (j.contains("esp_icons")) esp_icons = j["esp_icons"];
            if (j.contains("esp_font_size")) esp_font_size = j["esp_font_size"];
            if (j.contains("name_font_size")) name_font_size = j["name_font_size"];
            if (j.contains("distance_font_size")) distance_font_size = j["distance_font_size"];
            if (j.contains("weapon_font_size")) weapon_font_size = j["weapon_font_size"];
            if (j.contains("rank_font_size")) rank_font_size = j["rank_font_size"];
            if (j.contains("teamid_badge_size")) teamid_badge_size = j["teamid_badge_size"];
            if (j.contains("kill_font_size")) kill_font_size = j["kill_font_size"];
            if (j.contains("survival_level_font_size")) survival_level_font_size = j["survival_level_font_size"];
            if (j.contains("damage_font_size")) damage_font_size = j["damage_font_size"];
            if (j.contains("speed_font_size")) speed_font_size = j["speed_font_size"];
            if (j.contains("ammo_font_size")) ammo_font_size = j["ammo_font_size"];
            if (j.contains("spectated_font_size")) spectated_font_size = j["spectated_font_size"];
            if (j.contains("weapon_icon_size")) weapon_icon_size = j["weapon_icon_size"];
            if (j.contains("esp_text_background")) esp_text_background = j["esp_text_background"];
            if (j.contains("esp_text_bg_alpha")) esp_text_bg_alpha = j["esp_text_bg_alpha"];
            if (j.contains("esp_health_text")) esp_health_text = j["esp_health_text"];
            if (j.contains("esp_health_display_mode")) esp_health_display_mode = j["esp_health_display_mode"];
            if (j.contains("esp_health_bar_style")) esp_health_bar_style = j["esp_health_bar_style"];
            if (j.contains("esp_aim_warning")) esp_aim_warning = j["esp_aim_warning"];
            if (j.contains("esp_view_direction")) esp_view_direction = j["esp_view_direction"];
            if (j.contains("esp_status_badges")) esp_status_badges = j["esp_status_badges"];
            if (j.contains("esp_close_warning")) esp_close_warning = j["esp_close_warning"];
            if (j.contains("esp_close_warning_distance")) esp_close_warning_distance = j["esp_close_warning_distance"];
            if (j.contains("esp_view_direction_length")) esp_view_direction_length = j["esp_view_direction_length"];
            if (j.contains("box_thickness")) box_thickness = j["box_thickness"];
            if (j.contains("esp_skeleton_dots")) esp_skeleton_dots = j["esp_skeleton_dots"];
            if (j.contains("esp_spectator_list")) esp_spectator_list = j["esp_spectator_list"];
            if (j.contains("player_list_enabled")) player_list_enabled = j["player_list_enabled"];
            if (j.contains("player_list_hold_required")) player_list_hold_required = j["player_list_hold_required"];
            if (j.contains("player_list_hold_key")) player_list_hold_key = j["player_list_hold_key"];
            if (j.contains("debug_loot_resolver")) debug_loot_resolver = j["debug_loot_resolver"];
            if (j.contains("active_preset")) active_preset = j["active_preset"];
            if (j.contains("flick_enabled")) flick_enabled = j["flick_enabled"];
            if (j.contains("flick_visible_only")) flick_visible_only = j["flick_visible_only"];
            flick_auto_shot = true;
            if (j.contains("flick_shot_hold")) flick_shot_hold = j["flick_shot_hold"];
            if (j.contains("flick_return")) flick_return = j["flick_return"];
            if (j.contains("flick_fov")) flick_fov = j["flick_fov"];
            if (j.contains("flick_max_dist")) flick_max_dist = j["flick_max_dist"];
            if (j.contains("flick_target_part")) flick_target_part = j["flick_target_part"];
            if (j.contains("flick_key")) flick_key = j["flick_key"];
            if (j.contains("flick_key2")) flick_key2 = j["flick_key2"];
            if (j.contains("flick_weapon_s686")) flick_weapon_s686 = j["flick_weapon_s686"];
            if (j.contains("flick_weapon_s12k")) flick_weapon_s12k = j["flick_weapon_s12k"];
            if (j.contains("flick_weapon_s1897")) flick_weapon_s1897 = j["flick_weapon_s1897"];
            if (j.contains("flick_weapon_dbs")) flick_weapon_dbs = j["flick_weapon_dbs"];
            if (j.contains("flick_weapon_o12")) flick_weapon_o12 = j["flick_weapon_o12"];
            if (j.contains("flick_weapon_slr")) flick_weapon_slr = j["flick_weapon_slr"];
            if (j.contains("flick_weapon_mini14")) flick_weapon_mini14 = j["flick_weapon_mini14"];
            if (j.contains("flick_weapon_sks")) flick_weapon_sks = j["flick_weapon_sks"];
            if (j.contains("flick_weapon_vss")) flick_weapon_vss = j["flick_weapon_vss"];
            if (j.contains("flick_weapon_qbu")) flick_weapon_qbu = j["flick_weapon_qbu"];
            if (j.contains("flick_weapon_kar98k")) flick_weapon_kar98k = j["flick_weapon_kar98k"];
            if (j.contains("flick_weapon_m24")) flick_weapon_m24 = j["flick_weapon_m24"];
            if (j.contains("flick_weapon_awm")) flick_weapon_awm = j["flick_weapon_awm"];
            if (j.contains("flick_weapon_lynx")) flick_weapon_lynx = j["flick_weapon_lynx"];
            if (j.contains("flick_weapon_win94")) flick_weapon_win94 = j["flick_weapon_win94"];
            if (j.contains("flick_weapon_mosin")) flick_weapon_mosin = j["flick_weapon_mosin"];
            if (j.contains("flick_weapon_panzerfaust")) flick_weapon_panzerfaust = j["flick_weapon_panzerfaust"];
            if (j.contains("flick_weapon_mk12")) flick_weapon_mk12 = j["flick_weapon_mk12"];
            if (j.contains("flick_weapon_mk14")) flick_weapon_mk14 = j["flick_weapon_mk14"];
            if (j.contains("flick_weapon_dragunov")) flick_weapon_dragunov = j["flick_weapon_dragunov"];
            if (j.contains("esp_show_enemies")) esp_show_enemies = j["esp_show_enemies"];
            if (j.contains("esp_show_teammates")) {
                esp_show_teammates = j["esp_show_teammates"];
            } else if (j.contains("esp_team_check")) {
                // Legacy key: team_check=true means hide teammates.
                esp_show_teammates = !static_cast<bool>(j["esp_team_check"]);
            }
            if (j.contains("esp_box")) esp_box = j["esp_box"];
            if (j.contains("esp_box_type")) esp_box_type = j["esp_box_type"];
            if (j.contains("esp_fillbox")) esp_fillbox = j["esp_fillbox"];
            if (j.contains("esp_skeleton")) esp_skeleton = j["esp_skeleton"];
            if (j.contains("esp_name")) esp_name = j["esp_name"];
            if (j.contains("esp_distance")) esp_distance = j["esp_distance"];
            if (j.contains("esp_health")) esp_health = j["esp_health"];
            if (j.contains("esp_health_pos")) esp_health_pos = j["esp_health_pos"];
            if (j.contains("esp_distance_pos")) esp_distance_pos = j["esp_distance_pos"];
            if (j.contains("esp_name_pos")) esp_name_pos = j["esp_name_pos"];
            if (j.contains("esp_rank_pos")) esp_rank_pos = j["esp_rank_pos"];
            if (j.contains("esp_weapon_pos")) esp_weapon_pos = j["esp_weapon_pos"];
            if (j.contains("esp_spectated_pos")) esp_spectated_pos = j["esp_spectated_pos"];
            if (j.contains("esp_teamid_pos")) esp_teamid_pos = j["esp_teamid_pos"];
            if (j.contains("esp_killcount_pos")) esp_killcount_pos = j["esp_killcount_pos"];
            if (j.contains("esp_survival_level_pos")) esp_survival_level_pos = j["esp_survival_level_pos"];
            if (j.contains("esp_damage_pos")) esp_damage_pos = j["esp_damage_pos"];
            if (j.contains("esp_speed_pos")) esp_speed_pos = j["esp_speed_pos"];
            if (j.contains("esp_ammo_pos")) esp_ammo_pos = j["esp_ammo_pos"];
            if (j.contains("esp_items")) esp_items = j["esp_items"];
            if (j.contains("esp_items_toggle_key")) esp_items_toggle_key = j["esp_items_toggle_key"];
            if (j.contains("esp_vehicles_toggle_key")) esp_vehicles_toggle_key = j["esp_vehicles_toggle_key"];
            if (j.contains("esp_snapline")) esp_snapline = j["esp_snapline"];
            if (j.contains("snapline_type")) snapline_type = j["snapline_type"];
            if (j.contains("esp_weapon")) esp_weapon = j["esp_weapon"];
            if (j.contains("esp_damage")) esp_damage = j["esp_damage"];
            if (j.contains("esp_speed")) esp_speed = j["esp_speed"];
            if (j.contains("esp_ammo")) esp_ammo = j["esp_ammo"];
            if (j.contains("esp_weapon_type")) esp_weapon_type = j["esp_weapon_type"];
            if (j.contains("render_distance")) render_distance = j["render_distance"];
            if (j.contains("esp_offscreen")) esp_offscreen = j["esp_offscreen"];
            if (j.contains("esp_offscreen_text")) esp_offscreen_text = j["esp_offscreen_text"];
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
            if (j.contains("radar_enabled")) radar_enabled = j["radar_enabled"];
            if (j.contains("radar_offset_x")) radar_offset_x = j["radar_offset_x"];
            if (j.contains("radar_offset_y")) radar_offset_y = j["radar_offset_y"];
            if (j.contains("radar_zoom_multiplier")) radar_zoom_multiplier = j["radar_zoom_multiplier"];
            if (j.contains("radar_scale")) radar_scale = j["radar_scale"];
            if (j.contains("radar_dot_size")) radar_dot_size = j["radar_dot_size"];
            if (j.contains("radar_rotation_offset")) radar_rotation_offset = j["radar_rotation_offset"];
            if (j.contains("minimap_enabled")) minimap_enabled = j["minimap_enabled"];
            if (j.contains("bigmap_enabled")) bigmap_enabled = j["bigmap_enabled"];
            if (j.contains("minimap_show_direction")) minimap_show_direction = j["minimap_show_direction"];
            if (j.contains("minimap_fire_trace")) minimap_fire_trace = j["minimap_fire_trace"];
            if (j.contains("minimap_view_ray_length")) minimap_view_ray_length = j["minimap_view_ray_length"];
            if (j.contains("minimap_fire_ray_length")) minimap_fire_ray_length = j["minimap_fire_ray_length"];
            if (j.contains("minimap_fire_flash_ms")) minimap_fire_flash_ms = j["minimap_fire_flash_ms"];
            if (j.contains("minimap_ray_width")) minimap_ray_width = j["minimap_ray_width"];
            if (j.contains("bigmap_show_names")) bigmap_show_names = j["bigmap_show_names"];
            if (j.contains("bigmap_show_direction")) bigmap_show_direction = j["bigmap_show_direction"];
            if (j.contains("bigmap_name_background")) bigmap_name_background = j["bigmap_name_background"];
            if (j.contains("bigmap_show_legend")) bigmap_show_legend = j["bigmap_show_legend"];
            if (j.contains("bigmap_show_vehicles")) bigmap_show_vehicles = j["bigmap_show_vehicles"];
            if (j.contains("bigmap_show_airdrops")) bigmap_show_airdrops = j["bigmap_show_airdrops"];
            if (j.contains("bigmap_show_deadboxes")) bigmap_show_deadboxes = j["bigmap_show_deadboxes"];
            if (j.contains("bigmap_marker_size")) bigmap_marker_size = j["bigmap_marker_size"];
            if (j.contains("bigmap_marker_alpha")) bigmap_marker_alpha = j["bigmap_marker_alpha"];
            if (j.contains("bigmap_icon_size")) bigmap_icon_size = j["bigmap_icon_size"];
            if (j.contains("bigmap_name_font_size")) bigmap_name_font_size = j["bigmap_name_font_size"];
            if (j.contains("bigmap_name_bg_alpha")) bigmap_name_bg_alpha = j["bigmap_name_bg_alpha"];
            if (j.contains("bigmap_offset_x")) bigmap_offset_x = j["bigmap_offset_x"];
            if (j.contains("bigmap_offset_y")) bigmap_offset_y = j["bigmap_offset_y"];
            if (j.contains("bigmap_screen_scale")) bigmap_screen_scale = j["bigmap_screen_scale"];
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
            if (j.contains("color_names")) {
                for (int i = 0; i < 4; i++) {
                    name_color[i] = j["color_names"][i];
                    name_visible_color[i] = name_color[i];
                    name_invisible_color[i] = name_color[i];
                }
            }
            if (j.contains("color_name_vis")) for (int i = 0; i < 4; i++) name_visible_color[i] = j["color_name_vis"][i];
            if (j.contains("color_name_inv")) for (int i = 0; i < 4; i++) name_invisible_color[i] = j["color_name_inv"][i];
            if (j.contains("color_dist")) for (int i = 0; i < 4; i++) distance_color[i] = j["color_dist"][i];
            if (j.contains("color_weapon")) for (int i = 0; i < 4; i++) weapon_color[i] = j["color_weapon"][i];
            if (j.contains("color_rank")) for (int i = 0; i < 4; i++) rank_color[i] = j["color_rank"][i];
            if (j.contains("color_teamid")) for (int i = 0; i < 4; i++) teamid_color[i] = j["color_teamid"][i];
            if (j.contains("color_kill")) for (int i = 0; i < 4; i++) kill_color[i] = j["color_kill"][i];
            if (j.contains("color_survival_level")) for (int i = 0; i < 4; i++) survival_level_color[i] = j["color_survival_level"][i];
            if (j.contains("color_damage")) for (int i = 0; i < 4; i++) damage_color[i] = j["color_damage"][i];
            if (j.contains("color_speed")) for (int i = 0; i < 4; i++) speed_color[i] = j["color_speed"][i];
            if (j.contains("color_ammo")) for (int i = 0; i < 4; i++) ammo_color[i] = j["color_ammo"][i];
            if (j.contains("color_spectated")) for (int i = 0; i < 4; i++) spectated_color[i] = j["color_spectated"][i];
            if (j.contains("color_aim_warning")) for (int i = 0; i < 4; i++) aim_warning_color[i] = j["color_aim_warning"][i];
            if (j.contains("color_close_warning")) for (int i = 0; i < 4; i++) close_warning_color[i] = j["color_close_warning"][i];
            if (j.contains("color_view_direction")) for (int i = 0; i < 4; i++) view_direction_color[i] = j["color_view_direction"][i];
            if (j.contains("team_color_custom")) team_color_custom = j["team_color_custom"];
            if (j.contains("team_custom_colors") && j["team_custom_colors"].is_array()) {
                for (int row = 0; row < 4 && row < (int)j["team_custom_colors"].size(); ++row) {
                    if (!j["team_custom_colors"][row].is_array()) continue;
                    for (int col = 0; col < 4 && col < (int)j["team_custom_colors"][row].size(); ++col) {
                        team_custom_colors[row][col] = j["team_custom_colors"][row][col];
                    }
                }
            }
            if (j.contains("esp_multilayer_nameplate")) esp_multilayer_nameplate = j["esp_multilayer_nameplate"];

            if (j.contains("esp_items")) esp_items = j["esp_items"];
            if (j.contains("esp_vehicles")) esp_vehicles = j["esp_vehicles"];
            if (j.contains("esp_airdrops")) esp_airdrops = j["esp_airdrops"];
            if (j.contains("esp_deadboxes")) esp_deadboxes = j["esp_deadboxes"];
            if (j.contains("loot_max_dist")) loot_max_dist = j["loot_max_dist"];
            if (j.contains("vehicle_max_dist")) vehicle_max_dist = j["vehicle_max_dist"];
            if (j.contains("item_icon_size")) item_icon_size = j["item_icon_size"];
            if (j.contains("item_group_icon_size")) item_group_icon_size = j["item_group_icon_size"];
            if (j.contains("vehicle_icon_size")) vehicle_icon_size = j["vehicle_icon_size"];
            if (j.contains("loot_distance_font_size")) loot_distance_font_size = j["loot_distance_font_size"];
            if (j.contains("asset_animation_enabled")) asset_animation_enabled = j["asset_animation_enabled"];
            if (j.contains("asset_animation_glow")) asset_animation_glow = j["asset_animation_glow"];
            if (j.contains("asset_animation_shine")) asset_animation_shine = j["asset_animation_shine"];
            if (j.contains("asset_animation_strength")) asset_animation_strength = j["asset_animation_strength"];
            if (j.contains("asset_animation_speed")) asset_animation_speed = j["asset_animation_speed"];

            if (j.contains("share_radar")) share_radar = j["share_radar"];
            if (j.contains("share_radar_ip")) strcpy_s(share_radar_ip, j["share_radar_ip"].get<std::string>().c_str());
            if (j.contains("esp_grenade_prediction")) esp_grenade_prediction = j["esp_grenade_prediction"];
            if (j.contains("esp_projectile_tracer")) esp_projectile_tracer = j["esp_projectile_tracer"];
            if (j.contains("esp_threat_warning")) esp_threat_warning = j["esp_threat_warning"];

            if (j.contains("loot_armor_lv1")) loot_armor_lv1 = j["loot_armor_lv1"];
            if (j.contains("loot_armor_lv2")) loot_armor_lv2 = j["loot_armor_lv2"];
            if (j.contains("loot_armor_lv3")) loot_armor_lv3 = j["loot_armor_lv3"];
            if (j.contains("loot_helmet_lv1")) loot_helmet_lv1 = j["loot_helmet_lv1"];
            if (j.contains("loot_helmet_lv2")) loot_helmet_lv2 = j["loot_helmet_lv2"];
            if (j.contains("loot_helmet_lv3")) loot_helmet_lv3 = j["loot_helmet_lv3"];
            if (j.contains("loot_meds_boosts")) loot_meds_boosts = j["loot_meds_boosts"];
            if (j.contains("loot_meds_healing")) loot_meds_healing = j["loot_meds_healing"];
            if (j.contains("loot_ammo_all")) loot_ammo_all = j["loot_ammo_all"];
            if (j.contains("loot_ammo_high")) loot_ammo_high = j["loot_ammo_high"];
            if (j.contains("loot_scopes_all")) loot_scopes_all = j["loot_scopes_all"];
            if (j.contains("loot_scopes_high")) loot_scopes_high = j["loot_scopes_high"];
            if (j.contains("loot_attach_mag")) loot_attach_mag = j["loot_attach_mag"];
            if (j.contains("loot_attach_muzzle")) loot_attach_muzzle = j["loot_attach_muzzle"];
            if (j.contains("loot_weapon_special")) loot_weapon_special = j["loot_weapon_special"];
            if (j.contains("loot_weapon_all")) loot_weapon_all = j["loot_weapon_all"];
            if (j.contains("loot_backpack_lv1")) loot_backpack_lv1 = j["loot_backpack_lv1"];
            if (j.contains("loot_backpack_lv2")) loot_backpack_lv2 = j["loot_backpack_lv2"];
            if (j.contains("loot_backpack_lv3")) loot_backpack_lv3 = j["loot_backpack_lv3"];
            if (j.contains("loot_ghillie")) loot_ghillie = j["loot_ghillie"];
            if (j.contains("loot_utility")) loot_utility = j["loot_utility"];
            if (j.contains("loot_repair")) loot_repair = j["loot_repair"];
            if (j.contains("loot_ammo_556")) loot_ammo_556 = j["loot_ammo_556"];
            if (j.contains("loot_ammo_762")) loot_ammo_762 = j["loot_ammo_762"];
            if (j.contains("loot_ammo_9mm")) loot_ammo_9mm = j["loot_ammo_9mm"];
            if (j.contains("loot_ammo_45")) loot_ammo_45 = j["loot_ammo_45"];
            if (j.contains("loot_ammo_12g")) loot_ammo_12g = j["loot_ammo_12g"];
            if (j.contains("loot_ammo_300")) loot_ammo_300 = j["loot_ammo_300"];
            if (j.contains("loot_ammo_slug")) loot_ammo_slug = j["loot_ammo_slug"];
            if (j.contains("loot_ammo_57")) loot_ammo_57 = j["loot_ammo_57"];
            if (j.contains("loot_ammo_40")) loot_ammo_40 = j["loot_ammo_40"];
            if (j.contains("loot_ammo_bolt")) loot_ammo_bolt = j["loot_ammo_bolt"];
            if (j.contains("loot_ammo_flare")) loot_ammo_flare = j["loot_ammo_flare"];
            if (j.contains("loot_ammo_mortar")) loot_ammo_mortar = j["loot_ammo_mortar"];

            if (j.contains("loot_key_security")) loot_key_security = j["loot_key_security"];
            if (j.contains("loot_key_secret")) loot_key_secret = j["loot_key_secret"];
            if (j.contains("loot_key_taego")) loot_key_taego = j["loot_key_taego"];
            if (j.contains("loot_key_vikendi")) loot_key_vikendi = j["loot_key_vikendi"];
            if (j.contains("loot_key_chimera")) loot_key_chimera = j["loot_key_chimera"];
            if (j.contains("loot_key_haven")) loot_key_haven = j["loot_key_haven"];

            if (j.contains("loot_scope_reddot")) loot_scope_reddot = j["loot_scope_reddot"];
            if (j.contains("loot_scope_holo")) loot_scope_holo = j["loot_scope_holo"];
            if (j.contains("loot_scope_2x")) loot_scope_2x = j["loot_scope_2x"];
            if (j.contains("loot_scope_3x")) loot_scope_3x = j["loot_scope_3x"];
            if (j.contains("loot_scope_4x")) loot_scope_4x = j["loot_scope_4x"];
            if (j.contains("loot_scope_6x")) loot_scope_6x = j["loot_scope_6x"];
            if (j.contains("loot_scope_8x")) loot_scope_8x = j["loot_scope_8x"];
            if (j.contains("loot_scope_15x")) loot_scope_15x = j["loot_scope_15x"];
            if (j.contains("loot_scope_thermal")) loot_scope_thermal = j["loot_scope_thermal"];
            if (j.contains("loot_muzzle_comp")) loot_muzzle_comp = j["loot_muzzle_comp"];
            if (j.contains("loot_muzzle_flash")) loot_muzzle_flash = j["loot_muzzle_flash"];
            if (j.contains("loot_muzzle_supp")) loot_muzzle_supp = j["loot_muzzle_supp"];
            if (j.contains("loot_muzzle_choke")) loot_muzzle_choke = j["loot_muzzle_choke"];
            if (j.contains("loot_grip_vertical")) loot_grip_vertical = j["loot_grip_vertical"];
            if (j.contains("loot_grip_angled")) loot_grip_angled = j["loot_grip_angled"];
            if (j.contains("loot_grip_half")) loot_grip_half = j["loot_grip_half"];
            if (j.contains("loot_grip_thumb")) loot_grip_thumb = j["loot_grip_thumb"];
            if (j.contains("loot_grip_light")) loot_grip_light = j["loot_grip_light"];
            if (j.contains("loot_stock_heavy")) loot_stock_heavy = j["loot_stock_heavy"];
            if (j.contains("loot_stock_cheek")) loot_stock_cheek = j["loot_stock_cheek"];
            if (j.contains("loot_mag_ext")) loot_mag_ext = j["loot_mag_ext"];
            if (j.contains("loot_mag_quick")) loot_mag_quick = j["loot_mag_quick"];
            if (j.contains("loot_mag_ext_quick")) loot_mag_ext_quick = j["loot_mag_ext_quick"];

            if (j.contains("loot_attach_grip")) loot_attach_grip = j["loot_attach_grip"];
            if (j.contains("loot_attach_stock")) loot_attach_stock = j["loot_attach_stock"];
            if (j.contains("loot_attach_scope_all")) loot_attach_scope_all = j["loot_attach_scope_all"];
            if (j.contains("loot_attach_scope_high")) loot_attach_scope_high = j["loot_attach_scope_high"];

            // Weapon-specific booleans
            if (j.contains("lw_ace32")) loot_weapon_ace32 = j["lw_ace32"];
            if (j.contains("lw_ak47")) loot_weapon_ak47 = j["lw_ak47"];
            if (j.contains("lw_aug")) loot_weapon_aug = j["lw_aug"];
            if (j.contains("lw_beryl")) loot_weapon_beryl = j["lw_beryl"];
            if (j.contains("lw_g36c")) loot_weapon_g36c = j["lw_g36c"];
            if (j.contains("lw_groza")) loot_weapon_groza = j["lw_groza"];
            if (j.contains("lw_hk416")) loot_weapon_hk416 = j["lw_hk416"];
            if (j.contains("lw_k2")) loot_weapon_k2 = j["lw_k2"];
            if (j.contains("lw_m16")) loot_weapon_m16 = j["lw_m16"];
            if (j.contains("lw_mutant")) loot_weapon_mutant = j["lw_mutant"];
            if (j.contains("lw_qbz")) loot_weapon_qbz = j["lw_qbz"];
            if (j.contains("lw_scar")) loot_weapon_scar = j["lw_scar"];
            if (j.contains("lw_famas")) loot_weapon_famas = j["lw_famas"];
            if (j.contains("lw_awm")) loot_weapon_awm = j["lw_awm"];
            if (j.contains("lw_kar98")) loot_weapon_kar98 = j["lw_kar98"];
            if (j.contains("lw_m24")) loot_weapon_m24 = j["lw_m24"];
            if (j.contains("lw_mosin")) loot_weapon_mosin = j["lw_mosin"];
            if (j.contains("lw_win94")) loot_weapon_win94 = j["lw_win94"];
            if (j.contains("lw_dragunov")) loot_weapon_dragunov = j["lw_dragunov"];
            if (j.contains("lw_mini14")) loot_weapon_mini14 = j["lw_mini14"];
            if (j.contains("lw_mk12")) loot_weapon_mk12 = j["lw_mk12"];
            if (j.contains("lw_mk14")) loot_weapon_mk14 = j["lw_mk14"];
            if (j.contains("lw_qbu")) loot_weapon_qbu = j["lw_qbu"];
            if (j.contains("lw_sks")) loot_weapon_sks = j["lw_sks"];
            if (j.contains("lw_vss")) loot_weapon_vss = j["lw_vss"];
            if (j.contains("lw_slr")) loot_weapon_slr = j["lw_slr"];
            if (j.contains("lw_bizon")) loot_weapon_bizon = j["lw_bizon"];
            if (j.contains("lw_mp5")) loot_weapon_mp5 = j["lw_mp5"];
            if (j.contains("lw_mp9")) loot_weapon_mp9 = j["lw_mp9"];
            if (j.contains("lw_p90")) loot_weapon_p90 = j["lw_p90"];
            if (j.contains("lw_thompson")) loot_weapon_thompson = j["lw_thompson"];
            if (j.contains("lw_ump")) loot_weapon_ump = j["lw_ump"];
            if (j.contains("lw_uzi")) loot_weapon_uzi = j["lw_uzi"];
            if (j.contains("lw_vector")) loot_weapon_vector = j["lw_vector"];
            if (j.contains("lw_js9")) loot_weapon_js9 = j["lw_js9"];
            if (j.contains("lw_dp28")) loot_weapon_dp28 = j["lw_dp28"];
            if (j.contains("lw_m249")) loot_weapon_m249 = j["lw_m249"];
            if (j.contains("lw_mg3")) loot_weapon_mg3 = j["lw_mg3"];
            if (j.contains("lw_dp12")) loot_weapon_dp12 = j["lw_dp12"];
            if (j.contains("lw_saiga")) loot_weapon_saiga = j["lw_saiga"];
            if (j.contains("lw_deagle")) loot_weapon_deagle = j["lw_deagle"];
            if (j.contains("lw_m1911")) loot_weapon_m1911 = j["lw_m1911"];
            if (j.contains("lw_p92")) loot_weapon_p92 = j["lw_p92"];
            if (j.contains("lw_skorpion")) loot_weapon_skorpion = j["lw_skorpion"];
            if (j.contains("lw_nagant")) loot_weapon_nagant = j["lw_nagant"];
            if (j.contains("lw_rhino")) loot_weapon_rhino = j["lw_rhino"];
            if (j.contains("lw_stungun")) loot_weapon_stungun = j["lw_stungun"];
            if (j.contains("lw_pan")) loot_weapon_pan = j["lw_pan"];
            if (j.contains("lw_flare")) loot_weapon_flare = j["lw_flare"];
            if (j.contains("lw_crossbow")) loot_weapon_crossbow = j["lw_crossbow"];
            if (j.contains("lw_panzer")) loot_weapon_panzer = j["lw_panzer"];
            if (j.contains("lw_spike")) loot_weapon_spike = j["lw_spike"];
            if (j.contains("lw_m79")) loot_weapon_m79 = j["lw_m79"];

            if (j.contains("loot_throw_frag")) loot_throw_frag = j["loot_throw_frag"];
            if (j.contains("loot_throw_smoke")) loot_throw_smoke = j["loot_throw_smoke"];
            if (j.contains("loot_throw_flash")) loot_throw_flash = j["loot_throw_flash"];
            if (j.contains("loot_throw_molotov")) loot_throw_molotov = j["loot_throw_molotov"];
            if (j.contains("loot_throw_c4")) loot_throw_c4 = j["loot_throw_c4"];
            if (j.contains("loot_throw_sticky")) loot_throw_sticky = j["loot_throw_sticky"];
            if (j.contains("loot_throw_bz")) loot_throw_bz = j["loot_throw_bz"];
            if (j.contains("loot_throw_decoy")) loot_throw_decoy = j["loot_throw_decoy"];

            if (j.contains("loot_ghillie_arctic")) loot_ghillie_arctic = j["loot_ghillie_arctic"];
            if (j.contains("loot_ghillie_desert")) loot_ghillie_desert = j["loot_ghillie_desert"];
            if (j.contains("loot_ghillie_jungle")) loot_ghillie_jungle = j["loot_ghillie_jungle"];
            if (j.contains("loot_ghillie_forest")) loot_ghillie_forest = j["loot_ghillie_forest"];
            if (j.contains("loot_ghillie_mossy")) loot_ghillie_mossy = j["loot_ghillie_mossy"];
            if (j.contains("loot_ghillie_brown")) loot_ghillie_brown = j["loot_ghillie_brown"];
            if (j.contains("loot_utility_drone")) loot_utility_drone = j["loot_utility_drone"];
            if (j.contains("loot_utility_scope")) loot_utility_scope = j["loot_utility_scope"];
            if (j.contains("loot_repair_armor")) loot_repair_armor = j["loot_repair_armor"];
            if (j.contains("loot_repair_helmet")) loot_repair_helmet = j["loot_repair_helmet"];
            if (j.contains("loot_repair_vehicle")) loot_repair_vehicle = j["loot_repair_vehicle"];
            if (j.contains("loot_utility_jammer")) loot_utility_jammer = j["loot_utility_jammer"];
            if (j.contains("loot_utility_bluechip")) loot_utility_bluechip = j["loot_utility_bluechip"];
            if (j.contains("loot_utility_vtransmitter")) loot_utility_vtransmitter = j["loot_utility_vtransmitter"];
            if (j.contains("loot_utility_shield")) loot_utility_shield = j["loot_utility_shield"];

            if (j.contains("loot_vehicle_uaz")) loot_vehicle_uaz = j["loot_vehicle_uaz"];
            if (j.contains("loot_vehicle_dacia")) loot_vehicle_dacia = j["loot_vehicle_dacia"];
            if (j.contains("loot_vehicle_buggy")) loot_vehicle_buggy = j["loot_vehicle_buggy"];
            if (j.contains("loot_vehicle_bike")) loot_vehicle_bike = j["loot_vehicle_bike"];
            if (j.contains("loot_vehicle_boat")) loot_vehicle_boat = j["loot_vehicle_boat"];
            if (j.contains("loot_vehicle_air")) loot_vehicle_air = j["loot_vehicle_air"];
            if (j.contains("loot_vehicle_brdm")) loot_vehicle_brdm = j["loot_vehicle_brdm"];
            if (j.contains("loot_vehicle_scooter")) loot_vehicle_scooter = j["loot_vehicle_scooter"];
            if (j.contains("loot_vehicle_tuk")) loot_vehicle_tuk = j["loot_vehicle_tuk"];
            if (j.contains("loot_vehicle_snow")) loot_vehicle_snow = j["loot_vehicle_snow"];
            if (j.contains("loot_vehicle_bus")) loot_vehicle_bus = j["loot_vehicle_bus"];
            if (j.contains("loot_vehicle_truck")) loot_vehicle_truck = j["loot_vehicle_truck"];
            if (j.contains("loot_vehicle_train")) loot_vehicle_train = j["loot_vehicle_train"];
            if (j.contains("loot_vehicle_mirado")) loot_vehicle_mirado = j["loot_vehicle_mirado"];
            if (j.contains("loot_vehicle_pickup")) loot_vehicle_pickup = j["loot_vehicle_pickup"];
            if (j.contains("loot_vehicle_rony")) loot_vehicle_rony = j["loot_vehicle_rony"];
            if (j.contains("loot_vehicle_blanc")) loot_vehicle_blanc = j["loot_vehicle_blanc"];

            if (j.contains("esp_distance_lod")) esp_distance_lod = j["esp_distance_lod"];
            if (j.contains("skeleton_max_dist")) skeleton_max_dist = j["skeleton_max_dist"];
            if (j.contains("name_max_dist")) name_max_dist = j["name_max_dist"];
            if (j.contains("weapon_max_dist")) weapon_max_dist = j["weapon_max_dist"];
            if (j.contains("box_max_dist")) box_max_dist = j["box_max_dist"];
            if (j.contains("hp_max_dist")) hp_max_dist = j["hp_max_dist"];
            if (j.contains("distance_txt_max_dist")) distance_txt_max_dist = j["distance_txt_max_dist"];

            if (!j.contains("visual_style_version")) {
                ApplyModernVisualDefaults(*this);
            }

            UTN_DEV_LOG(std::cout << "[DEV] Loaded Config from: " << path << std::endl);
        }
    } catch (...) {}
}
