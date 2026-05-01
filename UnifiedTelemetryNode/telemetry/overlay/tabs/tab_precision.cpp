#include "../core/overlay_menu.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/flick_weapon_catalog.hpp"
#include "../core/overlay_hotkeys.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <protec/skCrypt.h>
#include <algorithm>
#include <vector>

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
    DrawKeyCombo(skCrypt("Primary Key"), &g_Menu.flick_key);
    DrawKeyCombo(skCrypt("Secondary Key"), &g_Menu.flick_key2);
    OverlayHotkeys::DrawKeyBind(skCrypt("Capture Primary"), &g_Menu.flick_key, g_Menu.waiting_for_key);
    ImGui::Separator();
    if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 35))) {
        g_Menu.SaveConfig("dataMacro/Config/settings.json");
        UploadActiveLoaderConfigAsync();
    }
    ImGui::TextDisabled("Current: %s", MacroEngine::current_weapon_name.empty() ? "None" : MacroEngine::current_weapon_name.c_str());
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##FlickWeapons"), skCrypt("FLICK CATEGORIES"), ImVec2(totalWidth / 3.0f - 20, 0));
    FlickWeaponCatalog::EnsureCategoryDefaults(g_Menu.flick_category_enabled);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_visible_only, g_Menu.flick_visible_only);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_shot_hold, g_Menu.flick_shot_hold);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_follow_auto_shot, g_Menu.flick_follow_auto_shot);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_behavior_mode, g_Menu.flick_behavior_mode);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_target_part, g_Menu.flick_target_part);
    FlickWeaponCatalog::EnsureCategoryMoveSpeedDefaults(g_Menu.flick_category_move_speed);
    FlickWeaponCatalog::EnsureCategoryFovDefaults(g_Menu.flick_category_fov, g_Menu.flick_fov);
    FlickWeaponCatalog::EnsureCategoryFloatDefaults(g_Menu.flick_category_max_dist, g_Menu.flick_max_dist);
    const auto& categories = FlickWeaponCatalog::Categories();
    g_Menu.flick_selected_category = std::clamp(
        g_Menu.flick_selected_category, 0, static_cast<int>(categories.size()) - 1);

    if (ImGui::Button(skCrypt("All"), ImVec2(74, 24))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("None"), ImVec2(74, 24))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = false;
    }
    ImGui::Separator();
    ImGui::Columns(2, skCrypt("FlickCategoryPicker"), false);
    ImGui::SetColumnWidth(0, 118.0f);
    for (int i = 0; i < static_cast<int>(categories.size()); ++i) {
        const auto& category = categories[i];
        const bool selected = g_Menu.flick_selected_category == i;
        const bool enabled = g_Menu.flick_category_enabled[category.key];
        ImGui::PushID(category.key);
        ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(0.75f, 0.93f, 1.0f, 1.0f) : ImVec4(0.55f, 0.62f, 0.70f, 1.0f));
        if (ImGui::Selectable(category.label, selected, 0, ImVec2(-1, 32))) {
            g_Menu.flick_selected_category = i;
        }
        ImGui::PopStyleColor();
        ImGui::PopID();
    }

    ImGui::NextColumn();
    const auto& selectedCategory = categories[g_Menu.flick_selected_category];
    bool& categoryEnabled = g_Menu.flick_category_enabled[selectedCategory.key];
    bool& visibleOnly = g_Menu.flick_category_visible_only[selectedCategory.key];
    bool& shotHold = g_Menu.flick_category_shot_hold[selectedCategory.key];
    bool& followAutoShot = g_Menu.flick_category_follow_auto_shot[selectedCategory.key];
    int& behaviorMode = g_Menu.flick_category_behavior_mode[selectedCategory.key];
    int& targetPart = g_Menu.flick_category_target_part[selectedCategory.key];
    float& moveSpeed = g_Menu.flick_category_move_speed[selectedCategory.key];
    float& categoryFov = g_Menu.flick_category_fov[selectedCategory.key];
    float& maxDistance = g_Menu.flick_category_max_dist[selectedCategory.key];
    behaviorMode = std::clamp(behaviorMode, 0, 1);
    targetPart = std::clamp(targetPart, 0, 15);
    moveSpeed = std::clamp(moveSpeed, 0.2f, 2.0f);
    categoryFov = std::clamp(categoryFov, 1.0f, 100.0f);
    maxDistance = std::clamp(maxDistance, 5.0f, 400.0f);
    ImGui::Text("%s", selectedCategory.label);
    ImGui::Separator();
    ImGui::Checkbox(skCrypt("Enabled"), &categoryEnabled);
    ImGui::Checkbox(skCrypt("Visible Only"), &visibleOnly);
    ImGui::Checkbox(skCrypt("Hold Until Shot"), &shotHold);
    ImGui::TextUnformatted(skCrypt("Flick Mode"));
    ImGui::RadioButton(skCrypt("Return After Shot"), &behaviorMode, 0);
    ImGui::RadioButton(skCrypt("Follow Target"), &behaviorMode, 1);
    if (behaviorMode == 1) {
        ImGui::Checkbox(skCrypt("Auto Shot While Hold"), &followAutoShot);
    }
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat(skCrypt("Max Distance"), &maxDistance, 5.0f, 400.0f, skCrypt("%.0f m"));
    DrawFlickTargetCombo(&targetPart);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat(skCrypt("Flick FOV"), &categoryFov, 1.0f, 100.0f, skCrypt("%.0f"));
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat(skCrypt("Move Speed"), &moveSpeed, 0.2f, 2.0f, skCrypt("%.2fx"));
    TextureInfo* preview = GetPreviewIcon(selectedCategory.folder, selectedCategory.asset);
    if (preview && preview->SRV) {
        ImGui::Spacing();
        ImGui::Image((ImTextureID)preview->SRV, ImVec2(58.0f, 58.0f));
    }
    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##FlickRules"), skCrypt("Flick Rules"), ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::TextWrapped(skCrypt("Flick runs once on key press, moves to target, then shoots."));
    ImGui::Separator();
    ImGui::BulletText(skCrypt("No release wait"));
    ImGui::BulletText(skCrypt("Move first, shot second"));
    ImGui::BulletText(skCrypt("Hold Until Shot can be disabled"));
    ImGui::BulletText(skCrypt("Return and Follow modes are exclusive"));
    ImGui::BulletText(skCrypt("Only selected weapon categories are allowed"));
    ImGui::Separator();
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);
    ImGui::EndChild();

    ImGui::Columns(1);
}
