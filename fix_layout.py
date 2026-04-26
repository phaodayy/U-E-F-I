import re

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# 1. Update BeginGlassCard to handle dynamic width (size.x == 0)
old_glass_card = """            auto BeginGlassCard = [&](const char* id, const char* label, ImVec2 size) {
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
            };"""

new_glass_card = """            auto BeginGlassCard = [&](const char* id, const char* label, ImVec2 size) {
                float actualWidth = size.x;
                if (actualWidth <= 0.0f) actualWidth = ImGui::GetContentRegionAvail().x;
                
                ImVec2 pos = ImGui::GetCursorScreenPos();
                // Card Background
                drawList->AddRectFilled(pos, ImVec2(pos.x + actualWidth, pos.y + size.y), IM_COL32(15, 30, 60, 100), 12.0f);
                // Card Inner Glow Border
                drawList->AddRect(pos, ImVec2(pos.x + actualWidth, pos.y + size.y), IM_COL32(0, 180, 255, 30), 12.0f, 0, 1.0f);
                
                // Top Highlight Bar
                drawList->AddLine(ImVec2(pos.x + 20, pos.y), ImVec2(pos.x + actualWidth - 20, pos.y), IM_COL32(0, 200, 255, 120), 2.0f);
                
                ImGui::BeginChild(id, size, false, ImGuiWindowFlags_NoBackground);
                ImGui::SetCursorPos(ImVec2(12, 8));
                ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), label);
                ImGui::Separator();
                ImGui::Spacing();
            };"""

content = content.replace(old_glass_card, new_glass_card)

# 2. Redesign VISUALS tab (Tab 0) to match Design and use Dynamic Columns
# We will use 4 columns: Core, Color, Style/Toggle, Preview
esp_tab_pattern = r'if \(g_Menu\.active_tab == 0\) \{.*?ImGui::Columns\(1\);'
new_esp_tab = """if (g_Menu.active_tab == 0) {
                float totalWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Columns(4, skCrypt("ESPColumns"), false);
                ImGui::SetColumnWidth(0, totalWidth * 0.22f);
                ImGui::SetColumnWidth(1, totalWidth * 0.22f);
                ImGui::SetColumnWidth(2, totalWidth * 0.22f);
                ImGui::SetColumnWidth(3, totalWidth * 0.34f); // Larger column for Preview
                
                // Col 1: Tổng Quan
                BeginGlassCard(skCrypt("##ESPCol1"), Lang.HeaderVisualCore, ImVec2(0, 0));
                ImGui::Checkbox(Lang.MasterToggle, &g_Menu.esp_toggle);
                ImGui::Checkbox(Lang.ESP_Offscreen, &g_Menu.esp_offscreen);
                ImGui::Checkbox(Lang.VisCheck, &g_Menu.aim_visible_only);
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.6f, 1.0f), Lang.DistThresholds);
                ImGui::SliderInt(Lang.RenderDist, &g_Menu.render_distance, 50, 1000);
                ImGui::SliderInt(Lang.InfoESP, &g_Menu.name_max_dist, 50, 600);
                ImGui::SliderFloat(skCrypt("Thick"), &g_Menu.skel_thickness, 1.0f, 5.0f, skCrypt("%.1f"));
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 2: Colors
                BeginGlassCard(skCrypt("##ESPColColors"), Lang.ColorsTitle, ImVec2(0, 0));
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
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 3: Style
                BeginGlassCard(skCrypt("##ESPColStyle"), Lang.HeaderRenderStyle, ImVec2(0, 0));
                ImGui::Checkbox(Lang.Box, &g_Menu.esp_box);
                ImGui::Checkbox(Lang.Skeleton, &g_Menu.esp_skeleton);
                ImGui::Checkbox(Lang.HealthBar, &g_Menu.esp_health);
                ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
                ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
                ImGui::Checkbox(Lang.Weapon, &g_Menu.esp_weapon);
                ImGui::Checkbox(Lang.ESP_Spectated, &g_Menu.esp_spectated);
                ImGui::EndChild();

                ImGui::NextColumn();
                // Col 4: PREVIEW (REVIEW)
                BeginGlassCard(skCrypt("##ESPColPreview"), (const char*)u8"Xem Trước ESP (Review)", ImVec2(0, 0));
                
                ImVec2 previewPos = ImGui::GetCursorScreenPos();
                float previewW = ImGui::GetContentRegionAvail().x;
                float previewH = 400.0f;
                
                // Draw a dummy 3D-ish character preview
                ImDrawList* d = ImGui::GetWindowDrawList();
                ImVec2 head = ImVec2(previewPos.x + previewW * 0.5f, previewPos.y + 80);
                ImVec2 pelvis = ImVec2(previewPos.x + previewW * 0.5f, previewPos.y + 180);
                ImVec2 lFoot = ImVec2(previewPos.x + previewW * 0.4f, previewPos.y + 320);
                ImVec2 rFoot = ImVec2(previewPos.x + previewW * 0.6f, previewPos.y + 320);
                
                ImU32 pCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color);
                ImU32 bCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color);
                
                if (g_Menu.esp_skeleton) {
                    d->AddCircleFilled(head, 15.0f, pCol);
                    d->AddLine(head, pelvis, pCol, 2.0f);
                    d->AddLine(pelvis, lFoot, pCol, 2.0f);
                    d->AddLine(pelvis, rFoot, pCol, 2.0f);
                }
                if (g_Menu.esp_box) {
                    d->AddRect(ImVec2(previewPos.x + previewW * 0.3f, previewPos.y + 50), ImVec2(previewPos.x + previewW * 0.7f, previewPos.y + 340), bCol, 5.0f, 0, 1.5f);
                }
                
                if (g_Menu.esp_name) {
                    const char* pName = "[GZ] phaodayy";
                    ImVec2 nSz = ImGui::CalcTextSize(pName);
                    d->AddText(ImVec2(head.x - nSz.x*0.5f, previewPos.y + 30), ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_color), pName);
                }
                
                if (g_Menu.esp_distance) {
                    const char* pDist = "26M | Lv.595";
                    ImVec2 dSz = ImGui::CalcTextSize(pDist);
                    d->AddText(ImVec2(head.x - dSz.x*0.5f, previewPos.y + 350), ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color), pDist);
                }

                ImGui::EndChild();
                
                ImGui::Columns(1);
            }"""

