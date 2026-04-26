import sys

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

start_marker = '                // Column 1: Controller\n                ImGui::BeginChild(skCrypt("##AimCol1"), ImVec2(0, 0), true);'
end_marker = '                ImGui::EndChild();\n                \n                ImGui::NextColumn();'

start_idx = content.find(start_marker)
end_idx = content.find(end_marker, start_idx)

if start_idx == -1 or end_idx == -1:
    print("Could not find AimCol1 boundaries")
    sys.exit(1)

new_block = """                // Column 1: Config
                ImGui::BeginChild(skCrypt("##AimCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cài Đặt Chung"));
                ImGui::Separator();
                ImGui::Spacing();
                
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("Phương Thức Aim"));
                ImGui::Text(skCrypt("Memory / Mouse Event"));
                
                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Phím Cấu Hình"));
                if (ImGui::Button(skCrypt("Lưu Cấu Hình"), ImVec2(-1, 30))) { g_Menu.SaveConfig("dataMacro/Config/settings.json"); }
                if (ImGui::Button(skCrypt("Thoát An Toàn"), ImVec2(-1, 30))) { exit(0); }
                ImGui::EndChild();\n"""

new_content = content[:start_idx] + new_block + content[end_idx:]

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("Patch applied successfully.")
