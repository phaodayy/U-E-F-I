#include "overlay_presets.hpp"
#include "flick_weapon_catalog.hpp"
#include "overlay_menu.hpp"

namespace {

void ApplySharedDefaults(OverlayMenu& menu) {
    menu.esp_toggle = true;
    menu.esp_icons = true;
    menu.esp_show_enemies = true;
    menu.esp_show_teammates = false;
    menu.esp_box_type = 1;
    menu.box_thickness = 1.5f;
    menu.esp_skeleton_dots = false;
    menu.name_font_size = 13.5f;
    menu.distance_font_size = 11.5f;
    menu.weapon_font_size = 12.0f;
    menu.damage_font_size = 12.0f;
    menu.speed_font_size = 12.0f;
    menu.ammo_font_size = 12.0f;
    menu.skel_thickness = 1.25f;
    menu.item_icon_size = 24.0f;
    menu.item_group_icon_size = 18.0f;
    menu.vehicle_icon_size = 34.0f;
    menu.loot_distance_font_size = 12.0f;
    menu.asset_animation_enabled = true;
    menu.asset_animation_glow = false;
    menu.asset_animation_shine = false;
    menu.asset_animation_strength = 1.0f;
    menu.asset_animation_speed = 1.0f;
    menu.esp_text_background = true;
    menu.esp_text_bg_alpha = 0.32f;
    menu.esp_health_text = true;
    menu.esp_health_display_mode = 0;
    menu.esp_health_bar_style = 0;
    menu.esp_aim_warning = true;
    menu.esp_view_direction = true;
    menu.esp_status_badges = true;
    menu.esp_close_warning = true;
    menu.esp_close_warning_distance = 65.0f;
    menu.esp_offscreen_text = true;
    menu.esp_view_direction_length = 35.0f;
    menu.player_list_enabled = true;
    menu.player_list_hold_required = true;
    menu.player_list_hold_key = VK_TAB;
    menu.esp_damage = false;
    menu.esp_speed = false;
    menu.esp_ammo = true;
    menu.minimap_enabled = true;
    menu.bigmap_enabled = true;
    menu.bigmap_show_names = true;
    menu.bigmap_show_direction = true;
    menu.minimap_show_direction = true;
    menu.minimap_fire_trace = true;
    menu.minimap_view_ray_length = 55.0f;
    menu.minimap_fire_ray_length = 240.0f;
    menu.minimap_fire_flash_ms = 420.0f;
    menu.minimap_ray_width = 1.1f;
    menu.bigmap_name_background = true;
    menu.bigmap_show_legend = true;
    menu.bigmap_show_vehicles = true;
    menu.bigmap_show_airdrops = true;
    menu.bigmap_show_deadboxes = false;
    menu.bigmap_marker_size = 8.0f;
    menu.bigmap_marker_alpha = 0.60f;
    menu.bigmap_icon_size = 20.0f;
    menu.bigmap_name_font_size = 13.0f;
    menu.bigmap_name_bg_alpha = 0.34f;
    menu.bigmap_offset_x = 0.0f;
    menu.bigmap_offset_y = 0.0f;
    menu.bigmap_screen_scale = 1.0f;
    menu.esp_multilayer_nameplate = true;
    menu.team_color_custom = false;
    menu.flick_fov = 100.0f;
    menu.flick_max_dist = 400.0f;
    menu.flick_target_part = 3;
    menu.flick_behavior_mode = 0;
    menu.flick_auto_shot = false;
    menu.flick_follow_auto_shot = false;
    menu.flick_return = true;
    menu.flick_weapon_enabled.clear();
    menu.flick_category_enabled.clear();
    menu.flick_category_visible_only.clear();
    menu.flick_category_auto_shot.clear();
    menu.flick_category_shot_hold.clear();
    menu.flick_category_follow_auto_shot.clear();
    menu.flick_category_behavior_mode.clear();
    menu.flick_category_target_part.clear();
    menu.flick_category_key.clear();
    menu.flick_category_move_speed.clear();
    menu.flick_category_fov.clear();
    menu.flick_category_max_dist.clear();
    menu.flick_selected_category = 0;
    FlickWeaponCatalog::EnsureCategoryDefaults(menu.flick_category_enabled);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(menu.flick_category_visible_only, menu.flick_visible_only);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(menu.flick_category_shot_hold, menu.flick_shot_hold);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(menu.flick_category_follow_auto_shot, menu.flick_follow_auto_shot);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(menu.flick_category_behavior_mode, menu.flick_behavior_mode);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(menu.flick_category_target_part, menu.flick_target_part);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(menu.flick_category_key, menu.flick_key);
    FlickWeaponCatalog::EnsureCategoryFloatDefaults(menu.flick_category_max_dist, menu.flick_max_dist);
    FlickWeaponCatalog::EnsureCategoryMoveSpeedDefaults(menu.flick_category_move_speed);
    FlickWeaponCatalog::EnsureCategoryFovDefaults(menu.flick_category_fov, menu.flick_fov);
    menu.damage_color[0] = 1.0f; menu.damage_color[1] = 0.48f; menu.damage_color[2] = 0.18f; menu.damage_color[3] = 1.0f;
    menu.speed_color[0] = 0.45f; menu.speed_color[1] = 1.0f; menu.speed_color[2] = 0.72f; menu.speed_color[3] = 1.0f;
    menu.ammo_color[0] = 0.95f; menu.ammo_color[1] = 0.95f; menu.ammo_color[2] = 0.72f; menu.ammo_color[3] = 1.0f;
    menu.close_warning_color[0] = 1.0f; menu.close_warning_color[1] = 0.20f; menu.close_warning_color[2] = 0.08f; menu.close_warning_color[3] = 1.0f;
    menu.aim_warning_color[0] = 1.0f; menu.aim_warning_color[1] = 0.12f; menu.aim_warning_color[2] = 0.08f; menu.aim_warning_color[3] = 1.0f;
    menu.view_direction_color[0] = 0.20f; menu.view_direction_color[1] = 0.82f; menu.view_direction_color[2] = 1.0f; menu.view_direction_color[3] = 0.85f;
    menu.debug_loot_resolver = false;
}

} // namespace

