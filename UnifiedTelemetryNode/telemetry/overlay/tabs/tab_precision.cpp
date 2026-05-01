#include "../core/overlay_menu.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/overlay_hotkeys.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <protec/skCrypt.h>
#include <algorithm>

namespace {

struct FlickWeaponTile {
    const char* label;
    const char* folder;
    const char* asset;
    bool* enabled;
};

void DrawFlickWeaponTile(const FlickWeaponTile& item, const ImVec2& tileSize, const Translation::Strings& lang) {
    ImGui::PushID(item.asset);
    ImVec2 tileMin = ImGui::GetCursorScreenPos();
    ImVec2 tileMax(tileMin.x + tileSize.x, tileMin.y + tileSize.y);

    if (ImGui::InvisibleButton(skCrypt("##FlickWeaponTile"), tileSize)) {
        *item.enabled = !*item.enabled;
    }

    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* tileDraw = ImGui::GetWindowDrawList();
    ImU32 bg = *item.enabled ? IM_COL32(0, 150, 255, 55) : IM_COL32(12, 24, 45, 135);
    ImU32 border = *item.enabled ? IM_COL32(0, 210, 255, 190) : IM_COL32(80, 110, 150, 85);
    if (hovered) bg = IM_COL32(0, 190, 255, 75);

    tileDraw->AddRectFilled(tileMin, tileMax, bg, 8.0f);
    tileDraw->AddRect(tileMin, tileMax, border, 8.0f, 0, *item.enabled ? 1.6f : 1.0f);

    TextureInfo* icon = GetPreviewIcon(item.folder, item.asset);
    const float iconTargetSize = 42.0f;
    if (icon && icon->SRV && icon->Width > 0 && icon->Height > 0) {
        OverlayAssetAnimation::DrawOptions anim{};
        anim.hovered = hovered;
        anim.selected = *item.enabled;
        anim.important = *item.enabled;
        anim.strength = hovered || *item.enabled ? 1.18f : 0.86f;
        OverlayAssetAnimation::DrawAnimatedImage(tileDraw, icon,
            ImVec2(tileMin.x + tileSize.x * 0.5f, tileMin.y + 8.0f + iconTargetSize * 0.5f),
            iconTargetSize,
            IM_COL32(255, 255, 255, *item.enabled ? 255 : 226),
            anim);
    }

    ImVec4 textClip(tileMin.x + 6.0f, tileMin.y + 56.0f, tileMax.x - 6.0f, tileMax.y - 6.0f);
    tileDraw->AddText(ImGui::GetFont(), 13.0f, ImVec2(textClip.x, textClip.y),
        IM_COL32(225, 238, 255, 235), item.label, nullptr, tileSize.x - 12.0f, &textClip);

    if (*item.enabled) {
        tileDraw->AddCircleFilled(ImVec2(tileMax.x - 12.0f, tileMin.y + 12.0f), 6.0f, IM_COL32(0, 220, 255, 230));
    }

    if (hovered) {
        ImGui::SetTooltip("%s", *item.enabled ? lang.ItemCatalogSelected : lang.ItemCatalogHint);
    }

    ImGui::PopID();
}

void DrawFlickWeaponGrid(FlickWeaponTile* items, int count) {
    const auto lang = Translation::Get();
    const float tileGap = 10.0f;
    const float tileHeight = 88.0f;
    const int perRow = 3;
    float available = ImGui::GetContentRegionAvail().x;
    float tileWidth = (available - (perRow - 1) * tileGap) / perRow;
    if (tileWidth > 110.0f) tileWidth = 110.0f;
    if (tileWidth < 80.0f) tileWidth = 80.0f;

    for (int i = 0; i < count; ++i) {
        DrawFlickWeaponTile(items[i], ImVec2(tileWidth, tileHeight), lang);
        if ((i % perRow) != (perRow - 1) && i != count - 1) {
            ImGui::SameLine(0, tileGap);
        }
    }
}

void DrawKeyCombo(const char* label, int* keyValue) {
    const char* keyLabels[] = { "NONE", "MOUSE LEFT", "MOUSE RIGHT", "L-ALT", "L-SHIFT", "X", "V" };
    int keyVals[] = { 0, VK_LBUTTON, VK_RBUTTON, VK_LMENU, VK_LSHIFT, 'X', 'V' };
    int keyIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(keyVals); ++i) {
        if (*keyValue == keyVals[i]) keyIdx = i;
    }
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo(label, &keyIdx, keyLabels, IM_ARRAYSIZE(keyLabels))) {
        *keyValue = keyVals[keyIdx];
    }
}

void DrawFlickTargetCombo(int* targetPart) {
    const char* targetLabels[] = {
        "Auto Box", "Head", "Neck", "Chest", "Pelvis",
        "L Shoulder", "R Shoulder", "L Elbow", "R Elbow",
        "L Hand", "R Hand", "L Thigh", "R Thigh",
        "L Knee", "R Knee", "Feet"
    };
    *targetPart = std::clamp(*targetPart, 0, IM_ARRAYSIZE(targetLabels) - 1);
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo(skCrypt("Target Part"), targetPart, targetLabels, IM_ARRAYSIZE(targetLabels));
}

} // namespace

