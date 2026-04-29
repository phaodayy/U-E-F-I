#include "overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/memory/memory.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <protec/skCrypt.h>

#include <algorithm>
#include <mutex>

namespace {

struct SpectatorPanelRow {
    std::string Name;
    int Dist = 0;
    float HP = 0.0f;
    int Count = 0;
    bool IsAiming = false;
    bool IsTeammate = false;
};

struct PlayerPanelRow {
    std::string Name;
    std::string Weapon;
    int Team = 0;
    int Dist = 0;
    int Kills = 0;
    int Level = 0;
    int Ammo = 0;
    int AmmoMax = 0;
    int HelmetLevel = 0;
    int VestLevel = 0;
    int BackpackLevel = 0;
    float HelmetDurability = 0.0f;
    float VestDurability = 0.0f;
    float BackpackDurability = 0.0f;
    float HP = 0.0f;
    float Damage = 0.0f;
    bool IsTeammate = false;
    bool IsBot = false;
    bool IsGroggy = false;
    bool IsVisible = false;
    bool IsAiming = false;
    bool IsSpectated = false;
    bool HasAmmo = false;
};

void DrawPanelText(ImDrawList* draw, float fontSize, ImVec2 pos, ImU32 color, const char* text) {
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f),
        IM_COL32(0, 0, 0, 170), text);
    draw->AddText(ImGui::GetFont(), fontSize, pos, color, text);
}

std::string FitPanelText(const std::string& text, float fontSize, float maxWidth) {
    if (text.empty()) return skCrypt("-");
    if (ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.c_str()).x <= maxWidth) {
        return text;
    }

    std::string fitted = text;
    while (fitted.size() > 3) {
        fitted.pop_back();
        const std::string candidate = fitted + skCrypt("...");
        if (ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, candidate.c_str()).x <= maxWidth) {
            return candidate;
        }
    }
    return skCrypt("...");
}

void DrawHealthMiniBar(ImDrawList* draw, ImVec2 pos, float width, float hp, bool groggy) {
    const float height = 5.0f;
    const float t = (std::clamp)(hp / 100.0f, 0.0f, 1.0f);
    const ImU32 hpCol = groggy ? IM_COL32(255, 76, 64, 245) :
        (t > 0.55f ? IM_COL32(58, 230, 130, 245) :
            (t > 0.28f ? IM_COL32(255, 190, 55, 245) : IM_COL32(255, 70, 65, 245)));
    draw->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), IM_COL32(30, 34, 40, 220), 2.0f);
    draw->AddRectFilled(pos, ImVec2(pos.x + width * t, pos.y + height), hpCol, 2.0f);
}

void DrawPanelShell(ImDrawList* draw, ImVec2 pos, ImVec2 size, const char* title, int count, ImU32 accent) {
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(5, 10, 18, 222), 8.0f);
    draw->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 200, 255, 58), 8.0f, 0, 1.2f);
    draw->AddRectFilledMultiColor(ImVec2(pos.x + 1.0f, pos.y + 1.0f),
        ImVec2(pos.x + size.x - 1.0f, pos.y + 33.0f),
        IM_COL32(0, 120, 255, 44), IM_COL32(0, 220, 255, 14),
        IM_COL32(0, 220, 255, 7), IM_COL32(0, 120, 255, 30));

    char header[128];
    sprintf_s(header, sizeof(header), skCrypt("%s  %d"), title, count);
    DrawPanelText(draw, 16.0f, ImVec2(pos.x + 12.0f, pos.y + 8.0f), accent, header);
}

std::string PlayerStatusText(const PlayerPanelRow& row) {
    std::string status;
    if (row.IsAiming) status += skCrypt("AIM");
    if (row.IsSpectated) {
        if (!status.empty()) status += skCrypt("/");
        status += skCrypt("SPEC");
    }
    if (row.IsGroggy) {
        if (!status.empty()) status += skCrypt("/");
        status += skCrypt("DBNO");
    }
    if (row.IsTeammate) {
        if (!status.empty()) status += skCrypt("/");
        status += skCrypt("TEAM");
    }
    if (row.IsBot) {
        if (!status.empty()) status += skCrypt("/");
        status += skCrypt("AI");
    }
    if (status.empty()) status = row.IsVisible ? skCrypt("VIS") : skCrypt("-");
    return status;
}

