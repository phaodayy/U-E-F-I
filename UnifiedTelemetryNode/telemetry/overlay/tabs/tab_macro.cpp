#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/Utils/SendInputMacro.h"
#include "../../sdk/core/console_log.hpp"
#include <iostream>
#include <protec/skCrypt.h>

void OverlayMenu::RenderTabMacro(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(3, skCrypt("MacroColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 3.0f);
    ImGui::SetColumnWidth(1, totalWidth / 3.0f);
    ImGui::SetColumnWidth(2, totalWidth / 3.0f);

    // Col 1: Core
    BeginGlassCard(skCrypt("##MacroCol1"), Lang.TabMacro, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::Checkbox(Lang.MacroEnabled, &g_Menu.macro_enabled);
    ImGui::Checkbox(Lang.MacroHumanize, &g_Menu.macro_humanize);
    ImGui::Checkbox(Lang.AdsOnly, &g_Menu.macro_ads_only);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseMacroWeaponProfiles);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 2: Settings
    BeginGlassCard(skCrypt("##MacroCol2"), Lang.HeaderPrecisionSettings, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::SliderFloat(Lang.MacroStrength, &g_Menu.macro_recoil_strength, 1.0f, 100.0f, skCrypt("%.0f")); 
    ImGui::Checkbox(Lang.ShowMacroOSD, &g_Menu.show_macro_overlay);
    ImGui::ColorEdit4(Lang.OsdColor, g_Menu.macro_overlay_color, ImGuiColorEditFlags_NoInputs);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseMacroSensitivity);
    DrawDisplayOnlyOption(Lang.ShowcaseMacroOverlayLayout);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 3: Utils
    BeginGlassCard(skCrypt("##MacroCol3"), Lang.HeaderEngineUtils, ImVec2(totalWidth / 3.0f - 20, 0));
    if (ImGui::Button(Lang.RescanAttach, ImVec2(-1, 35))) { /* Rescan */ }
    if (ImGui::Button(skCrypt("Test SendInput Y"), ImVec2(-1, 35))) {
        const bool ok1 = SendInputMacro::Move(0, 35);
        const bool ok2 = SendInputMacro::Move(0, -35);
        UTN_DEV_LOG(std::cout << "[DEV][MACRO][TEST] SendInput Y test ok="
            << ((ok1 && ok2) ? 1 : 0)
            << " err=" << SendInputMacro::LastErrorCode()
            << " convention=dy+down_dy-up"
            << std::endl);
    }
    ImGui::TextDisabled("%s", Lang.MacroStatusPreview);
    DrawDisplayOnlyOption(Lang.ShowcaseInputProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseHotkeyMatrix);
    ImGui::EndChild();

    ImGui::Columns(1);
}
