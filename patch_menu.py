import sys

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

start_str = "        // --- 3. MENU REDESIGN ---\n"
end_str = "        ImGui::Render();\n"

start_idx = content.find(start_str)
end_idx = content.find(end_str, start_idx)

if start_idx == -1 or end_idx == -1:
    print("Could not find boundaries")
    sys.exit(1)

old_block = content[start_idx:end_idx]

new_block = """        // --- 3. MENU REDESIGN ---
        if (showmenu) {
            Translation::CurrentLanguage = language;
            auto Lang = Translation::Get();
            
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            
            // Custom window styling for the new design
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
            
            ImGui::Begin(skCrypt("##overlay_new"), &showmenu, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
            
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            
            // Top Bar
            ImGui::BeginChild(skCrypt("##TopBar"), ImVec2(windowSize.x, 40), false);
            ImGui::SetCursorPos(ImVec2(15, 10));
            
            // Load Logo
            static TextureInfo* logoTex = GetWeaponImage("LogoHC"); // Assumes Assets/Weapon/LogoHC.png or similar, we will just use text if not found
            if (logoTex && logoTex->SRV) {
                ImGui::Image((ImTextureID)logoTex->SRV, ImVec2(24, 24));
                ImGui::SameLine();
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("(O)")); // Fallback logo
                ImGui::SameLine();
            }
            
            ImGui::SetCursorPosY(12);
            ImGui::Text(skCrypt("HC-Cheat"));
            
            // Center Title
            const char* tabTitles[] = { skCrypt("Aim"), skCrypt("ESP"), skCrypt("Vật Phẩm"), skCrypt("Thiết Lập"), skCrypt("Bản Đồ") };
            ImVec2 titleSize = ImGui::CalcTextSize(tabTitles[active_tab]);
            ImGui::SameLine((windowSize.x - titleSize.x) / 2.0f);
            ImGui::SetCursorPosY(12);
            ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), tabTitles[active_tab]);
            
            // Right Buttons
            ImGui::SameLine(windowSize.x - 70);
            ImGui::SetCursorPosY(10);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button(skCrypt("_"), ImVec2(24, 24))) { /* minimize action */ }
            ImGui::SameLine();
            if (ImGui::Button(skCrypt("X"), ImVec2(24, 24))) { showmenu = false; }
            ImGui::PopStyleColor();
            
            ImGui::EndChild(); // TopBar
            
            // Separator Line
            draw->AddLine(ImVec2(windowPos.x, windowPos.y + 40), ImVec2(windowPos.x + windowSize.x, windowPos.y + 40), IM_COL32(255, 32, 82, 100), 1.0f);
            
            // Main Content Area
            ImGui::SetCursorPos(ImVec2(10, 50));
            ImGui::BeginChild(skCrypt("##MainContent"), ImVec2(windowSize.x - 20, windowSize.y - 100), false);
            
            // --- AIM TAB ---
            if (active_tab == 0) {
                ImGui::Columns(3, skCrypt("AimColumns"), false);
                ImGui::SetColumnWidth(0, 250);
                ImGui::SetColumnWidth(1, 250);
                
                // Column 1: Controller
                ImGui::BeginChild(skCrypt("##AimCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Controller"));
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("Phần Cứng Aim"));
                ImGui::Text(skCrypt("Thiết Bị"));
                static int deviceIdx = 0;
                ImGui::Combo(skCrypt("##Device"), &deviceIdx, skCrypt("KMBox B+\\0None\\0"));
                ImGui::Text(skCrypt("Cổng COM"));
                static int comIdx = 0;
                ImGui::Combo(skCrypt("##COM"), &comIdx, skCrypt("Không Có Cổng COM\\0"));
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("Ngắt Kết Nối"));
                if (ImGui::Button(skCrypt("Kết Nối"), ImVec2(-1, 30))) {}
                if (ImGui::Button(skCrypt("Test Di Chuyển"), ImVec2(-1, 30))) {}
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Phím Cấu Hình"));
                if (ImGui::Button(skCrypt("Lưu Cấu Hình"), ImVec2(-1, 30))) { g_Menu.SaveConfig("dataMacro/Config/settings.json"); }
                if (ImGui::Button(skCrypt("Thoát An Toàn"), ImVec2(-1, 30))) { exit(0); }
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Column 2: Settings
                ImGui::BeginChild(skCrypt("##AimCol2"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cài Đặt Thông Số"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Bật Aim"), &aim_master_enabled);
                
                AimConfig* pCfg = &aim_configs[8]; // GLOBAL
                ImGui::Checkbox(skCrypt("Bật Đạn Ảo"), &pCfg->prediction); // Dummy mapping
                ImGui::Checkbox(skCrypt("Aim Ngẫu Nhiên"), &aim_adaptive_fov); // Dummy mapping
                ImGui::Checkbox(skCrypt("Bật FOV"), &pCfg->enabled);
                
                ImGui::Spacing();
                ImGui::SliderFloat(skCrypt("Vòng Ngắm (FOV)"), &pCfg->fov, 1.0f, 100.0f, skCrypt("%.0f px"));
                ImGui::SliderFloat(skCrypt("Tốc độ Ngang (X)"), &pCfg->smooth, 1.0f, 20.0f, skCrypt("%.1f"));
                ImGui::SliderFloat(skCrypt("Khoảng Cách Max"), &pCfg->max_dist, 10.0f, 800.0f, skCrypt("%.0f"));
                
                char keyDisplay[64];
                sprintf_s(keyDisplay, sizeof(keyDisplay), skCrypt("MOUSE LEFT")); // Stub
                ImGui::Text(skCrypt("Nút Aim Chính"));
                if (ImGui::Button(keyDisplay, ImVec2(-1, 25))) { waiting_for_key = &pCfg->key; }
                
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Column 3: Logic
                ImGui::BeginChild(skCrypt("##AimCol3"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cấu hình Ngắm Tự động"));
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("TARGETING LOGIC"));
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), skCrypt("AIM MAIN:"));
                ImGui::TextWrapped(skCrypt("Tự động nhắm vào vùng cơ thể đang lộ diện và nằm gần tâm ngắm của bạn nhất."));
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), skCrypt("AIM HEAD:"));
                ImGui::TextWrapped(skCrypt("Ưu tiên nhắm vào Đầu/Cổ nếu đang lộ diện. Nếu không thấy đầu, sẽ tự động chuyển sang vùng cơ thể khác."));
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- ESP TAB ---
            else if (active_tab == 1) {
                ImGui::Columns(4, skCrypt("ESPColumns"), false);
                
                // Col 1: Tổng Quan
                ImGui::BeginChild(skCrypt("##ESPCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cài Đặt Tổng Quan"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Bật ESP"), &esp_toggle);
                ImGui::Checkbox(skCrypt("Cảnh Báo Nguy Hiểm"), &g_Menu.esp_offscreen);
                ImGui::Checkbox(skCrypt("Check Vật Cản"), &aim_visible_only);
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.12f, 0.32f, 1.0f), skCrypt("Giá Trị Aim"));
                ImGui::SliderInt(skCrypt("Khoảng Cách ESP"), &render_distance, 50, 1000);
                ImGui::SliderInt(skCrypt("Khoảng Cách Info"), &name_max_dist, 50, 600);
                ImGui::SliderFloat(skCrypt("Độ Dày Xương"), &skel_thickness, 1.0f, 5.0f, skCrypt("%.1f"));
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Col 2: Màu Sắc
                ImGui::BeginChild(skCrypt("##ESPCol2"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Màu Sắc"));
                ImGui::Separator();
                ImGui::ColorEdit4(skCrypt("Info Thấy"), name_color, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4(skCrypt("Xương Thấy"), skeleton_visible_color, ImGuiColorEditFlags_NoInputs);
                ImGui::ColorEdit4(skCrypt("Xương Khuất"), skeleton_invisible_color, ImGuiColorEditFlags_NoInputs);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Col 3: Bật/Tắt ESP
                ImGui::BeginChild(skCrypt("##ESPCol3"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Bật/Tắt ESP"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Khung Người"), &esp_box);
                ImGui::Checkbox(skCrypt("Xương Người"), &esp_skeleton);
                ImGui::Checkbox(skCrypt("Thanh Máu"), &g_Menu.esp_health);
                
                ImGui::Spacing();
                ImGui::Text(skCrypt("Độ Dày"));
                ImGui::SliderFloat(skCrypt("##Thickness"), &skel_thickness, 1.0f, 10.0f, skCrypt("%.0f px"));
                
                ImGui::Checkbox(skCrypt("Tên Người Chơi"), &g_Menu.esp_name);
                ImGui::Checkbox(skCrypt("ID Teammate"), &g_Menu.esp_show_teammates);
                ImGui::Checkbox(skCrypt("Khoảng Cách"), &esp_distance);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Col 4: Preview
                ImGui::BeginChild(skCrypt("##ESPCol4"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Xem Trước ESP"));
                ImGui::Separator();
                ImGui::Text(skCrypt("[Preview Graphic Here]"));
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- VẬT PHẨM TAB ---
            else if (active_tab == 2) {
                ImGui::Columns(3, skCrypt("ItemColumns"), false);
                
                // Col 1: Config
                ImGui::BeginChild(skCrypt("##ItemCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cài Đặt"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Bật ESP Vật Phẩm"), &esp_items);
                ImGui::SliderInt(skCrypt("Khoảng Cách ESP"), &loot_max_dist, 10, 300);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Col 2: Grid
                ImGui::BeginChild(skCrypt("##ItemCol2"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Chọn Nhóm"));
                ImGui::Separator();
                ImGui::Text(skCrypt("AR | SR | SG | SMG"));
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                // Col 3: Radar Web
                ImGui::BeginChild(skCrypt("##ItemCol3"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Bản Đồ"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Hiện Xe"), &esp_vehicles);
                ImGui::Checkbox(skCrypt("Hiện Thính"), &esp_airdrops);
                ImGui::Checkbox(skCrypt("Hiện Hòm Xác"), &esp_deadboxes);
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- THIẾT LẬP TAB ---
            else if (active_tab == 3) {
                ImGui::Columns(3, skCrypt("SettingsColumns"), false);
                
                ImGui::BeginChild(skCrypt("##SetCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cài Đặt Chung"));
                ImGui::Separator();
                int currentLang = language;
                if (ImGui::Combo(skCrypt("Ngôn Ngữ"), &currentLang, skCrypt("English\\0Tiếng Việt\\0"))) language = currentLang;
                if (ImGui::Button(skCrypt("Thoát An Toàn"), ImVec2(-1, 30))) { exit(0); }
                
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), skCrypt("LICENSE"));
                extern std::string global_active_key;
                bool is_active = !global_active_key.empty();
                if (is_active) ImGui::TextColored(ImVec4(0, 1, 0, 1), skCrypt("ACTIVE"));
                else ImGui::TextColored(ImVec4(1, 0, 0, 1), skCrypt("INACTIVE"));
                
                static char key_buf[128] = {0};
                ImGui::InputText(skCrypt("Key"), key_buf, sizeof(key_buf));
                if (ImGui::Button(skCrypt("Kích Hoạt"), ImVec2(-1, 30))) {
                    extern std::string GetHWID();
                    extern bool DoAPIRequest(const std::string& key, const std::string& hwid, bool silent);
                    std::string key_str(key_buf);
                    if (DoAPIRequest(key_str, GetHWID(), false)) {
                        global_active_key = key_str;
                        std::ofstream outFile("key.txt");
                        if (outFile.is_open()) { outFile << key_str; outFile.close(); }
                    }
                }
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                ImGui::BeginChild(skCrypt("##SetCol2"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Cảnh Báo Sớm"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Bật Cảnh Báo"), &g_Menu.esp_spectated);
                ImGui::SliderInt(skCrypt("Khoảng Cách Max"), &render_distance, 50, 1000);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                ImGui::BeginChild(skCrypt("##SetCol3"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Tiện Ích"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Dự Đoán Đạn"), &aim_configs[8].prediction);
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            // --- BẢN ĐỒ TAB ---
            else if (active_tab == 4) {
                ImGui::Columns(3, skCrypt("MapColumns"), false);
                
                ImGui::BeginChild(skCrypt("##MapCol1"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Bản Đồ Thế Giới"));
                ImGui::Separator();
                ImGui::Checkbox(skCrypt("Bật/Tắt Toàn Bộ Radar"), &g_Menu.radar_enabled);
                ImGui::Checkbox(skCrypt("Hiện Xe Cộ"), &esp_vehicles);
                ImGui::Checkbox(skCrypt("Hiện Thính"), &esp_airdrops);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                ImGui::BeginChild(skCrypt("##MapCol2"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Bản Đồ Nhỏ (Mini Map)"));
                ImGui::Separator();
                ImGui::SliderFloat(skCrypt("Kích Thước Chấm"), &g_Menu.radar_dot_size, 1.0f, 10.0f);
                ImGui::EndChild();
                
                ImGui::NextColumn();
                
                ImGui::BeginChild(skCrypt("##MapCol3"), ImVec2(0, 0), true);
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), skCrypt("Trạng Thái Radar"));
                ImGui::Separator();
                ImGui::Text(skCrypt("World Map Widget: %s"), G_Radar.IsWorldMapVisible ? skCrypt("Hiển Thị") : skCrypt("Đóng"));
                ImGui::Text(skCrypt("Mini Map Widget: %s"), G_Radar.IsMiniMapVisible ? skCrypt("Hiển Thị") : skCrypt("Ẩn"));
                ImGui::EndChild();
                
                ImGui::Columns(1);
            }
            
            ImGui::EndChild(); // MainContent
            
            // Bottom Bar Line
            draw->AddLine(ImVec2(windowPos.x, windowPos.y + windowSize.y - 40), ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y - 40), IM_COL32(255, 32, 82, 100), 1.0f);
            
            // Bottom Nav Bar
            ImGui::SetCursorPos(ImVec2(0, windowSize.y - 35));
            ImGui::BeginChild(skCrypt("##BottomBar"), ImVec2(windowSize.x, 35), false);
            
            float btnWidth = 100.0f;
            float totalWidth = btnWidth * 5.0f + 40.0f;
            ImGui::SetCursorPosX((windowSize.x - totalWidth) / 2.0f);
            
            auto BottomTab = [&](const char* label, int id) {
                bool active = (active_tab == id);
                if (active) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.12f, 0.32f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.12f, 0.32f, 0.2f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.12f, 0.32f, 0.4f));
                
                if (ImGui::Button(label, ImVec2(btnWidth, 30))) active_tab = id;
                
                ImGui::PopStyleColor(4);
                ImGui::SameLine();
            };
            
            BottomTab(skCrypt("Aim"), 0);
            BottomTab(skCrypt("ESP"), 1);
            BottomTab(skCrypt("Vật Phẩm"), 2);
            BottomTab(skCrypt("Thiết Lập"), 3);
            BottomTab(skCrypt("Bản Đồ"), 4);
            
            ImGui::EndChild(); // BottomBar
            
            ImGui::PopStyleVar(2); // Padding, Rounding
            ImGui::End();
        }
\n"""

new_content = content[:start_idx] + new_block + content[end_idx:]

with open(r'UnifiedTelemetryNode\telemetry\overlay\overlay_menu.cpp', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("Patch applied successfully.")
