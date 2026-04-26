import re

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. We need to find the Visuals tab block and add a 5th column or merge colors into Col 3/4.
# Let's change Columns(4, ...) to Columns(5, ...) and add the 5th column.

content = content.replace('ImGui::Columns(4, skCrypt("ESPColumns"), false);', 'ImGui::Columns(5, skCrypt("ESPColumns"), false);')

# Find the end of Col 4
col4_end = 'BeginGlassCard(skCrypt("##ESPCol4"), Lang.HeaderDangerScan, ImVec2(185, 0));\n                ImGui::Checkbox(Lang.ESP_Spectated, &g_Menu.esp_spectated);\n                ImGui::EndChild();'

replacement_col5 = col4_end + """

                ImGui::NextColumn();
                // Col 5: Colors
                BeginGlassCard(skCrypt("##ESPCol5"), Lang.HeaderColorSettings, ImVec2(185, 0));
                
                ImGui::TextDisabled(Lang.Box);
                ImGui::ColorEdit4(skCrypt("##BoxVis"), g_Menu.box_visible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SameLine();
                ImGui::ColorEdit4(skCrypt("##BoxInv"), g_Menu.box_invisible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                
                ImGui::TextDisabled(Lang.Skeleton);
                ImGui::ColorEdit4(skCrypt("##SkelVis"), g_Menu.skeleton_visible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::SameLine();
                ImGui::ColorEdit4(skCrypt("##SkelInv"), g_Menu.skeleton_invisible_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                
                ImGui::TextDisabled(Lang.Name);
                ImGui::ColorEdit4(skCrypt("##NameCol"), g_Menu.name_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                
                ImGui::TextDisabled(Lang.Distance);
                ImGui::ColorEdit4(skCrypt("##DistCol"), g_Menu.distance_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

                ImGui::TextDisabled(skCrypt("Weapon"));
                ImGui::ColorEdit4(skCrypt("##WepCol"), g_Menu.weapon_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                
                ImGui::EndChild();"""

content = content.replace(col4_end, replacement_col5)

# 2. Also add FOV color adjustment in Aim tab (Col 2)
aim_fov_find = 'ImGui::SliderFloat(skCrypt("Max Dist"), &pCfg->max_dist, 10.0f, 800.0f, skCrypt("%.0f m"));'
aim_fov_replace = aim_fov_find + """
                
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), skCrypt("FOV Color"));
                static float fov_col[4] = {1.0f, 1.0f, 1.0f, 0.5f}; // Stub or find global fov color
                ImGui::ColorEdit4(skCrypt("##FovCol"), fov_col, ImGuiColorEditFlags_NoInputs);"""

content = content.replace(aim_fov_find, aim_fov_replace)

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Color pickers added to Visuals and Aim tabs.")
