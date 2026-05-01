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
    ImGui::TextUnformatted(label);
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo(skCrypt("##KeyCombo"), &keyIdx, keyLabels, IM_ARRAYSIZE(keyLabels))) {
        *keyValue = keyVals[keyIdx];
    }
    ImGui::PopID();
}

void DrawFlickTargetCombo(int* targetPart) {
    const char* targetLabels[] = {
        "Auto Box", "Head", "Neck", "Chest", "Pelvis",
        "L Shoulder", "R Shoulder", "L Elbow", "R Elbow",
        "L Hand", "R Hand", "L Thigh", "R Thigh",
        "L Knee", "R Knee", "Feet"
    };
    *targetPart = std::clamp(*targetPart, 0, IM_ARRAYSIZE(targetLabels) - 1);
    ImGui::TextUnformatted(skCrypt("Target Part"));
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo(skCrypt("##TargetPart"), targetPart, targetLabels, IM_ARRAYSIZE(targetLabels));
}

void DrawLabeledSliderFloat(const char* label, float* value, float minValue, float maxValue, const char* format) {
    ImGui::TextUnformatted(label);
    ImGui::PushID(label);
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat(skCrypt("##Slider"), value, minValue, maxValue, format);
    ImGui::PopID();
}

bool DrawCategoryRow(const FlickWeaponCatalog::Category& category, bool selected, bool enabled) {
    ImGui::PushID(category.key);

    const float rowHeight = 38.0f;
    const float rowWidth = (std::max)(ImGui::GetContentRegionAvail().x, 180.0f);
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
    draw->AddCircleFilled(ImVec2(rowMin.x + 15.0f, rowMin.y + rowHeight * 0.5f), 4.0f, dot);

    const ImVec2 textMin(rowMin.x + 30.0f, rowMin.y + 10.0f);
    const ImVec4 clip(rowMin.x + 30.0f, rowMin.y, rowMax.x - 10.0f, rowMax.y);
    draw->AddText(ImGui::GetFont(), 14.0f, textMin, text, category.label, nullptr, rowWidth - 40.0f, &clip);

    ImGui::PopID();
    return clicked;
}

} // namespace

