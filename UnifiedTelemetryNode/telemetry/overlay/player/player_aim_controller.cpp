#include "../core/overlay_menu.hpp"
#include "../core/flick_weapon_catalog.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <protec/skCrypt.h>
#include "../../sdk/features/Mortar.h"

namespace {

bool IsFlickWeaponEnabled(const OverlayMenu& menu, const std::string& weaponName) {
    return FlickWeaponCatalog::IsEnabled(menu.flick_category_enabled, weaponName);
}

int GetFlickShotHoldMicros(const std::string& weaponName) {
    int muzzleSpeed = 800;
    int baseMicros = 100000;

    if (weaponName == skCrypt("vss")) { muzzleSpeed = 330; baseMicros = 115000; }
    else if (weaponName.find(skCrypt("panzer")) != std::string::npos) { muzzleSpeed = 115; baseMicros = 125000; }
    else if (weaponName == skCrypt("win94") || weaponName == skCrypt("win1894")) { muzzleSpeed = 760; baseMicros = 110000; }
    else if (weaponName == skCrypt("kar98k") || weaponName == skCrypt("mosin") || weaponName == skCrypt("mosinnagant")) { muzzleSpeed = 760; baseMicros = 110000; }
    else if (weaponName == skCrypt("m24")) { muzzleSpeed = 790; baseMicros = 105000; }
    else if (weaponName == skCrypt("awm")) { muzzleSpeed = 945; baseMicros = 100000; }
    else if (weaponName == skCrypt("lynx") || weaponName == skCrypt("l6")) { muzzleSpeed = 990; baseMicros = 105000; }
    else if (weaponName == skCrypt("mini14")) { muzzleSpeed = 990; baseMicros = 95000; }
    else if (weaponName == skCrypt("qbu") || weaponName == skCrypt("qbu88") || weaponName == skCrypt("mk12")) { muzzleSpeed = 930; baseMicros = 95000; }
    else if (weaponName == skCrypt("sks")) { muzzleSpeed = 800; baseMicros = 100000; }
    else if (weaponName == skCrypt("slr") || weaponName == skCrypt("fnfal") || weaponName == skCrypt("madsfnfal")) { muzzleSpeed = 840; baseMicros = 100000; }
    else if (weaponName == skCrypt("mk14")) { muzzleSpeed = 853; baseMicros = 100000; }
    else if (weaponName == skCrypt("dragunov")) { muzzleSpeed = 830; baseMicros = 100000; }
    else if (weaponName == skCrypt("s686") || weaponName == skCrypt("berreta686") ||
        weaponName == skCrypt("s1897") || weaponName == skCrypt("winchester")) { muzzleSpeed = 360; baseMicros = 115000; }
    else if (weaponName == skCrypt("s12k") || weaponName == skCrypt("saiga12") ||
        weaponName == skCrypt("dbs") || weaponName == skCrypt("dp12") ||
        weaponName == skCrypt("o12") || weaponName == skCrypt("origin12") || weaponName == skCrypt("origins12")) { muzzleSpeed = 420; baseMicros = 105000; }

    const int slowProjectileExtra = std::clamp((850 - muzzleSpeed) * 30, 0, 25000);
    return std::clamp(baseMicros + slowProjectileExtra, 95000, 150000);
}

long AbsLong(long value) {
    return value < 0 ? -value : value;
}

long ClampMouseDelta(long value, long limit) {
    if (value > limit) return limit;
    if (value < -limit) return -limit;
    return value;
}

int GetFlickSettleDelayMs(long moveX, long moveY) {
    const long largestMove = (std::max)(AbsLong(moveX), AbsLong(moveY));
    return std::clamp(35 + static_cast<int>(largestMove / 24), 40, 85);
}

int GetFlickShotHoldMs(const std::string& weaponName, bool holdUntilShot) {
    const int holdMicros = holdUntilShot ? GetFlickShotHoldMicros(weaponName) : 12000;
    return (holdMicros / 1000) > 1 ? (holdMicros / 1000) : 1;
}

struct PendingFlickAction {
    bool active = false;
    bool shotDown = false;
    bool shouldClick = false;
    bool shouldReturn = false;
    long moveX = 0;
    long moveY = 0;
    ULONGLONG shotDownAt = 0;
    ULONGLONG shotUpAt = 0;
    ULONGLONG returnAt = 0;
};

void CancelPendingFlick(PendingFlickAction& pending) {
    if (!pending.active) return;
    if (pending.shotDown) {
        telemetryMemory::MoveMouse(0, 0, 0x0002);
    }
    if (pending.shouldReturn && (pending.moveX != 0 || pending.moveY != 0)) {
        telemetryMemory::MoveMouse(-pending.moveX, -pending.moveY);
    }
    pending = {};
}

void ProcessPendingFlick(PendingFlickAction& pending, bool canContinue, ULONGLONG now) {
    if (!pending.active) return;

    if (!canContinue) {
        CancelPendingFlick(pending);
        return;
    }

    if (pending.shouldClick && !pending.shotDown && now >= pending.shotDownAt) {
        telemetryMemory::MoveMouse(0, 0, 0x0001);
        pending.shotDown = true;
        return;
    }

    if (pending.shotDown && now >= pending.shotUpAt) {
        telemetryMemory::MoveMouse(0, 0, 0x0002);
        pending.shotDown = false;
        if (!pending.shouldReturn) {
            pending = {};
        }
        return;
    }

    if (!pending.shotDown && pending.shouldReturn && now >= pending.returnAt) {
        telemetryMemory::MoveMouse(-pending.moveX, -pending.moveY);
        pending = {};
        return;
    }

    if (!pending.shouldClick && !pending.shouldReturn) {
        pending = {};
    }
}

struct FlickTargetCandidate {
    bool valid = false;
    Vector2 aim = { 0.0f, 0.0f };
    float selectDist = 0.0f;
};

Vector3 ResolveFlickTargetBone(const PlayerData& player, int targetPart) {
    switch (targetPart) {
    case 1: return player.Bone_Head.IsZero() ? player.HeadPosition : player.Bone_Head;
    case 2: return player.Bone_Neck;
    case 3: return player.Bone_Chest;
    case 4: return player.Bone_Pelvis;
    case 5: return player.Bone_LShoulder;
    case 6: return player.Bone_RShoulder;
    case 7: return player.Bone_LElbow;
    case 8: return player.Bone_RElbow;
    case 9: return player.Bone_LHand;
    case 10: return player.Bone_RHand;
    case 11: return player.Bone_LThigh;
    case 12: return player.Bone_RThigh;
    case 13: return player.Bone_LKnee;
    case 14: return player.Bone_RKnee;
    case 15:
        if (!player.Bone_LFoot.IsZero() && !player.Bone_RFoot.IsZero()) {
            return {
                (player.Bone_LFoot.x + player.Bone_RFoot.x) * 0.5f,
                (player.Bone_LFoot.y + player.Bone_RFoot.y) * 0.5f,
                (player.Bone_LFoot.z + player.Bone_RFoot.z) * 0.5f
            };
        }
        return player.FeetPosition;
    default:
        return {};
    }
}

FlickTargetCandidate BuildFlickTargetFromSelectedBone(const PlayerData& player, const Vector3& delta,
    float screenCenterX, float screenCenterY, float fovLimit, int targetPart) {
    FlickTargetCandidate result{};
    const Vector3 boneWorld = ResolveFlickTargetBone(player, targetPart);
    if (boneWorld.IsZero()) {
        return result;
    }

    Vector2 screen;
    if (!telemetryContext::WorldToScreen(boneWorld + delta, screen)) {
        return result;
    }

    const float dx = screen.x - screenCenterX;
    const float dy = screen.y - screenCenterY;
    const float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > fovLimit) {
        return result;
    }

