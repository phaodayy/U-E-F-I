#include "../core/overlay_menu.hpp"
#include "../../sdk/core/context.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <algorithm>
#include <cmath>
#include <vector>

#include <protec/skCrypt.h>

class PIDController {
public:
    float kp, ki, kd;
    float last_error = 0;
    float integral = 0;

    PIDController(float p = 0.4f, float i = 0.01f, float d = 0.05f) : kp(p), ki(i), kd(d) {}

    void init(float p, float i, float d) {
        kp = p; ki = i; kd = d;
        last_error = 0;
        integral = 0;
    }

    float compute(float error, float dt = 0.01f) {
        integral += error * dt;
        float derivative = (error - last_error) / dt;
        float output = kp * error + ki * integral + kd * derivative;
        last_error = error;
        return output;
    }

    void clear() {
        last_error = 0;
        integral = 0;
    }
};

static PIDController AimPidX(0.4f, 0.01f, 0.05f);
static PIDController AimPidY(0.4f, 0.01f, 0.05f);

void OverlayMenu::RenderPlayersAndAim(ImDrawList* draw, std::vector<PlayerData>& localPlayers,
    const Vector2& local_feet_s, bool hasLocalS, float ScreenCenterX,
    float ScreenCenterY, float ScreenHeight, bool is_authenticated) {
    if (!draw) return;

        // --- 0. precision_calibration & MACRO SYNC ---
        // Weapon Check: Only aim if holding a weapon
        bool isHolstered = (MacroEngine::current_weapon_name == "" || MacroEngine::current_weapon_name == skCrypt("None") || MacroEngine::current_weapon_name == skCrypt("Holstered"));
        int category = (int)MacroEngine::current_category; // CAT_AR=0...CAT_NONE=8

        // Pick the active category config (Default GLOBAL=8 if not matched)
        AimConfig& activeConfig = aim_configs[category < 9 ? category : 8];
        int activeKey = activeConfig.key;

        bool canAim = aim_master_enabled && !isHolstered;

        // --- ADAPTIVE FOV (Scale based on Scope) ---
        float fovScale = 1.0f;
        if (aim_adaptive_fov) {
            int sc = MacroEngine::current_scope;
            if (sc == 3)      fovScale = 0.85f; // 2x
            else if (sc == 4) fovScale = 0.65f; // 3x
            else if (sc == 5) fovScale = 0.50f; // 4x
            else if (sc == 6) fovScale = 0.35f; // 6x
            else if (sc == 7) fovScale = 0.25f; // 8x
            else if (sc == 8) fovScale = 0.15f; // 15x
        }
        float finalFov = activeConfig.fov * fovScale;

        PlayerData* bestTarget = nullptr;
        Vector2 bestScreenPos = { 0, 0 };
        const float fovLimit = finalFov * 8.0f;
        float bestScore = 1000000000.0f;

        auto TargetScore = [&](const PlayerData& player, float screenDist) {
            switch (aim_target_priority) {
            case 1: return player.Distance;
            case 2: return (std::max)(0.0f, player.Health) + screenDist * 0.01f;
            case 3: return (player.IsAimingAtLocal ? -5000.0f : 0.0f) + screenDist + player.Distance * 0.05f;
            default: return screenDist;
            }
        };

        if (is_authenticated && canAim && activeConfig.enabled) {
            draw->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), finalFov * 8.0f, ImColor(255, 255, 255, 60), 64, 1.0f);
        }

            // --- 1. VISUALS (signal_overlay) & precision_calibration CORE ---
            if (is_authenticated && !showmenu) {
                float dt_esp = (GetTickCount64() - G_LastScanTime) / 1000.0f;
                if (!g_Menu.esp_skel_interp) dt_esp = 0.0f;
                else if (dt_esp > 0.10f) dt_esp = 0.10f;

            for (const auto& player_ref : localPlayers) {
            auto& player = const_cast<PlayerData&>(player_ref);

            if (player.Distance > render_distance) continue;
            if (player.Distance > activeConfig.max_dist) continue; // CATEGORY DISTANCE CHECK
            if (player.IsTeammate) {
                if (!g_Menu.esp_show_teammates) continue;
            } else {
                if (!g_Menu.esp_show_enemies) continue;
            }

            // precision_calibration FILTER
            bool isprecision_calibrationTarget = canAim && activeConfig.enabled;

            Vector3 delta = player.Velocity * dt_esp;

            // --- INTEGRATED precision_calibration TARGET CHECK (Optimization) ---
            const bool aimKeyDown = (activeKey != 0 && telemetryMemory::IsKeyDown(activeKey)) ||
                (aim_key2 != 0 && telemetryMemory::IsKeyDown(aim_key2));
            if (isprecision_calibrationTarget && aimKeyDown) {
                if (!player.IsTeammate && player.Health > 0) {
                    if (!aim_visible_only || player.IsVisible) {
                        // SMART-BONE LOGIC: Check Head, Neck, Chest on the player and pick the one closest to crosshair
                        struct BonePos { int id; Vector3 world; Vector2 screen; float dist; };
                        std::vector<BonePos> bones_to_check;

                        Vector3 scanBones[] = {
                            player.Bone_Head, player.Bone_Neck, player.Bone_Chest, player.Bone_Pelvis,
                            player.Bone_LShoulder, player.Bone_LElbow, player.Bone_LHand,
                            player.Bone_RShoulder, player.Bone_RElbow, player.Bone_RHand,
                            player.Bone_LThigh, player.Bone_LKnee, player.Bone_LFoot,
                            player.Bone_RThigh, player.Bone_RKnee, player.Bone_RFoot
                        };
                        int boneIDs[] = { 6, 5, 2, 1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 6 is HEAD

                        for (int b = 0; b < 16; b++) {
                            Vector2 s;
                            if (telemetryContext::WorldToScreen(scanBones[b], s)) {
                                float dx = s.x - ScreenCenterX;
                                float dy = s.y - ScreenCenterY;
                                bones_to_check.push_back({ boneIDs[b], scanBones[b], s, sqrtf(dx*dx + dy*dy) });
                            }
                        }

                        if (!bones_to_check.empty()) {
                            // Sort by distance from crosshair
                            std::sort(bones_to_check.begin(), bones_to_check.end(), [](const BonePos& a, const BonePos& b) {
                                return a.dist < b.dist;
                            });

                            // SHIFT OVERRIDE: If holding Shift, override the choice to HEAD (ID 6)
                            BonePos selected = bones_to_check[0];
                            if (telemetryMemory::IsKeyDown(VK_SHIFT)) {
                                for (auto& bp : bones_to_check) { if (bp.id == 6) { selected = bp; break; } }
                            }

                            Vector3 targetWorld = selected.world;
                            Vector2 targetScreen = selected.screen;

                            if (activeConfig.prediction) {
                                float bulletSpeed = 800.0f; // Default rifle speed
                                if (category == 5) bulletSpeed = 400.0f; // Shotgun
                                if (category == 7) bulletSpeed = 100.0f; // Panzerfaust
                                
                                float travelTime = player.Distance / (bulletSpeed * 100.0f); // Distance in cm, speed in m/s
                                targetWorld += (player.Velocity * travelTime);
                                
                                // Bù trọng lực cho Panzerfaust (Projectile bay chậm nên rơi nhanh)
                                if (category == 7) {
                                    float gravity = 9.8f * 100.0f; // cm/s^2
                                    targetWorld.z += 0.5f * gravity * (travelTime * travelTime);
                                }
                                
                                telemetryContext::WorldToScreen(targetWorld, targetScreen);
                            }

                            float finalDist = selected.dist; // Use the original screen distance for filter
                            if (finalDist < fovLimit) {
                                const float score = TargetScore(player, finalDist);
                                if (score < bestScore) {
                                    bestScore = score;
                                bestTarget = &player;
                                bestScreenPos = targetScreen;
                                }
                            }
                        }
                    }
                }
            }

            RenderSinglePlayerEsp(draw, player, delta, local_feet_s, hasLocalS, ScreenCenterX, ScreenHeight);
        }

        }

        // --- 2. APPLY precision_calibration MOVEMENT (Flick & Return for Shotgun & Panzer ONLY) ---
        static float totalFlickX = 0;
        static float totalFlickY = 0;
        static bool isFlicking = false;

        // 5 = CAT_SG, 7 = CAT_PANZER
        if (is_authenticated && bestTarget && (category == 5 || category == 7)) { 
            float errorX = (bestScreenPos.x - ScreenCenterX);
            float errorY = (bestScreenPos.y - ScreenCenterY);

            // Trigger Flick on Key Down
            if (!isFlicking) {
                // One-time snap to target (Flick)
                telemetryMemory::MoveMouse((long)errorX, (long)errorY);
                totalFlickX = errorX;
                totalFlickY = errorY;
                isFlicking = true;
            }
        } 
        
        // Reset and Return when key is released
        const bool aimKeyDown = (activeKey != 0 && telemetryMemory::IsKeyDown(activeKey)) ||
                                (aim_key2 != 0 && telemetryMemory::IsKeyDown(aim_key2));
                                
        if (!aimKeyDown && isFlicking) {
            // Return to original position accurately
            telemetryMemory::MoveMouse((long)-totalFlickX, (long)-totalFlickY);
            totalFlickX = 0;
            totalFlickY = 0;
            isFlicking = false;
            
            AimPidX.clear();
            AimPidY.clear();
        }
}