namespace OverlayPresets {

const char* Name(Preset preset) {
    switch (preset) {
    case Preset::Clean: return "Clean";
    case Preset::Loot: return "Loot";
    case Preset::Combat: return "Combat";
    case Preset::Debug: return "Debug";
    default: return "Unknown";
    }
}

void Apply(OverlayMenu& menu, Preset preset) {
    ApplySharedDefaults(menu);
    menu.active_preset = static_cast<int>(preset);

    if (preset == Preset::Clean) {
        menu.esp_box = true;
        menu.esp_skeleton = true;
        menu.esp_health = true;
        menu.esp_health_bar_style = 2;
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = false;
        menu.esp_ammo = false;
        menu.esp_speed = false;
        menu.esp_rank = false;
        menu.esp_killcount = false;
        menu.esp_survival_level = false;
        menu.esp_spectated = true;
        menu.esp_items = false;
        menu.esp_vehicles = false;
        menu.render_distance = 400;
        menu.name_max_dist = 180;
        menu.weapon_max_dist = 120;
        return;
    }

    if (preset == Preset::Loot) {
        menu.esp_box = true;
        menu.esp_skeleton = false;
        menu.esp_health = true;
        menu.esp_health_bar_style = 1;
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = false;
        menu.esp_items = true;
        menu.esp_vehicles = true;
        menu.loot_max_dist = 180;
        menu.vehicle_max_dist = 900;
        menu.item_icon_size = 28.0f;
        menu.item_group_icon_size = 20.0f;
        menu.vehicle_icon_size = 40.0f;
        return;
    }

    if (preset == Preset::Combat) {
        menu.esp_box = true;
        menu.esp_skeleton = true;
        menu.esp_health = true;
        menu.esp_health_bar_style = 4;
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = true;
        menu.esp_rank = true;
        menu.esp_killcount = true;
        menu.esp_damage = true;
        menu.esp_ammo = true;
        menu.esp_speed = true;
        menu.esp_survival_level = true;
        menu.esp_spectated = true;
        menu.esp_items = false;
        menu.esp_vehicles = false;
        menu.render_distance = 400;
        menu.name_max_dist = 260;
        menu.weapon_max_dist = 200;
        return;
    }

    if (preset == Preset::Debug) {
        menu.esp_box = true;
        menu.esp_skeleton = true;
        menu.esp_health = true;
        menu.esp_health_bar_style = 3;
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = true;
        menu.esp_items = true;
        menu.esp_vehicles = true;
        menu.debug_loot_resolver = true;
        menu.debug_actor_esp = true;
        menu.loot_max_dist = 300;
        menu.vehicle_max_dist = 1200;
    }
}

} // namespace OverlayPresets
