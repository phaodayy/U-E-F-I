#include "../core/overlay_menu.hpp"
#include "../core/colors.hpp"
#include "../core/overlay_health_bar.hpp"
#include "player_esp_layout.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/core/math.hpp"
#include <protec/skCrypt.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

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

int AlphaByte(float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return static_cast<int>(alpha * 255.0f);
}

ImU32 WithAlpha(ImU32 color, float alphaMult) {
    int alpha = static_cast<int>(((color >> 24) & 0xFF) * std::clamp(alphaMult, 0.0f, 1.0f));
    return (color & 0x00FFFFFF) | (alpha << 24);
}

ImU32 TeamColorFromMenu(int teamID) {
    if (g_Menu.team_color_custom) {
        const int slot = teamID < 0 ? 0 : (teamID % 4);
        const float* color = g_Menu.team_custom_colors[slot];
        return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
    }
    return telemetryColors::GetTeamColor(teamID);
}

ImVec2 TextSize(const char* text, float fontSize) {
    return ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
}

ImVec2 ChipSize(const ImVec2& textSize) {
    return ImVec2(textSize.x + 8.0f, textSize.y + 4.0f);
}

void DrawOutlinedText(ImDrawList* draw, const ImVec2& pos, ImU32 color,
                      const char* text, float fontSize, float alphaMult) {
    const ImU32 shadow = IM_COL32(0, 0, 0, AlphaByte(0.48f * alphaMult));
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), shadow, text);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x - 1.0f, pos.y + 1.0f), shadow, text);
    draw->AddText(ImGui::GetFont(), fontSize, pos, WithAlpha(color, alphaMult), text);
}

void DrawTextChip(ImDrawList* draw, const ImVec2& pos, const char* text, float fontSize,
                  ImU32 textColor, float alphaMult, bool warning = false) {
    const ImVec2 textSz = TextSize(text, fontSize);
    const ImVec2 chipMin(pos.x, pos.y);
    const ImVec2 chipMax(pos.x + textSz.x + 8.0f, pos.y + textSz.y + 4.0f);
    if (g_Menu.esp_text_background && g_Menu.esp_text_bg_alpha > 0.01f) {
        const float chipAlpha = warning ?
            std::clamp(g_Menu.esp_text_bg_alpha + 0.14f, 0.0f, 0.70f) :
            std::clamp(g_Menu.esp_text_bg_alpha, 0.0f, 0.70f);
        const ImU32 bg = warning ?
            IM_COL32(42, 22, 5, AlphaByte(chipAlpha * alphaMult)) :
            IM_COL32(5, 8, 12, AlphaByte(chipAlpha * alphaMult));
        const ImU32 border = warning ?
            IM_COL32(255, 178, 64, AlphaByte(0.34f * alphaMult)) :
            IM_COL32(255, 255, 255, AlphaByte(0.10f * alphaMult));

        draw->AddRectFilled(ImVec2(chipMin.x + 1.0f, chipMin.y + 1.0f), ImVec2(chipMax.x + 1.0f, chipMax.y + 1.0f),
            IM_COL32(0, 0, 0, AlphaByte(0.12f * alphaMult)), 4.0f);
        draw->AddRectFilled(chipMin, chipMax, bg, 4.0f);
        draw->AddRect(chipMin, chipMax, border, 4.0f, 0, 1.0f);
    }
    DrawOutlinedText(draw, ImVec2(pos.x + 4.0f, pos.y + 2.0f), textColor, text, fontSize, alphaMult);
}

void DrawNameplateChip(ImDrawList* draw, const ImVec2& pos, const char* text, float fontSize,
                       ImU32 textColor, ImU32 accentColor, float alphaMult, bool warning = false) {
    if (!g_Menu.esp_multilayer_nameplate) {
        DrawTextChip(draw, pos, text, fontSize, textColor, alphaMult, warning);
        return;
    }

    const ImVec2 textSz = TextSize(text, fontSize);
    const ImVec2 chipMin(pos.x, pos.y);
    const ImVec2 chipMax(pos.x + textSz.x + 14.0f, pos.y + textSz.y + 5.0f);
    const float bgAlpha = std::clamp(g_Menu.esp_text_bg_alpha, 0.0f, 0.58f);
    const ImU32 bg = warning ?
        IM_COL32(42, 20, 8, AlphaByte((bgAlpha + 0.12f) * alphaMult)) :
        IM_COL32(7, 12, 18, AlphaByte(bgAlpha * alphaMult));
    draw->AddRectFilled(ImVec2(chipMin.x + 1.0f, chipMin.y + 1.0f),
        ImVec2(chipMax.x + 1.0f, chipMax.y + 1.0f), IM_COL32(0, 0, 0, AlphaByte(0.12f * alphaMult)), 4.0f);
    draw->AddRectFilled(chipMin, chipMax, bg, 4.0f);
    draw->AddRectFilled(ImVec2(chipMin.x, chipMin.y), ImVec2(chipMin.x + 3.0f, chipMax.y),
        WithAlpha(accentColor, alphaMult), 4.0f, ImDrawFlags_RoundCornersLeft);
    draw->AddRect(chipMin, chipMax, WithAlpha(accentColor, 0.34f * alphaMult), 4.0f, 0, 1.0f);
    DrawOutlinedText(draw, ImVec2(pos.x + 8.0f, pos.y + 2.0f), textColor, text, fontSize, alphaMult);
}

