import sys
with open('telemetry/overlay/tabs/tab_admin.cpp', 'r') as f:
    content = f.read()

replacement = """    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), skCrypt("MORTAR AIMBOT - LIVE DEBUG INFO"));
    ImGui::Separator();
    
    bool isMortarActive = (G_LocalMortarEntity > 0x1000000) || (MacroEngine::current_weapon_name.find(skCrypt("mortar")) != std::string::npos);
    ImGui::Text("Is Mortar Active: "); ImGui::SameLine();
    if (isMortarActive) ImGui::TextColored(ImVec4(0, 1, 0, 1), "TRUE"); else ImGui::TextColored(ImVec4(1, 0, 0, 1), "FALSE");
    
    ImGui::Text("G_LocalPawn Address: 0x%llX", G_LocalPawn);
    ImGui::Text("G_LocalMortarEntity: 0x%llX", G_LocalMortarEntity);
    ImGui::Text("Mortar Pitch (X): %.2f", G_LocalMortarRotation.x);
    ImGui::Text("MacroEngine Weapon: %s", MacroEngine::current_weapon_name.c_str());
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), skCrypt("ADMIN SYSTEM EXPLORER - LIVE ACTOR BUFFER"));
    ImGui::Separator();"""

content = content.replace('    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), skCrypt("ADMIN SYSTEM EXPLORER - LIVE ACTOR BUFFER"));\n    ImGui::Separator();', replacement)

if "#include \"../../sdk/Utils/MacroEngine.h\"" not in content:
    content = content.replace('#include "../core/overlay_menu.hpp"', '#include "../core/overlay_menu.hpp"\n#include "../../sdk/Utils/MacroEngine.h"')

with open('telemetry/overlay/tabs/tab_admin.cpp', 'w') as f:
    f.write(content)
