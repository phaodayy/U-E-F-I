import re

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Change SetupStyle background to glass and colors to Cyan/Blue
new_style = """void OverlayMenu::SetupStyle() {
    auto& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(10, 10);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.04f, 0.85f); // Glass dark blue
    colors[ImGuiCol_Border] = ImVec4(0.0f, 0.8f, 1.0f, 0.3f);       // Cyan border
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.10f, 0.6f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.8f, 1.0f, 0.2f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.8f, 1.0f, 0.4f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.04f, 0.95f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.02f, 0.02f, 0.04f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.05f, 0.08f, 0.6f);
    colors[ImGuiCol_Header] = ImVec4(0.0f, 0.8f, 1.0f, 0.3f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.8f, 1.0f, 0.5f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.8f, 1.0f, 0.7f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.1f, 0.8f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.8f, 1.0f, 0.4f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.8f, 1.0f, 0.6f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.8f, 1.0f, 0.8f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.8f, 1.0f, 0.5f);
}"""

content = re.sub(r'void OverlayMenu::SetupStyle\(\)\s*\{.*?\n\}', new_style, content, flags=re.DOTALL)

# 2. Replace all hardcoded inline Pink with Cyan (ImVec4(1.0f, 0.12f, 0.32f -> ImVec4(0.0f, 0.8f, 1.0f)
content = content.replace("1.0f, 0.12f, 0.32f", "0.0f, 0.8f, 1.0f")

# Replace IM_COL32 pinks 255, 32, 82 -> 0, 200, 255
content = content.replace("255, 32, 82", "0, 200, 255")

# 3. Replace HC-Cheat with GZ
content = content.replace("HC-Cheat", "GZ")
content = content.replace("LogoHC", "LogoGZ")

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Theme patched successfully to Glass + Cyan/Blue for GZ.")