std::string GearText(int level, float durability, const char* prefix) {
    if (level <= 0) return std::string(prefix) + skCrypt("-");
    char buf[32];
    if (durability > 0.5f) {
        sprintf_s(buf, sizeof(buf), skCrypt("%s%d %.0f%%"), prefix, level, durability);
    } else {
        sprintf_s(buf, sizeof(buf), skCrypt("%s%d"), prefix, level);
    }
    return buf;
}

ImU32 PlayerRowColor(const PlayerPanelRow& row) {
    if (row.IsAiming) return IM_COL32(255, 78, 62, 255);
    if (row.IsSpectated) return IM_COL32(255, 170, 35, 255);
    if (row.IsGroggy) return IM_COL32(255, 85, 75, 255);
    if (row.IsTeammate) return IM_COL32(90, 190, 255, 255);
    if (row.IsBot) return IM_COL32(175, 220, 190, 255);
    if (row.IsVisible) return IM_COL32(235, 245, 255, 255);
    return IM_COL32(190, 200, 214, 255);
};

} // namespace

void OverlayMenu::DrawLicenseWatermark(ImDrawList* draw) {
    if (!draw) return;

    std::string alertTxt = skCrypt("LICENSE STATUS: INACTIVE - PLEASE ENTER KEY IN SETTINGS [F5]");
    ImVec2 txtSize = ImGui::GetFont()->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, alertTxt.c_str());
    draw->AddRectFilled(ImVec2(10, 10), ImVec2(20 + txtSize.x, 20 + txtSize.y),
        IM_COL32(0, 0, 0, 150), 5.0f);
    draw->AddText(ImGui::GetFont(), 18.0f, ImVec2(15, 15),
        IM_COL32(255, 50, 50, 255), alertTxt.c_str());
}

void OverlayMenu::DrawGlobalSpectatorWarning(ImDrawList* draw, float ScreenWidth) {
    if (!draw || !esp_toggle || !esp_spectated || G_LocalSpectatedCount <= 0) return;

    char specText[64];
    sprintf_s(specText, sizeof(specText), "SPECTATORS: %d", G_LocalSpectatedCount);

    const float fontSize = 23.0f;
    ImVec2 txtSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, specText);
    const float panelW = txtSize.x + 60.0f;
    const float panelH = txtSize.y + 20.0f;
    const float posX = (ScreenWidth - panelW) * 0.5f;
    const float posY = 60.0f;

    ImVec2 pMin(posX, posY);
    ImVec2 pMax(posX + panelW, posY + panelH);
    ImDrawList* fg = ImGui::GetForegroundDrawList();
    fg->AddRectFilled(pMin, pMax, IM_COL32(0, 0, 0, 180), 8.0f);
    fg->AddRect(pMin, pMax, IM_COL32(255, 120, 0, 255), 8.0f, 0, 2.5f);
    const float eyeX = posX + 22.0f;
    const float eyeY = posY + panelH * 0.5f;
    fg->AddCircle(ImVec2(eyeX, eyeY), 8.0f, IM_COL32(255, 120, 0, 255), 12, 1.5f);
    fg->AddCircleFilled(ImVec2(eyeX, eyeY), 3.0f, IM_COL32(255, 120, 0, 255));
    fg->AddText(ImGui::GetFont(), fontSize, ImVec2(posX + 45, posY + 10.0f),
        IM_COL32(255, 120, 0, 255), specText);
}

