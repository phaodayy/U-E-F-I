import re

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Redesign VISUALS tab (Tab 0) to use dynamic percentage-based columns like Aim tab
# And fix the gap issue by using ContentRegionAvail

esp_tab_pattern = r'if \(g_Menu\.active_tab == 0\) \{.*?ImGui::Columns\(1\);'

new_esp_tab = """if (g_Menu.active_tab == 0) {
                float totalAvailWidth = ImGui::GetContentRegionAvail().x;
                ImGui::Columns(4, skCrypt("ESPColumns"), false);
                
                // Set balanced column widths (22% per logic col, 34% for preview)
                ImGui::SetColumnWidth(0, totalAvailWidth * 0.22f);
                ImGui::SetColumnWidth(1, totalAvailWidth * 0.22f);
                ImGui::SetColumnWidth(2, totalAvailWidth * 0.22f);
                ImGui::SetColumnWidth(3, totalAvailWidth * 0.34f); 
                
                // Col 1: Core
                BeginGlassCard(skCrypt("##ESPCol1"), Lang.HeaderVisualCore, ImVec2(0, 0));
                ImGui::Checkbox(Lang.MasterToggle, &g_Menu.esp_toggle);
                ImGui::Checkbox(Lang.ESP_Offscreen, &g_Menu.esp_offscreen);
                ImGui::Checkbox(Lang.VisCheck, &g_Menu.aim_visible_only);
                
                ImGui::Spacing();
                ImGui::TextDisabled(Lang.DistThresholds);
                ImGui::PushItemWidth(-1);
                ImGui::SliderInt(Lang.RenderDist, &g_Menu.render_distance, 50, 1000);
                ImGui::SliderInt(Lang.InfoESP, &g_Menu.name_max_dist, 50, 600);
                ImGui::PopItemWidth();
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 2: Style & Selection
                BeginGlassCard(skCrypt("##ESPCol2"), Lang.HeaderRenderStyle, ImVec2(0, 0));
                ImGui::Checkbox(Lang.Box, &g_Menu.esp_box);
                ImGui::Checkbox(Lang.Skeleton, &g_Menu.esp_skeleton);
                ImGui::Checkbox(Lang.HealthBar, &g_Menu.esp_health);
                ImGui::Checkbox(Lang.Distance, &g_Menu.esp_distance);
                ImGui::Checkbox(Lang.Name, &g_Menu.esp_name);
                ImGui::Checkbox(Lang.Weapon, &g_Menu.esp_weapon);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                // Col 3: Colors
                BeginGlassCard(skCrypt("##ESPCol3"), Lang.ColorsTitle, ImVec2(0, 0));
                auto ColorEntry = [&](const char* label, float* col) {
                    ImGui::TextDisabled(label);
                    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 30);
                    ImGui::ColorEdit4(label, col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                };
                
                ColorEntry(Lang.Box, g_Menu.box_visible_color);
                ColorEntry(Lang.Skeleton, g_Menu.skeleton_visible_color);
                ColorEntry(Lang.Name, g_Menu.name_color);
                ColorEntry(Lang.Distance, g_Menu.distance_color);
                ColorEntry(skCrypt("Weapon"), g_Menu.weapon_color);
                ImGui::EndChild();

                ImGui::NextColumn();
                // Col 4: PREVIEW (REVIEW)
                BeginGlassCard(skCrypt("##ESPColPreview"), (const char*)u8"Xem Trước ESP (Review)", ImVec2(0, 0));
                
                ImVec2 previewPos = ImGui::GetCursorScreenPos();
                float previewW = ImGui::GetContentRegionAvail().x;
                
                ImDrawList* d = ImGui::GetWindowDrawList();
                ImVec2 head = ImVec2(previewPos.x + previewW * 0.5f, previewPos.y + 60);
                ImVec2 pelvis = ImVec2(previewPos.x + previewW * 0.5f, previewPos.y + 140);
                ImVec2 lFoot = ImVec2(previewPos.x + previewW * 0.4f, previewPos.y + 260);
                ImVec2 rFoot = ImVec2(previewPos.x + previewW * 0.6f, previewPos.y + 260);
                
                ImU32 pCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.skeleton_visible_color);
                ImU32 bCol = ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.box_visible_color);
                
                if (g_Menu.esp_skeleton) {
                    d->AddCircleFilled(head, 12.0f, pCol);
                    d->AddLine(head, pelvis, pCol, 2.0f);
                    d->AddLine(pelvis, lFoot, pCol, 2.0f);
                    d->AddLine(pelvis, rFoot, pCol, 2.0f);
                }
                if (g_Menu.esp_box) {
                    d->AddRect(ImVec2(previewPos.x + previewW * 0.3f, previewPos.y + 35), ImVec2(previewPos.x + previewW * 0.7f, previewPos.y + 280), bCol, 4.0f, 0, 1.5f);
                }
                
                if (g_Menu.esp_name) {
                    const char* pName = "[GZ] phaodayy";
                    ImVec2 nSz = ImGui::CalcTextSize(pName);
                    d->AddText(ImVec2(head.x - nSz.x*0.5f, previewPos.y + 15), ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.name_color), pName);
                }
                
                if (g_Menu.esp_distance) {
                    const char* pDist = "26M | Lv.595";
                    ImVec2 dSz = ImGui::CalcTextSize(pDist);
                    d->AddText(ImVec2(head.x - dSz.x*0.5f, previewPos.y + 290), ImGui::ColorConvertFloat4ToU32(*(ImVec4*)g_Menu.distance_color), pDist);
                }

                ImGui::EndChild();
                
                ImGui::Columns(1);
            }"""

content = re.sub(esp_tab_pattern, new_esp_tab, content, flags=re.DOTALL)

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Visuals tab layout updated to dynamic percentage-based columns.")
