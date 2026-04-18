#pragma once
#include <string>
#include <unordered_map>
#include "Common/Data.h"
#include "Utils/Utils.h"

// Define a macro for easy usage
#define TR(key) Translation::Get(key)

class Translation
{
public:
    enum class Language {
        English = 0,
        Vietnamese = 1
    };

    static void Initialize() {
        int gameLang = GameData.Config.Project.CurrentLanguage;
        if (gameLang == 1) { // 1 = Vietnamese
            LoadLanguage(Language::Vietnamese);
        } else { // 0 = English
            LoadLanguage(Language::English);
        }
    }

    static void LoadLanguage(Language lang) {
        currentLang = lang;
        translations.clear();
        if (lang == Language::Vietnamese) {
            LoadVietnamese();
        } else {
            LoadEnglish();
        }
    }

    static const char* Get(const std::string& key) {
        if (translations.find(key) != translations.end()) {
            return translations[key].c_str();
        }
        return key.c_str();
    }

private:
    static inline std::unordered_map<std::string, std::string> translations;
    static inline Language currentLang = Language::English;

    static void LoadEnglish() {
        // Menu & General
        translations["tab.esp"] = "ESP";
        translations["tab.aimbot"] = "Aimbot";
        translations["tab.world"] = "World";
        translations["tab.groups"] = "Categories";
        translations["tab.radar"] = "Radar";
        translations["tab.settings"] = "Settings";
        translations["tab.recoil"] = "Recoil";
        
        translations["menu.welcome"] = "[ Welcome to Use ]";
        translations["menu.expiration_time"] = "  Your Expiration Time: ";
        translations["menu.safe_exit"] = "Safe Exit";
        translations["menu.toggle_menu"] = "Toggle Menu";
        translations["menu.fusion_mode"] = "Dual Screen Key";
        translations["esp.combat_mode"] = "Combat Mode";
        translations["menu.item_settings"] = "Item Settings";
        translations["menu.other_weapons"] = "Other Weapons";
        translations["menu.perspective_display"] = "Perspective Display";
        translations["menu.value_settings"] = "Value Settings";
        translations["menu.aiming_values"] = "Aiming Values";
        translations["menu.aimbot_config"] = "AimBot Config";
        translations["menu.recoil_settings"] = "Recoil Settings";
        translations["menu.recoil_delay"] = "Recoil Delay";
        translations["menu.tire_lock"] = "Tire Lock Settings";
        translations["menu.search_player"] = "Search Player";
        translations["header.aim_switch"] = "Aim Switches";
        translations["debug.perf_monitor"] = "Show Performance Monitor";

        // Aimbot Settings
        translations["aim.aimbot_box"] = "AimBot Hardware";
        translations["aim.connect"] = "Connect";
        translations["aim.disconnect"] = "Disconnect";
        translations["aim.test_move"] = "Test Movement";
        translations["aim.save_config"] = "Save Config";
        translations["aim.config_hotkey"] = "Config Hotkey";
        translations["aim.com_port"] = "COM Port";
        translations["aim.no_com_port"] = "No Available COM Port";
        translations["aim.your_ip"] = "Your IP";
        translations["aim.your_port"] = "Your Port";
        translations["aim.your_uid"] = "Your UID";
        
        translations["aim.enable"] = "Enable Aim Assist";
        translations["aim.enable_missed_shot"] = "Enable Missed Shot";
        translations["aim.no_lock_empty"] = "No Lock on Empty Ammo";
        translations["aim.random_aim"] = "Random Aiming";
        translations["aim.random_site_params"] = "Random Site Parameters";
        translations["aim.bezier_movement"] = "Bezier Movement";
        translations["aim.scope_aim"] = "Aim on Scope";
        translations["aim.dynamic_range"] = "Dynamic Range";
        translations["aim.lock_toggle"] = "Enable FOV Limit";
        translations["aim.no_lock_cover"] = "No Lock through Cover";
        translations["aim.prediction"] = "Bullet Prediction";
        translations["aim.no_lock_downed"] = "No Lock on Downed";
        translations["aim.hotkey_merge"] = "Hotkey Merge";
        translations["aim.assist_range"] = "Show FOV Circle";
        translations["aim.kill_switch"] = "Kill Switch";
        translations["aim.auto_recoil"] = "Auto Recoil Control";
        translations["aim.original_recoil"] = "Original Ballistics";
        translations["aim.show_preaim"] = "Show Pre-Aim";
        translations["aim.primary_hotkey"] = "Primary Aim Hotkey";
        translations["aim.force_hotkey"] = "Force Aim Hotkey";
        translations["aim.downed_hotkey"] = "Downed Aim Hotkey";
        translations["aim.priority_screen"] = "Screen Distance Priority";
        translations["aim.priority_distance"] = "Player Distance Priority";
        translations["aim.fov"] = "Aim Assist FOV";
        translations["aim.smooth"] = "Aim Smoothing";
        translations["aim.x_speed"] = "X-Axis Speed";
        translations["aim.y_speed"] = "Y-Axis Speed";
        translations["aim.preaim_size"] = "Pre-Aim Size";
        translations["aim.max_distance"] = "Max Distance";
        translations["aim.preaim_color"] = "Pre-Aim Color";
        translations["aim.range_color"] = "Range Color";
        
        // Bone Names
        translations["bone.top_head"] = "Top Head";
        translations["bone.head"] = "Head";
        translations["bone.neck"] = "Neck";
        translations["bone.chest"] = "Chest";
        translations["bone.pelvis"] = "Pelvis";
        translations["bone.l_shoulder"] = "Left Shoulder";
        translations["bone.l_elbow"] = "Left Elbow";
        translations["bone.r_shoulder"] = "Right Shoulder";
        translations["bone.r_elbow"] = "Right Elbow";
        translations["bone.l_hand"] = "Left Hand";
        translations["bone.r_hand"] = "Right Hand";
        translations["bone.l_hip"] = "Left Hip";
        translations["bone.l_knee"] = "Left Knee";
        translations["bone.r_hip"] = "Right Hip";
        translations["bone.r_knee"] = "Right Knee";
        translations["bone.l_foot"] = "Left Foot";
        translations["bone.r_foot"] = "Right Foot";

        translations["aim.enable_tire_lock"] = "Enable Tire Lock";
        translations["aim.show_range"] = "Show Range";
        translations["aim.tire_hotkey"] = "Tire Lock Hotkey";
        translations["aim.tire_range"] = "Tire Lock Range";
        translations["aim.tire_smoothing"] = "Tire Lock Smoothing";
        
        translations["aim.mortar"] = "Mortar";
        translations["aim.panzer"] = "Panzer";
        translations["aim.mode"] = "Aim Mode";
        translations["aim.mode_main"] = "Auto (Closest Visible)";
        translations["aim.mode_head"] = "Head Priority (Visible)";
        translations["aim.mode_main_desc"] = "Automatically targets the visible body part closest to your crosshair.";
        translations["aim.mode_head_desc"] = "Prioritizes Head/Neck if visible. If the head is hidden, automatically switches to the closest visible body part.";
        translations["menu.aimbot_mode_settings"] = "Automatic Targeting Settings";

        // Trigger Settings
        translations["trigger.type"] = "Trigger Type";
        translations["trigger.auto1"] = "Auto Trigger 1";
        translations["trigger.auto2"] = "Auto Trigger 2";
        translations["trigger.enable1"] = "Enable Auto Trigger 1";
        translations["trigger.delay"] = "Trigger Delay";
        translations["trigger.fov"] = "Trigger FOV";
        translations["trigger.distance"] = "Trigger Distance";
        translations["trigger.enable2"] = "Enable Auto Trigger 2";
        translations["trigger.threshold"] = "Trigger Threshold";

        // Body Parts
        translations["body.main"] = "Main Body";
        translations["body.force"] = "Force Body";
        translations["body.downed"] = "Downed Body";
        translations["body.random"] = "Random Body";

        // Radar Settings
        translations["header.main_radar"] = "Main Map Radar";
        translations["radar.map_player"] = "Show Players";
        translations["radar.map_vehicle"] = "Show Vehicles";
        translations["radar.map_airdrop"] = "Show Airdrops";
        translations["radar.map_deadbox"] = "Show Deadboxes";
        translations["radar.map_scale"] = "Map Scale: %.1f";
        translations["radar.map_icon_scale"] = "Icon Scale: %d";

        translations["header.mini_radar"] = "Mini Map Radar";
        translations["radar.minimap_player"] = "Show Players";
        translations["radar.minimap_vehicle"] = "Show Vehicles";
        translations["radar.minimap_airdrop"] = "Show Airdrops";
        translations["radar.minimap_deadbox"] = "Show Deadboxes";
        translations["radar.map_room"] = "Show Key Rooms";
        translations["radar.font_size"] = "Font Size";
        translations["radar.map_distance"] = "Display Distance";
        translations["radar.status_title"] = "Radar Status";
        translations["radar.widget_found"] = "Found";
        translations["radar.widget_not_found"] = "Not Found";
        translations["radar.status_open"] = "Open";
        translations["radar.status_closed"] = "Closed";
        translations["radar.status_showing"] = "Showing";
        translations["radar.status_hidden"] = "Hidden";
        translations["radar.reset_widget"] = "Reset Widget";
        translations["radar.mini_map"] = "Mini Map";
        translations["radar.world_map"] = "World Map";
        translations["radar.world_map_m"] = "World Map (M)";
        translations["radar.enable"] = "Master Radar Switch";
        translations["radar.minimap_scale"] = "Minimap Scale: %.1f";
        translations["radar.minimap_icon_scale"] = "Icon Scale: %d";
        translations["radar.mini_room"] = "Room Keys";
        translations["radar.mini_font"] = "Font Size";
        translations["radar.reset_widget_label"] = "Reset Widget";
        translations["aim.device_selection"] = "Device";
        translations["aim.com_port_selection"] = "COM Port";
        translations["aim.net_ip"] = "IP Address";
        translations["aim.net_port"] = "Port";
        translations["aim.net_uuid"] = "UUID";

        translations["header.web_radar"] = "Web Radar";
        translations["radar.web.ip_hint"] = "Set Server IP (Local 127.0.0.1)";
        translations["radar.web.server_ip"] = "Server IP";
        translations["radar.web.open_browser"] = "Open Web Radar";
        translations["radar.web.instruction"] = "Ensure the server is running on the target PC before opening.";

        // Settings Tab
        translations["header.theme_color"] = "Theme & Colors";
        translations["header.bot_color"] = "Bot ESP Color";
        translations["header.grenade_warn"] = "Projectile Warning";
        translations["proj.grenade_alert"] = "Enable Warning";
        translations["proj.explosion_range"] = "Show Range";
        translations["proj.high_throw_pred"] = "High Throw Predict";
        translations["proj.grenade_traj"] = "Show Trajectory";
        translations["proj.grenade_countdown"] = "Show Timer";
        translations["proj.color_grenade"] = "Timer Color";
        translations["proj.color_range"] = "Range Color";
        translations["proj.color_traj"] = "Trajectory Color";
        translations["proj.size_alert"] = "Warning Size";
        translations["proj.size_traj"] = "Trajectory Size";
        translations["proj.dist_alert"] = "Warning Distance";
        translations["proj.size_grenade"] = "Timer Size";

        translations["header.software_other"] = "General & Settings";
        translations["setting.vsync"] = "Enable VSync";
        translations["setting.show_coords"] = "Show UI Coordinates";

        // Missing UI Translations
        translations["header.item_esp_config"] = "ITEM ESP CONFIG";
        translations["header.setting_group"] = "SETTING GROUP:";
        translations["header.environment"] = "ENVIRONMENT";
        translations["header.radar_settings"] = "RADAR";
        translations["header.general_config"] = "GENERAL CONFIG";
        translations["header.early_warning"] = "EARLY WARNING";
        translations["header.grenade_utils"] = "GRENADE UTILS";
        translations["header.model_settings"] = "3D MODEL SETTINGS";
        translations["menu.language"] = "Language";
        translations["esp.global_config"] = "Global Config";
        translations["header.preview_model"] = "ESP Preview";
        translations["menu.other_settings"] = "General Config";
        translations["overlay.vsync"] = "Enable VSync";
        translations["overlay.independent_thread"] = "Independent Thread";
        translations["menu.scan_offset"] = "Scanner";
        translations["menu.radar_warning"] = "Early Warning";
        translations["early.enable_alert"] = "Enable Alert";
        translations["early.max_distance"] = "Max Distance";
        translations["menu.grenade_settings"] = "Grenade Utils";
        translations["menu.model_settings"] = "3D Model Settings";
        translations["esp.low_model"] = "Low Model";
        translations["esp.medium_model"] = "Medium Model";
        translations["esp.high_model"] = "High Model";
        translations["esp.model_preview"] = "Model Preview";
        translations["menu.export_obj"] = "Export Scene (.OBJ)";
        translations["item.show_ray"] = "Show Ray";
        translations["item.esp_distance"] = "ESP Distance";
        translations["color.ray"] = "Ray Color";

        // Misc / Items
        translations["header.item_switch"] = "Item Filters";
        translations["item.esp_toggle"] = "Global Item ESP";
        translations["item.stack_display"] = "Stack Items";
        translations["item.show_icons"] = "Show Icons";
        translations["header.item_groups"] = "Categories";
        translations["item.rifle"] = "Rifles";
        translations["item.bolt_sniper"] = "Bolt Action";
        translations["item.dmr_sniper"] = "DMR";
        translations["item.machine_gun"] = "Machine Gun";
        translations["item.shotgun"] = "Shotgun";
        translations["item.pistol"] = "Pistol";
        translations["item.smg"] = "SMG";
        translations["item.attachments"] = "Attachments";
        translations["item.medicals"] = "Medicals";
        translations["item.armor"] = "Armor & Helmet";
        translations["item.ammo"] = "Ammo";
        translations["item.throwables"] = "Throwables";
        translations["item.keys"] = "Keys";
        translations["item.others"] = "Others";
        translations["item.mortar"] = "Mortar";
        translations["item.panzer"] = "Panzer";
        translations["item.smart_items"] = "Smart Loot";
        translations["header.smart_filter"] = "Smart Loot Settings";
        translations["smart.enable"] = "Enable Smart Filter";
        translations["smart.med_kit"] = "Medical Kit Limit";
        translations["smart.first_aid"] = "First Aid Limit";
        translations["smart.bandage"] = "Bandage Limit";
        translations["smart.adrenaline"] = "Adrenaline Limit";
        translations["smart.painkiller"] = "Painkiller Limit";
        translations["smart.energy_drink"] = "Energy Drink Limit";
        translations["smart.grenade"] = "Grenade Limit";
        translations["item.group_a_hotkey"] = "Group A Hotkey";
        translations["item.group_b_hotkey"] = "Group B Hotkey";
        translations["color.group_a"] = "Group A Color";
        translations["color.group_b"] = "Group B Color";
        translations["color.group_c"] = "Group C Color";
        translations["color.group_d"] = "Group D Color";

        // ESP Display Keys
        translations["esp.hitbox_highlight"] = "Skeleton Fill";
        translations["esp.aimed_ray"] = "Aimed Line";
        translations["esp.hide_rank"] = "Hide Rank";
        translations["esp.solo_tpp"] = "Solo TPP";
        translations["esp.squad_tpp"] = "Squad TPP";
        translations["esp.solo_fpp"] = "Solo FPP";
        translations["esp.squad_fpp"] = "Squad FPP";
        translations["menu.vehicle_perspective"] = "Vehicles";
        translations["vehicle.show"] = "Show Vehicle";
        translations["vehicle.fuel"] = "Show Fuel";
        translations["vehicle.health"] = "Show HP";
        translations["menu.airdrop_perspective"] = "Airdrops";
        translations["airdrop.show"] = "Show Airdrop";
        translations["airdrop.show_items"] = "Show Items";
        translations["menu.box_perspective"] = "Loot Boxes";
        translations["lootbox.show"] = "Show Deadbody";
        translations["lootbox.show_items"] = "Show Items";
        translations["header.esp_preview"] = "ESP Preview";
        translations["menu.grenade_tip"] = "Projectile Prediction";
        translations["aim.instant_grenade"] = "Instant Prediction";
        translations["aim.mortarPredict"] = "Mortar Prediction";
        translations["aim.panzerPredict"] = "Panzer Prediction";
        translations["aim.grenade"] = "Grenade Key";
        translations["esp.prediction_time"] = "DMA Latency (Prediction)";
        translations["esp.prediction_desc"] = "Adjust prediction time to match skeletons 100% with targets.\nDefault: 0.025 (25ms)";

        // ESP Settings
        translations["header.base_setting"] = "Base Settings";
        translations["header.colors"] = "Colors";
        translations["header.esp_switch"] = "ESP Switch";
        translations["esp.switch"] = "Perspective Switch";
        translations["esp.danger_warning"] = "Danger Warning";
        translations["esp.text_shadow"] = "Text Shadow";
        translations["esp.hide_locked"] = "Hide When Locked";
        translations["esp.visibility_check"] = "Visibility Check";
        translations["esp.lock_color_change"] = "Lock Color Change";
        translations["esp.distance_settings"] = "ESP Distance";
        translations["esp.info_distance"] = "Info Distance";
        translations["esp.bone_thickness"] = "Bone Thickness";
        translations["esp.ray_thickness"] = "Ray Thickness";
        translations["esp.stroke_size"] = "Text Stroke Size";
        translations["esp.health_bar"] = "Health Bar";
        translations["esp.bar_pos"] = "Bar Position";
        translations["esp.bar_style"] = "Bar Style";
        translations["esp.bar_width"] = "Bar Width";
        translations["esp.bar_height"] = "Bar Height";
        translations["esp.bar_alpha"] = "Bar Opacity";
        translations["esp.style_standard"] = "Standard (Dynamic)";
        translations["esp.style_solid"] = "Solid Color";
        translations["esp.style_wide_solid"] = "Wide (Solid)";
        translations["esp.style_wide_gradient"] = "Wide (Dynamic)";
        translations["esp.top_display"] = "Top Display";
        translations["esp.left_display"] = "Left Display";
        translations["esp.right_display"] = "Right Display";
        translations["esp.bottom_display"] = "Bottom Display";
        translations["esp.player_box"] = "Player Box";
        translations["esp.bones"] = "Bones";
        translations["esp.player_name"] = "Player Name";
        translations["esp.team_id"] = "Team ID";
        translations["esp.clan_name"] = "Clan Name";
        translations["esp.level"] = "Level";
        translations["esp.distance"] = "Distance";
        translations["esp.health"] = "Health";
        translations["esp.weapon"] = "Weapon";
        translations["esp.kills"] = "Kills";
        translations["esp.damage"] = "Damage";
        translations["esp.spectate"] = "Spectate";
        translations["esp.head_bones"] = "Head Bones";
        translations["esp.rank_icon"] = "Rank Icon";
        translations["esp.show_teammates"] = "Show Teammates";

        // ESP Colors
        translations["color.visible_info"] = "Visible Info";
        translations["color.visible_bones"] = "Visible Bones";
        translations["color.cover_info"] = "Cover Info";
        translations["color.cover_bones"] = "Cover Bones";
        translations["color.bot_info"] = "Bot Info";
        translations["color.bot_bones"] = "Bot Bones";
        translations["color.danger_info"] = "Danger Info";
        translations["color.downed_info"] = "Downed Info";
        translations["color.lock_color"] = "Lock Color";
        translations["color.ray_color"] = "Ray Color";
        translations["color.blacklist_info"] = "Blacklist Info";
        
        translations["proj.color_alert"] = "FOV Color";
        translations["aim.range_color"] = "Range Color";
        translations["aim.preaim_color"] = "Pre-Aim Color";

        // Recoil Settings
        translations["recoil.enable"] = "Enable Anti-Recoil";
        translations["recoil.red_dot"] = "Red Dot";
        translations["recoil.x2"] = "2x Scope";
        translations["recoil.x3"] = "3x Scope";
        translations["recoil.x4"] = "4x Scope";
        translations["recoil.x6"] = "6x Scope";
        translations["recoil.x8"] = "8x Scope";
        translations["recoil.delay_red_dot"] = "Delay Red Dot (ms)";
        translations["recoil.delay_x2"] = "Delay 2x (ms)";
        translations["recoil.delay_x3"] = "Delay 3x (ms)";
        translations["recoil.delay_x4"] = "Delay 4x (ms)";
        translations["recoil.delay_x6"] = "Delay 6x (ms)";
        translations["recoil.delay_x8"] = "Delay 8x (ms)";

        // Vehicle Labels
        translations["vehicle.armor"] = "Armor";
        translations["vehicle.energy"] = "Energy";
    }

