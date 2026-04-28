#include "overlay_config_sections.hpp"
#include "overlay_menu.hpp"

#include <algorithm>

namespace OverlayConfigSections {

void ClampPlayer(OverlayMenu& menu) {
    menu.name_font_size = std::clamp(menu.name_font_size, 8.0f, 26.0f);
    menu.distance_font_size = std::clamp(menu.distance_font_size, 8.0f, 24.0f);
    menu.weapon_font_size = std::clamp(menu.weapon_font_size, 8.0f, 24.0f);
    menu.rank_font_size = std::clamp(menu.rank_font_size, 8.0f, 24.0f);
    menu.teamid_badge_size = std::clamp(menu.teamid_badge_size, 10.0f, 34.0f);
    menu.kill_font_size = std::clamp(menu.kill_font_size, 8.0f, 24.0f);
    menu.survival_level_font_size = std::clamp(menu.survival_level_font_size, 8.0f, 24.0f);
    menu.spectated_font_size = std::clamp(menu.spectated_font_size, 8.0f, 28.0f);
    menu.weapon_icon_size = std::clamp(menu.weapon_icon_size, 18.0f, 90.0f);
}

void ClampLoot(OverlayMenu& menu) {
    menu.item_icon_size = std::clamp(menu.item_icon_size, 12.0f, 48.0f);
    menu.item_group_icon_size = std::clamp(menu.item_group_icon_size, 10.0f, 38.0f);
    menu.loot_distance_font_size = std::clamp(menu.loot_distance_font_size, 8.0f, 20.0f);
}

void ClampVehicle(OverlayMenu& menu) {
    menu.vehicle_icon_size = std::clamp(menu.vehicle_icon_size, 16.0f, 80.0f);
}

void ClampAll(OverlayMenu& menu) {
    ClampPlayer(menu);
    ClampLoot(menu);
    ClampVehicle(menu);
}

} // namespace OverlayConfigSections
