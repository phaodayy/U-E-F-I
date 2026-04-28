#include "overlay_presets.hpp"
#include "overlay_menu.hpp"

namespace {

void ApplySharedDefaults(OverlayMenu& menu) {
    menu.esp_toggle = true;
    menu.esp_icons = true;
    menu.esp_show_enemies = true;
    menu.esp_show_teammates = false;
    menu.esp_skeleton_dots = false;
    menu.item_icon_size = 24.0f;
    menu.item_group_icon_size = 18.0f;
    menu.vehicle_icon_size = 34.0f;
    menu.loot_distance_font_size = 12.0f;
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
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = false;
        menu.esp_rank = false;
        menu.esp_killcount = false;
        menu.esp_survival_level = false;
        menu.esp_spectated = true;
        menu.esp_items = false;
        menu.esp_vehicles = false;
        menu.render_distance = 450;
        menu.name_max_dist = 180;
        menu.weapon_max_dist = 120;
        return;
    }

    if (preset == Preset::Loot) {
        menu.esp_box = true;
        menu.esp_skeleton = false;
        menu.esp_health = true;
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
        menu.esp_distance = true;
        menu.esp_name = true;
        menu.esp_weapon = true;
        menu.esp_rank = true;
        menu.esp_killcount = true;
        menu.esp_survival_level = true;
        menu.esp_spectated = true;
        menu.esp_items = false;
        menu.esp_vehicles = false;
        menu.render_distance = 700;
        menu.name_max_dist = 260;
        menu.weapon_max_dist = 200;
        return;
    }

    if (preset == Preset::Debug) {
        menu.esp_box = true;
        menu.esp_skeleton = true;
        menu.esp_health = true;
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
