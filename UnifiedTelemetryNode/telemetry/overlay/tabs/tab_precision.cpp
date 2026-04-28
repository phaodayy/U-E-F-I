#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/context.hpp"
#include "../../protec/skCrypt.h"
#include <windows.h>

void OverlayMenu::RenderTabPrecision(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    extern void UploadActiveLoaderConfigAsync();
    float totalWidth = windowSize.x - 60;
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
    DrawDisplayOnlyOption(Lang.ShowcasePrecisionProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseControllerMatrix);
    DrawDisplayOnlyOption(Lang.ShowcaseKeyPresets);
    DrawDisplayOnlyOption(Lang.ShowcaseAimProfiles);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Column 2: Settings
    BeginGlassCard(skCrypt("##AimCol2"), Lang.HeaderPrecisionSettings, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::Checkbox(Lang.AimEnabled, &g_Menu.aim_master_enabled);

    AimConfig* pCfg = &g_Menu.aim_configs[8]; // GLOBAL
    ImGui::Checkbox(Lang.AimPrediction, &pCfg->prediction);
    ImGui::Checkbox(Lang.AimVisible, &g_Menu.aim_visible_only);

    ImGui::Spacing();
    ImGui::SliderFloat(Lang.AimFOV, &pCfg->fov, 1.0f, 100.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(Lang.AimSmooth, &pCfg->smooth, 1.0f, 20.0f, skCrypt("%.1f"));
    ImGui::SliderFloat(Lang.MaxDistance, &pCfg->max_dist, 10.0f, 800.0f, skCrypt("%.0f m"));

    ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.FovColor);
    static float fov_col[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // Stub or find global fov color
    ImGui::ColorEdit4(skCrypt("##FovCol"), fov_col, ImGuiColorEditFlags_NoInputs);

    char keyDisplay[64];
    sprintf_s(keyDisplay, sizeof(keyDisplay), skCrypt("MOUSE LEFT"));
    ImGui::Separator();
    ImGui::SliderFloat(Lang.SmoothRNG, &g_Menu.aim_smooth_rng, 0.0f, 10.0f, skCrypt("%.1f"));

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 1.0f, 1.0f), Lang.AimKey2);
    const char* keys[] = { "NONE", "MOUSE LEFT", "MOUSE RIGHT", "L-ALT", "L-SHIFT", "X", "V" };
    int keyVals[] = { 0, VK_LBUTTON, VK_RBUTTON, VK_LMENU, VK_LSHIFT, 'X', 'V' };
    int key2Idx = 0;
    for(int i=0; i<7; i++) if(g_Menu.aim_key2 == keyVals[i]) key2Idx = i;
    if (ImGui::Combo(skCrypt("##AimKey2Combo"), &key2Idx, keys, IM_ARRAYSIZE(keys))) {
        g_Menu.aim_key2 = keyVals[key2Idx];
    }

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
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), Lang.HeaderTactical);
    ImGui::Checkbox(Lang.GrenadeLine, &g_Menu.esp_grenade_prediction);
    ImGui::Checkbox(Lang.Projectiles, &g_Menu.esp_projectile_tracer);
    ImGui::Checkbox(Lang.ThreatWarning, &g_Menu.esp_threat_warning);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseAimCurve);
    DrawDisplayOnlyOption(Lang.ShowcaseAimPriority);

    ImGui::EndChild();

    ImGui::Columns(1);
}