    static void LoadVietnamese() {
        // Menu & General
        translations["tab.esp"] = U8("ESP");
        translations["tab.aimbot"] = U8("Aim");
        translations["tab.world"] = U8("Vật Phẩm");
        translations["tab.groups"] = U8("Phân Loại");
        translations["tab.radar"] = U8("Bản Đồ");
        translations["tab.settings"] = U8("Thiết Lập");
        translations["tab.recoil"] = U8("Giảm Giật");

        translations["menu.welcome"] = U8("[ Chào Mừng Sử Dụng ]");
        translations["menu.expiration_time"] = U8("  Thời Gian Hết Hạn: ");
        translations["menu.safe_exit"] = U8("Thoát An Toàn");
        translations["menu.toggle_menu"] = U8("Hiện/Ẩn Menu");
        translations["menu.fusion_mode"] = U8("Phím Màn Hình Phụ");
        translations["esp.combat_mode"] = U8("Chế Độ Combat");
        translations["menu.item_settings"] = U8("Cài Đặt Vật Phẩm");
        translations["menu.other_weapons"] = U8("Vũ Khí Khác");
        translations["menu.perspective_display"] = U8("Hiển Thị ESP");
        translations["menu.value_settings"] = U8("Cài Đặt Thông Số");
        translations["menu.aiming_values"] = U8("Giá Trị Aim");
        translations["menu.aimbot_config"] = U8("Cài Đặt Aim");
        translations["menu.recoil_settings"] = U8("Cài Đặt Giảm Giật");
        translations["menu.recoil_delay"] = U8("Trễ Giảm Giật");
        translations["menu.tire_lock"] = U8("Cài Đặt Khóa Lốp");
        translations["menu.search_player"] = U8("Tìm Player");
        translations["header.aim_switch"] = U8("Bật/Tắt Aim");
        translations["debug.perf_monitor"] = U8("Hiển thị Performance Monitor");

        // Aimbot Settings
        translations["aim.aimbot_box"] = U8("Phần Cứng Aim");
        translations["aim.connect"] = U8("Kết Nối");
        translations["aim.disconnect"] = U8("Ngắt Kết Nối");
        translations["aim.test_move"] = U8("Test Di Chuyển");
        translations["aim.save_config"] = U8("Lưu Cấu Hình");
        translations["aim.config_hotkey"] = U8("Phím Cấu Hình");
        translations["aim.com_port"] = U8("Cổng COM");
        translations["aim.no_com_port"] = U8("Không Có Cổng COM");
        translations["aim.your_ip"] = U8("Địa Chỉ IP");
        translations["aim.your_port"] = U8("Cổng");
        translations["aim.your_uid"] = U8("UID");

        translations["aim.enable"] = U8("Bật Aim");
        translations["aim.enable_missed_shot"] = U8("Bật Đạn Ảo");
        translations["aim.no_lock_empty"] = U8("Hết Đạn K Aim");
        translations["aim.random_aim"] = U8("Aim Ngẫu Nhiên");
        translations["aim.random_site_params"] = U8("Tham Số Ngẫu Nhiên");
        translations["aim.bezier_movement"] = U8("Aim Đường Cong");
        translations["aim.scope_aim"] = U8("Mở Kính Aim");
        translations["aim.dynamic_range"] = U8("Phạm Vi Động");
        translations["aim.lock_toggle"] = U8("Bật FOV");
        translations["aim.no_lock_cover"] = U8("Vật Cản Không Khóa");
        translations["aim.prediction"] = U8("Dự Đoán Đạn");
        translations["aim.no_lock_downed"] = U8("Không Khóa Khi Gục");
        translations["aim.hotkey_merge"] = U8("Gộp Phím Nóng");
        translations["aim.assist_range"] = U8("Hiện Vòng FOV");
        translations["aim.kill_switch"] = U8("Chuyển Khi Hạ Gục");
        translations["aim.auto_recoil"] = U8("Giảm Giật");
        translations["aim.original_recoil"] = U8("Đạn Đạo Gốc");
        translations["aim.show_preaim"] = U8("Hiển Thị Pre-Aim");
        translations["aim.primary_hotkey"] = U8("Nút Aim Chính (ngắm theo vị trí gần tâm nhất)");
        translations["aim.force_hotkey"] = U8("Nút Aim Đầu (ngắm theo vị trí đầu)");
        translations["aim.downed_hotkey"] = U8("Phím Ngắm Gục");
        translations["aim.priority_screen"] = U8("Ưu Tiên Tâm Ngắm");
        translations["aim.priority_distance"] = U8("Ưu Tiên Khoảng Cách");
        translations["aim.fov"] = U8("Vòng Ngắm (FOV)");
        translations["aim.smooth"] = U8("Độ Mượt (Smooth)");
        translations["aim.x_speed"] = U8("Tốc độ Ngang (X)");
        translations["aim.y_speed"] = U8("Tốc độ Dọc (Y)");
        translations["aim.preaim_size"] = U8("Kích Thước Pre-Aim");
        translations["aim.max_distance"] = U8("Khoảng Cách Max");
        translations["aim.preaim_color"] = U8("Màu Pre-Aim");
        translations["aim.range_color"] = U8("Màu Phạm Vi");

        translations["aim.enable_tire_lock"] = U8("Bật Khóa Lốp");
        translations["aim.show_range"] = U8("Hiện Phạm Vi");
        translations["aim.tire_hotkey"] = U8("Nút Aim Lốp (ngắm theo vị trí lốp xe)");
        translations["aim.tire_range"] = U8("Phạm Vi Khóa Lốp");
        translations["aim.tire_smoothing"] = U8("Mượt Khóa Lốp");
        
        translations["aim.mortar"] = U8("Cối");
        translations["aim.panzer"] = U8("Panzer");
        translations["aim.mode"] = U8("Chế Độ Aim");
        translations["aim.mode_main"] = U8("Tự động (Gần tâm)");
        translations["aim.mode_head"] = U8("Ưu tiên Đầu");
        translations["aim.mode_main_desc"] = U8("Tự động nhắm vào vùng cơ thể đang lộ diện và nằm gần tâm ngắm của bạn nhất.");
        translations["aim.mode_head_desc"] = U8("Ưu tiên nhắm vào Đầu/Cổ nếu đang lộ diện. Nếu không thấy đầu, sẽ tự động chuyển sang vùng cơ thể khác gần tâm nhất.");
        translations["menu.aimbot_mode_settings"] = U8("Cấu hình Ngắm Tự động");

        // Trigger Settings
        translations["trigger.type"] = U8("Loại Trigger");
        translations["trigger.auto1"] = U8("Tự Động 1");
        translations["trigger.auto2"] = U8("Tự Động 2");
        translations["trigger.enable1"] = U8("Bật Trigger 1");
        translations["trigger.delay"] = U8("Trễ Trigger");
        translations["trigger.fov"] = U8("Phạm Vi Trigger");
        translations["trigger.distance"] = U8("Khoảng Cách Trigger");
        translations["trigger.enable2"] = U8("Bật Trigger 2");
        translations["trigger.threshold"] = U8("Ngưỡng Trigger");

        // Radar Settings
        translations["header.main_radar"] = U8("Radar Bản Đồ Chính");
        translations["radar.map_player"] = U8("Hiện Người Chơi");
        translations["radar.map_vehicle"] = U8("Hiện Xe Cộ");
        translations["radar.map_airdrop"] = U8("Hiện Thính");
        translations["radar.map_deadbox"] = U8("Hiện Hòm Xác");
        translations["radar.map_scale"] = U8("Tỷ Lệ Bản Đồ: %.1f");
        translations["radar.map_icon_scale"] = U8("Tỷ Lệ Icon: %d");

        translations["header.mini_radar"] = U8("Radar Bản Đồ Nhỏ");
        translations["radar.minimap_player"] = U8("Hiện Người Chơi");
        translations["radar.minimap_vehicle"] = U8("Hiện Xe Cộ");
        translations["radar.minimap_airdrop"] = U8("Hiện Thính");
        translations["radar.minimap_deadbox"] = U8("Hiện Hòm Xác");
        translations["radar.map_room"] = U8("Phòng Chìa Khóa");
        translations["radar.font_size"] = U8("Cỡ Chữ");
        translations["radar.map_distance"] = U8("Khoảng Cách Hiển Thị");
        translations["radar.status_title"] = U8("Trạng Thái Radar");
        translations["radar.widget_found"] = U8("Đã Tìm Thấy");
        translations["radar.widget_not_found"] = U8("Chưa Tìm Thấy");
        translations["radar.status_open"] = U8("Đang Mở");
        translations["radar.status_closed"] = U8("Đóng");
        translations["radar.status_showing"] = U8("Hiện");
        translations["radar.status_hidden"] = U8("Ẩn");
        translations["radar.reset_widget"] = U8("Reset Widget");
        translations["radar.mini_map"] = U8("Bản Đồ Nhỏ (Mini Map)");
        translations["radar.world_map"] = U8("Bản Đồ Thế Giới");
        translations["radar.world_map_m"] = U8("Bản Đồ Thế Giới (M)");
        translations["radar.enable"] = U8("Bật/Tắt Toàn Bộ Radar");
        translations["radar.minimap_scale"] = U8("Tỷ Lệ Minimap: %.1f");
        translations["radar.minimap_icon_scale"] = U8("Tỷ Lệ Icon: %d");
        translations["radar.mini_room"] = U8("Phòng Chìa Khóa");
        translations["radar.mini_font"] = U8("Cỡ Chữ");
        translations["radar.reset_widget_label"] = U8("Thiết Lập Lại Widget");
        translations["aim.device_selection"] = U8("Thiết Bị");
        translations["aim.com_port_selection"] = U8("Cổng COM");
        translations["aim.net_ip"] = U8("Địa Chỉ IP");
        translations["aim.net_port"] = U8("Cổng");
        translations["aim.net_uuid"] = U8("UUID");

        translations["header.web_radar"] = U8("Radar Web");
        translations["radar.web.ip_hint"] = U8("Đặt IP Server (Mặc định 127.0.0.1)");
        translations["radar.web.server_ip"] = U8("IP Máy Chủ");
        translations["radar.web.open_browser"] = U8("Mở Radar Web");
        translations["radar.web.instruction"] = U8("Đảm bảo Server đang chạy trước khi mở trình duyệt.");

        // Settings Tab
        translations["header.theme_color"] = U8("Chủ Đề & Màu Sắc");
        translations["header.bot_color"] = U8("Màu ESP Bot");
        translations["header.grenade_warn"] = U8("Cảnh Báo Vật Ném");
        translations["proj.grenade_alert"] = U8("Bật Cảnh Báo");
        translations["proj.explosion_range"] = U8("Hiện Phạm Vi Nổ");
        translations["proj.high_throw_pred"] = U8("Dự Đoán Quăng Cao");
        translations["proj.grenade_traj"] = U8("Hiện Quỹ Đạo");
        translations["proj.grenade_countdown"] = U8("Hiện Đếm Ngược");
        translations["proj.color_grenade"] = U8("Màu Đếm Ngược");
        translations["proj.color_range"] = U8("Màu Phạm Vi");
        translations["proj.color_traj"] = U8("Màu Quỹ Đạo");
        translations["proj.size_alert"] = U8("Kích Thước Cảnh Báo");
        translations["proj.size_traj"] = U8("Kích Thước Quỹ Đạo");
        translations["proj.dist_alert"] = U8("Khoảng Cách Cảnh Báo");
        translations["proj.size_grenade"] = U8("Kích Thước Số");

        translations["header.software_other"] = U8("Cài Đặt Phần Mềm");
        translations["setting.vsync"] = U8("Bật VSync");
        translations["setting.show_coords"] = U8("Hiện Tọa Độ UI");

        // Missing UI Translations
        translations["header.item_esp_config"] = U8("CÀI ĐẶT ESP VẬT PHẨM");
        translations["header.setting_group"] = U8("CHỌN NHÓM:");
        translations["header.environment"] = U8("MÔI TRƯỜNG");
        translations["header.radar_settings"] = U8("RADAR");
        translations["header.general_config"] = U8("CÀI ĐẶT CHUNG");
        translations["header.early_warning"] = U8("CẢNH BÁO SỚM");
        translations["header.grenade_utils"] = U8("TIỆN ÍCH LỰU ĐẠN");
        translations["header.model_settings"] = U8("CÀI ĐẶT MÔ HÌNH 3D");
        translations["menu.language"] = U8("Ngôn Ngữ");
        translations["esp.global_config"] = U8("Cài Đặt Tổng Quan");
        translations["header.preview_model"] = U8("Xem Trước ESP");
        translations["menu.other_settings"] = U8("Cài Đặt Chung");
        translations["overlay.vsync"] = U8("Bật VSync");
        translations["overlay.independent_thread"] = U8("Xử Lý Đa Luồng");
        translations["menu.scan_offset"] = U8("Quét Lại Offset");
        translations["menu.radar_warning"] = U8("Cảnh Báo Sớm");
        translations["early.enable_alert"] = U8("Bật Cảnh Báo");
        translations["early.max_distance"] = U8("Khoảng Cách Max");
        translations["menu.grenade_settings"] = U8("Tiện Ích Lựu Đạn");
        translations["menu.model_settings"] = U8("Cài Đặt Mô Hình 3D");
        translations["esp.low_model"] = U8("Mô Hình Thấp");
        translations["esp.medium_model"] = U8("Mô Hình Vừa");
        translations["esp.high_model"] = U8("Mô Hình Cao");
        translations["esp.model_preview"] = U8("Xem Trước Mô Hình");
        translations["menu.export_obj"] = U8("Xuất Map (.OBJ)");
        translations["item.show_ray"] = U8("Hiện Tia Vật Thể");
        translations["item.esp_distance"] = U8("Khoảng Cách ESP");
        translations["color.ray"] = U8("Màu Tia Cột");

        // Misc / Items
        translations["header.item_switch"] = U8("Lọc Vật Phẩm");
        translations["item.esp_toggle"] = U8("Bật ESP Vật Phẩm");
        translations["item.stack_display"] = U8("Gộp Loại Vật Phẩm");
        translations["item.show_icons"] = U8("Hiện Icon");
        translations["header.item_groups"] = U8("Danh Mục");
        translations["item.rifle"] = U8("Súng Trường (AR)");
        translations["item.bolt_sniper"] = U8("Súng Tỉa (SR)");
        translations["item.dmr_sniper"] = U8("Súng Tỉa (DMR)");
        translations["item.machine_gun"] = U8("Súng Máy (LMG)");
        translations["item.shotgun"] = U8("Súng Shotgun");
        translations["item.pistol"] = U8("Súng Lục (HG)");
        translations["item.smg"] = U8("Tiểu Liên (SMG)");
        translations["item.attachments"] = U8("Phụ Kiện");
        translations["item.medicals"] = U8("Hồi Phục");
        translations["item.armor"] = U8("Giáp & Mũ");
        translations["item.ammo"] = U8("Đạn Dược");
        translations["item.throwables"] = U8("Vật Ném");
        translations["item.keys"] = U8("Chìa Khóa");
        translations["item.others"] = U8("Khác");
        translations["item.mortar"] = U8("Súng Cối (Mortar)");
        translations["item.panzer"] = U8("Súng Panzer");
        translations["item.smart_items"] = U8("Lọc Thông Minh");
        translations["header.smart_filter"] = U8("Cấu Hình Lọc Thông Minh");
        translations["smart.enable"] = U8("Bật Lọc Thông Minh");
        translations["smart.med_kit"] = U8("Giới Hạn Medkit");
        translations["smart.first_aid"] = U8("Giới Hạn First Aid");
        translations["smart.bandage"] = U8("Giới Hạn Băng Gạc");
        translations["smart.adrenaline"] = U8("Giới Hạn Adrenaline");
        translations["smart.painkiller"] = U8("Giới Hạn Thuốc Giảm Đau");
        translations["smart.energy_drink"] = U8("Giới Hạn Nước Tăng Lực");
        translations["smart.grenade"] = U8("Giới Hạn Lựu Đạn");
        translations["item.group_a_hotkey"] = U8("Phím Nóng Nhóm A");
        translations["item.group_b_hotkey"] = U8("Phím Nóng Nhóm B");
        translations["color.group_a"] = U8("Màu Nhóm A");
        translations["color.group_b"] = U8("Màu Nhóm B");
        translations["color.group_c"] = U8("Màu Nhóm C");
        translations["color.group_d"] = U8("Màu Nhóm D");

        // ESP Display Keys
        translations["esp.hitbox_highlight"] = U8("Tô Màu Xương");
        translations["esp.aimed_ray"] = U8("Tia Nhắm Mục Tiêu");
        translations["esp.hide_rank"] = U8("Ẩn Rank");
        translations["esp.solo_tpp"] = U8("Solo TPP");
        translations["esp.squad_tpp"] = U8("Squad TPP");
        translations["esp.solo_fpp"] = U8("Solo FPP");
        translations["esp.squad_fpp"] = U8("Squad FPP");
        translations["menu.vehicle_perspective"] = U8("Xe Cộ");
        translations["vehicle.show"] = U8("Hiện Xe");
        translations["vehicle.fuel"] = U8("Hiện Nhiên Liệu");
        translations["vehicle.health"] = U8("Hiện Máu Xe");
        translations["menu.airdrop_perspective"] = U8("Thính (Airdrop)");
        translations["airdrop.show"] = U8("Hiện Thính");
        translations["airdrop.show_items"] = U8("Hiện Đồ Trong Thính");
        translations["menu.box_perspective"] = U8("Hòm Xác");
        translations["lootbox.show"] = U8("Hiện Hòm Xác");
        translations["lootbox.show_items"] = U8("Hiện Đồ Trong Hòm");
        translations["header.esp_preview"] = U8("Xem Trước ESP");
        translations["menu.grenade_tip"] = U8("Dự Đoán Quỹ Đạo");
        translations["aim.instant_grenade"] = U8("Dự Đoán Tức Thì");
        translations["aim.mortarPredict"] = U8("Dự Đoán Cối");
        translations["aim.panzerPredict"] = U8("Dự Đoán Panzer");
        translations["aim.grenade"] = U8("Phím Vật Ném");
        translations["esp.prediction_time"] = U8("Bù trễ DMA (Prediction)");
        translations["esp.prediction_desc"] = U8("Điều chỉnh thời gian dự báo để xương khớp 100% với mục tiêu.\nMặc định: 0.025 (25ms)");

        // ESP Settings
        translations["header.base_setting"] = U8("Cài Đặt Tổng Quan");
        translations["header.colors"] = U8("Màu Sắc");
        translations["header.esp_switch"] = U8("Bật/Tắt ESP");
        translations["esp.switch"] = U8("Bật ESP");
        translations["esp.danger_warning"] = U8("Cảnh Báo Nguy Hiểm");
        translations["esp.text_shadow"] = U8("Đổ Bóng Chữ");
        translations["esp.hide_locked"] = U8("Khóa Không Hiện");
        translations["esp.visibility_check"] = U8("Check Vật Cản");
        translations["esp.lock_color_change"] = U8("Khóa Đổi Màu");
        translations["esp.distance_settings"] = U8("Khoảng Cách ESP");
        translations["esp.info_distance"] = U8("Khoảng Cách Info");
        translations["esp.bone_thickness"] = U8("Độ Dày Xương");
        translations["esp.ray_thickness"] = U8("Độ Dày Tia");
        translations["esp.stroke_size"] = U8("Độ Dày Chữ");
        translations["esp.health_bar"] = U8("Thanh Máu");
        translations["esp.bar_pos"] = U8("Vị Trí");
        translations["esp.bar_style"] = U8("Kiểu Dáng");
        translations["esp.bar_width"] = U8("Độ Rộng");
        translations["esp.bar_height"] = U8("Độ Dày");
        translations["esp.bar_alpha"] = U8("Độ Trong Suốt");
        translations["esp.style_standard"] = U8("Cơ Bản (Đổi Màu)");
        translations["esp.style_solid"] = U8("Màu Cố Định");
        translations["esp.style_wide_solid"] = U8("Kiểu Rộng (Một Màu)");
        translations["esp.style_wide_gradient"] = U8("Kiểu Rộng (Đổi Màu)");
        translations["esp.top_display"] = U8("Hiển Thị Trên");
        translations["esp.left_display"] = U8("Hiển Thị Trái");
        translations["esp.right_display"] = U8("Hiển Thị Phải");
        translations["esp.bottom_display"] = U8("Hiển Thị Dưới");
        translations["esp.player_box"] = U8("Khung Người");
        translations["esp.bones"] = U8("Xương Người");
        translations["esp.player_name"] = U8("Tên Người Chơi");
        translations["esp.team_id"] = U8("ID Teammate");
        translations["esp.clan_name"] = U8("Tên Clan");
        translations["esp.level"] = U8("Level");
        translations["esp.distance"] = U8("Khoảng Cách");
        translations["esp.health"] = U8("Lượng Máu");
        translations["esp.weapon"] = U8("Vũ Khí");
        translations["esp.kills"] = U8("Kills");
        translations["esp.damage"] = U8("Damage");
        translations["esp.spectate"] = U8("Người Xem");
        translations["esp.head_bones"] = U8("Xương Đầu");
        translations["esp.rank_icon"] = U8("Icon Rank");
        translations["esp.show_teammates"] = U8("Hiện Đồng Đội");

        // ESP Colors
        translations["color.visible_info"] = U8("Info Thấy");
        translations["color.visible_bones"] = U8("Xương Thấy");
        translations["color.cover_info"] = U8("Info Khuất");
        translations["color.cover_bones"] = U8("Xương Khuất");
        translations["color.bot_info"] = U8("Info Bot");
        translations["color.bot_bones"] = U8("Xương Bot");
        translations["color.danger_info"] = U8("Info Nguy Hiểm");
        translations["color.downed_info"] = U8("Info Bị Gục");
        translations["color.lock_color"] = U8("Màu Khóa");
        translations["color.ray_color"] = U8("Màu Tia");
        translations["color.blacklist_info"] = U8("Info List Đen");
        
        translations["proj.color_alert"] = U8("Màu FOV");
        translations["aim.range_color"] = U8("Màu Phạm Vi");
        translations["aim.preaim_color"] = U8("Màu Pre-Aim");

        // Recoil Settings
        translations["recoil.enable"] = U8("Bật Tự Động Giảm Giật");
        translations["recoil.red_dot"] = U8("Ống Ngắm Reddot");
        translations["recoil.x2"] = U8("Ống Ngắm X2");
        translations["recoil.x3"] = U8("Ống Ngắm X3");
        translations["recoil.x4"] = U8("Ống Ngắm X4");
        translations["recoil.x6"] = U8("Ống Ngắm X6");
        translations["recoil.x8"] = U8("Ống Ngắm X8");
        translations["recoil.delay_red_dot"] = U8("Độ Trễ Reddot (ms)");
        translations["recoil.delay_x2"] = U8("Độ Trễ X2 (ms)");
        translations["recoil.delay_x3"] = U8("Độ Trễ X3 (ms)");
        translations["recoil.delay_x4"] = U8("Độ Trễ X4 (ms)");
        translations["recoil.delay_x6"] = U8("Độ Trễ X6 (ms)");
        translations["recoil.delay_x8"] = U8("Độ Trễ X8 (ms)");

        // Vehicle Labels
        translations["vehicle.armor"] = U8("Giáp Xe");
        translations["vehicle.energy"] = U8("Năng Lượng");
    }
};