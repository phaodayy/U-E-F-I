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
        OverlayAssetAnimation::DrawStaticImage(tileDraw, icon,
            ImVec2(tileMin.x + tileSize.x * 0.5f, tileMin.y + 8.0f + iconTargetSize * 0.5f),
            iconTargetSize,
            IM_COL32(255, 255, 255, *item.enabled ? 255 : 226),
            1.0f);
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

void DrawKeyCombo(const char* label, int* keyValue) {
    const char* keyLabels[] = { "NONE", "MOUSE LEFT", "MOUSE RIGHT", "L-ALT", "L-SHIFT", "X", "V" };
    int keyVals[] = { 0, VK_LBUTTON, VK_RBUTTON, VK_LMENU, VK_LSHIFT, 'X', 'V' };
    int keyIdx = 0;
    for (int i = 0; i < IM_ARRAYSIZE(keyVals); ++i) {
        if (*keyValue == keyVals[i]) keyIdx = i;
    }
    ImGui::SetNextItemWidth(130);
    if (ImGui::Combo(label, &keyIdx, keyLabels, IM_ARRAYSIZE(keyLabels))) {
        *keyValue = keyVals[keyIdx];
    }
}

void DrawFlickTargetCombo(const char* label, int* targetPart) {
    const char* targetLabels[] = {
        "Auto Box", "Head", "Neck", "Chest", "Pelvis",
        "L Shoulder", "R Shoulder", "L Elbow", "R Elbow",
        "L Hand", "R Hand", "L Thigh", "R Thigh",
        "L Knee", "R Knee", "Feet"
    };
    *targetPart = std::clamp(*targetPart, 0, IM_ARRAYSIZE(targetLabels) - 1);
    ImGui::SetNextItemWidth(130);
    ImGui::Combo(label, targetPart, targetLabels, IM_ARRAYSIZE(targetLabels));
}

bool DrawCategoryRow(const FlickWeaponCatalog::Category& category, bool selected, bool enabled) {
    ImGui::PushID(category.key);

    const float rowHeight = 36.0f;
    const float rowWidth = ImGui::GetContentRegionAvail().x;
    const ImVec2 rowMin = ImGui::GetCursorScreenPos();
    const ImVec2 rowSize(rowWidth, rowHeight);
    const bool clicked = ImGui::InvisibleButton(skCrypt("##CategoryRow"), rowSize);
    const bool hovered = ImGui::IsItemHovered();
    const ImVec2 rowMax(rowMin.x + rowSize.x, rowMin.y + rowSize.y);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImU32 bg = selected ? IM_COL32(0, 150, 255, 54) :
        (hovered ? IM_COL32(0, 140, 210, 32) : IM_COL32(8, 22, 39, 70));
    const ImU32 border = selected ? IM_COL32(0, 210, 255, 165) : IM_COL32(70, 105, 145, 70);
    const ImU32 text = enabled ? IM_COL32(220, 240, 255, 245) : IM_COL32(140, 155, 175, 230);
    const ImU32 dot = enabled ? IM_COL32(0, 220, 255, 235) : IM_COL32(80, 95, 110, 210);

    draw->AddRectFilled(rowMin, rowMax, bg, 4.0f);
    draw->AddRect(rowMin, rowMax, border, 4.0f);
    draw->AddCircleFilled(ImVec2(rowMin.x + 15.0f, rowMin.y + rowHeight * 0.5f), 3.5f, dot);

    const ImVec2 textMin(rowMin.x + 32.0f, rowMin.y + 10.0f);
    const ImVec4 clip(rowMin.x + 30.0f, rowMin.y, rowMax.x - 10.0f, rowMax.y);
    draw->AddText(ImGui::GetFont(), 13.5f, textMin, text, category.label, nullptr, rowWidth - 40.0f, &clip);

    ImGui::PopID();
    return clicked;
}

} // namespace