ImVec2 AmmoBadgeSize(int ammo, int ammoMax, float fontSize) {
    char ammoText[32] = {};
    if (ammoMax > 0) sprintf_s(ammoText, sizeof(ammoText), skCrypt("%d/%d"), ammo, ammoMax);
    else sprintf_s(ammoText, sizeof(ammoText), skCrypt("%d"), ammo);
    const ImVec2 textSz = TextSize(ammoText, fontSize);
    return ImVec2((std::max)(48.0f, textSz.x + 18.0f), textSz.y + 8.0f);
}

void DrawAmmoBadge(ImDrawList* draw, const ImVec2& pos, int ammo, int ammoMax, float fontSize,
                   ImU32 color, float alphaMult, bool warning) {
    const ImVec2 size = AmmoBadgeSize(ammo, ammoMax, fontSize);
    const ImVec2 badgeMin(pos.x, pos.y);
    const ImVec2 badgeMax(pos.x + size.x, pos.y + size.y);
    const float pulse = warning ? (0.55f + 0.45f * std::sin(GetTickCount64() * 0.018f)) : 0.0f;
    const ImU32 bg = warning ? IM_COL32(50, 12, 8, AlphaByte((0.56f + 0.16f * pulse) * alphaMult)) :
        IM_COL32(5, 8, 12, AlphaByte(0.64f * alphaMult));

    draw->AddRectFilled(ImVec2(badgeMin.x + 1.0f, badgeMin.y + 1.0f), ImVec2(badgeMax.x + 1.0f, badgeMax.y + 1.0f),
        IM_COL32(0, 0, 0, AlphaByte(0.22f * alphaMult)), 4.0f);
    draw->AddRectFilled(badgeMin, badgeMax, bg, 4.0f);
    draw->AddRect(badgeMin, badgeMax, WithAlpha(color, warning ? 0.78f : 0.34f), 4.0f, 0, 1.0f);

    const float percent = ammoMax > 0 ? std::clamp(static_cast<float>(ammo) / static_cast<float>(ammoMax), 0.0f, 1.0f) : 1.0f;
    const float fillW = (size.x - 8.0f) * percent;
    draw->AddRectFilled(ImVec2(badgeMin.x + 4.0f, badgeMax.y - 4.0f),
        ImVec2(badgeMin.x + 4.0f + fillW, badgeMax.y - 2.0f),
        WithAlpha(color, warning ? 1.0f : 0.92f), 2.0f);

    char ammoText[32] = {};
    if (ammoMax > 0) sprintf_s(ammoText, sizeof(ammoText), skCrypt("%d/%d"), ammo, ammoMax);
    else sprintf_s(ammoText, sizeof(ammoText), skCrypt("%d"), ammo);
    const ImVec2 textSz = TextSize(ammoText, fontSize);
    DrawOutlinedText(draw, ImVec2(badgeMin.x + (size.x - textSz.x) * 0.5f, badgeMin.y + 3.0f),
        color, ammoText, fontSize, alphaMult);
}

