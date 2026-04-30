#include "../core/overlay_menu.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <protec/skCrypt.h>

namespace {

bool IsFlickWeaponEnabled(const OverlayMenu& menu, const std::string& weaponName) {
    if (weaponName == skCrypt("s686") || weaponName == skCrypt("berreta686")) return menu.flick_weapon_s686;
    if (weaponName == skCrypt("s12k") || weaponName == skCrypt("saiga12")) return menu.flick_weapon_s12k;
    if (weaponName == skCrypt("s1897") || weaponName == skCrypt("winchester")) return menu.flick_weapon_s1897;
    if (weaponName == skCrypt("dbs") || weaponName == skCrypt("dp12")) return menu.flick_weapon_dbs;
    if (weaponName == skCrypt("o12") || weaponName == skCrypt("origin12") || weaponName == skCrypt("origins12")) return menu.flick_weapon_o12;
    return false;
}

} // namespace

void OverlayMenu::RenderPlayersAndAim(ImDrawList* draw, std::vector<PlayerData>& localPlayers,
    const Vector2& local_feet_s, bool hasLocalS, float ScreenCenterX,
    float ScreenCenterY, float ScreenHeight, bool is_authenticated) {
    if (!draw) return;

    float dt_esp = (GetTickCount64() - G_LastScanTime) / 1000.0f;
    if (!g_Menu.esp_skel_interp) dt_esp = 0.0f;
    else if (dt_esp > 0.10f) dt_esp = 0.10f;

    PlayerData* bestTarget = nullptr;
    Vector2 bestScreenPos = { 0, 0 };
    float bestDist = 1000000000.0f;

    const bool flickWeaponAllowed = IsFlickWeaponEnabled(*this, MacroEngine::current_weapon_name);
    const bool canFlick = is_authenticated && !showmenu && flick_enabled && flickWeaponAllowed;
    const int activeKey = flick_key;
    const bool flickKeyDown = (activeKey != 0 && telemetryMemory::IsKeyDown(activeKey)) ||
        (flick_key2 != 0 && telemetryMemory::IsKeyDown(flick_key2));

    if (canFlick) {
        draw->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), flick_fov * 8.0f, ImColor(255, 255, 255, 60), 64, 1.0f);
    }

    for (const auto& player_ref : localPlayers) {
        auto& player = const_cast<PlayerData&>(player_ref);

        if (player.Distance > render_distance) continue;
        if (player.IsTeammate) {
            if (!g_Menu.esp_show_teammates) continue;
        } else {
            if (!g_Menu.esp_show_enemies) continue;
        }

        if (canFlick && flickKeyDown && !player.IsTeammate && player.Health > 0.0f && player.Distance <= flick_max_dist) {
            if (!flick_visible_only || player.IsVisible) {
                struct BonePos { Vector3 world; Vector2 screen; float dist; };
                std::vector<BonePos> bones;
                Vector3 scanBones[] = {
                    player.Bone_Head, player.Bone_Neck, player.Bone_Chest, player.Bone_Pelvis
                };

                for (Vector3 bone : scanBones) {
                    Vector2 screen;
                    if (telemetryContext::WorldToScreen(bone, screen)) {
                        const float dx = screen.x - ScreenCenterX;
                        const float dy = screen.y - ScreenCenterY;
                        bones.push_back({ bone, screen, std::sqrt(dx * dx + dy * dy) });
                    }
                }

                if (!bones.empty()) {
                    const auto selected = std::min_element(bones.begin(), bones.end(),
                        [](const BonePos& a, const BonePos& b) { return a.dist < b.dist; });

                    const float fovLimit = flick_fov * 8.0f;
                    if (selected->dist < fovLimit && selected->dist < bestDist) {
                        bestDist = selected->dist;
                        bestTarget = &player;
                        bestScreenPos = selected->screen;
                    }
                }
            }
        }

        if (is_authenticated && esp_toggle) {
            Vector3 delta = player.Velocity * dt_esp;
            RenderSinglePlayerEsp(draw, player, delta, local_feet_s, hasLocalS, ScreenCenterX, ScreenHeight);
        }
    }

    static float totalFlickX = 0.0f;
    static float totalFlickY = 0.0f;
    static bool isFlicking = false;
    static bool lastFlickKeyDown = false;

    const bool justPressed = flickKeyDown && !lastFlickKeyDown;
    lastFlickKeyDown = flickKeyDown;

    if (canFlick && justPressed && bestTarget && !isFlicking) {
        const float errorX = bestScreenPos.x - ScreenCenterX;
        const float errorY = bestScreenPos.y - ScreenCenterY;

        if (telemetryMemory::MoveMouse((long)errorX, (long)errorY)) {
            totalFlickX = errorX;
            totalFlickY = errorY;
            isFlicking = true;

            if (flick_auto_shot) {
                telemetryMemory::MoveMouse(0, 0, 0x0001);
                telemetryMemory::StealthSleep(15);
                telemetryMemory::MoveMouse(0, 0, 0x0002);
            }
        }
    }

    if ((!flickKeyDown || !canFlick) && isFlicking) {
        telemetryMemory::MoveMouse((long)-totalFlickX, (long)-totalFlickY);
        totalFlickX = 0.0f;
        totalFlickY = 0.0f;
        isFlicking = false;
    }
}