void OverlayMenu::RenderTabPrecision(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    extern void UploadActiveLoaderConfigAsync();

    const float totalWidth = windowSize.x - 60.0f;
    const float cardHeight = (std::max)(windowSize.y - 190.0f, 420.0f);

    FlickWeaponCatalog::EnsureCategoryDefaults(g_Menu.flick_category_enabled);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_visible_only, g_Menu.flick_visible_only);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_shot_hold, g_Menu.flick_shot_hold);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_auto_shot, g_Menu.flick_auto_shot);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_follow_auto_shot, g_Menu.flick_follow_auto_shot);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_behavior_mode, g_Menu.flick_behavior_mode);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_target_part, g_Menu.flick_target_part);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_key, g_Menu.flick_key);
    FlickWeaponCatalog::EnsureCategoryFloatDefaults(g_Menu.flick_category_max_dist, g_Menu.flick_max_dist);
    FlickWeaponCatalog::EnsureCategoryMoveSpeedDefaults(g_Menu.flick_category_move_speed);
    FlickWeaponCatalog::EnsureCategoryFovDefaults(g_Menu.flick_category_fov, g_Menu.flick_fov);
    FlickWeaponCatalog::EnsureCategoryShotDelayDefaults(g_Menu.flick_category_shot_delay, g_Menu.flick_shot_delay);
    FlickWeaponCatalog::EnsureCategoryJitterDefaults(g_Menu.flick_category_jitter, g_Menu.flick_jitter);

    const auto& categories = FlickWeaponCatalog::Categories();
    g_Menu.flick_selected_category = std::clamp(
        g_Menu.flick_selected_category, 0, static_cast<int>(categories.size()) - 1);
    const auto& selectedCategory = categories[g_Menu.flick_selected_category];

    ImGui::Columns(3, skCrypt("FlickColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth * 0.28f);
    ImGui::SetColumnWidth(1, totalWidth * 0.38f);
    ImGui::SetColumnWidth(2, totalWidth * 0.34f);

    // --- Column 1: Category Selection ---
    BeginGlassCard(skCrypt("##FlickCategories"), Lang.WeaponCategories, ImVec2(totalWidth * 0.28f - 15, 0));
    
    if (ImGui::Button(Lang.All, ImVec2(70, 24))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(Lang.None, ImVec2(70, 24))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = false;
    }
    ImGui::Separator();

    ImGui::BeginChild(skCrypt("CategoryListInner"), ImVec2(-1, cardHeight - 110.0f), false);
    for (int i = 0; i < static_cast<int>(categories.size()); ++i) {
        const auto& category = categories[i];
        const bool selected = g_Menu.flick_selected_category == i;
        const bool enabled = g_Menu.flick_category_enabled[category.key];
        if (DrawCategoryRow(category, selected, enabled)) {
            g_Menu.flick_selected_category = i;
        }
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.CurrentWeapon);
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "%s",
        MacroEngine::current_weapon_name.empty() ? Lang.None : MacroEngine::current_weapon_name.c_str());
    
    ImGui::EndChild();

    ImGui::NextColumn();

    // --- Column 2: Specific Category Settings ---
    BeginGlassCard(skCrypt("##FlickSettings"), Lang.HeaderPrecisionSettings, ImVec2(totalWidth * 0.38f - 15, 0));

    bool& categoryEnabled = g_Menu.flick_category_enabled[selectedCategory.key];
    bool& visibleOnly = g_Menu.flick_category_visible_only[selectedCategory.key];
    bool& shotHold = g_Menu.flick_category_shot_hold[selectedCategory.key];
    bool& flickAutoShot = g_Menu.flick_category_auto_shot[selectedCategory.key];
    bool& followAutoShot = g_Menu.flick_category_follow_auto_shot[selectedCategory.key];
    int& behaviorMode = g_Menu.flick_category_behavior_mode[selectedCategory.key];
    int& targetPart = g_Menu.flick_category_target_part[selectedCategory.key];
    int& categoryKey = g_Menu.flick_category_key[selectedCategory.key];
    float& moveSpeed = g_Menu.flick_category_move_speed[selectedCategory.key];
    float& categoryFov = g_Menu.flick_category_fov[selectedCategory.key];
    float& maxDistance = g_Menu.flick_category_max_dist[selectedCategory.key];
    float& smoothness = g_Menu.flick_category_smoothness[selectedCategory.key];
    float& shotDelay = g_Menu.flick_category_shot_delay[selectedCategory.key];
    float& jitter = g_Menu.flick_category_jitter[selectedCategory.key];
    bool& fovCircle = g_Menu.flick_category_fov_circle[selectedCategory.key];

    ImGui::TextColored(ImVec4(0.0f, 0.7f, 1.0f, 1.0f), "%s", selectedCategory.label);
    ImGui::Separator();

    ImGui::TextDisabled("%s", Lang.HeaderSystemCore);
    ImGui::Checkbox(Lang.AimEnabled, &categoryEnabled);
    DrawKeyCombo(Lang.FlickKey, &categoryKey);
    OverlayHotkeys::DrawKeyBind(Lang.CaptureKey, &categoryKey, g_Menu.waiting_for_key);
    
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.HeaderAimStructure);
    ImGui::Checkbox(Lang.AimVisible, &visibleOnly);
    ImGui::Checkbox(Lang.FlickAutoShot, &flickAutoShot);
    ImGui::Checkbox(Lang.HoldUntilShot, &shotHold);
    ImGui::Checkbox(skCrypt("Lock Flick Target"), &g_Menu.flick_lock_target);
    
    ImGui::Spacing();
    ImGui::TextUnformatted(Lang.FlickMode);
    ImGui::RadioButton(Lang.ReturnAfterShot, &behaviorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton(Lang.FollowTarget, &behaviorMode, 1);
    
    if (behaviorMode == 1) {
        ImGui::Checkbox(Lang.AutoShotHold, &followAutoShot);
    }

    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.HeaderDangerScan);
    ImGui::SliderFloat(Lang.Distance, &maxDistance, 5.0f, 400.0f, skCrypt("%.0f m"));
    DrawFlickTargetCombo(Lang.Target, &targetPart);
    ImGui::SliderFloat(Lang.AimFOV, &categoryFov, 1.0f, 20.0f, skCrypt("%.0f"));
    ImGui::Checkbox(Lang.FOVCircle, &fovCircle);
    ImGui::SliderFloat(Lang.Speed, &moveSpeed, 0.0f, 100.0f, skCrypt("%.0f"));
    ImGui::SliderFloat(Lang.Smoothness, &smoothness, 0.0f, 100.0f, skCrypt("%.0f"));
    ImGui::SliderFloat(Lang.ShotDelay, &shotDelay, 0.0f, 1000.0f, skCrypt("%.0f ms"));
    ImGui::SliderFloat(Lang.Jitter, &jitter, 0.0f, 20.0f, skCrypt("%.1f"));

    ImGui::Separator();
    TextureInfo* preview = GetPreviewIcon(selectedCategory.folder, selectedCategory.asset);
    if (preview && preview->SRV) {
        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - 160) * 0.5f);
        ImGui::Image(preview->SRV, ImVec2(160, 80));
    }
    
    ImGui::EndChild();

    ImGui::NextColumn();

    // --- Column 3: Rules & Miscellaneous ---
    BeginGlassCard(skCrypt("##FlickRules"), Lang.FlickRules, ImVec2(totalWidth * 0.34f - 15, 0));
    
    ImGui::TextDisabled("%s", Lang.ShowcasePrecisionProfile);
    ImGui::TextWrapped(Lang.FlickRuleDesc);
    ImGui::Separator();
    
    ImGui::BulletText(Lang.FlickRule1);
    ImGui::BulletText(Lang.FlickRule2);
    ImGui::BulletText(Lang.FlickRule3);
    ImGui::BulletText(Lang.FlickRule4);
    ImGui::BulletText(Lang.FlickRule5);
    
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.FOVColor);
    ImGui::ColorEdit4(skCrypt("##FovCircleColor"), g_Menu.flick_fov_circle_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.HeaderTactical);
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);
    ImGui::Checkbox(skCrypt("Ping Aim Targeting"), &g_Menu.ping_aim_enabled);
    
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), skCrypt("HEAVY WEAPON - MORTAR"));
    DrawKeyCombo(skCrypt("Mortar Aim Key"), &g_Menu.mortar_aim_key);
    OverlayHotkeys::DrawKeyBind(skCrypt("Capture Aim Key"), &g_Menu.mortar_aim_key, g_Menu.waiting_for_key);
    ImGui::SliderFloat(skCrypt("Mortar FOV Strip"), &g_Menu.mortar_fov, 1.0f, 20.0f, skCrypt("%.0f"));
    
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::EndChild();

    ImGui::Columns(1);
}