content = re.sub(esp_tab_pattern, new_esp_tab, content, flags=re.DOTALL)

# 3. Fix LOOT and SETTINGS tabs to use dynamic widths
content = content.replace('BeginGlassCard(skCrypt("##ItemCol1"), Lang.HeaderLootEngine, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##ItemCol1"), Lang.HeaderLootEngine, ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##ItemCol2"), Lang.HeaderPickupFilter, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##ItemCol2"), Lang.HeaderPickupFilter, ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##ItemCol3"), Lang.HeaderWorldEntities, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##ItemCol3"), Lang.HeaderWorldEntities, ImVec2(0, 0));')

content = content.replace('BeginGlassCard(skCrypt("##SetCol1"), Lang.HeaderSystemCore, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##SetCol1"), Lang.HeaderSystemCore, ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##SetCol2"), Lang.HeaderAntiTracking, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##SetCol2"), Lang.HeaderAntiTracking, ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##SetCol3"), Lang.HeaderEngineUtils, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##SetCol3"), Lang.HeaderEngineUtils, ImVec2(0, 0));')

content = content.replace('BeginGlassCard(skCrypt("##MapCol1"), Lang.TabRadar, ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##MapCol1"), Lang.TabRadar, ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##MapCol2"), skCrypt("MINI MAP"), ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##MapCol2"), skCrypt("MINI MAP"), ImVec2(0, 0));')
content = content.replace('BeginGlassCard(skCrypt("##MapCol3"), skCrypt("RADAR STATUS"), ImVec2(250, 0));', 'BeginGlassCard(skCrypt("##MapCol3"), skCrypt("RADAR STATUS"), ImVec2(0, 0));')

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("ESP Review column added and dynamic layout implemented for all tabs.")