    result.valid = true;
    result.aim = screen;
    result.selectDist = dist;
    return result;
}

FlickTargetCandidate BuildFlickTargetFromEspBox(const PlayerData& player, const Vector3& delta,
    float screenCenterX, float screenCenterY, float fovLimit) {
    FlickTargetCandidate result{};

    Vector2 headS, feetS;
    if (!telemetryContext::WorldToScreen(player.HeadPosition + delta, headS) ||
        !telemetryContext::WorldToScreen(player.FeetPosition + delta, feetS)) {
        return result;
    }

    float finalBoxTop = 0.0f;
    float finalBoxBottom = 0.0f;
    float finalBoxLeft = 0.0f;
    float finalBoxRight = 0.0f;
    bool useDynamicBox = false;

    float minX = 100000.0f, maxX = -100000.0f;
    float minY = 100000.0f, maxY = -100000.0f;
    bool foundValidBone = false;

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
        const float boxH = maxY - minY;
        const float boxW = maxX - minX;
        const float referenceH = std::fabs(headS.y - feetS.y);
        const float aspect = boxH > 1.0f ? boxW / boxH : 999.0f;
        const bool saneBoneBounds =
            referenceH > 8.0f &&
            boxH > referenceH * 0.55f &&
            boxH < referenceH * 1.70f &&
            aspect > 0.12f &&
            aspect < 0.78f;

        const float paddingW = (std::max)(boxW * 0.15f, 4.0f);
        const float paddingH = (std::max)(boxH * 0.12f, 4.0f);

        finalBoxTop = minY - (boxH * 0.25f) - 5.0f;
        finalBoxBottom = maxY + paddingH;
        finalBoxLeft = minX - paddingW;
        finalBoxRight = maxX + paddingW;

        if (saneBoneBounds && finalBoxBottom - finalBoxTop > 5.0f && finalBoxRight - finalBoxLeft > 2.0f) {
            useDynamicBox = true;
        }
    }

    if (!useDynamicBox) {
        const float h = std::fabs(headS.y - feetS.y);
        const float w = h * 0.50f;
        finalBoxTop = headS.y - (h * 0.12f);
        finalBoxBottom = feetS.y + (h * 0.05f);
        finalBoxLeft = headS.x - w / 2.0f;
        finalBoxRight = headS.x + w / 2.0f;
    }

    if (finalBoxLeft > finalBoxRight) std::swap(finalBoxLeft, finalBoxRight);
    if (finalBoxTop > finalBoxBottom) std::swap(finalBoxTop, finalBoxBottom);

    const float closestX = std::clamp(screenCenterX, finalBoxLeft, finalBoxRight);
    const float closestY = std::clamp(screenCenterY, finalBoxTop, finalBoxBottom);
    const float boxDx = closestX - screenCenterX;
    const float boxDy = closestY - screenCenterY;
    const float boxDist = std::sqrt(boxDx * boxDx + boxDy * boxDy);
    if (boxDist > fovLimit) {
        return result;
    }

    result.valid = true;
    result.selectDist = boxDist;
    result.aim = {
        (finalBoxLeft + finalBoxRight) * 0.5f,
        finalBoxTop + (finalBoxBottom - finalBoxTop) * 0.42f
    };

    Vector2 chestS;
    if (!player.Bone_Chest.IsZero() && telemetryContext::WorldToScreen(player.Bone_Chest + delta, chestS) &&
        chestS.x >= finalBoxLeft && chestS.x <= finalBoxRight &&
        chestS.y >= finalBoxTop && chestS.y <= finalBoxBottom) {
        result.aim = chestS;
    }

    return result;
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
    ItemData* bestItemTarget = nullptr;
    Vector3 bestTargetWorldPos = { 0, 0, 0 };
    float bestTargetDistance = 0.0f;
    Vector2 bestScreenPos = { 0, 0 };
    float bestDist = 1000000000.0f;

    const bool isMortarActive = (G_LocalMortarEntity > 0x1000000) || (MacroEngine::current_weapon_name.find(skCrypt("mortar")) != std::string::npos);
    const float actualRenderDist = isMortarActive ? 800.0f : (float)render_distance;

    const bool flickWeaponAllowed = isMortarActive || IsFlickWeaponEnabled(*this, MacroEngine::current_weapon_name);
    const bool canFlick = is_authenticated && !showmenu && flickWeaponAllowed;
    const bool activeVisibleOnly = FlickWeaponCatalog::BoolForWeapon(
        flick_category_visible_only, MacroEngine::current_weapon_name, flick_visible_only);
    const bool activeShotHold = FlickWeaponCatalog::BoolForWeapon(
        flick_category_shot_hold, MacroEngine::current_weapon_name, flick_shot_hold);
    const bool activeFlickAutoShot = FlickWeaponCatalog::BoolForWeapon(
        flick_category_auto_shot, MacroEngine::current_weapon_name, flick_auto_shot);
    const bool activeFollowAutoShot = FlickWeaponCatalog::BoolForWeapon(
        flick_category_follow_auto_shot, MacroEngine::current_weapon_name, flick_follow_auto_shot);
    const int activeBehaviorMode = FlickWeaponCatalog::IntForWeapon(
        flick_category_behavior_mode, MacroEngine::current_weapon_name, flick_behavior_mode, 0, 1);
    const int activeTargetPart = FlickWeaponCatalog::IntForWeapon(
        flick_category_target_part, MacroEngine::current_weapon_name, flick_target_part, 0, 15);
    const float activeMaxDist = isMortarActive ? 800.0f : FlickWeaponCatalog::FloatForWeapon(
        flick_category_max_dist, MacroEngine::current_weapon_name, flick_max_dist, 5.0f, 400.0f);
    const float activeSmoothness = FlickWeaponCatalog::SmoothnessForWeapon(
        flick_category_smoothness, MacroEngine::current_weapon_name, flick_smoothness);
    const bool activeFovCircle = FlickWeaponCatalog::BoolForWeapon(
        flick_category_fov_circle, MacroEngine::current_weapon_name, flick_fov_circle);
    const float activeShotDelay = FlickWeaponCatalog::FloatForWeapon(
        flick_category_shot_delay, MacroEngine::current_weapon_name, flick_shot_delay, 0.0f, 1000.0f);
    const float activeJitter = FlickWeaponCatalog::FloatForWeapon(
        flick_category_jitter, MacroEngine::current_weapon_name, flick_jitter, 0.0f, 20.0f);

    const bool flickFollowMode = (activeBehaviorMode == 1);
    const float flickMoveSpeed = FlickWeaponCatalog::MoveSpeedForWeapon(
        flick_category_move_speed, MacroEngine::current_weapon_name);
    const float activeFlickFov = isMortarActive ? mortar_fov : FlickWeaponCatalog::FovForWeapon(
        flick_category_fov, MacroEngine::current_weapon_name, flick_fov);
    const int activeKey = isMortarActive ? mortar_aim_key : FlickWeaponCatalog::IntForWeapon(
        flick_category_key, MacroEngine::current_weapon_name, flick_key, 0, 0xFE);
    const bool flickKeyDown = activeKey != 0 && telemetryMemory::IsKeyDown(activeKey);
    
    static uint64_t lockedPlayerActor = 0;
    static Vector3 lockedItemPos = {0, 0, 0};
    static bool isLocked = false;
    
    if (!flickKeyDown || !isMortarActive) {
        isLocked = false;
        lockedPlayerActor = 0;
        lockedItemPos = {0, 0, 0};
    }
    
    const bool shouldScanTargets = (canFlick && flickKeyDown) || (isMortarActive && is_authenticated && !showmenu);

    if ((canFlick && activeFovCircle) || isMortarActive) {
        ImVec4 col = ImGui::ColorConvertU32ToFloat4(ImColor(g_Menu.flick_fov_circle_color[0], g_Menu.flick_fov_circle_color[1], g_Menu.flick_fov_circle_color[2], g_Menu.flick_fov_circle_color[3]));
        float fovPx = activeFlickFov * 8.0f;
        if (isMortarActive) {
            // Draw vertical strip lines
            draw->AddLine(ImVec2(ScreenCenterX - fovPx, 0), ImVec2(ScreenCenterX - fovPx, ScreenHeight), ImColor(col), 1.5f);
            draw->AddLine(ImVec2(ScreenCenterX + fovPx, 0), ImVec2(ScreenCenterX + fovPx, ScreenHeight), ImColor(col), 1.5f);
            // Draw subtle fill
            draw->AddRectFilled(ImVec2(ScreenCenterX - fovPx, 0), ImVec2(ScreenCenterX + fovPx, ScreenHeight), ImColor(col.x, col.y, col.z, col.w * 0.15f));
        } else {
            draw->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), fovPx, ImColor(col), 64, 1.5f);
        }
    }


    for (const auto& player_ref : localPlayers) {
        auto& player = const_cast<PlayerData&>(player_ref);

        if (player.Distance > actualRenderDist) continue;
        
        if (isMortarActive && isLocked) {
            if (lockedPlayerActor != 0 && player.ActorPtr != lockedPlayerActor) continue;
            if (lockedPlayerActor == 0) continue; // Locked to an item, skip all players
        }
        
        if (player.IsTeammate) {
            if (!g_Menu.esp_show_teammates) continue;
        } else {
            if (!g_Menu.esp_show_enemies) continue;
        }

        float muzzleSpeed = 850.0f; 
        float gravity = 9.8f;
        if (MacroEngine::current_weapon_name == skCrypt("vss")) { muzzleSpeed = 330.0f; gravity = 9.8f; }
        else if (MacroEngine::current_weapon_name.find(skCrypt("awm")) != std::string::npos) { muzzleSpeed = 945.0f; gravity = 9.8f; }
        else if (MacroEngine::current_weapon_name.find(skCrypt("panzer")) != std::string::npos) { muzzleSpeed = 115.0f; gravity = 15.0f; } // Panzerfaust has more drop
        
        const float bulletTime = player.Distance / (std::max)(muzzleSpeed, 10.0f);
        const float predictionTime = bulletTime + dt_esp;
        
        // Target Prediction (Velocity)
        Vector3 delta = player.Velocity * predictionTime;
        
        // Bullet Drop Compensation (Gravity)
        // 0.5 * g * t^2
        float drop = 0.5f * gravity * (bulletTime * bulletTime) * 100.0f; // Convert m to cm (assuming game units are cm)
        delta.z += drop;


        if (shouldScanTargets && !player.IsTeammate && player.Health > 0.0f && player.Distance <= activeMaxDist) {
            if (!activeVisibleOnly || player.IsVisible || isMortarActive) {
                // For mortar, we use horizontal strip, so we pass a huge radial FOV to builder but filter manually
                const float fovLimit = isMortarActive ? 5000.0f : (activeFlickFov * 8.0f);
                FlickTargetCandidate target{};
                if (activeTargetPart > 0) {
                    target = BuildFlickTargetFromSelectedBone(
                        player, delta, ScreenCenterX, ScreenCenterY, fovLimit, activeTargetPart);
                }
                if (!target.valid) {
                    target = BuildFlickTargetFromEspBox(
                        player, delta, ScreenCenterX, ScreenCenterY, fovLimit);
                }

                if (target.valid) {
                    float selectionMetric = target.selectDist; // radial for normal
                    if (isMortarActive) {
                        float xDist = std::abs(target.aim.x - ScreenCenterX);
                        if (xDist > (activeFlickFov * 8.0f)) continue; // Horizontal strip filter
                        selectionMetric = xDist; // Prioritize closest to center X
                    }

                    if (selectionMetric < bestDist) {
                        bestDist = selectionMetric;
                        bestTarget = &player;
                        bestItemTarget = nullptr;
                        bestTargetWorldPos = player.Position;
                        bestTargetDistance = player.Distance;
                        bestScreenPos = target.aim;
                    }
                }
            }
        }

        if (is_authenticated && esp_toggle) {
            RenderSinglePlayerEsp(draw, player, delta, local_feet_s, hasLocalS, ScreenCenterX, ScreenHeight);
        }
    }

    // --- ITEM SCAN FOR MORTAR (VEHICLES & AIRDROPS) ---
    if (shouldScanTargets && isMortarActive) {
        std::lock_guard<std::mutex> lock(CachedItemsMutex);
        for (const auto& item_ref : CachedItems) {
            auto& item = const_cast<ItemData&>(item_ref);
            if (item.RenderType == ItemRenderType::Vehicle || item.RenderType == ItemRenderType::AirDrop) {
                if (item.Distance > activeMaxDist) continue;
                
                if (isLocked) {
                    if (!lockedItemPos.IsZero() && item.Position.Distance(lockedItemPos) > 50.0f) continue;
                    if (lockedItemPos.IsZero()) continue; // Locked to a player, skip all items
                }
                
                Vector2 screen;
                if (telemetryContext::WorldToScreen(item.Position, screen)) {
                    const float fovPx = activeFlickFov * 8.0f;
                    float selectionMetric = 0.0f;

                    if (isMortarActive) {
                        float xDist = std::abs(screen.x - ScreenCenterX);
                        if (xDist > fovPx) continue;
                        selectionMetric = xDist;
                    } else {
                        const float dx = screen.x - ScreenCenterX;
                        const float dy = screen.y - ScreenCenterY;
                        selectionMetric = std::sqrt(dx * dx + dy * dy);
                        if (selectionMetric > fovPx) continue;
                    }
                    
                    if (selectionMetric < bestDist) {
                        bestDist = selectionMetric;
                        bestTarget = nullptr; 
                        bestItemTarget = &item;
                        bestTargetWorldPos = item.Position;
                        bestTargetDistance = item.Distance;
                        bestScreenPos = screen;
                    }
                }
            }
        }
    }
    // -------------------------------------------------

    static bool lastFlickKeyDown = false;
    static PendingFlickAction pendingFlick;
    static ULONGLONG lastFollowMoveAt = 0;
    static bool followShotDown = false;
    static ULONGLONG followShotUpAt = 0;
    static ULONGLONG nextFollowShotAt = 0;
    const ULONGLONG nowMs = GetTickCount64();

    const bool justPressed = flickKeyDown && !lastFlickKeyDown;
    lastFlickKeyDown = flickKeyDown;

    if (isMortarActive && justPressed) {
        if (bestTarget != nullptr) {
            isLocked = true;
            lockedPlayerActor = bestTarget->ActorPtr;
            lockedItemPos = {0, 0, 0};
        } else if (bestItemTarget != nullptr) {
            isLocked = true;
            lockedPlayerActor = 0;
            lockedItemPos = bestItemTarget->Position;
        }
    }

    // --- MORTAR AIMBOT TARGET VISUALS ---
    if (isMortarActive && (bestTarget != nullptr || bestItemTarget != nullptr)) {
        if (bestScreenPos.x > 0 && bestScreenPos.y > 0) {
            draw->AddLine(ImVec2(ScreenCenterX, ScreenCenterY), ImVec2(bestScreenPos.x, bestScreenPos.y), ImColor(255, 0, 50, 200), 1.5f);
            draw->AddCircleFilled(ImVec2(bestScreenPos.x, bestScreenPos.y), 4.0f, ImColor(255, 0, 50, 255));
        }
    }

    // --- MORTAR AIMBOT CALIBRATION ---
    if (isMortarActive && canFlick && flickKeyDown && (bestTarget != nullptr || bestItemTarget != nullptr)) {
        float heightDiff = (bestTargetWorldPos.z - G_LocalPlayerPos.z) / 100.0f; // Convert cm to m
        float targetPitch = Mortar::CalculateRequiredPitch(bestTargetDistance, heightDiff);
        
        if (targetPitch >= 0.0f) {

            // Horizontal Auto Calibration (Yaw)
            if (lastFollowMoveAt == 0 || nowMs - lastFollowMoveAt >= 10) {
                float errorX = bestScreenPos.x - ScreenCenterX;
                long moveX = std::lround(errorX * 0.5f); // Smooth mortar panning
                moveX = ClampMouseDelta(moveX, 50); // Limit speed
                
                if (moveX != 0) {
                    telemetryMemory::MoveMouse(moveX, 0);
                }
                lastFollowMoveAt = nowMs;
            }

            // Vertical Auto Calibration (Pitch via MouseWheel)
            static ULONGLONG lastMortarScroll = 0;
            if (nowMs - lastMortarScroll > 40) { // Limit scroll frequency to 25Hz to allow game to catch up
                float pitchError = targetPitch - G_LocalMortarRotation.x;
                if (std::abs(pitchError) > 0.5f) { // Increased deadzone to 0.5 to prevent back-and-forth jitter
                    // WHEEL_DELTA is 120. If we need to increase pitch, we scroll UP (120).
                    int scrollAmount = (pitchError > 0.0f) ? 120 : -120;
                    telemetryMemory::MouseWheel(scrollAmount);
                }
                lastMortarScroll = nowMs;
            }
        }
        
        // Skip normal flick aim when mortaring
        return;
    } else if (isMortarActive) {
        // Still skip normal flick aim even if key not pressed
        return;
    }
    // ---------------------------------

    auto resetFollowShot = []() {
        if (followShotDown) {
            telemetryMemory::MoveMouse(0, 0, 0x0002);
        }
        followShotDown = false;
        followShotUpAt = 0;
        nextFollowShotAt = 0;
    };

    if (!canFlick || !flickKeyDown || !flickFollowMode) {
        lastFollowMoveAt = 0;
        resetFollowShot();
    }

    ProcessPendingFlick(pendingFlick, canFlick && flickKeyDown, nowMs);

    if (canFlick && justPressed && bestTarget) {
        if (activeJitter > 0.05f) {
            float rx = (static_cast<float>(rand() % 1001) / 500.0f - 1.0f) * activeJitter;
            float ry = (static_cast<float>(rand() % 1001) / 500.0f - 1.0f) * activeJitter;
            bestScreenPos.x += rx;
            bestScreenPos.y += ry;
        }

        const float errorX = bestScreenPos.x - ScreenCenterX;
        const float errorY = bestScreenPos.y - ScreenCenterY;
        const long moveX = std::lround(errorX * flickMoveSpeed);
        const long moveY = std::lround(errorY * flickMoveSpeed);

        CancelPendingFlick(pendingFlick);
        resetFollowShot();
        telemetryMemory::MoveMouse(moveX, moveY);

        const int settleMs = GetFlickSettleDelayMs(moveX, moveY);
        const int shotHoldMs = GetFlickShotHoldMs(MacroEngine::current_weapon_name, activeShotHold);

        pendingFlick.active = true;
        pendingFlick.shouldClick = activeFlickAutoShot;
        pendingFlick.shouldReturn = !flickFollowMode;
        pendingFlick.moveX = moveX;
        pendingFlick.moveY = moveY;
        pendingFlick.shotDownAt = nowMs + static_cast<ULONGLONG>(settleMs) + static_cast<ULONGLONG>(activeShotDelay);
        pendingFlick.shotUpAt = pendingFlick.shotDownAt + static_cast<ULONGLONG>(shotHoldMs);
        pendingFlick.returnAt = activeFlickAutoShot ? pendingFlick.shotUpAt + 2 : nowMs + static_cast<ULONGLONG>(settleMs);
    }

    if (canFlick && flickFollowMode && flickKeyDown && bestTarget && !pendingFlick.active) {
        if (lastFollowMoveAt == 0 || nowMs - lastFollowMoveAt >= 8) {
            float baseMoveX = (bestScreenPos.x - ScreenCenterX) * flickMoveSpeed;
            float baseMoveY = (bestScreenPos.y - ScreenCenterY) * flickMoveSpeed;
            
            // Apply Smoothness
            float smoothFactor = 1.0f - (std::clamp(activeSmoothness, 0.0f, 99.0f) / 100.0f);
            long moveX = std::lround(baseMoveX * smoothFactor);
            long moveY = std::lround(baseMoveY * smoothFactor);
            
            moveX = ClampMouseDelta(moveX, 180);
            moveY = ClampMouseDelta(moveY, 180);

            if (moveX != 0 || moveY != 0) {
                telemetryMemory::MoveMouse(moveX, moveY);
                lastFollowMoveAt = nowMs;
            }
        }
    }

    const bool canFollowAutoShot =
        canFlick && flickFollowMode && activeFollowAutoShot && flickKeyDown && bestTarget && !pendingFlick.active;

    if (!canFollowAutoShot) {
        if (!canFlick || !activeFollowAutoShot || !flickKeyDown || !flickFollowMode || !bestTarget) {
            resetFollowShot();
        }
    } else {
        const int shotHoldMs = GetFlickShotHoldMs(MacroEngine::current_weapon_name, activeShotHold);
        const ULONGLONG shotGapMs = static_cast<ULONGLONG>(std::clamp(shotHoldMs + 35, 80, 190));

        if (followShotDown && nowMs >= followShotUpAt) {
            telemetryMemory::MoveMouse(0, 0, 0x0002);
            followShotDown = false;
            nextFollowShotAt = nowMs + shotGapMs;
        }

        if (!followShotDown && (nextFollowShotAt == 0 || nowMs >= nextFollowShotAt)) {
            telemetryMemory::MoveMouse(0, 0, 0x0001);
            followShotDown = true;
            followShotUpAt = nowMs + static_cast<ULONGLONG>(shotHoldMs);
        }
    }
}
