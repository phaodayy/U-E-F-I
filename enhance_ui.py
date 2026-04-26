import re

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Update SetupStyle for Premium Cyan/Blue Glass Theme
new_style = """void OverlayMenu::SetupStyle() {
    auto& style = ImGui::GetStyle();
    
    // --- Advanced Layout & Rounding ---
    style.WindowRounding    = 16.0f;
    style.ChildRounding     = 12.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;
    
    style.WindowBorderSize  = 0.0f; 
    style.ChildBorderSize   = 0.0f; 
    style.FrameBorderSize   = 1.0f;
    
    style.WindowPadding     = ImVec2(20, 20);
    style.FramePadding      = ImVec2(12, 6);
    style.ItemSpacing       = ImVec2(12, 12);
    style.ItemInnerSpacing  = ImVec2(8, 8);
    
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    
    // --- Palette: Deep Sea & Electric Cyan ---
    ImVec4* colors = style.Colors;
    
    // Core surfaces
    colors[ImGuiCol_WindowBg]             = ImVec4(0.02f, 0.01f, 0.05f, 0.90f); // Deep near-black blue
    colors[ImGuiCol_ChildBg]              = ImVec4(0.05f, 0.10f, 0.20f, 0.35f); // Transparent blue card
    colors[ImGuiCol_PopupBg]              = ImVec4(0.02f, 0.04f, 0.08f, 1.00f);
    colors[ImGuiCol_Border]               = ImVec4(0.00f, 0.80f, 1.00f, 0.25f); // Electric Cyan border
    
    // Logic controls
    colors[ImGuiCol_FrameBg]              = ImVec4(0.05f, 0.15f, 0.25f, 0.50f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.00f, 0.80f, 1.00f, 0.15f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.00f, 0.80f, 1.00f, 0.30f);
    
    // Title & Headers
    colors[ImGuiCol_TitleBg]              = ImVec4(0.02f, 0.01f, 0.05f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.04f, 0.08f, 0.15f, 1.00f);
    
    // Selection highlight
    colors[ImGuiCol_Header]               = ImVec4(0.00f, 0.80f, 1.00f, 0.25f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.00f, 0.80f, 1.00f, 0.40f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.00f, 0.80f, 1.00f, 0.60f);
    
    // Buttons (Cyber Cyan)
    colors[ImGuiCol_Button]               = ImVec4(0.00f, 0.80f, 1.00f, 0.10f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.00f, 0.80f, 1.00f, 0.35f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.00f, 0.80f, 1.00f, 0.55f);
    
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.00f, 0.80f, 1.00f, 0.70f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.20f, 0.90f, 1.00f, 1.00f);
    
    colors[ImGuiCol_CheckMark]            = ImVec4(0.00f, 0.80f, 1.00f, 1.00f);
    
    colors[ImGuiCol_Text]                 = ImVec4(0.90f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.40f, 0.50f, 0.60f, 1.00f);
    
    colors[ImGuiCol_Separator]            = ImVec4(0.00f, 0.80f, 1.00f, 0.20f);
}"""

content = re.sub(r'void OverlayMenu::SetupStyle\(\)\s*\{.*?\n\}', new_style, content, flags=re.DOTALL)

# 2. Update RenderFrame with Enhanced Visuals
# Find the start of the menu background drawing
start_marker = r'// --- PREMIUM STEALTH ENGINE BACKGROUND ---'
end_marker = r'// Content Card Effect'

