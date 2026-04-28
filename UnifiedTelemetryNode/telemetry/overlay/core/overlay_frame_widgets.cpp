#include "overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <protec/skCrypt.h>

#include <algorithm>
#include <mutex>

namespace {

struct VividThreat {
    std::string Name;
    int Dist = 0;
    float HP = 0.0f;
    bool IsSpectator = false;
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
                                            float ScreenWidth) {
    if (!draw || !esp_spectator_list ||
        (current_scene != Scene::Gaming && current_scene != Scene::Lobby)) return;

    std::vector<VividThreat> threats;
    int totalSpectators = 0;
    for (const auto& p : localPlayers) {
        if (p.SpectatedCount > 0) {
            totalSpectators += p.SpectatedCount;
            threats.push_back({ p.Name, static_cast<int>(p.Distance), p.Health, true });
        } else if (p.Distance < 120.0f && !p.IsTeammate && p.Distance > 0.0f) {
            threats.push_back({ p.Name, static_cast<int>(p.Distance), p.Health, false });
        }
    }

    std::sort(threats.begin(), threats.end(),
        [](const VividThreat& a, const VividThreat& b) { return a.Dist < b.Dist; });
    if (threats.empty() && totalSpectators <= 0) return;

    auto Lang = Translation::Get();
    const float listWidth = 240.0f;
    const float listX = ScreenWidth - listWidth - 20.0f;
    const float listY = 150.0f;
    const float entryHeight = 28.0f;
    const float headerHeight = 35.0f;
    const float totalHeight = headerHeight + (threats.size() * entryHeight) + 8.0f;

    draw->AddRectFilled(ImVec2(listX, listY), ImVec2(listX + listWidth, listY + totalHeight),
        IM_COL32(5, 15, 30, 220), 10.0f);
    draw->AddRect(ImVec2(listX, listY), ImVec2(listX + listWidth, listY + totalHeight),
        IM_COL32(0, 200, 255, 60), 10.0f, 0, 1.5f);
    draw->AddRectFilledMultiColor(ImVec2(listX + 2, listY + 2),
        ImVec2(listX + listWidth - 2, listY + headerHeight),
        IM_COL32(0, 100, 255, 30), IM_COL32(0, 200, 255, 10),
        IM_COL32(0, 200, 255, 10), IM_COL32(0, 100, 255, 30));

    char headerBuf[128];
    sprintf_s(headerBuf, skCrypt("%s (%d)"), Lang.ESP_SpectatorList, totalSpectators);
    draw->AddText(ImVec2(listX + 12, listY + 8), IM_COL32(0, 220, 255, 255), headerBuf);

    float currentY = listY + headerHeight;
    for (const auto& t : threats) {
        const ImU32 textCol = t.IsSpectator ? IM_COL32(255, 80, 80, 255) : IM_COL32(220, 220, 220, 255);
        char entryBuf[128];
        sprintf_s(entryBuf, skCrypt("%s (%dm)"), t.Name.c_str(), t.Dist);
        draw->AddText(ImVec2(listX + 12, currentY), textCol, entryBuf);

        const float hpW = 45.0f;
        const float hpH = 5.0f;
        const float hpx = listX + listWidth - hpW - 12.0f;
        const float hpy = currentY + 10.0f;
        draw->AddRectFilled(ImVec2(hpx, hpy), ImVec2(hpx + hpW, hpy + hpH),
            IM_COL32(40, 40, 40, 200), 2.0f);
        draw->AddRectFilled(ImVec2(hpx, hpy), ImVec2(hpx + hpW * (t.HP / 100.0f), hpy + hpH),
            IM_COL32(0, 255, 120, 255), 2.0f);
        currentY += entryHeight;
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