void DrawCornerBox(ImDrawList* draw, const ImVec2& min, const ImVec2& max, ImU32 color,
                   float thickness, float alphaMult) {
    const float w = max.x - min.x;
    const float h = max.y - min.y;
    const float len = std::clamp((std::min)(w, h) * 0.28f, 8.0f, 22.0f);
    const ImU32 shadow = IM_COL32(0, 0, 0, AlphaByte(0.72f * alphaMult));
    const float shadowThick = thickness + 1.35f;

    auto line = [&](ImVec2 a, ImVec2 b, ImU32 col, float thick) {
        draw->AddLine(a, b, col, thick);
    };

    const ImVec2 tl(min.x, min.y);
    const ImVec2 tr(max.x, min.y);
    const ImVec2 bl(min.x, max.y);
    const ImVec2 br(max.x, max.y);

    for (int pass = 0; pass < 2; ++pass) {
        const ImU32 col = pass == 0 ? shadow : WithAlpha(color, alphaMult);
        const float thick = pass == 0 ? shadowThick : thickness;
        line(tl, ImVec2(tl.x + len, tl.y), col, thick);
        line(tl, ImVec2(tl.x, tl.y + len), col, thick);
        line(tr, ImVec2(tr.x - len, tr.y), col, thick);
        line(tr, ImVec2(tr.x, tr.y + len), col, thick);
        line(bl, ImVec2(bl.x + len, bl.y), col, thick);
        line(bl, ImVec2(bl.x, bl.y - len), col, thick);
        line(br, ImVec2(br.x - len, br.y), col, thick);
        line(br, ImVec2(br.x, br.y - len), col, thick);
    }
}