void OverlayMenu::RenderAdminDebugEsp(ImDrawList* draw) {
    if (!draw || !debug_actor_esp) return;

    extern std::vector<DebugActorData> G_DebugActors;
    extern std::mutex G_DebugActorsMutex;
    std::lock_guard<std::mutex> lock(G_DebugActorsMutex);
    for (const auto& actor : G_DebugActors) {
        Vector2 screen;
        if (!telemetryContext::WorldToScreen(actor.Position, screen)) continue;

        char buf[256];
        sprintf_s(buf, skCrypt("[%s]\n0x%llX\n%.1fm"),
            actor.ClassName.c_str(), actor.Address, actor.Distance);
        draw->AddCircleFilled(ImVec2(screen.x, screen.y), 2.5f, IM_COL32(255, 255, 0, 255));
        draw->AddText(ImVec2(screen.x + 1, screen.y + 1), IM_COL32(0, 0, 0, 200), buf);
        draw->AddText(ImVec2(screen.x, screen.y), IM_COL32(255, 255, 0, 255), buf);
    }
}

void OverlayMenu::RenderMacroOsd(ImDrawList* draw, float ScreenWidth, float ScreenHeight) {
    if (!draw || !show_macro_overlay || current_scene != Scene::Gaming) return;

    char buf[256];
    std::string scopeName = Translation::GetAttachmentName(0, MacroEngine::current_scope);
    std::string muzzleName = Translation::GetAttachmentName(1, MacroEngine::current_muzzle);
    std::string gripName = Translation::GetAttachmentName(2, MacroEngine::current_grip);

    auto Lang = Translation::Get();
    if (MacroEngine::current_weapon_name.empty() || MacroEngine::current_weapon_name == "None") {
        sprintf_s(buf, sizeof(buf), "[ %s ]", Lang.NoWeapon);
    } else {
        sprintf_s(buf, sizeof(buf), "[ %s | %s | %s | %s ]",
            MacroEngine::current_weapon_name.c_str(), scopeName.c_str(),
            muzzleName.c_str(), gripName.c_str());
    }

    ImVec2 textSize = ImGui::CalcTextSize(buf);
    const float hudX = (ScreenWidth - textSize.x) * 0.5f;
    const float hudY = ScreenHeight * 0.85f;
    draw->AddText(ImVec2(hudX + 1, hudY + 1), ImColor(0, 0, 0, 200), buf);
    draw->AddText(ImVec2(hudX, hudY), ImColor(macro_overlay_color[0],
        macro_overlay_color[1], macro_overlay_color[2], macro_overlay_color[3]), buf);
}