void OverlayMenu::RenderTabPrecision(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    extern void UploadActiveLoaderConfigAsync();

    const float totalWidth = windowSize.x - 60.0f;
    const float spacing = 8.0f;
    const float colWidth = (totalWidth - (spacing * 2.0f)) / 3.0f;
    const float cardHeight = (std::max)(windowSize.y - 190.0f, 420.0f);

    FlickWeaponCatalog::EnsureCategoryDefaults(g_Menu.flick_category_enabled);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_visible_only, g_Menu.flick_visible_only);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_shot_hold, g_Menu.flick_shot_hold);
    FlickWeaponCatalog::EnsureCategoryBoolDefaults(g_Menu.flick_category_follow_auto_shot, g_Menu.flick_follow_auto_shot);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_behavior_mode, g_Menu.flick_behavior_mode);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_target_part, g_Menu.flick_target_part);
    FlickWeaponCatalog::EnsureCategoryIntDefaults(g_Menu.flick_category_key, g_Menu.flick_key);
    FlickWeaponCatalog::EnsureCategoryFloatDefaults(g_Menu.flick_category_max_dist, g_Menu.flick_max_dist);
    FlickWeaponCatalog::EnsureCategoryMoveSpeedDefaults(g_Menu.flick_category_move_speed);
    FlickWeaponCatalog::EnsureCategoryFovDefaults(g_Menu.flick_category_fov, g_Menu.flick_fov);

    const auto& categories = FlickWeaponCatalog::Categories();
    g_Menu.flick_selected_category = std::clamp(
        g_Menu.flick_selected_category, 0, static_cast<int>(categories.size()) - 1);
    const auto& selectedCategory = categories[g_Menu.flick_selected_category];

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(spacing, 0));

    ImGui::BeginGroup();
    BeginGlassCard(skCrypt("##FlickCategories"), skCrypt("WEAPON CATEGORIES"), ImVec2(colWidth - 10.0f, cardHeight));
    if (ImGui::Button(skCrypt("All"), ImVec2(colWidth * 0.35f, 24.0f))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("None"), ImVec2(colWidth * 0.35f, 24.0f))) {
        for (const auto& category : categories) g_Menu.flick_category_enabled[category.key] = false;
    }
    ImGui::Separator();

    ImGui::BeginChild(skCrypt("CategoryListInner"), ImVec2(-1, -32.0f), false);
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
    ImGui::TextDisabled("Current: %s",
        MacroEngine::current_weapon_name.empty() ? "None" : MacroEngine::current_weapon_name.c_str());
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();
    BeginGlassCard(skCrypt("##FlickCategorySettings"), skCrypt("CATEGORY SETTING"), ImVec2(colWidth - 10.0f, cardHeight));

    bool& categoryEnabled = g_Menu.flick_category_enabled[selectedCategory.key];
    bool& visibleOnly = g_Menu.flick_category_visible_only[selectedCategory.key];
    bool& shotHold = g_Menu.flick_category_shot_hold[selectedCategory.key];
    bool& followAutoShot = g_Menu.flick_category_follow_auto_shot[selectedCategory.key];
    int& behaviorMode = g_Menu.flick_category_behavior_mode[selectedCategory.key];
    int& targetPart = g_Menu.flick_category_target_part[selectedCategory.key];
    int& categoryKey = g_Menu.flick_category_key[selectedCategory.key];
    float& moveSpeed = g_Menu.flick_category_move_speed[selectedCategory.key];
    float& categoryFov = g_Menu.flick_category_fov[selectedCategory.key];
    float& maxDistance = g_Menu.flick_category_max_dist[selectedCategory.key];

    behaviorMode = std::clamp(behaviorMode, 0, 1);
    targetPart = std::clamp(targetPart, 0, 15);
    categoryKey = std::clamp(categoryKey, 0, 0xFE);
    moveSpeed = std::clamp(moveSpeed, 0.2f, 2.0f);
    categoryFov = std::clamp(categoryFov, 1.0f, 100.0f);
    maxDistance = std::clamp(maxDistance, 5.0f, 400.0f);

    ImGui::Text("%s", selectedCategory.label);
    ImGui::Separator();
    ImGui::Checkbox(skCrypt("Enabled"), &categoryEnabled);
    DrawKeyCombo(skCrypt("Flick Key"), &categoryKey);
    OverlayHotkeys::DrawKeyBind(skCrypt("Capture Key"), &categoryKey, g_Menu.waiting_for_key);
    ImGui::Checkbox(skCrypt("Visible Only"), &visibleOnly);
    ImGui::Checkbox(skCrypt("Hold Until Shot"), &shotHold);
    ImGui::TextUnformatted(skCrypt("Flick Mode"));
    ImGui::RadioButton(skCrypt("Return After Shot"), &behaviorMode, 0);
    ImGui::RadioButton(skCrypt("Follow Target"), &behaviorMode, 1);
    if (behaviorMode == 1) {
        ImGui::Checkbox(skCrypt("Auto Shot While Hold"), &followAutoShot);
    }

    DrawLabeledSliderFloat(skCrypt("Max Distance"), &maxDistance, 5.0f, 400.0f, skCrypt("%.0f m"));
    DrawFlickTargetCombo(&targetPart);
    DrawLabeledSliderFloat(skCrypt("Flick FOV"), &categoryFov, 1.0f, 100.0f, skCrypt("%.0f"));
    DrawLabeledSliderFloat(skCrypt("Move Speed"), &moveSpeed, 0.2f, 2.0f, skCrypt("%.2f x"));

    TextureInfo* preview = GetPreviewIcon(selectedCategory.folder, selectedCategory.asset);
    if (preview && preview->SRV) {
        ImGui::Spacing();
        ImGui::Image(preview->SRV, ImVec2(160, 80));
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();
    BeginGlassCard(skCrypt("##FlickRules"), skCrypt("FLICK RULES"), ImVec2(colWidth - 10.0f, cardHeight));
    ImGui::TextWrapped(skCrypt("Flick runs once on key press, moves to target, then shoots."));
    ImGui::Separator();
    ImGui::BulletText(skCrypt("No release wait"));
    ImGui::BulletText(skCrypt("Move first, shot second"));
    ImGui::BulletText(skCrypt("Hold Until Shot can be disabled"));
    ImGui::BulletText(skCrypt("Return and Follow modes are exclusive"));
    ImGui::BulletText(skCrypt("Each category uses its own flick key"));
    ImGui::Separator();
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);
    ImGui::Separator();
    if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 35.0f))) {
        g_Menu.SaveConfig("dataMacro/Config/settings.json");
        UploadActiveLoaderConfigAsync();
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::PopStyleVar();
}
