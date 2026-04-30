#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>
#include <windows.h>
#include <algorithm>

void OverlayMenu::RenderTabPrecision(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    extern void UploadActiveLoaderConfigAsync();
    float totalWidth = windowSize.x - 60;

    const char* aimCategories[] = {
        skCrypt("Shotgun (Flick)"), skCrypt("Panzerfaust (Flick)"),
        skCrypt("AR (Disabled)"), skCrypt("SR (Disabled)"), 
        skCrypt("DMR (Disabled)"), skCrypt("SMG (Disabled)"),
        skCrypt("Pistol (Disabled)"), skCrypt("Global Settings")
    };

    // Mapping menu index to actual WeaponCategory enum values
    // Menu Index 0 -> Shotgun (5)
    // Menu Index 1 -> Panzer (7)
    int actualCategory = 8; // Default to Global/Other
    if (g_Menu.aim_category_idx == 0) actualCategory = 5;      // Shotgun
    else if (g_Menu.aim_category_idx == 1) actualCategory = 7; // Panzerfaust
    else actualCategory = g_Menu.aim_category_idx; // Others

    AimConfig* pCfg = &g_Menu.aim_configs[actualCategory];

    const char* keyLabels[] = { "NONE", "MOUSE LEFT", "MOUSE RIGHT", "L-ALT", "L-SHIFT", "X", "V" };
    int keyVals[] = { 0, VK_LBUTTON, VK_RBUTTON, VK_LMENU, VK_LSHIFT, 'X', 'V' };
    auto DrawKeyCombo = [&](const char* label, int* keyValue) {
        int keyIdx = 0;
        for (int i = 0; i < IM_ARRAYSIZE(keyVals); ++i) {
            if (*keyValue == keyVals[i]) keyIdx = i;
        }
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo(label, &keyIdx, keyLabels, IM_ARRAYSIZE(keyLabels))) {
            *keyValue = keyVals[keyIdx];
        }
    };

    auto ApplyActiveAimPreset = [&](int preset) {
        if (preset == 0) {
            pCfg->fov = 7.0f; pCfg->smooth = 8.0f; pCfg->max_dist = 220.0f; pCfg->bone = 5; pCfg->prediction = true;
        } else if (preset == 1) {
            pCfg->fov = 11.0f; pCfg->smooth = 4.5f; pCfg->max_dist = 450.0f; pCfg->bone = 6; pCfg->prediction = true;
        } else if (preset == 2) {
            pCfg->fov = 15.0f; pCfg->smooth = 2.2f; pCfg->max_dist = 260.0f; pCfg->bone = 2; pCfg->prediction = false;
        }
    };

    ImGui::Columns(3, skCrypt("AimColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 3.0f);
    ImGui::SetColumnWidth(1, totalWidth / 3.0f);
    ImGui::SetColumnWidth(2, totalWidth / 3.0f);

    // Column 1: Config
    BeginGlassCard(skCrypt("##AimCol1"), Lang.HeaderSystemConfig, ImVec2(totalWidth / 3.0f - 20, 0));

    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.CurrentMethod);
    ImGui::TextDisabled(skCrypt("Telemetry Bridge Preview"));

    ImGui::Spacing();
    if (ImGui::Button(Lang.SaveConfig, ImVec2(-1, 35))) { g_Menu.SaveConfig("dataMacro/Config/settings.json"); UploadActiveLoaderConfigAsync(); }
    if (ImGui::Button(Lang.CloseOverlay, ImVec2(-1, 35))) { showmenu = false; }
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcasePrecisionProfile);
    ImGui::Checkbox(Lang.AimEnabled, &g_Menu.aim_master_enabled);
    ImGui::Checkbox(Lang.AimVisible, &g_Menu.aim_visible_only);
    ImGui::Checkbox(Lang.AimPrediction, &pCfg->prediction);
    ImGui::Checkbox(skCrypt("Adaptive FOV"), &g_Menu.aim_adaptive_fov);
    ImGui::SliderFloat(Lang.SmoothRNG, &g_Menu.aim_smooth_rng, 0.0f, 10.0f, skCrypt("%.1f"));
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseControllerMatrix);
    DrawKeyCombo(skCrypt("Primary Key"), &pCfg->key);
    DrawKeyCombo(skCrypt("Secondary Key"), &g_Menu.aim_key2);
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseKeyPresets);
    if (ImGui::Button(skCrypt("RMB"), ImVec2(70, 24))) pCfg->key = VK_RBUTTON;
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("L-ALT"), ImVec2(70, 24))) pCfg->key = VK_LMENU;
    if (ImGui::Button(skCrypt("L-SHIFT"), ImVec2(70, 24))) pCfg->key = VK_LSHIFT;
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("LMB"), ImVec2(70, 24))) pCfg->key = VK_LBUTTON;
    ImGui::EndChild();

    ImGui::NextColumn();
    // Column 2: Settings
    BeginGlassCard(skCrypt("##AimCol2"), Lang.HeaderPrecisionSettings, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::TextDisabled("%s", Lang.ShowcaseAimProfiles);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo(skCrypt("##AimProfileCategory"), &g_Menu.aim_category_idx, aimCategories, IM_ARRAYSIZE(aimCategories))) {
        // Refresh mapping after selection
        if (g_Menu.aim_category_idx == 0) actualCategory = 5;
        else if (g_Menu.aim_category_idx == 1) actualCategory = 7;
        else actualCategory = g_Menu.aim_category_idx;
        pCfg = &g_Menu.aim_configs[actualCategory];
    }
    
    if (g_Menu.aim_category_idx > 1 && g_Menu.aim_category_idx < 7) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), skCrypt("(!) AIMBOT IS DISABLED FOR THIS WEAPON"));
        ImGui::TextDisabled(skCrypt("Only Shotgun and Panzerfaust support Flick Aim."));
    } else {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), skCrypt("[ACTIVE] Flick & Return Mode Enabled"));
    }

    ImGui::Checkbox(skCrypt("Enable Flick Profile"), &pCfg->enabled);
    ImGui::Checkbox(Lang.AimPrediction, &pCfg->prediction);
    ImGui::Checkbox(Lang.AimVisible, &g_Menu.aim_visible_only);

    ImGui::Spacing();
    ImGui::SliderFloat(Lang.AimFOV, &pCfg->fov, 1.0f, 100.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(Lang.AimSmooth, &pCfg->smooth, 1.0f, 20.0f, skCrypt("%.1f"));
    ImGui::SliderFloat(Lang.MaxDistance, &pCfg->max_dist, 10.0f, 800.0f, skCrypt("%.0f m"));
    ImGui::Separator();
    if (ImGui::Button(skCrypt("Safe"), ImVec2(90, 24))) ApplyActiveAimPreset(0);
    ImGui::SameLine();
    if (ImGui::Button(skCrypt("Balanced"), ImVec2(90, 24))) ApplyActiveAimPreset(1);
    if (ImGui::Button(skCrypt("Fast"), ImVec2(90, 24))) ApplyActiveAimPreset(2);

    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.FovColor);
    static float fov_col[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // Stub or find global fov color
    ImGui::ColorEdit4(skCrypt("##FovCol"), fov_col, ImGuiColorEditFlags_NoInputs);

    char keyDisplay[64];
    sprintf_s(keyDisplay, sizeof(keyDisplay), skCrypt("MOUSE LEFT"));
    ImGui::Separator();
    ImGui::SliderFloat(Lang.SmoothRNG, &g_Menu.aim_smooth_rng, 0.0f, 10.0f, skCrypt("%.1f"));

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), Lang.AimKey2);
    DrawKeyCombo(skCrypt("##AimKey2Combo"), &g_Menu.aim_key2);

    ImGui::EndChild();

    ImGui::NextColumn();

    // Column 3: aim structure
    BeginGlassCard(skCrypt("##AimCol3"), Lang.HeaderAimStructure, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), Lang.AimBone);
    const char* bones[] = { "Head", "Neck", "Upper Chest", "Pelvis" };
    int boneIdx = 0;
    if (pCfg->bone == 6) boneIdx = 0; else if (pCfg->bone == 5) boneIdx = 1; else if (pCfg->bone == 4) boneIdx = 2; else boneIdx = 3;
    if (ImGui::Combo(skCrypt("##AimBoneCombo"), &boneIdx, bones, IM_ARRAYSIZE(bones))) {
        if (boneIdx == 0) pCfg->bone = 6; else if (boneIdx == 1) pCfg->bone = 5; else if (boneIdx == 2) pCfg->bone = 4; else pCfg->bone = 1;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("%s", Lang.ShowcaseAimCurve);
    const char* curveItems[] = { skCrypt("Linear"), skCrypt("Soft"), skCrypt("Stable"), skCrypt("Fast") };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo(skCrypt("##AimCurve"), &g_Menu.aim_smooth_curve, curveItems, IM_ARRAYSIZE(curveItems));
    ImGui::TextDisabled("%s", Lang.ShowcaseAimPriority);
    const char* priorityItems[] = { skCrypt("Crosshair"), skCrypt("Distance"), skCrypt("Low HP"), skCrypt("Threat") };
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo(skCrypt("##AimPriority"), &g_Menu.aim_target_priority, priorityItems, IM_ARRAYSIZE(priorityItems));
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), Lang.HeaderTactical);
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);

    ImGui::EndChild();

    ImGui::Columns(1);
}
