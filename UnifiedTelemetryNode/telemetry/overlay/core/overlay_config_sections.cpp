#include "overlay_config_sections.hpp"
#include "overlay_menu.hpp"

#include <algorithm>

namespace OverlayConfigSections {

void ClampPlayer(OverlayMenu& menu) {
    menu.esp_box_type = std::clamp(menu.esp_box_type, 0, 1);
    menu.name_font_size = std::clamp(menu.name_font_size, 8.0f, 26.0f);
    menu.distance_font_size = std::clamp(menu.distance_font_size, 8.0f, 24.0f);
    menu.weapon_font_size = std::clamp(menu.weapon_font_size, 8.0f, 24.0f);
    menu.rank_font_size = std::clamp(menu.rank_font_size, 8.0f, 24.0f);
    menu.teamid_badge_size = std::clamp(menu.teamid_badge_size, 10.0f, 34.0f);
    menu.kill_font_size = std::clamp(menu.kill_font_size, 8.0f, 24.0f);
    menu.survival_level_font_size = std::clamp(menu.survival_level_font_size, 8.0f, 24.0f);
    menu.damage_font_size = std::clamp(menu.damage_font_size, 8.0f, 24.0f);
    menu.speed_font_size = std::clamp(menu.speed_font_size, 8.0f, 24.0f);
    menu.ammo_font_size = std::clamp(menu.ammo_font_size, 8.0f, 24.0f);
    menu.spectated_font_size = std::clamp(menu.spectated_font_size, 8.0f, 28.0f);
    menu.weapon_icon_size = std::clamp(menu.weapon_icon_size, 18.0f, 90.0f);
    menu.esp_text_bg_alpha = std::clamp(menu.esp_text_bg_alpha, 0.0f, 0.70f);
    menu.esp_health_display_mode = std::clamp(menu.esp_health_display_mode, 0, 1);
    menu.esp_health_bar_style = std::clamp(menu.esp_health_bar_style, 0, 4);
    menu.esp_weapon_type = std::clamp(menu.esp_weapon_type, 0, 1);
    menu.snapline_type = std::clamp(menu.snapline_type, 0, 3);
    menu.esp_view_direction_length = std::clamp(menu.esp_view_direction_length, 5.0f, 120.0f);
    menu.esp_close_warning_distance = std::clamp(menu.esp_close_warning_distance, 10.0f, 150.0f);
    menu.player_list_hold_key = std::clamp(menu.player_list_hold_key, 0, 0xFE);
    menu.flick_fov = std::clamp(menu.flick_fov, 1.0f, 100.0f);
    menu.flick_max_dist = std::clamp(menu.flick_max_dist, 5.0f, 400.0f);
    menu.flick_target_part = std::clamp(menu.flick_target_part, 0, 15);
    menu.flick_key = std::clamp(menu.flick_key, 0, 0xFE);
    menu.flick_key2 = std::clamp(menu.flick_key2, 0, 0xFE);
    menu.flick_behavior_mode = std::clamp(menu.flick_behavior_mode, 0, 1);
    menu.flick_return = (menu.flick_behavior_mode == 0);
}

void ClampRadar(OverlayMenu& menu) {
    menu.radar_zoom_multiplier = std::clamp(menu.radar_zoom_multiplier, 0.5f, 5.0f);
    menu.radar_dot_size = std::clamp(menu.radar_dot_size, 1.0f, 18.0f);
    menu.minimap_view_ray_length = std::clamp(menu.minimap_view_ray_length, 10.0f, 180.0f);
    menu.minimap_fire_ray_length = std::clamp(menu.minimap_fire_ray_length, 40.0f, 500.0f);
    menu.minimap_fire_flash_ms = std::clamp(menu.minimap_fire_flash_ms, 80.0f, 1200.0f);
    menu.minimap_ray_width = std::clamp(menu.minimap_ray_width, 0.5f, 4.0f);
    menu.bigmap_marker_size = std::clamp(menu.bigmap_marker_size, 3.0f, 18.0f);
    menu.bigmap_marker_alpha = std::clamp(menu.bigmap_marker_alpha, 0.25f, 1.0f);
    menu.bigmap_icon_size = std::clamp(menu.bigmap_icon_size, 10.0f, 42.0f);
    menu.bigmap_name_font_size = std::clamp(menu.bigmap_name_font_size, 9.0f, 22.0f);
    menu.bigmap_name_bg_alpha = std::clamp(menu.bigmap_name_bg_alpha, 0.0f, 0.70f);
    menu.bigmap_offset_x = std::clamp(menu.bigmap_offset_x, -400.0f, 400.0f);
    menu.bigmap_offset_y = std::clamp(menu.bigmap_offset_y, -400.0f, 400.0f);
    menu.bigmap_screen_scale = std::clamp(menu.bigmap_screen_scale, 0.70f, 1.25f);
}

void ClampLoot(OverlayMenu& menu) {
    menu.item_icon_size = std::clamp(menu.item_icon_size, 12.0f, 48.0f);
    menu.item_group_icon_size = std::clamp(menu.item_group_icon_size, 10.0f, 38.0f);
    menu.loot_distance_font_size = std::clamp(menu.loot_distance_font_size, 8.0f, 20.0f);
    menu.asset_animation_strength = std::clamp(menu.asset_animation_strength, 0.0f, 2.0f);
    menu.asset_animation_speed = std::clamp(menu.asset_animation_speed, 0.10f, 3.0f);
}

void ClampVehicle(OverlayMenu& menu) {
    menu.vehicle_icon_size = std::clamp(menu.vehicle_icon_size, 16.0f, 80.0f);
}

void ClampAll(OverlayMenu& menu) {
    ClampPlayer(menu);
    ClampRadar(menu);
    ClampLoot(menu);
    ClampVehicle(menu);
}

} // namespace OverlayConfigSections