void OverlayMenu::RenderTabPrecision(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    extern void UploadActiveLoaderConfigAsync();
    const float totalWidth = windowSize.x - 60;

    ImGui::Columns(3, skCrypt("FlickSettingColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 3.0f);
    ImGui::SetColumnWidth(1, totalWidth / 3.0f);
    ImGui::SetColumnWidth(2, totalWidth / 3.0f);

    BeginGlassCard(skCrypt("##FlickCore"), skCrypt("FLICK SETTING"), ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::Checkbox(skCrypt("Enable Flick"), &g_Menu.flick_enabled);
    ImGui::Checkbox(skCrypt("Visible Only"), &g_Menu.flick_visible_only);
    ImGui::Checkbox(skCrypt("Hold Until Shot"), &g_Menu.flick_shot_hold);
    g_Menu.flick_behavior_mode = std::clamp(g_Menu.flick_behavior_mode, 0, 1);
    ImGui::TextUnformatted(skCrypt("Flick Mode"));
    ImGui::RadioButton(skCrypt("Return After Shot"), &g_Menu.flick_behavior_mode, 0);
    ImGui::RadioButton(skCrypt("Follow Target"), &g_Menu.flick_behavior_mode, 1);
    g_Menu.flick_return = (g_Menu.flick_behavior_mode == 0);
    ImGui::Separator();
    DrawKeyCombo(skCrypt("Primary Key"), &g_Menu.flick_key);
    DrawKeyCombo(skCrypt("Secondary Key"), &g_Menu.flick_key2);
    OverlayHotkeys::DrawKeyBind(skCrypt("Capture Primary"), &g_Menu.flick_key, g_Menu.waiting_for_key);
    ImGui::Separator();
    ImGui::SliderFloat(skCrypt("Flick FOV"), &g_Menu.flick_fov, 1.0f, 100.0f, skCrypt("%.0f"));
    ImGui::SliderFloat(skCrypt("Max Distance"), &g_Menu.flick_max_dist, 5.0f, 400.0f, skCrypt("%.0f m"));
    DrawFlickTargetCombo(&g_Menu.flick_target_part);
    ImGui::Separator();
    if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 35))) {
        g_Menu.SaveConfig("dataMacro/Config/settings.json");
        UploadActiveLoaderConfigAsync();
    }
    ImGui::TextDisabled("Current: %s", MacroEngine::current_weapon_name.empty() ? "None" : MacroEngine::current_weapon_name.c_str());
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##FlickWeapons"), skCrypt("FLICK WEAPONS"), ImVec2(totalWidth / 3.0f - 20, 0));
    FlickWeaponTile weaponTiles[] = {
        { skCrypt("S686"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_Berreta686_C"), &g_Menu.flick_weapon_s686 },
        { skCrypt("S12K"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_Saiga12_C"), &g_Menu.flick_weapon_s12k },
        { skCrypt("S1897"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_Winchester_C"), &g_Menu.flick_weapon_s1897 },
        { skCrypt("DBS"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_DP12_C"), &g_Menu.flick_weapon_dbs },
        { skCrypt("O12"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_OriginS12_C"), &g_Menu.flick_weapon_o12 },
        { skCrypt("SLR"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_FNFal_C"), &g_Menu.flick_weapon_slr },
        { skCrypt("Mini14"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mini14_C"), &g_Menu.flick_weapon_mini14 },
        { skCrypt("SKS"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_SKS_C"), &g_Menu.flick_weapon_sks },
        { skCrypt("VSS"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_VSS_C"), &g_Menu.flick_weapon_vss },
        { skCrypt("QBU"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_QBU88_C"), &g_Menu.flick_weapon_qbu },
        { skCrypt("Kar98k"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Kar98k_C"), &g_Menu.flick_weapon_kar98k },
        { skCrypt("M24"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_M24_C"), &g_Menu.flick_weapon_m24 },
        { skCrypt("AWM"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_AWM_C"), &g_Menu.flick_weapon_awm },
        { skCrypt("Lynx AMR"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_L6_C"), &g_Menu.flick_weapon_lynx },
        { skCrypt("Win94"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Win1894_C"), &g_Menu.flick_weapon_win94 },
        { skCrypt("Mosin"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Mosin_C"), &g_Menu.flick_weapon_mosin },
        { skCrypt("Panzer"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_PanzerFaust100M_C"), &g_Menu.flick_weapon_panzerfaust },
        { skCrypt("Mk12"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mk12_C"), &g_Menu.flick_weapon_mk12 },
        { skCrypt("MK 14"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mk14_C"), &g_Menu.flick_weapon_mk14 },
        { skCrypt("Dragunov"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Dragunov_C"), &g_Menu.flick_weapon_dragunov }
    };
    if (ImGui::Button(skCrypt("All"), ImVec2(74, 24))) {
        for (auto& item : weaponTiles) *item.enabled = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("None"), ImVec2(74, 24))) {
        for (auto& item : weaponTiles) *item.enabled = false;
    }
    ImGui::Separator();
    DrawFlickWeaponGrid(weaponTiles, IM_ARRAYSIZE(weaponTiles));
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##FlickRules"), skCrypt("Flick Rules"), ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::TextWrapped(skCrypt("Flick runs once on key press, moves to target, then shoots."));
    ImGui::Separator();
    ImGui::BulletText(skCrypt("No release wait"));
    ImGui::BulletText(skCrypt("Move first, shot second"));
    ImGui::BulletText(skCrypt("Hold Until Shot can be disabled"));
    ImGui::BulletText(skCrypt("Return and Follow modes are exclusive"));
    ImGui::BulletText(skCrypt("Only selected weapon tiles are allowed"));
    ImGui::Separator();
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);
    ImGui::EndChild();

    ImGui::Columns(1);
}
