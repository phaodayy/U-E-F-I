#include "../core/overlay_menu.hpp"
#include "../core/colors.hpp"
#include "player_esp_layout.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../../sdk/context.hpp"
#include "../../sdk/math.hpp"
#include "../../../protec/skCrypt.h"
#include <algorithm>
#include <cmath>

namespace {

std::string GetRankTierName(int kills) {
    if (kills < 1) return skCrypt("Unranked");
    if (kills < 5) return skCrypt("Bronze");
    if (kills < 20) return skCrypt("Silver");
    if (kills < 50) return skCrypt("Gold");
    if (kills < 100) return skCrypt("Platinum");
    if (kills < 250) return skCrypt("Diamond");
    return skCrypt("Master");
}

void DrawPlayerTeamBadge(ImDrawList* draw, const ImVec2& min, float size,
                         int teamID, ImU32 fillColor, float alpha) {
    if (teamID <= 0 || size <= 2.0f) return;

    const ImVec2 center(min.x + size * 0.5f, min.y + size * 0.5f);
    const float radius = size * 0.5f;
    const int alphaByte = static_cast<int>(220.0f * alpha);
    draw->AddCircleFilled(ImVec2(center.x + 1.0f, center.y + 1.0f), radius, IM_COL32(0, 0, 0, alphaByte));
    draw->AddCircleFilled(center, radius, fillColor, 20);
    draw->AddCircle(center, radius - 1.0f, IM_COL32(255, 255, 255, static_cast<int>(90.0f * alpha)), 20, 1.0f);

    char teamText[16] = {};
    sprintf_s(teamText, "%d", teamID % 100);
    const float fontSize = (std::max)(8.0f, size * 0.62f);
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, teamText);
    const ImVec2 textPos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(textPos.x + 1.0f, textPos.y + 1.0f),
        IM_COL32(0, 0, 0, static_cast<int>(220.0f * alpha)), teamText);
    draw->AddText(ImGui::GetFont(), fontSize, textPos,
        IM_COL32(255, 255, 255, static_cast<int>(255.0f * alpha)), teamText);
}

} // namespace

