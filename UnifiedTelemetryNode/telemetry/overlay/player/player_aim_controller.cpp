#include "../core/overlay_menu.hpp"
#include "../../sdk/context.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include <algorithm>
#include <cmath>
#include <vector>

void OverlayMenu::RenderPlayersAndAim(ImDrawList* draw, std::vector<PlayerData>& localPlayers,
    const Vector2& local_feet_s, bool hasLocalS, float ScreenCenterX,
    float ScreenCenterY, float ScreenHeight, bool is_authenticated) {
    if (!draw) return;

        // --- 0. precision_calibration & MACRO SYNC ---
        // Weapon Check: Only aim if holding a weapon
        bool isHolstered = (MacroEngine::current_weapon_name == "" || MacroEngine::current_weapon_name == "None" || MacroEngine::current_weapon_name == "Holstered");
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
        float closestDist = finalFov * 8.0f; // Reset each frame

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
            if (isprecision_calibrationTarget && telemetryMemory::IsKeyDown(activeKey)) {
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
                                float travelTime = player.Distance / 800.0f;
                                targetWorld += (player.Velocity * travelTime);
                                // Refresh screen pos with prediction
                                telemetryContext::WorldToScreen(targetWorld, targetScreen);
                            }

                            float finalDist = selected.dist; // Use the original screen distance for filter
                            if (finalDist < closestDist) {
                                closestDist = finalDist;
                                bestTarget = &player;
                                bestScreenPos = targetScreen;
                            }
                        }
                    }
                }
            }

            RenderSinglePlayerEsp(draw, player, delta, local_feet_s, hasLocalS, ScreenCenterX, ScreenHeight);
        }

        }

        // --- 2. APPLY precision_calibration MOVEMENT ---
        // Finalize the mouse movement after all player iteration is DONE.
        if (is_authenticated && bestTarget) {
            float moveX = (bestScreenPos.x - ScreenCenterX);
            float moveY = (bestScreenPos.y - ScreenCenterY);

            if (activeConfig.smooth > 1.0f) {
                moveX /= activeConfig.smooth;
                moveY /= activeConfig.smooth;
            }

            telemetryMemory::MoveMouse((long)moveX, (long)moveY);
        }
}