void DrawEspBox(ImDrawList* draw, const ImVec2& min, const ImVec2& max, ImU32 color,
                float thickness, float alphaMult, bool cornerStyle, bool fill) {
    if (fill) {
        draw->AddRectFilled(min, max, IM_COL32(0, 0, 0, AlphaByte(0.08f * alphaMult)), 3.0f);
    }

    if (cornerStyle) {
        DrawCornerBox(draw, min, max, color, thickness, alphaMult);
        return;
    }

    draw->AddRect(min, max, IM_COL32(0, 0, 0, AlphaByte(0.72f * alphaMult)), 3.0f, 0, thickness + 1.35f);
    draw->AddRect(min, max, WithAlpha(color, alphaMult), 3.0f, 0, thickness);
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
                            const float referenceH = std::fabs(head_s.y - feet_s.y);
                            const float aspect = boxH > 1.0f ? boxW / boxH : 999.0f;
                            const bool saneBoneBounds =
                                referenceH > 8.0f &&
                                boxH > referenceH * 0.55f &&
                                boxH < referenceH * 1.70f &&
                                aspect > 0.12f &&
                                aspect < 0.78f;

                            // Padding for "Breathing Room" (15% Width, 12% Height)
                            float paddingW = (std::max)(boxW * 0.15f, 4.0f);
                            float paddingH = (std::max)(boxH * 0.12f, 4.0f);

                            finalBoxTop = minY - (boxH * 0.25f) - 5.0f; // 25% Height + 5px buffer to ensure it clears Helmet/Face
                            finalBoxBottom = maxY + paddingH;
                            finalBoxLeft = minX - paddingW;
                            finalBoxRight = maxX + paddingW;

                            // Sanity check to avoid zero-sized boxes
                            if (saneBoneBounds && finalBoxBottom - finalBoxTop > 5.0f && finalBoxRight - finalBoxLeft > 2.0f) {
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

                    if (esp_box && player.Distance < g_Menu.box_max_dist) {
                        float thick = g_Menu.box_thickness;
                        DrawEspBox(draw,
                            ImVec2(finalBoxLeft, finalBoxTop),
                            ImVec2(finalBoxRight, finalBoxBottom),
                            boxCol,
                            thick,
                            alphaMult,
                            g_Menu.esp_box_type == 1,
                            g_Menu.esp_fillbox);
                    }

                    if (g_Menu.esp_aim_warning && player.IsAimingAtLocal && player.Distance < g_Menu.box_max_dist) {
                        ImU32 aimCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.aim_warning_color);
                        draw->AddRect(
                            ImVec2(finalBoxLeft - 2.0f, finalBoxTop - 2.0f),
                            ImVec2(finalBoxRight + 2.0f, finalBoxBottom + 2.0f),
                            ApplyAlpha(aimCol, alphaMult),
                            4.0f,
                            0,
                            (std::max)(2.0f, g_Menu.box_thickness + 0.8f));
                    }

                    PlayerEspLayout::Stack espLayout(finalBoxLeft, finalBoxTop, finalBoxRight, finalBoxBottom);

                    // --- HEALTH DISPLAY ---
                    if (g_Menu.esp_health && player.Distance < g_Menu.hp_max_dist) {
                        float displayHealth = player.IsGroggy ? player.GroggyHealth : player.Health;
                        float healthPercent = displayHealth / 100.0f;
                        healthPercent = std::clamp(healthPercent, 0.0f, 1.0f);

                        float boxH = finalBoxBottom - finalBoxTop;
                        ImU32 hpColor = IM_COL32(68, 230, 132, 255);
                        if (g_Menu.esp_health_color_mode == 1) {
                            hpColor = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.health_color);
                        } else if (healthPercent < 0.35f) {
                            hpColor = IM_COL32(245, 76, 76, 255);
                        } else if (healthPercent < 0.70f) {
                            hpColor = IM_COL32(245, 189, 71, 255);
                        }
                        if (g_Menu.esp_health_display_mode == 1) {
                            char hpText[32];
                            if (player.IsGroggy) {
                                sprintf_s(hpText, sizeof(hpText), skCrypt("DBNO %.0f"), displayHealth);
                            } else {
                                sprintf_s(hpText, sizeof(hpText), skCrypt("%.0f HP"), displayHealth);
                            }
                            const float hpFont = (std::max)(9.0f, g_Menu.distance_font_size - 1.0f);
                            const ImVec2 hpSize = TextSize(hpText, hpFont);
                            const ImVec2 chipSize = ChipSize(hpSize);
                            const ImVec2 hpPos = espLayout.Take(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_health_pos),
                                chipSize,
                                2.0f);
                            DrawTextChip(draw, hpPos, hpText, hpFont, hpColor, alphaMult,
                                player.IsGroggy || healthPercent < 0.35f);
                        } else {
                            float barThickness = std::clamp(boxH * 0.045f, 5.0f, 9.0f);
                            if (g_Menu.esp_health_bar_style == 1 || g_Menu.esp_health_bar_style == 3) {
                                barThickness = (std::max)(barThickness, 7.0f);
                            }
                            const float barOffset = std::clamp(boxH * 0.060f, 3.0f, 9.0f);
                            const auto healthSlot = espLayout.TakeBar(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_health_pos),
                                barThickness,
                                barOffset);
                            OverlayHealthBar::Draw(draw, healthSlot.Min, healthSlot.Max, healthPercent, hpColor,
                                !healthSlot.Horizontal, g_Menu.esp_health_bar_style, alphaMult,
                                player.IsGroggy || healthPercent < 0.35f);
                        }
                    }

                    if (g_Menu.esp_skeleton && player.Distance < g_Menu.skeleton_max_dist) {
                        auto DrawLine = [&](Vector3 b1, Vector3 b2) {
                            if (b1.IsZero() || b2.IsZero()) return;
                            Vector2 s1, s2;
                            if (telemetryContext::WorldToScreen(b1 + delta, s1) && telemetryContext::WorldToScreen(b2 + delta, s2)) {
                                const float boxH = finalBoxBottom - finalBoxTop;
                                const float boxW = finalBoxRight - finalBoxLeft;
                                const float marginX = (std::max)(boxW * 0.38f, 18.0f);
                                const float marginY = (std::max)(boxH * 0.18f, 18.0f);
                                auto insideRigBounds = [&](const Vector2& p) {
                                    return p.x >= finalBoxLeft - marginX && p.x <= finalBoxRight + marginX &&
                                           p.y >= finalBoxTop - marginY && p.y <= finalBoxBottom + marginY;
                                };
                                const float dx = s1.x - s2.x;
                                const float dy = s1.y - s2.y;
                                const float lineLen = std::sqrt(dx * dx + dy * dy);
                                if (!insideRigBounds(s1) || !insideRigBounds(s2) || lineLen > boxH * 0.58f) return;

                                ImU32 skelCol = player.IsVisible ?
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color) :
                                    ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_invisible_color);

                                const ImVec2 p1(s1.x, s1.y);
                                const ImVec2 p2(s2.x, s2.y);
                                draw->AddLine(p1, p2, ApplyAlpha(skelCol, alphaMult), g_Menu.skel_thickness);
                                if (g_Menu.esp_skeleton_dots && (finalBoxBottom - finalBoxTop) > 45.0f) {
                                    const float jointRadius = std::clamp((finalBoxBottom - finalBoxTop) * 0.012f, 1.0f, 2.1f);
                                    draw->AddCircleFilled(p1, jointRadius, ApplyAlpha(skelCol, alphaMult * 0.92f), 8);
                                }
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

                    if (g_Menu.esp_head_circle && player.Distance < g_Menu.skeleton_max_dist) {
                        const float boxW = finalBoxRight - finalBoxLeft;
                        const float radius = std::clamp(boxW * 0.18f, 3.5f, 10.0f);
                        ImU32 headCol = player.IsVisible ?
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color) :
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_invisible_color);
                        draw->AddCircle(ImVec2(head_s.x, head_s.y), radius, ApplyAlpha(headCol, alphaMult), 22, 1.2f);
                    }

                    if (esp_snapline) {
                        ImVec2 start(ScreenCenterX, ScreenHeight);
                        if (g_Menu.snapline_type == 1) {
                            start = ImVec2(ScreenCenterX, ScreenHeight * 0.5f);
                        } else if (g_Menu.snapline_type == 2) {
                            start = ImVec2(ScreenCenterX, 0.0f);
                        } else if (g_Menu.snapline_type == 3 && hasLocalS) {
                            start = ImVec2(local_feet_s.x, local_feet_s.y);
                        }
                        ImU32 snapCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.snapline_color);
                        draw->AddLine(start, ImVec2(feet_s.x, feet_s.y), IM_COL32(0, 0, 0, AlphaByte(0.34f * alphaMult)), 2.3f);
                        draw->AddLine(start, ImVec2(feet_s.x, feet_s.y), ApplyAlpha(snapCol, alphaMult * 0.62f), 1.2f);
                    }

                    if (g_Menu.esp_view_direction && player.HasAimYaw && player.Distance < g_Menu.name_max_dist) {
                        Vector3 start = player.Bone_Chest.IsZero() ? (player.Position + Vector3{0.0f, 0.0f, 90.0f}) : player.Bone_Chest;
                        const float yawRad = player.AimYaw * 0.017453292519943f;
                        const float pitchRad = player.AimPitch * 0.017453292519943f;
                        const float length = (std::max)(5.0f, g_Menu.esp_view_direction_length) * 100.0f;
                        Vector3 end = start + Vector3{
                            static_cast<float>(std::cos(yawRad) * length),
                            static_cast<float>(std::sin(yawRad) * length),
                            static_cast<float>(std::sin(pitchRad) * length * 0.45f)
                        };

                        Vector2 dirStart, dirEnd;
                        if (telemetryContext::WorldToScreen(start + delta, dirStart) &&
                            telemetryContext::WorldToScreen(end + delta, dirEnd)) {
                            ImU32 dirCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.view_direction_color);
                            if (player.IsAimingAtLocal) {
                                dirCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.aim_warning_color);
                            }
                            draw->AddLine(ImVec2(dirStart.x + 1.0f, dirStart.y + 1.0f),
                                ImVec2(dirEnd.x + 1.0f, dirEnd.y + 1.0f),
                                IM_COL32(0, 0, 0, AlphaByte(0.45f * alphaMult)), 2.8f);
                            draw->AddLine(ImVec2(dirStart.x, dirStart.y), ImVec2(dirEnd.x, dirEnd.y),
                                ApplyAlpha(dirCol, alphaMult), 1.5f);
                            draw->AddCircleFilled(ImVec2(dirEnd.x, dirEnd.y), 2.2f, ApplyAlpha(dirCol, alphaMult), 8);
                        }
                    }

                    if (g_Menu.esp_teamid && player.Distance < g_Menu.name_max_dist) {
                        const ImVec2 badgeSize(g_Menu.teamid_badge_size, g_Menu.teamid_badge_size);
                        ImVec2 badgePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_teamid_pos),
                            badgeSize,
                            2.0f);
                        ImU32 badgeCol = g_Menu.team_color_custom ?
                            TeamColorFromMenu(player.TeamID) :
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.teamid_color);
                        DrawPlayerTeamBadge(draw, badgePos, g_Menu.teamid_badge_size, player.TeamID,
                            ApplyAlpha(badgeCol, alphaMult), alphaMult);
                    }

                    if ((g_Menu.esp_aim_warning || g_Menu.esp_status_badges) && player.Distance < g_Menu.name_max_dist) {
                        std::string statusText;
                        bool statusWarning = false;
                        if (g_Menu.esp_aim_warning && player.IsAimingAtLocal) {
                            statusText += skCrypt("AIM");
                            statusWarning = true;
                        }
                        if (g_Menu.esp_status_badges && player.IsBot) {
                            if (!statusText.empty()) statusText += skCrypt(" | ");
                            statusText += skCrypt("AI");
                        }
                        if (g_Menu.esp_status_badges && player.IsGroggy) {
                            if (!statusText.empty()) statusText += skCrypt(" | ");
                            statusText += skCrypt("DBNO");
                            statusWarning = true;
                        }
                        if (g_Menu.esp_status_badges && player.IsReloading) {
                            if (!statusText.empty()) statusText += skCrypt(" | ");
                            statusText += skCrypt("RELOAD");
                            statusWarning = true;
                        }
                        if (g_Menu.esp_status_badges && player.IsScoping) {
                            if (!statusText.empty()) statusText += skCrypt(" | ");
                            statusText += skCrypt("ADS");
                        }

                        if (!statusText.empty()) {
                            const float statusFont = (std::max)(9.0f, g_Menu.spectated_font_size - 2.0f);
                            ImVec2 statusSize = ChipSize(TextSize(statusText.c_str(), statusFont));
                            ImVec2 statusPos = espLayout.Take(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_spectated_pos),
                                statusSize,
                                2.0f);
                            ImU32 statusCol = statusWarning ?
                                ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.aim_warning_color) :
                                ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.view_direction_color);
                            DrawTextChip(draw, statusPos, statusText.c_str(), statusFont, statusCol, alphaMult, statusWarning);
                        }
                    }

                    if (g_Menu.esp_close_warning && !player.IsTeammate &&
                        player.Distance > 0.0f && player.Distance <= g_Menu.esp_close_warning_distance) {
                        char closeText[32];
                        sprintf_s(closeText, sizeof(closeText), skCrypt("! %dm"), static_cast<int>(player.Distance));
                        const float closeFont = (std::max)(10.0f, g_Menu.spectated_font_size - 1.0f);
                        ImVec2 closeSize = ChipSize(TextSize(closeText, closeFont));
                        ImVec2 closePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_spectated_pos),
                            closeSize,
                            2.0f);
                        ImU32 closeCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.close_warning_color);
                        DrawTextChip(draw, closePos, closeText, closeFont, closeCol, alphaMult, true);
                    }

                    if (g_Menu.esp_killcount && player.Distance < g_Menu.name_max_dist) {
                        const std::string killText = std::string(skCrypt("K: ")) + std::to_string(player.Kills);
                        ImVec2 killSize = ChipSize(TextSize(killText.c_str(), g_Menu.kill_font_size));
                        ImVec2 killPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_killcount_pos),
                            killSize,
                            1.0f);
                        ImU32 killCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.kill_color);
                        DrawTextChip(draw, killPos, killText.c_str(), g_Menu.kill_font_size, killCol, alphaMult);
                    }

                    if (g_Menu.esp_damage && player.DamageDealt > 0.5f && player.Distance < g_Menu.name_max_dist) {
                        const std::string damageText = std::string(skCrypt("DMG: ")) + std::to_string(static_cast<int>(player.DamageDealt));
                        ImVec2 damageSize = ChipSize(TextSize(damageText.c_str(), g_Menu.damage_font_size));
                        ImVec2 damagePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_damage_pos),
                            damageSize,
                            1.0f);
                        ImU32 damageCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.damage_color);
                        DrawTextChip(draw, damagePos, damageText.c_str(), g_Menu.damage_font_size, damageCol, alphaMult);
                    }

                    if (g_Menu.esp_speed && player.Distance < g_Menu.name_max_dist) {
                        const int speedKmh = static_cast<int>(player.Velocity.Length() * 0.036f);
                        if (speedKmh >= 3) {
                            const std::string speedText = std::string(skCrypt("SPD: ")) + std::to_string(speedKmh);
                            ImVec2 speedSize = ChipSize(TextSize(speedText.c_str(), g_Menu.speed_font_size));
                            ImVec2 speedPos = espLayout.Take(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_speed_pos),
                                speedSize,
                                1.0f);
                            ImU32 speedCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.speed_color);
                            DrawTextChip(draw, speedPos, speedText.c_str(), g_Menu.speed_font_size, speedCol, alphaMult);
                        }
                    }

                    if (g_Menu.esp_survival_level && player.Distance < g_Menu.name_max_dist) {
                        const std::string levelText = std::string(skCrypt("Lv.")) + std::to_string(player.SurvivalLevel);
                        ImVec2 levelSize = ChipSize(TextSize(levelText.c_str(), g_Menu.survival_level_font_size));
                        ImVec2 levelPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_survival_level_pos),
                            levelSize,
                            1.0f);
                        ImU32 levelCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.survival_level_color);
                        DrawTextChip(draw, levelPos, levelText.c_str(), g_Menu.survival_level_font_size, levelCol, alphaMult);
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
                        ImVec2 distSize = ChipSize(TextSize(distStr, baseFontSize * textScale));

                        ImU32 distCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color);
                        ImVec2 distPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_distance_pos),
                            distSize,
                            2.0f);
                        DrawTextChip(draw, distPos, distStr, baseFontSize * textScale, distCol, alphaMult);
                    }

                    if (g_Menu.esp_ammo && player.HasAmmo && player.Distance < g_Menu.weapon_max_dist) {
                        ImVec2 ammoSize = AmmoBadgeSize(player.Ammo, player.AmmoMax, g_Menu.ammo_font_size);
                        ImVec2 ammoPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_ammo_pos),
                            ammoSize,
                            1.0f);
                        ImU32 ammoCol = player.IsReloading ?
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.aim_warning_color) :
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.ammo_color);
                        DrawAmmoBadge(draw, ammoPos, player.Ammo, player.AmmoMax,
                            g_Menu.ammo_font_size, ammoCol, alphaMult, player.IsReloading || player.IsFiring);
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

                                const float frameWidth = static_cast<float>(tex->Width);
                                float scale = targetWidth / frameWidth;
                                float iconW = frameWidth * scale;
                                float iconH = tex->Height * scale;

                                ImVec2 iconPos = espLayout.Take(
                                    PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                    ImVec2(iconW + 6.0f, iconH + 4.0f),
                                    2.0f);
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                draw->AddRectFilled(iconPos, ImVec2(iconPos.x + iconW + 6.0f, iconPos.y + iconH + 4.0f),
                                    IM_COL32(5, 8, 12, AlphaByte(0.46f * alphaMult)), 4.0f);
                                draw->AddRect(iconPos, ImVec2(iconPos.x + iconW + 6.0f, iconPos.y + iconH + 4.0f),
                                    ApplyAlpha(weaponCol, alphaMult * 0.55f), 4.0f, 0, 1.0f);
                                OverlayAssetAnimation::DrawStaticImageRect(draw, tex,
                                    ImVec2(iconPos.x + 3.0f, iconPos.y + 2.0f),
                                    ImVec2(iconPos.x + iconW + 3.0f, iconPos.y + iconH + 2.0f),
                                    IM_COL32(255, 255, 255, 255),
                                    alphaMult * 0.88f);
                            } else {
                                ImVec2 ws = ChipSize(TextSize(player.WeaponName.c_str(), g_Menu.weapon_font_size));
                                ImVec2 weaponPos = espLayout.Take(
                                    PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                    ws,
                                    2.0f);
                                ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                                DrawTextChip(draw, weaponPos, player.WeaponName.c_str(), g_Menu.weapon_font_size, weaponCol, alphaMult);
                            }
                        } else { // TEXT
                            ImU32 weaponCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.weapon_color);
                            float baseFontSize = g_Menu.weapon_font_size;
                            ImVec2 ws = ChipSize(TextSize(player.WeaponName.c_str(), baseFontSize));
                            ImVec2 weaponPos = espLayout.Take(
                                PlayerEspLayout::SideFromMenu(g_Menu.esp_weapon_pos),
                                ws,
                                2.0f);
                            DrawTextChip(draw, weaponPos, player.WeaponName.c_str(), baseFontSize, weaponCol, alphaMult);
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
                        ImVec2 rs = ChipSize(TextSize(rankStr.c_str(), baseFontSize * textScale));
                        ImVec2 rankPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_rank_pos),
                            rs,
                            1.0f);
                        ImU32 rankCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.rank_color);

                        DrawTextChip(draw, rankPos, rankStr.c_str(), baseFontSize * textScale, rankCol, alphaMult);
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
                        const ImVec2 nameTextSize = TextSize(nameText.c_str(), baseFontSize * textScale);
                        ImVec2 ns = g_Menu.esp_multilayer_nameplate ?
                            ImVec2(nameTextSize.x + 14.0f, nameTextSize.y + 5.0f) :
                            ChipSize(nameTextSize);
                        ImVec2 namePos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_name_pos),
                            ns,
                            2.0f);

                        ImU32 nameCol = player.IsVisible ?
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_visible_color) :
                            ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_invisible_color);
                        if (player.IsGroggy) nameCol = IM_COL32(255, 0, 0, 255);

                        DrawNameplateChip(draw, namePos, nameText.c_str(), baseFontSize * textScale,
                            nameCol, TeamColorFromMenu(player.TeamID), alphaMult, player.IsGroggy);
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
                        ImVec2 ss = ChipSize(TextSize(specBuf, baseFontSize * textScale));
                        ImVec2 specPos = espLayout.Take(
                            PlayerEspLayout::SideFromMenu(g_Menu.esp_spectated_pos),
                            ss,
                            2.0f);

                        // Vibrant Orange/Yellow Warning Color
                        ImU32 specCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.spectated_color);

                        DrawTextChip(draw, specPos, specBuf, baseFontSize * textScale, specCol, alphaMult, true);
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

                        if (player.IsAimingAtLocal) {
                            arrowCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.aim_warning_color);
                        } else if (player.SpectatedCount > 0) {
                            arrowCol = IM_COL32(255, 170, 0, 220);
                        }

                        float sz = g_Menu.offscreen_size;

                        if (g_Menu.esp_offscreen_style == 0) { // TRIANGLE
                            ImVec2 p1 = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 p2 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 p3 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddTriangleFilled(
                                ImVec2(p1.x + 1.0f, p1.y + 1.0f),
                                ImVec2(p2.x + 1.0f, p2.y + 1.0f),
                                ImVec2(p3.x + 1.0f, p3.y + 1.0f),
                                IM_COL32(0, 0, 0, AlphaByte(0.55f * alphaMult)));
                            draw->AddTriangleFilled(p1, p2, p3, ApplyAlpha(arrowCol, alphaMult));
                        }
                        else if (g_Menu.esp_offscreen_style == 1) { // CHEVRON (V-SHAPE)
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.5f, arrowPos.y + sinf(rel_angle) * sz * 1.5f);
                            ImVec2 side1 = ImVec2(arrowPos.x + cosf(rel_angle + 2.4f) * sz, arrowPos.y + sinf(rel_angle + 2.4f) * sz);
                            ImVec2 side2 = ImVec2(arrowPos.x + cosf(rel_angle - 2.4f) * sz, arrowPos.y + sinf(rel_angle - 2.4f) * sz);
                            draw->AddLine(ImVec2(tip.x + 1.0f, tip.y + 1.0f), ImVec2(side1.x + 1.0f, side1.y + 1.0f), IM_COL32(0, 0, 0, AlphaByte(0.55f * alphaMult)), 3.8f);
                            draw->AddLine(ImVec2(tip.x + 1.0f, tip.y + 1.0f), ImVec2(side2.x + 1.0f, side2.y + 1.0f), IM_COL32(0, 0, 0, AlphaByte(0.55f * alphaMult)), 3.8f);
                            draw->AddLine(tip, side1, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                            draw->AddLine(tip, side2, ApplyAlpha(arrowCol, alphaMult), 2.5f);
                        }
                        else if (g_Menu.esp_offscreen_style == 2) { // ARC / MODERN
                            draw->PathArcTo(ImVec2(arrowPos.x + 1.0f, arrowPos.y + 1.0f), sz, rel_angle - 0.8f, rel_angle + 0.8f, 10);
                            draw->PathStroke(IM_COL32(0, 0, 0, AlphaByte(0.55f * alphaMult)), 0, 4.2f);
                            draw->PathArcTo(arrowPos, sz, rel_angle - 0.8f, rel_angle + 0.8f, 10);
                            draw->PathStroke(ApplyAlpha(arrowCol, alphaMult), 0, 3.0f);
                            // Add a small tip
                            ImVec2 tip = ImVec2(arrowPos.x + cosf(rel_angle) * sz * 1.2f, arrowPos.y + sinf(rel_angle) * sz * 1.2f);
                            draw->AddCircleFilled(tip, 2.0f, ApplyAlpha(arrowCol, alphaMult));
                        }

                        if (g_Menu.esp_offscreen_text) {
                            std::string offText;
                            if (g_Menu.esp_name && !player.Name.empty() && player.Name != skCrypt("Player")) {
                                offText = player.Name + skCrypt(" ");
                            }
                            offText += std::to_string(static_cast<int>(player.Distance)) + skCrypt("m");

                            const float offFont = (std::max)(9.0f, g_Menu.distance_font_size);
                            ImVec2 textSize = ChipSize(TextSize(offText.c_str(), offFont));
                            ImVec2 textPos(
                                arrowPos.x - textSize.x * 0.5f,
                                arrowPos.y + sz + 6.0f);
                            DrawTextChip(draw, textPos, offText.c_str(), offFont, arrowCol, alphaMult,
                                player.IsAimingAtLocal || player.SpectatedCount > 0);
                        }
                    }
                }
            }
}
