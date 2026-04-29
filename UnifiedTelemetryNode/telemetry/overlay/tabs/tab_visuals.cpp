#include "../core/overlay_menu.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/overlay_health_bar.hpp"
#include "../core/overlay_hotkeys.hpp"
#include "../core/overlay_presets.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../player/player_esp_layout.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>
#include <algorithm>
#include <cmath>
#include <map>

void OverlayMenu::RenderTabVisuals(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(4, skCrypt("ESPColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 4.0f);
    ImGui::SetColumnWidth(1, totalWidth / 4.0f);
    ImGui::SetColumnWidth(2, totalWidth / 4.0f);
    ImGui::SetColumnWidth(3, totalWidth / 4.0f);

    // Column 1: visual core
    BeginGlassCard(skCrypt("##ESPCol1"), Lang.HeaderVisualCore, ImVec2(totalWidth / 4.0f - 15, 0));
    ImGui::Checkbox(Lang.MasterToggle, &g_Menu.esp_toggle);
    ImGui::Checkbox(Lang.ESP_Icons, &g_Menu.esp_icons);
    ImGui::Checkbox(Lang.ESP_Offscreen, &g_Menu.esp_offscreen);
    ImGui::Checkbox(Lang.VisCheck, &g_Menu.aim_visible_only);
    ImGui::Checkbox(Lang.ESP_Spectated, &g_Menu.esp_spectated);
    ImGui::Checkbox(Lang.ESP_SpectatorList, &g_Menu.esp_spectator_list);
    ImGui::Checkbox(Lang.PlayerList, &g_Menu.player_list_enabled);
    ImGui::Checkbox(Lang.HoldPanelKey, &g_Menu.player_list_hold_required);
    OverlayHotkeys::DrawKeyBind(Lang.PanelKey, &g_Menu.player_list_hold_key, g_Menu.waiting_for_key);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseVisualProfile);
    if (ImGui::Button(skCrypt("Clean"), ImVec2(70, 24))) OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Clean);
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("Loot"), ImVec2(70, 24))) OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Loot);
    if (ImGui::Button(skCrypt("Combat"), ImVec2(70, 24))) OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Combat);
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("Debug"), ImVec2(70, 24))) OverlayPresets::Apply(g_Menu, OverlayPresets::Preset::Debug);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseRankLayout);
    ImGui::Checkbox(Lang.Rank, &g_Menu.esp_rank);
    ImGui::SameLine();
    ImGui::Checkbox(Lang.SurvivalLevel, &g_Menu.esp_survival_level);
    ImGui::Checkbox(Lang.KillCount, &g_Menu.esp_killcount);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseMarkerLayout);
    ImGui::Checkbox(Lang.TeamID, &g_Menu.esp_teamid);
    ImGui::SameLine();
    ImGui::Checkbox(Lang.Snaplines, &g_Menu.esp_snapline);
    ImGui::Checkbox(skCrypt("Compact Dots"), &g_Menu.esp_skeleton_dots);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseEspThreatBands);
    ImGui::Checkbox(skCrypt("Aim"), &g_Menu.esp_aim_warning);
    ImGui::SameLine();
    ImGui::Checkbox(skCrypt("Close"), &g_Menu.esp_close_warning);
    ImGui::Checkbox(skCrypt("Status"), &g_Menu.esp_status_badges);
    ImGui::SameLine();
    ImGui::Checkbox(skCrypt("View"), &g_Menu.esp_view_direction);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseEspTeamColors);
    ImGui::Checkbox(skCrypt("Custom Team Colors"), &g_Menu.team_color_custom);
    for (int i = 0; i < 4; ++i) {
        ImGui::PushID(i);
        ImGui::ColorEdit4(skCrypt("##TeamCustom"), g_Menu.team_custom_colors[i],
            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        if (i < 3) ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::NextColumn();
    // Column 2: render style and HUD
    BeginGlassCard(skCrypt("##ESPCol2"), Lang.HeaderRenderStyle, ImVec2(totalWidth / 4.0f - 15, 0));
    ImGui::Checkbox(Lang.Box, &g_Menu.esp_box);
    ImGui::SetNextItemWidth(130);
    const char* boxStyleItems[] = { skCrypt("Full"), skCrypt("Corner") };
    ImGui::Combo(skCrypt("Box Style"), &g_Menu.esp_box_type, boxStyleItems, IM_ARRAYSIZE(boxStyleItems));
    ImGui::SliderFloat(Lang.BoxThick, &g_Menu.box_thickness, 1.0f, 5.0f, skCrypt("%.1f px"));
    ImGui::Checkbox(skCrypt("Fill Box"), &g_Menu.esp_fillbox);
    ImGui::Checkbox(Lang.Skeleton, &g_Menu.esp_skeleton);
    ImGui::Checkbox(Lang.HealthBar, &g_Menu.esp_health);
    ImGui::SetNextItemWidth(130);
    const char* healthDisplayItems[] = { Lang.HealthColumn, Lang.HealthTextOnly };
    ImGui::Combo(Lang.HealthDisplay, &g_Menu.esp_health_display_mode,
        healthDisplayItems, IM_ARRAYSIZE(healthDisplayItems));
    if (g_Menu.esp_health_display_mode == 0) {
        ImGui::SetNextItemWidth(130);
        const char* healthStyleItems[] = {
            skCrypt("Pill"),
            skCrypt("Segment"),
            skCrypt("Glass"),
            skCrypt("Tactical"),
            skCrypt("Pulse")
        };
        ImGui::Combo(Lang.HealthBarStyle, &g_Menu.esp_health_bar_style,
            healthStyleItems, IM_ARRAYSIZE(healthStyleItems));
    }
    ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
    ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
    ImGui::Checkbox(Lang.TeamID, &g_Menu.esp_teamid);
    ImGui::Checkbox(Lang.KillCount, &g_Menu.esp_killcount);
    ImGui::Checkbox(Lang.Damage, &g_Menu.esp_damage);
    ImGui::Checkbox(Lang.Ammo, &g_Menu.esp_ammo);
    ImGui::Checkbox(Lang.Speed, &g_Menu.esp_speed);
    ImGui::Checkbox(Lang.Rank, &g_Menu.esp_rank);
    ImGui::Checkbox(Lang.SurvivalLevel, &g_Menu.esp_survival_level);
    ImGui::Checkbox(Lang.HeadCircle, &g_Menu.esp_head_circle);
    ImGui::Checkbox(Lang.Snaplines, &g_Menu.esp_snapline);
    ImGui::Checkbox(Lang.AimWarn, &g_Menu.esp_aim_warning);
    ImGui::Checkbox(Lang.ViewRay, &g_Menu.esp_view_direction);
    ImGui::Checkbox(skCrypt("Status Badges"), &g_Menu.esp_status_badges);
    ImGui::Checkbox(skCrypt("Close Warning"), &g_Menu.esp_close_warning);
    ImGui::Checkbox(skCrypt("Offscreen Text"), &g_Menu.esp_offscreen_text);
    ImGui::Separator();
    ImGui::Checkbox(Lang.ShowcaseEspNameplates, &g_Menu.esp_multilayer_nameplate);
    ImGui::SameLine();
    ImGui::Checkbox(skCrypt("BG"), &g_Menu.esp_text_background);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Column 3: thresholds and colors
    BeginGlassCard(skCrypt("##ESPCol3"), Lang.HeaderOverlayHUD, ImVec2(totalWidth / 4.0f - 15, 0));
    ImGui::SliderFloat(Lang.FontSize, &g_Menu.esp_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::Checkbox(skCrypt("Text BG"), &g_Menu.esp_text_background);
    ImGui::SliderFloat(skCrypt("Text BG Alpha"), &g_Menu.esp_text_bg_alpha, 0.0f, 0.70f, skCrypt("%.2f"));
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.DistThresholds);
    ImGui::SliderInt(Lang.RenderDist, &g_Menu.render_distance, 50, 1000, skCrypt("%d m"));
    ImGui::SliderInt(Lang.InfoESP, &g_Menu.name_max_dist, 50, 600, skCrypt("%d m"));
    ImGui::SliderFloat(skCrypt("Thick"), &g_Menu.skel_thickness, 1.0f, 5.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("View Dir"), &g_Menu.esp_view_direction_length, 5.0f, 120.0f, skCrypt("%.0f m"));
    ImGui::SliderFloat(skCrypt("Close Warn"), &g_Menu.esp_close_warning_distance, 10.0f, 150.0f, skCrypt("%.0f m"));

    ImGui::SetNextItemWidth(120);
    const char* snapItems[] = { skCrypt("Bottom"), skCrypt("Center"), skCrypt("Top"), skCrypt("Local") };
    ImGui::Combo(skCrypt("Snap From"), &g_Menu.snapline_type, snapItems, IM_ARRAYSIZE(snapItems));

    ImGui::Separator();
    ImGui::SliderFloat(skCrypt("Name Size"), &g_Menu.name_font_size, 8.0f, 26.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Distance Size"), &g_Menu.distance_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Weapon Size"), &g_Menu.weapon_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Weapon Icon"), &g_Menu.weapon_icon_size, 18.0f, 90.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(skCrypt("Rank Size"), &g_Menu.rank_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Team Badge"), &g_Menu.teamid_badge_size, 10.0f, 34.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(skCrypt("Kills Size"), &g_Menu.kill_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Damage Size"), &g_Menu.damage_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Ammo Size"), &g_Menu.ammo_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Speed Size"), &g_Menu.speed_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Level Size"), &g_Menu.survival_level_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(skCrypt("Watch Size"), &g_Menu.spectated_font_size, 8.0f, 28.0f, skCrypt("%.1f px"));

    ImGui::SetNextItemWidth(120);
    const char* modeItems[] = { Lang.ModeDynamic, Lang.ModeStatic };
    ImGui::Combo(Lang.HealthMode, &g_Menu.esp_health_color_mode, modeItems, IM_ARRAYSIZE(modeItems));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto ColorPicker = [&](const char* label, float* col1, float* col2 = nullptr) {
        ImGui::PushID(label);
        ImGui::TextDisabled(label);
        ImGui::SameLine(100);
        ImGui::ColorEdit4(skCrypt("##Col1"), col1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        if (col2) {
            ImGui::SameLine();
            ImGui::ColorEdit4(skCrypt("##Col2"), col2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        }
        ImGui::PopID();
    };

    ColorPicker(Lang.Box, g_Menu.box_visible_color, g_Menu.box_invisible_color);
    ColorPicker(Lang.Skeleton, g_Menu.skeleton_visible_color, g_Menu.skeleton_invisible_color);
    ColorPicker(Lang.Name, g_Menu.name_visible_color, g_Menu.name_invisible_color);
    ColorPicker(Lang.Distance, g_Menu.distance_color);
    ColorPicker(Lang.Weapon, g_Menu.weapon_color);
    ColorPicker(Lang.Rank, g_Menu.rank_color);
    ColorPicker(Lang.TeamID, g_Menu.teamid_color);
    ColorPicker(Lang.KillCount, g_Menu.kill_color);
    ColorPicker(Lang.Damage, g_Menu.damage_color);
    ColorPicker(Lang.Ammo, g_Menu.ammo_color);
    ColorPicker(Lang.Speed, g_Menu.speed_color);
    ColorPicker(Lang.SurvivalLevel, g_Menu.survival_level_color);
    ColorPicker(Lang.ESP_Spectated, g_Menu.spectated_color);
    ColorPicker(Lang.AimWarn, g_Menu.aim_warning_color);
    ColorPicker(Lang.CloseWarn, g_Menu.close_warning_color);
    ColorPicker(Lang.ViewRay, g_Menu.view_direction_color);
    ColorPicker(Lang.ColorHealth, g_Menu.health_color);

    ImGui::EndChild();

    ImGui::NextColumn();
    // Column 4: preview
    BeginGlassCard(skCrypt("##ESPCol4"), Lang.PreviewPanel, ImVec2(totalWidth / 4.0f - 15, 0));

    static bool bPreviewOccluded = false;
    static int previewHealthPercent = 100; // Int slider for better control (stops snapping to 0/1)

    ImGui::Checkbox(Lang.SimulateWall, &bPreviewOccluded);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderInt(skCrypt("##HP"), &previewHealthPercent, 0, 100, skCrypt("%d%% HP"));
    ImGui::Spacing();

    float previewHealthSim = (float)previewHealthPercent / 100.0f;

    float cardW = totalWidth / 4.0f - 15;
    float previewW = cardW * 0.55f;
    float previewH = previewW * 2.3f;

    ImGui::Dummy(ImVec2(0.0f, previewH * 0.25f));

    // Centering logic
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cardW - previewW) / 2.0f);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (PreviewInstructor.SRV) {
        ImGui::InvisibleButton(skCrypt("##PreviewInstructorImage"), ImVec2(previewW, previewH));
        ImDrawList* draw = ImGui::GetWindowDrawList();
        OverlayAssetAnimation::DrawStaticImageRect(draw, &PreviewInstructor,
            cursorPos,
            ImVec2(cursorPos.x + previewW, cursorPos.y + previewH),
            IM_COL32(255, 255, 255, 255),
            bPreviewOccluded ? 0.40f : 1.0f);

        // --- Wall Simulation Effect ---
        if (bPreviewOccluded) {
            draw->AddRectFilled(ImVec2(cursorPos.x - 5, cursorPos.y + previewH * 0.2f),
                                ImVec2(cursorPos.x + previewW + 5, cursorPos.y + previewH * 0.8f),
                                IM_COL32(50, 50, 50, 180), 4.0f); // Semi-transparent wall
        }

        // 1. Box Demo
        if (g_Menu.esp_box) {
            float* targetCol = bPreviewOccluded ? g_Menu.box_invisible_color : g_Menu.box_visible_color;
            ImU32 uBoxCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));
            if (g_Menu.esp_box_type == 1) {
                const float len = (std::min)(previewW, previewH) * 0.24f;
                const ImVec2 tl = cursorPos;
                const ImVec2 tr(cursorPos.x + previewW, cursorPos.y);
                const ImVec2 bl(cursorPos.x, cursorPos.y + previewH);
                const ImVec2 br(cursorPos.x + previewW, cursorPos.y + previewH);
                draw->AddLine(tl, ImVec2(tl.x + len, tl.y), uBoxCol, 2.0f);
                draw->AddLine(tl, ImVec2(tl.x, tl.y + len), uBoxCol, 2.0f);
                draw->AddLine(tr, ImVec2(tr.x - len, tr.y), uBoxCol, 2.0f);
                draw->AddLine(tr, ImVec2(tr.x, tr.y + len), uBoxCol, 2.0f);
                draw->AddLine(bl, ImVec2(bl.x + len, bl.y), uBoxCol, 2.0f);
                draw->AddLine(bl, ImVec2(bl.x, bl.y - len), uBoxCol, 2.0f);
                draw->AddLine(br, ImVec2(br.x - len, br.y), uBoxCol, 2.0f);
                draw->AddLine(br, ImVec2(br.x, br.y - len), uBoxCol, 2.0f);
            } else {
                draw->AddRect(cursorPos, ImVec2(cursorPos.x + previewW, cursorPos.y + previewH), uBoxCol, 4.0f, 0, 2.0f);
            }
        }

        PlayerEspLayout::Stack previewLayout(
            cursorPos.x,
            cursorPos.y,
            cursorPos.x + previewW,
            cursorPos.y + previewH);

        auto DrawPreviewDropZones = [&]() {
            const ImU32 zoneCol = IM_COL32(0, 220, 255, 34);
            const ImU32 lineCol = IM_COL32(0, 220, 255, 115);
            const float band = 22.0f;
            draw->AddRectFilled(ImVec2(cursorPos.x - band, cursorPos.y),
                ImVec2(cursorPos.x - 4.0f, cursorPos.y + previewH), zoneCol, 5.0f);
            draw->AddRectFilled(ImVec2(cursorPos.x + previewW + 4.0f, cursorPos.y),
                ImVec2(cursorPos.x + previewW + band, cursorPos.y + previewH), zoneCol, 5.0f);
            draw->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y - band),
                ImVec2(cursorPos.x + previewW, cursorPos.y - 4.0f), zoneCol, 5.0f);
            draw->AddRectFilled(ImVec2(cursorPos.x, cursorPos.y + previewH + 4.0f),
                ImVec2(cursorPos.x + previewW, cursorPos.y + previewH + band), zoneCol, 5.0f);
            draw->AddRect(ImVec2(cursorPos.x - band, cursorPos.y - band),
                ImVec2(cursorPos.x + previewW + band, cursorPos.y + previewH + band),
                lineCol, 7.0f, 0, 1.0f);
        };

        auto SideFromPreviewMouse = [&]() {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            const float distLeft = std::fabs(mouse.x - cursorPos.x);
            const float distRight = std::fabs(mouse.x - (cursorPos.x + previewW));
            const float distTop = std::fabs(mouse.y - cursorPos.y);
            const float distBottom = std::fabs(mouse.y - (cursorPos.y + previewH));
            float best = distLeft;
            int side = 0;
            if (distRight < best) { best = distRight; side = 1; }
            if (distTop < best) { best = distTop; side = 2; }
            if (distBottom < best) { side = 3; }
            return side;
        };

        auto DragPreviewHandle = [&](const char* id, ImVec2 pos, ImVec2 size, int* target) {
            const ImVec2 savedCursor = ImGui::GetCursorScreenPos();
            const ImVec2 hitPad(10.0f, 8.0f);
            const ImVec2 hitMin(pos.x - hitPad.x, pos.y - hitPad.y);
            const ImVec2 hitSize((std::max)(22.0f, size.x + hitPad.x * 2.0f),
                (std::max)(20.0f, size.y + hitPad.y * 2.0f));
            ImGui::SetCursorScreenPos(hitMin);
            ImGui::InvisibleButton(id, hitSize);
            if (ImGui::IsItemActive() && (ImGui::IsMouseDragging(0) || ImGui::IsMouseClicked(0))) {
                *target = SideFromPreviewMouse();
            }
            if (ImGui::IsItemHovered() || ImGui::IsItemActive()) {
                draw->AddRect(hitMin, ImVec2(hitMin.x + hitSize.x, hitMin.y + hitSize.y),
                    IM_COL32(0, 220, 255, 145), 5.0f, 0, 1.0f);
                draw->AddCircleFilled(ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f),
                    3.0f, IM_COL32(0, 220, 255, 190), 10);
            }
            if (ImGui::IsItemActive()) {
                DrawPreviewDropZones();
            }
            ImGui::SetCursorScreenPos(savedCursor);
        };

        auto ColorFromFloats = [](const float* col) {
            return ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], col[3]));
        };

        auto DrawPreviewChip = [&](const char* id, const char* text, float fontSize,
                                   ImU32 color, int* target, float spacing = 1.0f,
                                   bool warning = false) {
            const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
            const ImVec2 chipSize(textSize.x + 10.0f, textSize.y + 5.0f);
            ImVec2 chipPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(*target),
                chipSize,
                spacing);
            const int bgAlpha = static_cast<int>((std::clamp)(g_Menu.esp_text_bg_alpha, 0.0f, 0.70f) * (warning ? 255.0f : 210.0f));
            if (g_Menu.esp_text_background || warning) {
                draw->AddRectFilled(chipPos, ImVec2(chipPos.x + chipSize.x, chipPos.y + chipSize.y),
                    warning ? IM_COL32(50, 8, 6, bgAlpha) : IM_COL32(5, 8, 12, bgAlpha), 4.0f);
                if (warning) {
                    draw->AddRect(chipPos, ImVec2(chipPos.x + chipSize.x, chipPos.y + chipSize.y),
                        IM_COL32(255, 85, 55, 135), 4.0f, 0, 1.0f);
                }
            }
            draw->AddText(ImGui::GetFont(), fontSize, ImVec2(chipPos.x + 5.0f, chipPos.y + 2.0f), color, text);
            DragPreviewHandle(id, chipPos, chipSize, target);
        };

        // 2. Health Demo (Multi-Position & Multi-Color Mode)
        if (g_Menu.esp_health) {
            ImU32 hpColor;
            if (g_Menu.esp_health_color_mode == 1) { // Static
                hpColor = ImColor(ImVec4(g_Menu.health_color[0], g_Menu.health_color[1], g_Menu.health_color[2], g_Menu.health_color[3]));
            } else { // Dynamic/Gradient
                hpColor = IM_COL32(0, 255, 100, 255);
                if (previewHealthSim < 0.3f) hpColor = IM_COL32(255, 50, 50, 255);
                else if (previewHealthSim < 0.6f) hpColor = IM_COL32(255, 200, 50, 255);
            }

            if (g_Menu.esp_health_display_mode == 1) {
                DrawPreviewChip(skCrypt("##DragHealthText"), skCrypt("76 HP"), 11.0f,
                    hpColor, &g_Menu.esp_health_pos, 2.0f, previewHealthSim < 0.35f);
            } else {
                const float previewHealthThickness =
                    (g_Menu.esp_health_bar_style == 1 || g_Menu.esp_health_bar_style == 3) ? 9.0f : 8.0f;
                auto healthSlot = previewLayout.TakeBar(
                    PlayerEspLayout::SideFromMenu(g_Menu.esp_health_pos),
                    previewHealthThickness,
                    4.0f);
                const ImVec2 hpTop = healthSlot.Min;
                const ImVec2 hpBot = healthSlot.Max;
                OverlayHealthBar::Draw(draw, hpTop, hpBot, previewHealthSim, hpColor,
                    !healthSlot.Horizontal, g_Menu.esp_health_bar_style, 1.0f,
                    previewHealthSim < 0.35f);
                DragPreviewHandle(skCrypt("##DragHealth"), hpTop,
                    ImVec2(hpBot.x - hpTop.x, hpBot.y - hpTop.y), &g_Menu.esp_health_pos);
            }
        }

        if (g_Menu.esp_view_direction) {
            const ImVec2 rayStart(cursorPos.x + previewW * 0.50f, cursorPos.y + previewH * 0.20f);
            const ImVec2 rayEnd(cursorPos.x + previewW * 0.74f, cursorPos.y + previewH * 0.13f);
            ImU32 rayCol = ColorFromFloats(g_Menu.view_direction_color);
            draw->AddLine(ImVec2(rayStart.x + 1.0f, rayStart.y + 1.0f),
                ImVec2(rayEnd.x + 1.0f, rayEnd.y + 1.0f), IM_COL32(0, 0, 0, 120), 3.0f);
            draw->AddLine(rayStart, rayEnd, rayCol, 1.6f);
            draw->AddCircleFilled(rayEnd, 2.4f, rayCol, 10);
        }

        if (g_Menu.esp_teamid) {
            const float size = g_Menu.teamid_badge_size;
            ImVec2 badgePos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_teamid_pos),
                ImVec2(size, size),
                2.0f);
            ImVec2 center(badgePos.x + size * 0.5f, badgePos.y + size * 0.5f);
            ImU32 badgeCol = g_Menu.team_color_custom ?
                ImGui::ColorConvertFloat4ToU32(ImVec4(g_Menu.team_custom_colors[0][0], g_Menu.team_custom_colors[0][1], g_Menu.team_custom_colors[0][2], g_Menu.team_custom_colors[0][3])) :
                ImGui::ColorConvertFloat4ToU32(ImVec4(g_Menu.teamid_color[0], g_Menu.teamid_color[1], g_Menu.teamid_color[2], g_Menu.teamid_color[3]));
            draw->AddCircleFilled(center, size * 0.5f, badgeCol, 20);
            draw->AddCircle(center, size * 0.5f - 1.0f, IM_COL32(255, 255, 255, 100), 20, 1.0f);
            const char* teamText = skCrypt("4");
            const float fontSize = (std::max)(8.0f, size * 0.62f);
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, teamText);
            draw->AddText(ImGui::GetFont(), fontSize, ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f),
                IM_COL32(255, 255, 255, 255), teamText);
            DragPreviewHandle(skCrypt("##DragTeam"), badgePos, ImVec2(size, size), &g_Menu.esp_teamid_pos);
        }

        if (g_Menu.esp_aim_warning || g_Menu.esp_status_badges) {
            DrawPreviewChip(skCrypt("##DragStatus"), skCrypt("AIM | ADS"),
                (std::max)(9.0f, g_Menu.spectated_font_size - 2.0f),
                ColorFromFloats(g_Menu.aim_warning_color), &g_Menu.esp_spectated_pos, 1.0f, true);
        }

        if (g_Menu.esp_close_warning) {
            DrawPreviewChip(skCrypt("##DragClose"), skCrypt("! 42m"),
                (std::max)(10.0f, g_Menu.spectated_font_size - 1.0f),
                ColorFromFloats(g_Menu.close_warning_color), &g_Menu.esp_spectated_pos, 1.0f, true);
        }

        if (g_Menu.esp_killcount) {
            DrawPreviewChip(skCrypt("##DragKills"), skCrypt("K: 12"), g_Menu.kill_font_size,
                ColorFromFloats(g_Menu.kill_color), &g_Menu.esp_killcount_pos);
        }

        if (g_Menu.esp_damage) {
            DrawPreviewChip(skCrypt("##DragDamage"), skCrypt("DMG: 184"), g_Menu.damage_font_size,
                ColorFromFloats(g_Menu.damage_color), &g_Menu.esp_damage_pos);
        }

        if (g_Menu.esp_speed) {
            DrawPreviewChip(skCrypt("##DragSpeed"), skCrypt("SPD: 42"), g_Menu.speed_font_size,
                ColorFromFloats(g_Menu.speed_color), &g_Menu.esp_speed_pos);
        }

        if (g_Menu.esp_survival_level) {
            DrawPreviewChip(skCrypt("##DragLevel"), skCrypt("Lv.50"), g_Menu.survival_level_font_size,
                ColorFromFloats(g_Menu.survival_level_color), &g_Menu.esp_survival_level_pos);
        }

        if (g_Menu.esp_distance) {
            DrawPreviewChip(skCrypt("##DragDistance"), skCrypt("[145m]"), g_Menu.distance_font_size,
                ColorFromFloats(g_Menu.distance_color), &g_Menu.esp_distance_pos, 2.0f);
        }

        if (g_Menu.esp_ammo) {
            const char* ammoText = skCrypt("24/40");
            const float ammoFont = g_Menu.ammo_font_size;
            const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(ammoFont, FLT_MAX, 0.0f, ammoText);
            const ImVec2 badgeSize((std::max)(48.0f, textSize.x + 18.0f), textSize.y + 8.0f);
            ImVec2 ammoPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_ammo_pos),
                badgeSize,
                1.0f);
            const ImU32 ammoCol = ColorFromFloats(g_Menu.ammo_color);
            draw->AddRectFilled(ammoPos, ImVec2(ammoPos.x + badgeSize.x, ammoPos.y + badgeSize.y),
                IM_COL32(5, 8, 12, 165), 4.0f);
            draw->AddRectFilled(ImVec2(ammoPos.x + 4.0f, ammoPos.y + badgeSize.y - 4.0f),
                ImVec2(ammoPos.x + 4.0f + (badgeSize.x - 8.0f) * 0.60f, ammoPos.y + badgeSize.y - 2.0f),
                ammoCol, 2.0f);
            draw->AddText(ImGui::GetFont(), ammoFont, ImVec2(ammoPos.x + 8.0f, ammoPos.y + 3.0f), ammoCol, ammoText);
            DragPreviewHandle(skCrypt("##DragAmmo"), ammoPos, badgeSize, &g_Menu.esp_ammo_pos);
        }

        if (g_Menu.esp_weapon) {
            TextureInfo* weaponTex = OverlayTextures::GetWeaponImage(skCrypt("M416"));
            if (weaponTex && weaponTex->SRV && g_Menu.esp_weapon_type == 1) {
                const float iconW = g_Menu.weapon_icon_size * 0.76f;
                const float frameWidth = static_cast<float>(weaponTex->Width);
                const float iconH = frameWidth > 0.0f ?
                    iconW * (static_cast<float>(weaponTex->Height) / frameWidth) :
                    g_Menu.weapon_icon_size * 0.32f;
                const ImVec2 iconSize(iconW + 6.0f, iconH + 4.0f);
                ImVec2 weaponPos = previewLayout.Take(
                    PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                    iconSize,
                    2.0f);
                ImU32 weaponCol = ColorFromFloats(g_Menu.weapon_color);
                draw->AddRectFilled(weaponPos, ImVec2(weaponPos.x + iconSize.x, weaponPos.y + iconSize.y),
                    IM_COL32(5, 8, 12, 118), 4.0f);
                draw->AddRect(weaponPos, ImVec2(weaponPos.x + iconSize.x, weaponPos.y + iconSize.y),
                    weaponCol, 4.0f, 0, 1.0f);
                OverlayAssetAnimation::DrawStaticImageRect(draw, weaponTex,
                    ImVec2(weaponPos.x + 3.0f, weaponPos.y + 2.0f),
                    ImVec2(weaponPos.x + iconW + 3.0f, weaponPos.y + iconH + 2.0f),
                    IM_COL32(255, 255, 255, 255),
                    0.88f);
                DragPreviewHandle(skCrypt("##DragWeapon"), weaponPos, iconSize, &g_Menu.esp_weapon_pos);
            } else {
                DrawPreviewChip(skCrypt("##DragWeapon"), skCrypt("M416"), g_Menu.weapon_font_size,
                    ColorFromFloats(g_Menu.weapon_color), &g_Menu.esp_weapon_pos, 2.0f);
            }
        }

        if (g_Menu.esp_rank) {
            DrawPreviewChip(skCrypt("##DragRank"), skCrypt("Diamond"), g_Menu.rank_font_size,
                ColorFromFloats(g_Menu.rank_color), &g_Menu.esp_rank_pos);
        }

        if (g_Menu.esp_name) {
            const float* targetCol = bPreviewOccluded ? g_Menu.name_invisible_color : g_Menu.name_visible_color;
            DrawPreviewChip(skCrypt("##DragName"), skCrypt("GZ-Preview"), g_Menu.name_font_size,
                ColorFromFloats(targetCol), &g_Menu.esp_name_pos, 2.0f);
        }

        if (g_Menu.esp_spectated) {
            DrawPreviewChip(skCrypt("##DragWatch"), skCrypt("EYE: 2"), g_Menu.spectated_font_size,
                ColorFromFloats(g_Menu.spectated_color), &g_Menu.esp_spectated_pos, 2.0f, true);
        }

        // 3. Skeleton Demo (PIXEL-PERFECT FROM USER COORDINATES)
        if (g_Menu.esp_skeleton) {
            float* targetCol = bPreviewOccluded ? g_Menu.skeleton_invisible_color : g_Menu.skeleton_visible_color;
            ImU32 uSkelCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));

            auto P = [&](float x, float y) {
                return ImVec2(cursorPos.x + previewW * x, cursorPos.y + previewH * y);
            };

            // Compact rig fitted to the preview mesh silhouette. Keep it inside
            // the body image so the skeleton does not look offset or oversized.
            ImVec2 pHead = P(0.500f, 0.112f);
            ImVec2 pNeck = P(0.500f, 0.166f);
            ImVec2 pChest = P(0.500f, 0.238f);
            ImVec2 pVaiP = P(0.382f, 0.232f);
            ImVec2 pVaiT = P(0.618f, 0.232f);
            ImVec2 pKhuP = P(0.340f, 0.342f);
            ImVec2 pKhuT = P(0.660f, 0.342f);
            ImVec2 pTayP = P(0.304f, 0.456f);
            ImVec2 pTayT = P(0.696f, 0.456f);
            ImVec2 pEo = P(0.500f, 0.382f);
            ImVec2 pHipsCenter = P(0.500f, 0.482f);
            ImVec2 pHongP = P(0.424f, 0.486f);
            ImVec2 pHongT = P(0.576f, 0.486f);
            ImVec2 pGoiP = P(0.446f, 0.690f);
            ImVec2 pGoiT = P(0.554f, 0.690f);
            ImVec2 pChnP = P(0.424f, 0.872f);
            ImVec2 pChnT = P(0.576f, 0.872f);

            // Draw Connections
            // pHead exclusively for the Head Circle feature as requested
            if (g_Menu.esp_head_circle) draw->AddCircle(pHead, previewW * 0.105f, uSkelCol, 16, 1.5f);

            // Skeleton starts from pNeck (Chin/Neck) downwards
            draw->AddLine(pNeck, pChest, uSkelCol, 1.5f);
            draw->AddLine(pChest, pVaiP, uSkelCol, 1.5f);
            draw->AddLine(pChest, pVaiT, uSkelCol, 1.5f);
            draw->AddLine(pVaiP, pKhuP, uSkelCol, 1.5f);
            draw->AddLine(pKhuP, pTayP, uSkelCol, 1.5f);
            draw->AddLine(pVaiT, pKhuT, uSkelCol, 1.5f);
            draw->AddLine(pKhuT, pTayT, uSkelCol, 1.5f);
            draw->AddLine(pChest, pEo, uSkelCol, 1.5f);
            draw->AddLine(pEo, pHipsCenter, uSkelCol, 1.5f);
            draw->AddLine(pHipsCenter, pHongP, uSkelCol, 1.5f);
            draw->AddLine(pHipsCenter, pHongT, uSkelCol, 1.5f);
            draw->AddLine(pHongP, pGoiP, uSkelCol, 1.5f);
            draw->AddLine(pGoiP, pChnP, uSkelCol, 1.5f);
            draw->AddLine(pHongT, pGoiT, uSkelCol, 1.5f);
            draw->AddLine(pGoiT, pChnT, uSkelCol, 1.5f);
        }
    } else {
        ImGui::Dummy(ImVec2(previewW, previewH));
        ImGui::TextDisabled(skCrypt("Loading Pixel-Perfect Preview..."));
    }

    ImGui::EndChild();

    ImGui::Columns(1);
}