void OverlayMenu::RenderSpectatorThreatList(ImDrawList* draw,
                                            const std::vector<PlayerData>& localPlayers,
                                            float ScreenWidth,
                                            float ScreenHeight) {
    if (!draw || (!esp_spectator_list && !player_list_enabled) ||
        (current_scene != Scene::Gaming && current_scene != Scene::Lobby)) return;

    if (player_list_hold_required) {
        if (player_list_hold_key <= 0 || player_list_hold_key > 0xFE ||
            !telemetryMemory::IsKeyDown(player_list_hold_key)) {
            return;
        }
    }

    auto Lang = Translation::Get();
    std::vector<SpectatorPanelRow> spectators;
    std::vector<PlayerPanelRow> players;
    int totalSpectators = 0;
    for (const auto& p : localPlayers) {
        if (p.SpectatedCount > 0) {
            totalSpectators += p.SpectatedCount;
            spectators.push_back({
                p.Name.empty() ? std::string(skCrypt("Unknown")) : p.Name,
                static_cast<int>(p.Distance),
                p.Health,
                p.SpectatedCount,
                p.IsAimingAtLocal,
                p.IsTeammate
            });
        }

        if (p.Distance <= 0.0f && p.Name.empty()) continue;
        players.push_back({
            p.Name.empty() ? std::string(skCrypt("Unknown")) : p.Name,
            p.WeaponName.empty() ? std::string(skCrypt("-")) : p.WeaponName,
            p.TeamID,
            static_cast<int>(p.Distance),
            p.Kills,
            p.SurvivalLevel,
            p.Ammo,
            p.AmmoMax,
            p.HelmetLevel,
            p.VestLevel,
            p.BackpackLevel,
            p.HelmetDurability,
            p.VestDurability,
            p.BackpackDurability,
            p.IsGroggy ? p.GroggyHealth : p.Health,
            p.DamageDealt,
            p.IsTeammate,
            p.IsBot,
            p.IsGroggy,
            p.IsVisible,
            p.IsAimingAtLocal,
            p.SpectatedCount > 0,
            p.HasAmmo
        });
    }

    std::sort(spectators.begin(), spectators.end(),
        [](const SpectatorPanelRow& a, const SpectatorPanelRow& b) {
            if (a.Count != b.Count) return a.Count > b.Count;
            if (a.IsAiming != b.IsAiming) return a.IsAiming > b.IsAiming;
            return a.Dist < b.Dist;
        });

    std::sort(players.begin(), players.end(),
        [](const PlayerPanelRow& a, const PlayerPanelRow& b) {
            const int aPrio = (a.IsAiming ? 0 : a.IsSpectated ? 1 : a.IsGroggy ? 2 : a.IsTeammate ? 4 : 3);
            const int bPrio = (b.IsAiming ? 0 : b.IsSpectated ? 1 : b.IsGroggy ? 2 : b.IsTeammate ? 4 : 3);
            if (aPrio != bPrio) return aPrio < bPrio;
            if (a.Team != b.Team) return a.Team < b.Team;
            return a.Dist < b.Dist;
        });

    const float margin = 20.0f;
    const float panelMaxW = (std::max)(640.0f, ScreenWidth - margin * 2.0f);
    const float panelW = std::clamp(ScreenWidth * 0.94f, 900.0f, panelMaxW);
    const float panelX = (ScreenWidth - panelW) * 0.5f;
    float nextY = 44.0f;

    if (esp_spectator_list && (!spectators.empty() || totalSpectators > 0)) {
        const float specW = (std::min)(420.0f, panelW);
        const float rowH = 25.0f;
        const int rows = (std::min)(static_cast<int>(spectators.size()), 5);
        const float specH = 40.0f + rows * rowH + 8.0f;
        const float specX = panelX;
        const ImVec2 pos(specX, nextY);

        DrawPanelShell(draw, pos, ImVec2(specW, specH), Lang.Spectators, totalSpectators,
            IM_COL32(255, 170, 35, 255));

        float y = pos.y + 39.0f;
        for (int i = 0; i < rows; ++i) {
            const auto& row = spectators[i];
            if (i % 2 == 0) {
                draw->AddRectFilled(ImVec2(pos.x + 7.0f, y - 2.0f),
                    ImVec2(pos.x + specW - 7.0f, y + rowH - 3.0f), IM_COL32(255, 255, 255, 12), 4.0f);
            }

            const ImU32 rowCol = row.IsAiming ? IM_COL32(255, 76, 64, 255) :
                (row.IsTeammate ? IM_COL32(90, 190, 255, 255) : IM_COL32(255, 196, 64, 255));
            const std::string name = FitPanelText(row.Name, 12.5f, 210.0f);
            char countBuf[24];
            sprintf_s(countBuf, sizeof(countBuf), skCrypt("x%d"), row.Count);
            char distBuf[24];
            sprintf_s(distBuf, sizeof(distBuf), skCrypt("%dm"), row.Dist);
            DrawPanelText(draw, 12.5f, ImVec2(pos.x + 12.0f, y + 1.0f), rowCol, name.c_str());
            DrawPanelText(draw, 12.5f, ImVec2(pos.x + 238.0f, y + 1.0f), IM_COL32(255, 220, 110, 255), countBuf);
            DrawPanelText(draw, 12.0f, ImVec2(pos.x + 282.0f, y + 1.0f), IM_COL32(185, 205, 224, 255), distBuf);
            DrawHealthMiniBar(draw, ImVec2(pos.x + 335.0f, y + 8.0f), 58.0f, row.HP, false);
            y += rowH;
        }
        nextY += specH + 10.0f;
    }

    if (!player_list_enabled || players.empty()) return;

    const float rowH = 30.0f;
    const float headerH = 70.0f;
    const float headerFont = 12.5f;
    const float rowFont = 14.0f;
    const float smallFont = 13.0f;
    const int maxRows = (std::max)(8, (std::min)(48,
        static_cast<int>((ScreenHeight - nextY - headerH - margin) / rowH)));
    const int rows = (std::min)(static_cast<int>(players.size()), maxRows);
    const float panelH = headerH + rows * rowH + 9.0f;
    const ImVec2 panelPos(panelX, nextY);

    DrawPanelShell(draw, panelPos, ImVec2(panelW, panelH), Lang.PlayerList,
        static_cast<int>(players.size()), IM_COL32(0, 220, 255, 255));

    const float x = panelPos.x;
    const float sx = panelW / 1180.0f;
    auto cx = [&](float value) { return x + value * sx; };
    const float yHead = panelPos.y + 43.0f;
    DrawPanelText(draw, headerFont, ImVec2(cx(14.0f), yHead), IM_COL32(125, 145, 170, 255), skCrypt("T"));
    DrawPanelText(draw, headerFont, ImVec2(cx(58.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.Name);
    DrawPanelText(draw, headerFont, ImVec2(cx(225.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.Distance);
    DrawPanelText(draw, headerFont, ImVec2(cx(280.0f), yHead), IM_COL32(125, 145, 170, 255), skCrypt("HP"));
    DrawPanelText(draw, headerFont, ImVec2(cx(350.0f), yHead), IM_COL32(125, 145, 170, 255), skCrypt("K"));
    DrawPanelText(draw, headerFont, ImVec2(cx(385.0f), yHead), IM_COL32(125, 145, 170, 255), skCrypt("DMG"));
    DrawPanelText(draw, headerFont, ImVec2(cx(445.0f), yHead), IM_COL32(125, 145, 170, 255), skCrypt("Lv"));
    DrawPanelText(draw, headerFont, ImVec2(cx(485.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.Ammo);
    DrawPanelText(draw, headerFont, ImVec2(cx(545.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.Weapon);
    DrawPanelText(draw, headerFont, ImVec2(cx(655.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.HelmetShort);
    DrawPanelText(draw, headerFont, ImVec2(cx(765.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.VestShort);
    DrawPanelText(draw, headerFont, ImVec2(cx(875.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.BackpackShort);
    DrawPanelText(draw, headerFont, ImVec2(cx(985.0f), yHead), IM_COL32(125, 145, 170, 255), Lang.State);
    draw->AddLine(ImVec2(x + 9.0f, yHead + 20.0f), ImVec2(x + panelW - 9.0f, yHead + 20.0f),
        IM_COL32(255, 255, 255, 28), 1.0f);

    float y = panelPos.y + headerH;
    for (int i = 0; i < rows; ++i) {
        const auto& row = players[i];
        if (i % 2 == 0) {
            draw->AddRectFilled(ImVec2(x + 7.0f, y - 2.0f),
                ImVec2(x + panelW - 7.0f, y + rowH - 3.0f), IM_COL32(255, 255, 255, 11), 4.0f);
        }

        const ImU32 rowCol = PlayerRowColor(row);
        draw->AddRectFilled(ImVec2(x + 7.0f, y - 2.0f), ImVec2(x + 10.0f, y + rowH - 3.0f),
            rowCol, 2.0f);

        char teamBuf[16];
        sprintf_s(teamBuf, sizeof(teamBuf), skCrypt("%d"), row.Team);
        char distBuf[24];
        sprintf_s(distBuf, sizeof(distBuf), skCrypt("%dm"), row.Dist);
        char killBuf[16];
        sprintf_s(killBuf, sizeof(killBuf), skCrypt("%d"), row.Kills);
        char damageBuf[24];
        sprintf_s(damageBuf, sizeof(damageBuf), skCrypt("%d"), static_cast<int>(row.Damage));
        char levelBuf[16];
        sprintf_s(levelBuf, sizeof(levelBuf), skCrypt("%d"), row.Level);
        char ammoBuf[24];
        if (row.HasAmmo) {
            sprintf_s(ammoBuf, sizeof(ammoBuf), skCrypt("%d/%d"), row.Ammo, row.AmmoMax);
        } else {
            sprintf_s(ammoBuf, sizeof(ammoBuf), skCrypt("-"));
        }

        const std::string name = FitPanelText(row.Name, rowFont, 158.0f * sx);
        const std::string weapon = FitPanelText(row.Weapon, rowFont, 100.0f * sx);
        const std::string helmet = FitPanelText(GearText(row.HelmetLevel, row.HelmetDurability, skCrypt("H")), smallFont, 100.0f * sx);
        const std::string vest = FitPanelText(GearText(row.VestLevel, row.VestDurability, skCrypt("V")), smallFont, 100.0f * sx);
        const std::string pack = FitPanelText(GearText(row.BackpackLevel, row.BackpackDurability, skCrypt("B")), smallFont, 100.0f * sx);
        const std::string state = FitPanelText(PlayerStatusText(row), smallFont, 150.0f * sx);

        DrawPanelText(draw, rowFont, ImVec2(cx(15.0f), y + 4.0f), rowCol, teamBuf);
        DrawPanelText(draw, rowFont, ImVec2(cx(58.0f), y + 4.0f), rowCol, name.c_str());
        DrawPanelText(draw, rowFont, ImVec2(cx(225.0f), y + 4.0f), IM_COL32(190, 210, 230, 255), distBuf);
        DrawHealthMiniBar(draw, ImVec2(cx(280.0f), y + 12.0f), 64.0f * sx, row.HP, row.IsGroggy);
        DrawPanelText(draw, rowFont, ImVec2(cx(350.0f), y + 4.0f), IM_COL32(255, 210, 80, 255), killBuf);
        DrawPanelText(draw, rowFont, ImVec2(cx(385.0f), y + 4.0f), IM_COL32(255, 140, 80, 255), damageBuf);
        DrawPanelText(draw, rowFont, ImVec2(cx(445.0f), y + 4.0f), IM_COL32(110, 230, 255, 255), levelBuf);
        DrawPanelText(draw, rowFont, ImVec2(cx(485.0f), y + 4.0f), IM_COL32(238, 238, 178, 255), ammoBuf);
        DrawPanelText(draw, rowFont, ImVec2(cx(545.0f), y + 4.0f), IM_COL32(190, 236, 255, 255), weapon.c_str());
        DrawPanelText(draw, smallFont, ImVec2(cx(655.0f), y + 5.0f), IM_COL32(170, 220, 255, 255), helmet.c_str());
        DrawPanelText(draw, smallFont, ImVec2(cx(765.0f), y + 5.0f), IM_COL32(180, 235, 190, 255), vest.c_str());
        DrawPanelText(draw, smallFont, ImVec2(cx(875.0f), y + 5.0f), IM_COL32(225, 205, 135, 255), pack.c_str());
        DrawPanelText(draw, smallFont, ImVec2(cx(985.0f), y + 5.0f), rowCol, state.c_str());
        y += rowH;
    }
}

void OverlayMenu::DrawAntiScreenshotWarning(ImDrawList* draw, float ScreenHeight) {
    if (!draw || anti_screenshot) return;

    auto Lang = Translation::Get();
    char statusBuf[128];
    sprintf_s(statusBuf, sizeof(statusBuf), "[ %s: %s ]", Lang.AntiScreenshot,
        (language == 1) ? "DANG TAT (KHONG AN TOAN)" : "INACTIVE (NOT SAFE)");
    draw->AddText(ImVec2(20, ScreenHeight - 40), ImColor(255, 50, 50, 200), statusBuf);
}
