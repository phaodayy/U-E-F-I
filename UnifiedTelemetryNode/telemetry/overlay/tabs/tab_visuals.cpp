#include "../overlay_menu.hpp"
#include "../translation.hpp"
#include "../../sdk/context.hpp"
#include "../../protec/skCrypt.h"
#include <map>

void OverlayMenu::RenderTabVisuals(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(4, skCrypt("ESPColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 4.0f);
    ImGui::SetColumnWidth(1, totalWidth / 4.0f);
    ImGui::SetColumnWidth(2, totalWidth / 4.0f);
    ImGui::SetColumnWidth(3, totalWidth / 4.0f);

    // Col 1: LÃµi Hiá»ƒn Thá»‹
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
    // Col 2: Kiá»ƒu Váº½ & HUD
    BeginGlassCard(skCrypt("##ESPCol2"), Lang.HeaderRenderStyle, ImVec2(totalWidth / 4.0f - 15, 0));
    ImGui::Checkbox(Lang.Box, &g_Menu.esp_box);
    ImGui::SliderFloat(Lang.BoxThick, &g_Menu.box_thickness, 1.0f, 5.0f, skCrypt("%.1f px"));
    ImGui::Checkbox(Lang.Skeleton, &g_Menu.esp_skeleton);
    ImGui::Checkbox(Lang.ESP_SkeletonDots, &g_Menu.esp_skeleton_dots);
    ImGui::Checkbox(Lang.HealthBar, &g_Menu.esp_health);
    ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
    ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
    ImGui::Checkbox(Lang.TeamID, &g_Menu.esp_teamid);
    ImGui::Checkbox(Lang.KillCount, &g_Menu.esp_killcount);
    ImGui::Checkbox(Lang.Rank, &g_Menu.esp_rank);
    ImGui::Checkbox(Lang.SurvivalLevel, &g_Menu.esp_survival_level);
    ImGui::Checkbox(Lang.HeadCircle, &g_Menu.esp_head_circle);
    ImGui::Checkbox(Lang.Snaplines, &g_Menu.esp_snapline);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseEspNameplates);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 3: ThÃ´ng sá»‘ & MÃ u Sáº¯c
    BeginGlassCard(skCrypt("##ESPCol3"), Lang.HeaderOverlayHUD, ImVec2(totalWidth / 4.0f - 15, 0));
    ImGui::SliderFloat(Lang.FontSize, &g_Menu.esp_font_size, 8.0f, 24.0f, skCrypt("%.1f px"));
    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.DistThresholds);
    ImGui::SliderInt(Lang.RenderDist, &g_Menu.render_distance, 50, 1000, skCrypt("%d m"));
    ImGui::SliderInt(Lang.InfoESP, &g_Menu.name_max_dist, 50, 600, skCrypt("%d m"));
    ImGui::SliderFloat(skCrypt("Thick"), &g_Menu.skel_thickness, 1.0f, 5.0f, skCrypt("%.1f px"));

    ImGui::Spacing();
    ImGui::SetNextItemWidth(120);
    const char* posItems[] = { Lang.PosLeft, Lang.PosRight, Lang.PosTop, Lang.PosBottom };
    ImGui::Combo(Lang.HealthPos, &g_Menu.esp_health_pos, posItems, IM_ARRAYSIZE(posItems));

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
    ColorPicker(Lang.Name, g_Menu.name_color);
    ColorPicker(Lang.Distance, g_Menu.distance_color);
    ColorPicker(Lang.Weapon, g_Menu.weapon_color);
    ColorPicker(Lang.ColorHealth, g_Menu.health_color);

    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 4: Xem TrÆ°á»›c (PREVIEW)
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
            draw->AddRect(cursorPos, ImVec2(cursorPos.x + previewW, cursorPos.y + previewH), uBoxCol, 4.0f, 0, 2.0f);
        }

        // 2. Health Demo (Multi-Position & Multi-Color Mode)
        if (g_Menu.esp_health) {
            ImVec2 hpTop, hpBot;
            bool horizontal = false;

            // Position calculation
            switch (g_Menu.esp_health_pos) {
            case 1: // Right
                hpTop = ImVec2(cursorPos.x + previewW + 4, cursorPos.y);
                hpBot = ImVec2(cursorPos.x + previewW + 16, cursorPos.y + previewH);
                break;
            case 2: // Top
                hpTop = ImVec2(cursorPos.x, cursorPos.y - 16);
                hpBot = ImVec2(cursorPos.x + previewW, cursorPos.y - 4);
                horizontal = true;
                break;
            case 3: // Bottom
                hpTop = ImVec2(cursorPos.x, cursorPos.y + previewH + 4);
                hpBot = ImVec2(cursorPos.x + previewW, cursorPos.y + previewH + 16);
                horizontal = true;
                break;
            default: // 0: Left
                hpTop = ImVec2(cursorPos.x - 16, cursorPos.y);
                hpBot = ImVec2(cursorPos.x - 4, cursorPos.y + previewH);
                break;
            }

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
            ImU32 rsCol = IM_COL32(180, 100, 255, 255);
            ImVec2 rsSize = ImGui::CalcTextSize(szRank);
            draw->AddText(ImVec2(cursorPos.x + (previewW - rsSize.x) / 2.0f, cursorPos.y - rsSize.y - 22), rsCol, szRank);
        }

        // 4. Name & Distance Demo
        if (g_Menu.esp_name) {
            float* targetCol = g_Menu.name_color;
            ImU32 uNameCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));

            std::string pName = "";
            if (g_Menu.esp_teamid) pName += "[4] ";
            pName += skCrypt("GZ-Preview");
            if (g_Menu.esp_killcount) pName += " | K: 12";
            if (g_Menu.esp_survival_level) pName += " | Lv.50";

            ImVec2 textSize = ImGui::CalcTextSize(pName.c_str());
            draw->AddText(ImVec2(cursorPos.x + (previewW - textSize.x) / 2.0f, cursorPos.y - textSize.y - 5), uNameCol, pName.c_str());
        }

        if (g_Menu.esp_distance) {
            float* targetCol = g_Menu.distance_color;
            ImU32 uDistCol = ImColor(ImVec4(targetCol[0], targetCol[1], targetCol[2], targetCol[3]));
            char distBuf[32];
            sprintf_s(distBuf, skCrypt("[145m]"));
            ImVec2 textSize = ImGui::CalcTextSize(distBuf);
            draw->AddText(ImVec2(cursorPos.x + (previewW - textSize.x) / 2.0f, cursorPos.y + previewH + 5), uDistCol, distBuf);
        }

        // 5. Weapon Demo
        float* weaponColPtr = g_Menu.weapon_color;
        ImU32 uWeaponCol = ImColor(ImVec4(weaponColPtr[0], weaponColPtr[1], weaponColPtr[2], weaponColPtr[3]));
        const char* szWeapon = skCrypt("SCAR-L");
        ImVec2 wpnSize = ImGui::CalcTextSize(szWeapon);
        draw->AddText(ImVec2(cursorPos.x + (previewW - wpnSize.x) / 2.0f, cursorPos.y + previewH + 20), uWeaponCol, szWeapon);

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
