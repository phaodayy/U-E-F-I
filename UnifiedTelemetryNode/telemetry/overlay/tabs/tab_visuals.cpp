#include "../core/overlay_menu.hpp"
#include "../player/player_esp_layout.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>
#include <algorithm>
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
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseVisualProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseRankLayout);
    DrawDisplayOnlyOption(Lang.ShowcaseMarkerLayout);
    DrawDisplayOnlyOption(Lang.ShowcaseEspThreatBands);
    DrawDisplayOnlyOption(Lang.ShowcaseEspTeamColors);
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
    ImGui::Checkbox(skCrypt("Health Text"), &g_Menu.esp_health_text);
    ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
    ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
    ImGui::Checkbox(Lang.TeamID, &g_Menu.esp_teamid);
    ImGui::Checkbox(Lang.KillCount, &g_Menu.esp_killcount);
    ImGui::Checkbox(skCrypt("Damage"), &g_Menu.esp_damage);
    ImGui::Checkbox(skCrypt("Ammo"), &g_Menu.esp_ammo);
    ImGui::Checkbox(skCrypt("Speed"), &g_Menu.esp_speed);
    ImGui::Checkbox(Lang.Rank, &g_Menu.esp_rank);
    ImGui::Checkbox(Lang.SurvivalLevel, &g_Menu.esp_survival_level);
    ImGui::Checkbox(Lang.HeadCircle, &g_Menu.esp_head_circle);
    ImGui::Checkbox(Lang.Snaplines, &g_Menu.esp_snapline);
    ImGui::Checkbox(skCrypt("Aim Warning"), &g_Menu.esp_aim_warning);
    ImGui::Checkbox(skCrypt("View Direction"), &g_Menu.esp_view_direction);
    ImGui::Checkbox(skCrypt("Status Badges"), &g_Menu.esp_status_badges);
    ImGui::Checkbox(skCrypt("Close Warning"), &g_Menu.esp_close_warning);
    ImGui::Checkbox(skCrypt("Offscreen Text"), &g_Menu.esp_offscreen_text);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseEspNameplates);
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

    ImGui::Spacing();
    ImGui::SetNextItemWidth(120);
    const char* posItems[] = { Lang.PosLeft, Lang.PosRight, Lang.PosTop, Lang.PosBottom };
    auto DrawPosCombo = [&](const char* label, int* value) {
        ImGui::SetNextItemWidth(120);
        ImGui::Combo(label, value, posItems, IM_ARRAYSIZE(posItems));
    };
    DrawPosCombo(Lang.HealthPos, &g_Menu.esp_health_pos);
    DrawPosCombo(Lang.NamePos, &g_Menu.esp_name_pos);
    DrawPosCombo(Lang.DistancePos, &g_Menu.esp_distance_pos);
    DrawPosCombo(Lang.WeaponPos, &g_Menu.esp_weapon_pos);
    DrawPosCombo(Lang.RankPos, &g_Menu.esp_rank_pos);
    DrawPosCombo(Lang.SpectatedPos, &g_Menu.esp_spectated_pos);
    DrawPosCombo(skCrypt("Team ID Pos"), &g_Menu.esp_teamid_pos);
    DrawPosCombo(skCrypt("Kills Pos"), &g_Menu.esp_killcount_pos);
    DrawPosCombo(skCrypt("Damage Pos"), &g_Menu.esp_damage_pos);
    DrawPosCombo(skCrypt("Ammo Pos"), &g_Menu.esp_ammo_pos);
    DrawPosCombo(skCrypt("Speed Pos"), &g_Menu.esp_speed_pos);
    DrawPosCombo(skCrypt("Level Pos"), &g_Menu.esp_survival_level_pos);

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
    ColorPicker(skCrypt("Damage"), g_Menu.damage_color);
    ColorPicker(skCrypt("Ammo"), g_Menu.ammo_color);
    ColorPicker(skCrypt("Speed"), g_Menu.speed_color);
    ColorPicker(Lang.SurvivalLevel, g_Menu.survival_level_color);
    ColorPicker(Lang.ESP_Spectated, g_Menu.spectated_color);
    ColorPicker(skCrypt("Aim Warn"), g_Menu.aim_warning_color);
    ColorPicker(skCrypt("Close Warn"), g_Menu.close_warning_color);
    ColorPicker(skCrypt("View Dir"), g_Menu.view_direction_color);
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

    // Centering logic
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cardW - previewW) / 2.0f);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (PreviewInstructor.SRV) {
        ImGui::Image((void*)PreviewInstructor.SRV, ImVec2(previewW, previewH), ImVec2(0,0), ImVec2(1,1), bPreviewOccluded ? ImVec4(1,1,1,0.4f) : ImVec4(1,1,1,1.0f), ImVec4(0,0,0,0));

        ImDrawList* draw = ImGui::GetWindowDrawList();

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

        // 2. Health Demo (Multi-Position & Multi-Color Mode)
        if (g_Menu.esp_health) {
            ImVec2 hpTop, hpBot;
            bool horizontal = false;

            auto healthSlot = previewLayout.TakeBar(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_health_pos),
                12.0f,
                4.0f);
            hpTop = healthSlot.Min;
            hpBot = healthSlot.Max;
            horizontal = healthSlot.Horizontal;

            // Background
            draw->AddRectFilled(hpTop, hpBot, IM_COL32(0, 0, 0, 180), 2.0f);

            // Color calculation
            ImU32 hpColor;
            if (g_Menu.esp_health_color_mode == 1) { // Static
                    hpColor = ImColor(ImVec4(g_Menu.health_color[0], g_Menu.health_color[1], g_Menu.health_color[2], g_Menu.health_color[3]));
            } else { // Dynamic/Gradient
                hpColor = IM_COL32(0, 255, 100, 255);
                if (previewHealthSim < 0.3f) hpColor = IM_COL32(255, 50, 50, 255);
                else if (previewHealthSim < 0.6f) hpColor = IM_COL32(255, 200, 50, 255);
            }

            // Bar rendering
            if (!horizontal) {
                float hpHeight = (hpBot.y - hpTop.y - 2) * previewHealthSim;
                draw->AddRectFilled(ImVec2(hpTop.x + 1, hpBot.y - hpHeight - 1), ImVec2(hpBot.x - 1, hpBot.y - 1), hpColor, 2.0f);
                for (int i = 1; i < 5; i++) {
                    float segY = hpTop.y + ((hpBot.y - hpTop.y) * i / 5.0f);
                    draw->AddLine(ImVec2(hpTop.x, segY), ImVec2(hpBot.x, segY), IM_COL32(0, 0, 0, 255), 1.0f);
                }
            } else {
                float hpWidth = (hpBot.x - hpTop.x - 2) * previewHealthSim;
                draw->AddRectFilled(ImVec2(hpTop.x + 1, hpTop.y + 1), ImVec2(hpTop.x + 1 + hpWidth, hpBot.y - 1), hpColor, 2.0f);
                for (int i = 1; i < 5; i++) {
                    float segX = hpTop.x + ((hpBot.x - hpTop.x) * i / 5.0f);
                    draw->AddLine(ImVec2(segX, hpTop.y), ImVec2(segX, hpBot.y), IM_COL32(0, 0, 0, 255), 1.0f);
                }
            }
        }

        // 2.5 Rank Preview
        if (g_Menu.esp_rank) {
            const char* szRank = skCrypt("Diamond");
            ImU32 rsCol = ImColor(ImVec4(g_Menu.rank_color[0], g_Menu.rank_color[1], g_Menu.rank_color[2], g_Menu.rank_color[3]));
            ImVec2 rsSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.rank_font_size, FLT_MAX, 0.0f, szRank);
            ImVec2 rankPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_rank_pos),
                rsSize,
                1.0f);
            draw->AddText(ImGui::GetFont(), g_Menu.rank_font_size, rankPos, rsCol, szRank);
        }

        if (g_Menu.esp_teamid) {
            const float size = g_Menu.teamid_badge_size;
            ImVec2 badgePos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_teamid_pos),
                ImVec2(size, size),
                2.0f);
            ImVec2 center(badgePos.x + size * 0.5f, badgePos.y + size * 0.5f);
            ImU32 badgeCol = ImColor(ImVec4(g_Menu.teamid_color[0], g_Menu.teamid_color[1], g_Menu.teamid_color[2], g_Menu.teamid_color[3]));
            draw->AddCircleFilled(center, size * 0.5f, badgeCol, 20);
            draw->AddCircle(center, size * 0.5f - 1.0f, IM_COL32(255, 255, 255, 100), 20, 1.0f);
            const char* teamText = skCrypt("4");
            const float fontSize = (std::max)(8.0f, size * 0.62f);
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, teamText);
            draw->AddText(ImGui::GetFont(), fontSize, ImVec2(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f),
                IM_COL32(255, 255, 255, 255), teamText);
        }

        if (g_Menu.esp_killcount) {
            const char* killText = skCrypt("K: 12");
            ImU32 killCol = ImColor(ImVec4(g_Menu.kill_color[0], g_Menu.kill_color[1], g_Menu.kill_color[2], g_Menu.kill_color[3]));
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.kill_font_size, FLT_MAX, 0.0f, killText);
            ImVec2 killPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_killcount_pos),
                textSize,
                1.0f);
            draw->AddText(ImGui::GetFont(), g_Menu.kill_font_size, killPos, killCol, killText);
        }

        if (g_Menu.esp_survival_level) {
            const char* levelText = skCrypt("Lv.50");
            ImU32 levelCol = ImColor(ImVec4(g_Menu.survival_level_color[0], g_Menu.survival_level_color[1], g_Menu.survival_level_color[2], g_Menu.survival_level_color[3]));
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.survival_level_font_size, FLT_MAX, 0.0f, levelText);
            ImVec2 levelPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_survival_level_pos),
                textSize,
                1.0f);
            draw->AddText(ImGui::GetFont(), g_Menu.survival_level_font_size, levelPos, levelCol, levelText);
        }

        // 4. Name & Distance Demo
        if (g_Menu.esp_name) {
            float* targetCol = bPreviewOccluded ? g_Menu.name_invisible_color : g_Menu.name_visible_color;
            ImU32 uNameCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));

            std::string pName = skCrypt("GZ-Preview");

            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.name_font_size, FLT_MAX, 0.0f, pName.c_str());
            ImVec2 namePos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_name_pos),
                textSize,
                2.0f);
            draw->AddText(ImGui::GetFont(), g_Menu.name_font_size, namePos, uNameCol, pName.c_str());
        }

        if (g_Menu.esp_distance) {
            float* targetCol = g_Menu.distance_color;
            ImU32 uDistCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));
            char distBuf[32];
            sprintf_s(distBuf, skCrypt("[145m]"));
            ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.distance_font_size, FLT_MAX, 0.0f, distBuf);
            ImVec2 distPos = previewLayout.Take(
                PlayerEspLayout::SideFromMenu(g_Menu.esp_distance_pos),
                textSize,
                2.0f);
            draw->AddText(ImGui::GetFont(), g_Menu.distance_font_size, distPos, uDistCol, distBuf);
        }

        // 5. Weapon Demo
        float* weaponColPtr = g_Menu.weapon_color;
        ImU32 uWeaponCol = ImColor(ImVec4(weaponColPtr[0], weaponColPtr[1], weaponColPtr[2], weaponColPtr[3]));
        const char* szWeapon = skCrypt("SCAR-L");
        ImVec2 wpnSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.weapon_font_size, FLT_MAX, 0.0f, szWeapon);
        ImVec2 weaponPos = previewLayout.Take(
            PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
            wpnSize,
            2.0f);
        draw->AddText(ImGui::GetFont(), g_Menu.weapon_font_size, weaponPos, uWeaponCol, szWeapon);

        // 3. Skeleton Demo (PIXEL-PERFECT FROM USER COORDINATES)
        if (g_Menu.esp_skeleton) {
            float* targetCol = bPreviewOccluded ? g_Menu.skeleton_invisible_color : g_Menu.skeleton_visible_color;
            ImU32 uSkelCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));

            // Points mapping
            ImVec2 pHead = ImVec2(cursorPos.x + previewW * 0.469f, cursorPos.y + previewH * 0.105f); // Lowered head center further
            ImVec2 pNeck = ImVec2(cursorPos.x + previewW * 0.469f, cursorPos.y + previewH * 0.145f);
            ImVec2 pChest = ImVec2(cursorPos.x + previewW * 0.473f, cursorPos.y + previewH * 0.189f);
            ImVec2 pVaiP = ImVec2(cursorPos.x + previewW * 0.289f, cursorPos.y + previewH * 0.201f);
            ImVec2 pVaiT = ImVec2(cursorPos.x + previewW * 0.670f, cursorPos.y + previewH * 0.187f);
            ImVec2 pKhuP = ImVec2(cursorPos.x + previewW * 0.276f, cursorPos.y + previewH * 0.356f);
            ImVec2 pKhuT = ImVec2(cursorPos.x + previewW * 0.756f, cursorPos.y + previewH * 0.342f);
            ImVec2 pTayP = ImVec2(cursorPos.x + previewW * 0.145f, cursorPos.y + previewH * 0.483f);
            ImVec2 pTayT = ImVec2(cursorPos.x + previewW * 0.842f, cursorPos.y + previewH * 0.495f);
            ImVec2 pEo = ImVec2(cursorPos.x + previewW * 0.486f, cursorPos.y + previewH * 0.363f);
            ImVec2 pHipsCenter = ImVec2(cursorPos.x + previewW * 0.461f, cursorPos.y + previewH * 0.445f);
            ImVec2 pHongP = ImVec2(cursorPos.x + previewW * 0.313f, cursorPos.y + previewH * 0.459f);
            ImVec2 pHongT = ImVec2(cursorPos.x + previewW * 0.613f, cursorPos.y + previewH * 0.465f);
            ImVec2 pGoiP = ImVec2(cursorPos.x + previewW * 0.383f, cursorPos.y + previewH * 0.699f);
            ImVec2 pGoiT = ImVec2(cursorPos.x + previewW * 0.633f, cursorPos.y + previewH * 0.707f);
            ImVec2 pChnP = ImVec2(cursorPos.x + previewW * 0.420f, cursorPos.y + previewH * 0.914f);
            ImVec2 pChnT = ImVec2(cursorPos.x + previewW * 0.715f, cursorPos.y + previewH * 0.911f);

            // Draw Connections
            // pHead exclusively for the Head Circle feature as requested
            if (g_Menu.esp_head_circle) draw->AddCircle(pHead, previewW * 0.16f, uSkelCol, 16, 1.5f); // Doubled radius from baseline

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