void OverlayMenu::RenderSinglePlayerEsp(ImDrawList* draw, PlayerData& player,
    const Vector3& delta, const Vector2& local_feet_s, bool hasLocalS,
    float ScreenCenterX, float ScreenHeight) {            if (esp_toggle) {
                float alphaMult = 1.0f;
                if (g_Menu.esp_distance_lod) {
                    float fadeStart = (float)g_Menu.render_distance * 0.65f;
                    if (player.Distance > fadeStart) {
                        alphaMult = 1.0f - ((player.Distance - fadeStart) / ((float)g_Menu.render_distance - fadeStart));
                        if (alphaMult < 0.15f) alphaMult = 0.15f;
                    }
                }
                auto ApplyAlpha = [&](ImU32 col, float mult) -> ImU32 {
                    int a = (int)((col >> 24) & 0xFF);
                    a = (int)(a * mult);
                    return (col & 0x00FFFFFF) | (a << 24);
                };

                Vector2 head_s, feet_s;
                if (telemetryContext::WorldToScreen(player.HeadPosition + delta, head_s) &&
                    telemetryContext::WorldToScreen(player.FeetPosition + delta, feet_s)) {

                    // --- PERFECT DYNAMIC BOUNDING BOX (BONE SCANNING) ---
                    float finalBoxTop, finalBoxBottom, finalBoxLeft, finalBoxRight;
                    bool useDynamicBox = false;

                    if (true) { // Try to build dynamic box from PlayerData members
                        float minX = 100000.0f, maxX = -100000.0f;
                        float minY = 100000.0f, maxY = -100000.0f;
                        bool foundValidBone = false;

                        // List of all bones available in PlayerData to form the bounding box
                        Vector3 bones[] = {
                            player.Bone_Head, player.Bone_Neck, player.Bone_Chest, player.Bone_Pelvis,
                            player.Bone_LShoulder, player.Bone_LElbow, player.Bone_LHand,
                            player.Bone_RShoulder, player.Bone_RElbow, player.Bone_RHand,
                            player.Bone_LThigh, player.Bone_LKnee, player.Bone_LFoot,
                            player.Bone_RThigh, player.Bone_RKnee, player.Bone_RFoot
                        };

                        for (const auto& boneWorld : bones) {
                            if (boneWorld.IsZero()) continue;
                            Vector2 screen;
                            if (telemetryContext::WorldToScreen(boneWorld + delta, screen)) {
                                minX = (std::min)(minX, screen.x);
                                maxX = (std::max)(maxX, screen.x);
                                minY = (std::min)(minY, screen.y);
                                maxY = (std::max)(maxY, screen.y);
                                foundValidBone = true;
                            }
                        }

                        if (foundValidBone) {
                            float boxH = maxY - minY;
                            float boxW = maxX - minX;

                            // Padding for "Breathing Room" (15% Width, 12% Height)
                            float paddingW = (std::max)(boxW * 0.15f, 4.0f);
                            float paddingH = (std::max)(boxH * 0.12f, 4.0f);

                            finalBoxTop = minY - (boxH * 0.25f) - 5.0f; // 25% Height + 5px buffer to ensure it clears Helmet/Face
                            finalBoxBottom = maxY + paddingH;
                            finalBoxLeft = minX - paddingW;
                            finalBoxRight = maxX + paddingW;

                            // Sanity check to avoid zero-sized boxes
                            if (finalBoxBottom - finalBoxTop > 5.0f && finalBoxRight - finalBoxLeft > 2.0f) {
                                useDynamicBox = true;
                            }
                        }
                    }

                    if (!useDynamicBox) {
                        // Fallback to Ratio Box (using DMA-style 0.5 ratio for "tight" feel)
                        float h = abs(head_s.y - feet_s.y);
                        float w = h * 0.50f;
                        finalBoxTop = head_s.y - (h * 0.12f);
                        finalBoxBottom = feet_s.y + (h * 0.05f);
                        finalBoxLeft = head_s.x - w/2;
                        finalBoxRight = head_s.x + w/2;
                    }


                    ImU32 boxCol = player.IsVisible ?
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color) :
                        ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_invisible_color);

                    if (player.IsTeammate) boxCol = telemetryColors::Teammate;
                    boxCol = ApplyAlpha(boxCol, alphaMult);

                    if (esp_box && player.Distance < g_Menu.box_max_dist) {
                        float thick = g_Menu.box_thickness;
                        draw->AddRect(ImVec2(finalBoxLeft, finalBoxTop), ImVec2(finalBoxRight, finalBoxBottom), IM_COL32(0,0,0,(int)(180 * alphaMult)), 2.5f, 0, thick + 1.25f); // Border
                        draw->AddRect(ImVec2(finalBoxLeft, finalBoxTop), ImVec2(finalBoxRight, finalBoxBottom), boxCol, 2.5f, 0, thick);       // Main Box
                    }

                    PlayerEspLayout::Stack espLayout(finalBoxLeft, finalBoxTop, finalBoxRight, finalBoxBottom);

                    // --- PREMIUM HEALTH BAR (DYNAMIC SCALING) ---
                    if (g_Menu.esp_health && player.Distance < g_Menu.hp_max_dist) {
                        float displayHealth = player.IsGroggy ? player.GroggyHealth : player.Health;
                        float healthPercent = displayHealth / 100.0f;
                        healthPercent = std::clamp(healthPercent, 0.0f, 1.0f);

                        // Linear Scaling based on Box Height (PERFECT SCALING)
                        float boxH = finalBoxBottom - finalBoxTop;
                        float boxW = finalBoxRight - finalBoxLeft;
                        float barThickness = (std::max)(1.0f, boxH * 0.045f); // 4.5% of height, min 1px
                        float barOffset = (std::max)(2.0f, boxH * 0.075f);    // 7.5% of height, min 2px

                        // Disable heavy effects for tiny boxes to avoid "blob" look
                        bool isTiny = (boxH < 35.0f);

                        const auto healthSlot = espLayout.TakeBar(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_health_pos),
                            barThickness,
                            barOffset);

                        ImU32 hpColor = IM_COL32(0, 255, 100, 255); // Super Vibrant Neon Green
                        if (healthPercent < 0.75f) hpColor = IM_COL32(255, 255, 0, 255);
                        if (healthPercent < 0.35f) hpColor = IM_COL32(255, 50, 50, 255);
                        hpColor = ApplyAlpha(hpColor, alphaMult);
                        ImU32 bgCol = IM_COL32(0, 0, 0, (int)(150 * alphaMult));

                        auto DrawHealthSegmented = [&](ImVec2 pMin, ImVec2 pMax, bool vertical) {
                            if (!isTiny) {
                                // 1. Deep Shadow / Outer Border (Only for large boxes)
                                draw->AddRect(ImVec2(pMin.x - 1, pMin.y - 1), ImVec2(pMax.x + 1, pMax.y + 1), IM_COL32(0, 0, 0, (int)(180 * alphaMult)), 1.5f);
                            }

                            // 2. Glass Background
                            draw->AddRectFilled(pMin, pMax, bgCol, 1.0f);

                            if (vertical) {
                                float h = pMax.y - pMin.y;
                                float barH = h * healthPercent;
                                ImVec2 hpMax = pMax;
                                ImVec2 hpMin = ImVec2(pMin.x, pMax.y - barH);

                                // 3. Vibrant Health Fill
                                draw->AddRectFilled(hpMin, hpMax, hpColor, 1.0f);

                                // 4. Glass Glint (Only if not tiny)
                                if (!isTiny) {
                                    float glintW = (pMax.x - pMin.x) * 0.45f;
                                    draw->AddRectFilled(hpMin, ImVec2(hpMin.x + glintW, hpMax.y), IM_COL32(255, 255, 255, (int)(50 * alphaMult)), 1.0f);

                                    // 5. Segments
                                    if (boxH > 45.0f) {
                                        for (int i = 1; i <= 3; i++) {
                                            float lineY = pMax.y - (h * (i * 0.25f));
                                            draw->AddLine(ImVec2(pMin.x, lineY), ImVec2(pMax.x, lineY), IM_COL32(0, 0, 0, 100));
                                        }
                                    }
                                }
                            } else {
                                float w_bar = pMax.x - pMin.x;
                                float barW = w_bar * healthPercent;
                                ImVec2 hpMin = pMin;
                                ImVec2 hpMax = ImVec2(pMin.x + barW, pMax.y);

                                // 3. Vibrant Health Fill
                                draw->AddRectFilled(hpMin, hpMax, hpColor, 1.0f);

                                if (!isTiny) {
                                    float glintH = (pMax.y - pMin.y) * 0.45f;
                                    draw->AddRectFilled(hpMin, ImVec2(hpMax.x, hpMin.y + glintH), IM_COL32(255, 255, 255, (int)(50 * alphaMult)), 1.0f);

                                    if (boxH > 45.0f) {
                                        for (int i = 1; i <= 3; i++) {
                                            float lineX = pMin.x + (w_bar * (i * 0.25f));
                                            draw->AddLine(ImVec2(lineX, pMin.y), ImVec2(lineX, pMax.y), IM_COL32(0, 0, 0, 100));
                                        }
                                    }
                                }
                            }
                        };

                        DrawHealthSegmented(healthSlot.Min, healthSlot.Max, !healthSlot.Horizontal);
                    }

                    if (g_Menu.esp_skeleton && player.Distance < g_Menu.skeleton_max_dist) {
                        auto DrawLine = [&](Vector3 b1, Vector3 b2) {
                            if (b1.IsZero() || b2.IsZero()) return;
                            Vector2 s1, s2;
                            if (telemetryContext::WorldToScreen(b1 + delta, s1) && telemetryContext::WorldToScreen(b2 + delta, s2)) {
                                ImU32 skelCol = player.IsVisible ?
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color) :
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_invisible_color);

                                draw->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), ApplyAlpha(skelCol, alphaMult), g_Menu.skel_thickness);
                            }
                        };
                        DrawLine(player.Bone_Head, player.Bone_Neck);
                        DrawLine(player.Bone_Neck, player.Bone_Chest);
                        DrawLine(player.Bone_Chest, player.Bone_Pelvis);
                        DrawLine(player.Bone_Neck, player.Bone_LShoulder); // Changed from Chest to Neck
                        DrawLine(player.Bone_LShoulder, player.Bone_LElbow);
                        DrawLine(player.Bone_LElbow, player.Bone_LHand);
                        DrawLine(player.Bone_Neck, player.Bone_RShoulder); // Changed from Chest to Neck
                        DrawLine(player.Bone_RShoulder, player.Bone_RElbow);
                        DrawLine(player.Bone_RElbow, player.Bone_RHand);
                        DrawLine(player.Bone_Pelvis, player.Bone_LThigh);
                        DrawLine(player.Bone_LThigh, player.Bone_LKnee);
                        DrawLine(player.Bone_LKnee, player.Bone_LFoot);
                        DrawLine(player.Bone_Pelvis, player.Bone_RThigh);
                        DrawLine(player.Bone_RThigh, player.Bone_RKnee);
                        DrawLine(player.Bone_RKnee, player.Bone_RFoot);
                    }

                    if (esp_snapline) {
                        ImVec2 start = hasLocalS ? ImVec2(local_feet_s.x, local_feet_s.y) : ImVec2(ScreenCenterX, ScreenHeight);
                        draw->AddLine(start, ImVec2(feet_s.x, feet_s.y), IM_COL32(255, 255, 255, 120), 1.25f);
                    }

                    if (g_Menu.esp_teamid && player.Distance < g_Menu.name_max_dist) {
                        const ImVec2 badgeSize(g_Menu.teamid_badge_size, g_Menu.teamid_badge_size);
                        ImVec2 badgePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_teamid_pos),
                            badgeSize,
                            2.0f);
                        ImU32 badgeCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.teamid_color);
                        DrawPlayerTeamBadge(draw, badgePos, g_Menu.teamid_badge_size, player.TeamID,
                            ApplyAlpha(badgeCol, alphaMult), alphaMult);
                    }

                    if (g_Menu.esp_killcount && player.Distance < g_Menu.name_max_dist) {
                        const std::string killText = std::string(skCrypt("K: ")) + std::to_string(player.Kills);
                        ImVec2 killSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.kill_font_size, FLT_MAX, 0.0f, killText.c_str());
                        ImVec2 killPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_killcount_pos),
                            killSize,
                            1.0f);
                        ImU32 killCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.kill_color);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), g_Menu.kill_font_size, killPos,
                            ApplyAlpha(killCol, alphaMult), killText.c_str());
                    }

                    if (g_Menu.esp_survival_level && player.Distance < g_Menu.name_max_dist) {
                        const std::string levelText = std::string(skCrypt("Lv.")) + std::to_string(player.SurvivalLevel);
                        ImVec2 levelSize = ImGui::GetFont()->CalcTextSizeA(g_Menu.survival_level_font_size, FLT_MAX, 0.0f, levelText.c_str());
                        ImVec2 levelPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_survival_level_pos),
                            levelSize,
                            1.0f);
                        ImU32 levelCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.survival_level_color);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), g_Menu.survival_level_font_size, levelPos,
                            ApplyAlpha(levelCol, alphaMult), levelText.c_str());
                    }

                    // 1. Distance below feet
                    if (g_Menu.esp_distance && player.Distance < g_Menu.distance_txt_max_dist) {
                        char distStr[32];
                        sprintf_s(distStr, sizeof(distStr), "[%dm]", (int)player.Distance);

                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = g_Menu.distance_font_size;
                        ImVec2 distSize = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, distStr);

                        ImU32 distCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color);
                        ImVec2 distPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_distance_pos),
                            distSize,
                            2.0f);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, distPos, ApplyAlpha(distCol, alphaMult), distStr);
                    }

                    // 2. Weapon above head
                    if (g_Menu.esp_weapon && !player.WeaponName.empty() && player.Distance < g_Menu.weapon_max_dist) {
                        if (g_Menu.esp_weapon_type == 1) { // IMAGE
                            TextureInfo* tex = OverlayTextures::GetWeaponImage(player.WeaponName);
                            if (tex && tex->SRV) {
                                float distanceScale = (player.Distance < 5.0f) ? 5.0f : player.Distance;
                                float targetWidth = (g_Menu.weapon_icon_size * 20.0f) / distanceScale;
                                if (targetWidth < g_Menu.weapon_icon_size * 0.35f) targetWidth = g_Menu.weapon_icon_size * 0.35f;
                                if (targetWidth > g_Menu.weapon_icon_size) targetWidth = g_Menu.weapon_icon_size;

                                float scale = targetWidth / tex->Width;
                                float iconW = tex->Width * scale;
                                float iconH = tex->Height * scale;

                                ImVec2 iconPos = espLayout.Take(
                                    PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                    ImVec2(iconW, iconH),
                                    2.0f);
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddImage((ImTextureID)tex->SRV, iconPos, ImVec2(iconPos.x + iconW, iconPos.y + iconH), ImVec2(0,0), ImVec2(1,1), ApplyAlpha(weaponCol, alphaMult * 0.85f));
                            } else {
                                ImVec2 ws = ImGui::GetFont()->CalcTextSizeA(g_Menu.weapon_font_size, FLT_MAX, 0.0f, player.WeaponName.c_str());
                                ImVec2 weaponPos = espLayout.Take(
                                    PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                    ws,
                                    2.0f);
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddText(ImGui::GetFont(), g_Menu.weapon_font_size, weaponPos,
                                    ApplyAlpha(weaponCol, alphaMult), player.WeaponName.c_str());
                            }
                        } else { // TEXT
                            ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                            float baseFontSize = g_Menu.weapon_font_size;
                            ImVec2 ws = ImGui::GetFont()->CalcTextSizeA(baseFontSize, FLT_MAX, 0.0f, player.WeaponName.c_str());
                            ImVec2 weaponPos = espLayout.Take(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                ws,
                                2.0f);
                            draw->AddText(ImGui::GetFont(), baseFontSize, weaponPos, ApplyAlpha(weaponCol, alphaMult), player.WeaponName.c_str());
                        }
                    }

                    // 2.5. RANK TIER
                    if (g_Menu.esp_rank && player.Distance < g_Menu.name_max_dist) {
                        std::string rankStr = GetRankTierName(player.Kills);
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }
                        float baseFontSize = g_Menu.rank_font_size;
                        ImVec2 rs = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, rankStr.c_str());
                        ImVec2 rankPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_rank_pos),
                            rs,
                            1.0f);
                        ImU32 rankCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.rank_color);

                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, rankPos, ApplyAlpha(rankCol, alphaMult), rankStr.c_str());
                    }

                    // 3. Name and status
                    if (g_Menu.esp_name && player.Distance < g_Menu.name_max_dist) {
                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        std::string nameText = player.Name;
                        if (nameText.empty() || nameText == "Player") nameText = "Unknown";

                        float baseFontSize = g_Menu.name_font_size;
                        ImVec2 ns = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, nameText.c_str());
                        ImVec2 namePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_name_pos),
                            ns,
                            2.0f);

                        ImU32 nameCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_color);
                        if (player.IsVisible) nameCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color);
                        if (player.IsGroggy) nameCol = IM_COL32(255, 0, 0, 255);

                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, namePos, ApplyAlpha(nameCol, alphaMult), nameText.c_str());
                    }

                    // 4. SPECTATED COUNT (EYE WARNING)
                    if (g_Menu.esp_spectated && player.SpectatedCount > 0 && player.Distance < g_Menu.name_max_dist) {
                        char specBuf[64];
                        sprintf_s(specBuf, sizeof(specBuf), "EYE: %d", player.SpectatedCount);

                        float textScale = 1.0f;
                        if (player.Distance > 50.0f) {
                            textScale = 1.0f - ((player.Distance - 50.0f) / 1000.0f);
                            if (textScale < 0.65f) textScale = 0.65f;
                        }

                        float baseFontSize = g_Menu.spectated_font_size; // Slightly larger for alert
                        ImVec2 ss = ImGui::GetFont()->CalcTextSizeA(baseFontSize * textScale, FLT_MAX, 0.0f, specBuf);
                        ImVec2 specPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_spectated_pos),
                            ss,
                            2.0f);

                        // Vibrant Orange/Yellow Warning Color
                        ImU32 specCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.spectated_color);

                        // Outline
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, ImVec2(specPos.x + 1, specPos.y + 1), IM_COL32(0, 0, 0, (int)(200 * alphaMult)), specBuf);
                        ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), baseFontSize * textScale, specPos, ApplyAlpha(specCol, alphaMult), specBuf);
                    }
                } else {
                    // --- 4. ADVANCED OFF-SCREEN INDICATORS ---
                    if (g_Menu.esp_offscreen && !player.IsTeammate && player.Distance < g_Menu.render_distance) {
                        float dx = player.Position.x - G_CameraLocation.x;
                        float dy = player.Position.y - G_CameraLocation.y;

                        float angle_rad = atan2f(dy, dx);
                        float cam_yaw_rad = G_CameraRotation.y * (3.14159265f / 180.0f);
                        float rel_angle = angle_rad - cam_yaw_rad - (3.14159265f / 2.0f);

                        float radius = g_Menu.offscreen_radius;
                        ImVec2 arrowPos = ImVec2(ScreenCenterX + cosf(rel_angle) * radius, (ScreenHeight / 2.0f) + sinf(rel_angle) * radius);

                        // --- DISTANCE-BASED COLORING ---
                        ImU32 arrowCol;
                        if (g_Menu.offscreen_color_mode == 1) { // Distance Gradient
                            float t = player.Distance / g_Menu.render_distance;
                            if (t > 1.0f) t = 1.0f;

                            float r = (g_Menu.offscreen_near_color[0] * (1.0f - t) + g_Menu.offscreen_far_color[0] * t) * 255.0f;
                            float g = (g_Menu.offscreen_near_color[1] * (1.0f - t) + g_Menu.offscreen_far_color[1] * t) * 255.0f;
                            float b = (g_Menu.offscreen_near_color[2] * (1.0f - t) + g_Menu.offscreen_far_color[2] * t) * 255.0f;
                            float a = (g_Menu.offscreen_near_color[3] * (1.0f - t) + g_Menu.offscreen_far_color[3] * t) * 255.0f;
                            arrowCol = IM_COL32((int)r, (int)g, (int)b, (int)a);
                        } else { // Static / Visibility
                            arrowCol = player.IsVisible ? IM_COL32(0, 255, 150, 180) : IM_COL32(255, 255, 255, 100);
                        }

                        if (player.SpectatedCount > 0) arrowCol = IM_COL32(255, 170, 0, 220);

                        float sz = g_Menu.offscreen_size;

                        if (g_Menu.esp_offscreen_style == 0) { // TRIANGLE
                            ImVec2 p1 = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 p2 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 p3 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddTriangleFilled(p1, p2, p3, ApplyAlpha(arrowCol, alphaMult));
                        }
                        else if (g_Menu.esp_offscreen_style == 1) { // CHEVRON (V-SHAPE)
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 side1 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 side2 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddPolyline(&tip, 1, ApplyAlpha(arrowCol, alphaMult), 0, 2.0f); // Just points for now, need proper V
                            // Standard V-shape
                            draw->AddLine(tip, side1, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                            draw->AddLine(tip, side2, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                        }
                        else if (g_Menu.esp_offscreen_style == 2) { // ARC / MODERN
                            draw->PathArcTo(arrowPos, sz, rel_angle - 0.8f, rel_angle + 0.8f, 10);
                            draw->PathStroke(ApplyAlpha(arrowCol, alphaMult), 0, 3.0f);
                            // Add a small tip
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.2f, arrowPos.y + sinf(rel_angle) * sz * 1.2f);
                            draw->AddCircleFilled(tip, 2.0f, ApplyAlpha(arrowCol, alphaMult));
                        }
                    }
                }
            }
}
