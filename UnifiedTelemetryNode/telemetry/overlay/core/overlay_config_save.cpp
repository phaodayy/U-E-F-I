#include "overlay_menu.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include "../../../nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>
void OverlayMenu::SaveConfig(const char* path) {
    try {
        nlohmann::json j;
        j["esp_toggle"] = esp_toggle;
        j["esp_icons"] = esp_icons;
        j["esp_font_size"] = esp_font_size;
        j["name_font_size"] = name_font_size;
        j["distance_font_size"] = distance_font_size;
        j["weapon_font_size"] = weapon_font_size;
        j["rank_font_size"] = rank_font_size;
        j["teamid_badge_size"] = teamid_badge_size;
        j["kill_font_size"] = kill_font_size;
        j["survival_level_font_size"] = survival_level_font_size;
        j["spectated_font_size"] = spectated_font_size;
        j["weapon_icon_size"] = weapon_icon_size;
        j["box_thickness"] = box_thickness;
        j["esp_skeleton_dots"] = esp_skeleton_dots;
        j["esp_spectator_list"] = esp_spectator_list;
        j["debug_loot_resolver"] = debug_loot_resolver;
        j["active_preset"] = active_preset;
        j["aim_smooth_rng"] = aim_smooth_rng;
        j["aim_key2"] = aim_key2;
        j["esp_show_enemies"] = esp_show_enemies;
        j["esp_show_teammates"] = esp_show_teammates;
        j["esp_offscreen"] = esp_offscreen;
        j["esp_offscreen_style"] = esp_offscreen_style;
        j["offscreen_color_mode"] = offscreen_color_mode;
        j["offscreen_radius"] = offscreen_radius;
        j["offscreen_size"] = offscreen_size;
        j["offscreen_near_color"] = { offscreen_near_color[0], offscreen_near_color[1], offscreen_near_color[2], offscreen_near_color[3] };
        j["offscreen_far_color"] = { offscreen_far_color[0], offscreen_far_color[1], offscreen_far_color[2], offscreen_far_color[3] };
        j["esp_box"] = esp_box;
        j["esp_skeleton"] = esp_skeleton;
        j["esp_name"] = esp_name;
        j["esp_distance"] = esp_distance;
        j["esp_health"] = esp_health;
        j["esp_health_pos"] = esp_health_pos;
        j["esp_distance_pos"] = esp_distance_pos;
        j["esp_name_pos"] = esp_name_pos;
        j["esp_rank_pos"] = esp_rank_pos;
        j["esp_weapon_pos"] = esp_weapon_pos;
        j["esp_spectated_pos"] = esp_spectated_pos;
        j["esp_teamid_pos"] = esp_teamid_pos;
        j["esp_killcount_pos"] = esp_killcount_pos;
        j["esp_survival_level_pos"] = esp_survival_level_pos;
        j["esp_items"] = esp_items;
        j["esp_items_toggle_key"] = esp_items_toggle_key;
        j["esp_vehicles_toggle_key"] = esp_vehicles_toggle_key;
        j["esp_snapline"] = esp_snapline;
        j["esp_weapon"] = esp_weapon;
        j["esp_weapon_type"] = esp_weapon_type;
        j["render_distance"] = render_distance;
        j["language"] = language;
        j["show_macro_overlay"] = show_macro_overlay;
        j["show_radar_center"] = show_radar_center;
        j["anti_screenshot"] = anti_screenshot;
        j["macro_enabled"] = macro_enabled;
        j["macro_humanize"] = macro_humanize;
        j["macro_ads_only"] = macro_ads_only;
        j["macro_global_multiplier"] = MacroEngine::global_multiplier;
        j["macro_overlay_color"] = { macro_overlay_color[0], macro_overlay_color[1], macro_overlay_color[2], macro_overlay_color[3] };

        j["esp_items"] = esp_items;
        j["esp_vehicles"] = esp_vehicles;
        j["esp_airdrops"] = esp_airdrops;
        j["esp_deadboxes"] = esp_deadboxes;
        j["loot_max_dist"] = loot_max_dist;
        j["vehicle_max_dist"] = vehicle_max_dist;
        j["item_icon_size"] = item_icon_size;
        j["item_group_icon_size"] = item_group_icon_size;
        j["vehicle_icon_size"] = vehicle_icon_size;
        j["loot_distance_font_size"] = loot_distance_font_size;

        j["share_radar"] = share_radar;
        j["share_radar_ip"] = std::string(share_radar_ip);
        j["esp_grenade_prediction"] = esp_grenade_prediction;
        j["esp_projectile_tracer"] = esp_projectile_tracer;
        j["esp_threat_warning"] = esp_threat_warning;

        j["loot_armor_lv1"] = loot_armor_lv1;
        j["loot_armor_lv2"] = loot_armor_lv2;
        j["loot_armor_lv3"] = loot_armor_lv3;
        j["loot_helmet_lv1"] = loot_helmet_lv1;
        j["loot_helmet_lv2"] = loot_helmet_lv2;
        j["loot_helmet_lv3"] = loot_helmet_lv3;
        j["loot_meds_boosts"] = loot_meds_boosts;
        j["loot_meds_healing"] = loot_meds_healing;
        j["loot_ammo_all"] = loot_ammo_all;
        j["loot_ammo_high"] = loot_ammo_high;
        j["loot_scopes_all"] = loot_scopes_all;
        j["loot_scopes_high"] = loot_scopes_high;
        j["loot_attach_mag"] = loot_attach_mag;
        j["loot_attach_muzzle"] = loot_attach_muzzle;
        j["loot_weapon_special"] = loot_weapon_special;
        j["loot_weapon_all"] = loot_weapon_all;
        j["loot_backpack_lv1"] = loot_backpack_lv1;
        j["loot_backpack_lv2"] = loot_backpack_lv2;
        j["loot_backpack_lv3"] = loot_backpack_lv3;
        j["loot_ghillie"] = loot_ghillie;
        j["loot_utility"] = loot_utility;
        j["loot_repair"] = loot_repair;
        j["loot_ammo_556"] = loot_ammo_556; j["loot_ammo_762"] = loot_ammo_762;
        j["loot_ammo_9mm"] = loot_ammo_9mm; j["loot_ammo_45"] = loot_ammo_45;
        j["loot_ammo_12g"] = loot_ammo_12g; j["loot_ammo_300"] = loot_ammo_300;
        j["loot_ammo_slug"] = loot_ammo_slug; j["loot_ammo_57"] = loot_ammo_57;
        j["loot_ammo_40"] = loot_ammo_40;
        j["loot_ammo_bolt"] = loot_ammo_bolt; j["loot_ammo_flare"] = loot_ammo_flare;
        j["loot_ammo_mortar"] = loot_ammo_mortar;

        j["loot_key_security"] = loot_key_security; j["loot_key_secret"] = loot_key_secret;
        j["loot_key_taego"] = loot_key_taego; j["loot_key_vikendi"] = loot_key_vikendi;
        j["loot_key_chimera"] = loot_key_chimera; j["loot_key_haven"] = loot_key_haven;

        j["loot_scope_reddot"] = loot_scope_reddot; j["loot_scope_holo"] = loot_scope_holo;
        j["loot_scope_2x"] = loot_scope_2x; j["loot_scope_3x"] = loot_scope_3x;
        j["loot_scope_4x"] = loot_scope_4x; j["loot_scope_6x"] = loot_scope_6x;
        j["loot_scope_8x"] = loot_scope_8x; j["loot_scope_15x"] = loot_scope_15x;
        j["loot_scope_thermal"] = loot_scope_thermal;
        j["loot_muzzle_comp"] = loot_muzzle_comp; j["loot_muzzle_flash"] = loot_muzzle_flash;
        j["loot_muzzle_supp"] = loot_muzzle_supp; j["loot_muzzle_choke"] = loot_muzzle_choke;
        j["loot_grip_vertical"] = loot_grip_vertical; j["loot_grip_angled"] = loot_grip_angled;
        j["loot_grip_half"] = loot_grip_half; j["loot_grip_thumb"] = loot_grip_thumb;
        j["loot_grip_light"] = loot_grip_light; j["loot_stock_heavy"] = loot_stock_heavy;
        j["loot_stock_cheek"] = loot_stock_cheek; j["loot_mag_ext"] = loot_mag_ext;
        j["loot_mag_quick"] = loot_mag_quick; j["loot_mag_ext_quick"] = loot_mag_ext_quick;

        j["loot_throw_frag"] = loot_throw_frag; j["loot_throw_smoke"] = loot_throw_smoke;
        j["loot_throw_flash"] = loot_throw_flash; j["loot_throw_molotov"] = loot_throw_molotov;
        j["loot_throw_c4"] = loot_throw_c4; j["loot_throw_sticky"] = loot_throw_sticky;
        j["loot_throw_bz"] = loot_throw_bz; j["loot_throw_decoy"] = loot_throw_decoy;

        j["loot_attach_grip"] = loot_attach_grip;
        j["loot_attach_stock"] = loot_attach_stock;
        j["loot_attach_scope_all"] = loot_attach_scope_all;
        j["loot_attach_scope_high"] = loot_attach_scope_high;

        // Weapon-specific booleans
        j["lw_ace32"] = loot_weapon_ace32; j["lw_ak47"] = loot_weapon_ak47;
        j["lw_aug"] = loot_weapon_aug; j["lw_beryl"] = loot_weapon_beryl;
        j["lw_g36c"] = loot_weapon_g36c; j["lw_groza"] = loot_weapon_groza;
        j["lw_hk416"] = loot_weapon_hk416; j["lw_k2"] = loot_weapon_k2;
        j["lw_m16"] = loot_weapon_m16; j["lw_mutant"] = loot_weapon_mutant;
        j["lw_qbz"] = loot_weapon_qbz; j["lw_scar"] = loot_weapon_scar;
        j["lw_famas"] = loot_weapon_famas; j["lw_awm"] = loot_weapon_awm;
        j["lw_kar98"] = loot_weapon_kar98; j["lw_m24"] = loot_weapon_m24;
        j["lw_mosin"] = loot_weapon_mosin; j["lw_win94"] = loot_weapon_win94;
        j["lw_dragunov"] = loot_weapon_dragunov; j["lw_mini14"] = loot_weapon_mini14;
        j["lw_mk12"] = loot_weapon_mk12; j["lw_mk14"] = loot_weapon_mk14;
        j["lw_qbu"] = loot_weapon_qbu; j["lw_sks"] = loot_weapon_sks;
        j["lw_vss"] = loot_weapon_vss; j["lw_slr"] = loot_weapon_slr;
        j["lw_bizon"] = loot_weapon_bizon; j["lw_mp5"] = loot_weapon_mp5;
        j["lw_mp9"] = loot_weapon_mp9; j["lw_p90"] = loot_weapon_p90;
        j["lw_thompson"] = loot_weapon_thompson; j["lw_ump"] = loot_weapon_ump;
        j["lw_uzi"] = loot_weapon_uzi; j["lw_vector"] = loot_weapon_vector;
        j["lw_js9"] = loot_weapon_js9; j["lw_dp28"] = loot_weapon_dp28;
        j["lw_m249"] = loot_weapon_m249; j["lw_mg3"] = loot_weapon_mg3;
        j["lw_dp12"] = loot_weapon_dp12; j["lw_saiga"] = loot_weapon_saiga;
        j["lw_deagle"] = loot_weapon_deagle; j["lw_m1911"] = loot_weapon_m1911;
        j["lw_p92"] = loot_weapon_p92; j["lw_skorpion"] = loot_weapon_skorpion;
        j["lw_nagant"] = loot_weapon_nagant; j["lw_rhino"] = loot_weapon_rhino;
        j["lw_stungun"] = loot_weapon_stungun;
        j["lw_pan"] = loot_weapon_pan; j["lw_flare"] = loot_weapon_flare;
        j["lw_crossbow"] = loot_weapon_crossbow; j["lw_panzer"] = loot_weapon_panzer;
        j["lw_spike"] = loot_weapon_spike; j["lw_m79"] = loot_weapon_m79;

        j["loot_ghillie_arctic"] = loot_ghillie_arctic;
        j["loot_ghillie_desert"] = loot_ghillie_desert;
        j["loot_ghillie_jungle"] = loot_ghillie_jungle;
        j["loot_ghillie_forest"] = loot_ghillie_forest;
        j["loot_ghillie_mossy"] = loot_ghillie_mossy;
        j["loot_ghillie_brown"] = loot_ghillie_brown;

        j["loot_utility_drone"] = loot_utility_drone;
        j["loot_utility_scope"] = loot_utility_scope;
        j["loot_repair_armor"] = loot_repair_armor;
        j["loot_repair_helmet"] = loot_repair_helmet;
        j["loot_repair_vehicle"] = loot_repair_vehicle;
        j["loot_utility_jammer"] = loot_utility_jammer;
        j["loot_utility_bluechip"] = loot_utility_bluechip;
        j["loot_utility_vtransmitter"] = loot_utility_vtransmitter;
        j["loot_utility_shield"] = loot_utility_shield;

        j["loot_vehicle_uaz"] = loot_vehicle_uaz;
        j["loot_vehicle_dacia"] = loot_vehicle_dacia;
        j["loot_vehicle_buggy"] = loot_vehicle_buggy;
        j["loot_vehicle_bike"] = loot_vehicle_bike;
        j["loot_vehicle_boat"] = loot_vehicle_boat;
        j["loot_vehicle_air"] = loot_vehicle_air;
        j["loot_vehicle_brdm"] = loot_vehicle_brdm;
        j["loot_vehicle_scooter"] = loot_vehicle_scooter;
        j["loot_vehicle_tuk"] = loot_vehicle_tuk;
        j["loot_vehicle_snow"] = loot_vehicle_snow;
        j["loot_vehicle_bus"] = loot_vehicle_bus;
        j["loot_vehicle_truck"] = loot_vehicle_truck;
        j["loot_vehicle_train"] = loot_vehicle_train;
        j["loot_vehicle_mirado"] = loot_vehicle_mirado;
        j["loot_vehicle_pickup"] = loot_vehicle_pickup;
        j["loot_vehicle_rony"] = loot_vehicle_rony;
        j["loot_vehicle_blanc"] = loot_vehicle_blanc;

        j["esp_distance_lod"] = esp_distance_lod;
        j["skeleton_max_dist"] = skeleton_max_dist;
        j["name_max_dist"] = name_max_dist;
        j["weapon_max_dist"] = weapon_max_dist;
        j["box_max_dist"] = box_max_dist;
        j["hp_max_dist"] = hp_max_dist;
        j["distance_txt_max_dist"] = distance_txt_max_dist;

        j["color_box_vis"] = { box_visible_color[0], box_visible_color[1], box_visible_color[2], box_visible_color[3] };
        j["color_box_inv"] = { box_invisible_color[0], box_invisible_color[1], box_invisible_color[2], box_invisible_color[3] };
        j["color_skel_vis"] = { skeleton_visible_color[0], skeleton_visible_color[1], skeleton_visible_color[2], skeleton_visible_color[3] };
        j["color_skel_inv"] = { skeleton_invisible_color[0], skeleton_invisible_color[1], skeleton_invisible_color[2], skeleton_invisible_color[3] };
        j["color_names"] = { name_color[0], name_color[1], name_color[2], name_color[3] };
        j["color_dist"] = { distance_color[0], distance_color[1], distance_color[2], distance_color[3] };
        j["color_weapon"] = { weapon_color[0], weapon_color[1], weapon_color[2], weapon_color[3] };
        j["color_rank"] = { rank_color[0], rank_color[1], rank_color[2], rank_color[3] };
        j["color_teamid"] = { teamid_color[0], teamid_color[1], teamid_color[2], teamid_color[3] };
        j["color_kill"] = { kill_color[0], kill_color[1], kill_color[2], kill_color[3] };
        j["color_survival_level"] = { survival_level_color[0], survival_level_color[1], survival_level_color[2], survival_level_color[3] };
        j["color_spectated"] = { spectated_color[0], spectated_color[1], spectated_color[2], spectated_color[3] };

        // precision_calibration Configs
        j["aim_master_enabled"] = aim_master_enabled;
        j["aim_adaptive_fov"] = aim_adaptive_fov;
        j["aim_visible_only"] = aim_visible_only;

        nlohmann::json aim_array = nlohmann::json::array();
        for (int i = 0; i < 9; i++) {
            nlohmann::json c;
            c["enabled"] = aim_configs[i].enabled;
            c["fov"] = aim_configs[i].fov;
            c["smooth"] = aim_configs[i].smooth;
            c["bone"] = aim_configs[i].bone;
            c["key"] = aim_configs[i].key;
            c["max_dist"] = aim_configs[i].max_dist;
            c["prediction"] = aim_configs[i].prediction;
            aim_array.push_back(c);
        }
        j["aim_configs"] = aim_array;

        std::ofstream file(path);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
            std::cout << "[+] Saved Config to: " << path << std::endl;
        }
    } catch (...) {}
}