# I will replace the block from start_marker to just before 'ImGui::BeginChild(skCrypt("##TopBar")'
replacement_block = """// --- PREMIUM STEALTH ENGINE BACKGROUND ---
            // 1. Shadow / Glow surrounding the whole window
            drawList->AddRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(0, 200, 255, 40), 16.0f, 0, 8.0f);
            
            // 2. Main Window Fill (Deep Blue Glass Gradient)
            drawList->AddRectFilledMultiColor(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
                                             IM_COL32(5, 10, 20, 250),   // Top Left
                                             IM_COL32(10, 30, 60, 250),  // Top Right
                                             IM_COL32(5, 10, 25, 250),   // Bottom Right
                                             IM_COL32(2, 5, 15, 250));   // Bottom Left

            // 3. Subtle Cyber Grid background
            for (float i = 0; i < windowSize.x; i += 50.0f) {
                drawList->AddLine(ImVec2(windowPos.x + i, windowPos.y), ImVec2(windowPos.x + i, windowPos.y + windowSize.y), IM_COL32(0, 180, 255, 8), 1.0f);
            }
            for (float i = 0; i < windowSize.y; i += 50.0f) {
                drawList->AddLine(ImVec2(windowPos.x, windowPos.y + i), ImVec2(windowPos.x + windowSize.x, windowPos.y + i), IM_COL32(0, 180, 255, 8), 1.0f);
            }
            
            // 4. Vibrant Outer Border (Cyan Glow)
            drawList->AddRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(0, 200, 255, 80), 16.0f, 0, 1.5f);

            // Helpers for consistent premium UI components
            auto BeginGlassCard = [&](const char* id, const char* label, ImVec2 size) {
                ImVec2 pos = ImGui::GetCursorScreenPos();
                // Card Background
                drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(15, 30, 60, 100), 12.0f);
                // Card Inner Glow Border
                drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 180, 255, 30), 12.0f, 0, 1.0f);
                
                // Top Highlight Bar
                drawList->AddLine(ImVec2(pos.x + 20, pos.y), ImVec2(pos.x + size.x - 20, pos.y), IM_COL32(0, 200, 255, 120), 2.0f);
                
                ImGui::BeginChild(id, size, false, ImGuiWindowFlags_NoBackground);
                ImGui::SetCursorPos(ImVec2(12, 8));
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), label);
                ImGui::Separator();
                ImGui::Spacing();
            };

            // Top Bar
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::BeginChild(skCrypt("##TopBar"), ImVec2(windowSize.x, 60), false, ImGuiWindowFlags_NoBackground);
            
            // Top Bar Background with separator
            drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + 60), IM_COL32(10, 20, 40, 100), 16.0f, ImDrawFlags_RoundCornersTop);
            drawList->AddLine(ImVec2(windowPos.x, windowPos.y + 60), ImVec2(windowPos.x + windowSize.x, windowPos.y + 60), IM_COL32(0, 200, 255, 60), 1.0f);

            ImGui::SetCursorPos(ImVec2(25, 20));
            
            // Logo / Name
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), skCrypt("GZ"));
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.4f, 0.5f, 0.7f, 1.0f), skCrypt("|"));
            ImGui::SameLine();
            ImGui::Text(Lang.MainTelemetry);
            
            // Safe Badge
            ImGui::SameLine(180);
            ImGui::SetCursorPosY(20);
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), skCrypt("● "));
            ImGui::SameLine(0, 2);
            ImGui::Text(Lang.SafeStatus);
            
            // Right Buttons
            ImGui::SameLine(windowSize.x - 80);
            ImGui::SetCursorPosY(18);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button(skCrypt("—"), ImVec2(28, 28))) { /* min */ }
            ImGui::SameLine(0, 10);
            if (ImGui::Button(skCrypt("✕"), ImVec2(28, 28))) { showmenu = false; }
            ImGui::PopStyleColor();
            
            ImGui::EndChild(); // TopBar\n"""

pattern = re.escape(start_marker) + r".*?" + re.escape(r'ImGui::SetCursorPos(ImVec2(15, 55));')
content = re.sub(pattern, replacement_block + "\n            ImGui::SetCursorPos(ImVec2(15, 70));", content, flags=re.DOTALL)

# Also update the bottom bar colors and style
bottom_rail_pattern = r'ImGui::PushStyleColor\(ImGuiCol_ChildBg, ImVec4\(0\.15f, 0\.08f, 0\.32f, 0\.3f\)\);'
content = content.replace(bottom_rail_pattern, 'ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.15f, 0.25f, 0.3f));')

bottom_rail_draw_pattern = r'drawList->AddRectFilled\(railPos, ImVec2\(railPos\.x \+ railSize\.x, railPos\.y \+ railSize\.y\), IM_COL32\(40, 20, 80, 80\), 22\.0f\);\r?\n\s+drawList->AddRect\(railPos, ImVec2\(railPos\.x \+ railSize\.x, railPos\.y \+ railSize\.y\), IM_COL32\(150, 80, 255, 50\), 22\.0f\);'
new_bottom_rail_draw = """drawList->AddRectFilled(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(20, 40, 80, 100), 22.0f);
            drawList->AddRect(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(0, 200, 255, 60), 22.0f);"""
content = re.sub(bottom_rail_draw_pattern, new_bottom_rail_draw, content)

# Update tab glow colors
content = content.replace("IM_COL32(150, 60, 255, 40/i)", "IM_COL32(0, 180, 255, 60/i)")
content = content.replace("IM_COL32(180, 100, 255, 255)", "IM_COL32(0, 220, 255, 255)")
content = content.replace("ImVec4(0.6f, 0.3f, 1.0f, 1.0f)", "ImVec4(0.0f, 0.9f, 1.0f, 1.0f)")
content = content.replace("ImVec4(0.5f, 0.5f, 0.7f, 1.0f)", "ImVec4(0.5f, 0.6f, 0.8f, 1.0f)")

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("UI Enhanced to Premium Glass Cyan/Blue theme successfully.")
