#include "overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/app_shutdown.hpp"
#include <protec/skCrypt.h>
#include <cctype>
#include <string>

extern std::string global_account_token;
extern std::string global_active_key;
extern std::string global_account_role;

void OverlayMenu::RenderMainMenuWindow(ImDrawList* draw, float ScreenWidth, float ScreenHeight) {
    if (!draw) return;

        // --- 3. MENU REDESIGN ---
        if (showmenu) {
            Translation::CurrentLanguage = language;
            auto Lang = Translation::Get();

            ImGui::SetNextWindowSize(ImVec2(1420, 800), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(ScreenWidth * 0.5f, ScreenHeight * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));

            // Custom window styling for the new design
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

            // --- PREMIUM SCROLLBAR STYLING ---
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 12.0f);
            ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.05f, 0.08f, 0.15f, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.00f, 0.78f, 1.00f, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.00f, 0.78f, 1.00f, 0.50f));
            ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.00f, 0.78f, 1.00f, 0.70f));

            ImGui::Begin(skCrypt("##overlay_new"), &showmenu, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize);

            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // --- PREMIUM STEALTH ENGINE BACKGROUND ---
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

            // 4. Vibrant Outer Border (Cyan Glow)
            drawList->AddRect(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y), IM_COL32(0, 200, 255, 80), 16.0f, 0, 1.5f);

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
            ImGui::SameLine(300);
            ImGui::SetCursorPosY(20);
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), skCrypt("\xE2\x97\x8F "));
            ImGui::SameLine(0, 2);
            ImGui::Text(Lang.SafeStatus);

            // Right Buttons
            ImGui::SameLine(windowSize.x - 80);
            ImGui::SetCursorPosY(18);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if (ImGui::Button(skCrypt("\xE2\x80\x94"), ImVec2(28, 28))) { /* min */ }
            ImGui::SameLine(0, 10);
            if (ImGui::Button(skCrypt("\xE2\x9C\x95"), ImVec2(28, 28))) {
                AppShutdown::Request();
            }
            ImGui::PopStyleColor();

            ImGui::EndChild(); // TopBar

            // Reduced gap between header and content: 110 -> 75 to optimize space usage
            ImGui::SetCursorPos(ImVec2(20, 75));
            ImGui::BeginChild(skCrypt("##MainContent"), ImVec2(windowSize.x - 40, windowSize.y - 150), false, ImGuiWindowFlags_None);

            const bool hasAccountSession = !global_account_token.empty();
            const bool hasValidKey = hasAccountSession && !global_active_key.empty();
            std::string accountRoleLower = global_account_role;
            for (char& c : accountRoleLower) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
            const bool isAdmin = accountRoleLower == skCrypt("admin");

            if (!hasValidKey && active_tab != 5) {
                active_tab = 5;
            }
            if (!isAdmin && active_tab == 6) {
                active_tab = hasValidKey ? 0 : 5;
            }

            if (hasValidKey) {
                if      (active_tab == 0) RenderTabVisuals(windowSize);
                else if (active_tab == 1) RenderTabPrecision(windowSize);
                else if (active_tab == 2) RenderTabMacro(windowSize);
                else if (active_tab == 3) RenderTabLoot(windowSize);
                else if (active_tab == 4) RenderTabRadar(windowSize);
                else if (active_tab == 5) RenderTabSettings(windowSize);
                else if (active_tab == 6 && isAdmin) RenderTabAdmin(windowSize);
                else RenderTabSettings(windowSize);
            } else {
                RenderTabSettings(windowSize);
            }

            ImGui::EndChild(); // MainContent

            // Bottom Bar Line
            draw->AddLine(ImVec2(windowPos.x, windowPos.y + windowSize.y - 40), ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y - 40), IM_COL32(0, 200, 255, 100), 1.0f);

            // Bottom Nav Rail (Floating Pill Design)
            float barW = windowSize.x - 100.0f;
            ImGui::SetCursorPos(ImVec2(50, windowSize.y - 55));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.08f, 0.32f, 0.3f));
            ImGui::BeginChild(skCrypt("##BottomRail"), ImVec2(barW, 45), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

            ImVec2 railPos = ImGui::GetWindowPos();
            ImVec2 railSize = ImGui::GetWindowSize();
            drawList->AddRectFilled(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(20, 40, 80, 100), 22.0f);
            drawList->AddRect(railPos, ImVec2(railPos.x + railSize.x, railPos.y + railSize.y), IM_COL32(0, 200, 255, 60), 22.0f);

            int totalTabs = 6 + (isAdmin ? 1 : 0);
            if (!hasValidKey) totalTabs = 1;
            float navTotalWidth = (110.0f * totalTabs) + (10.0f * (totalTabs - 1));
            ImGui::SetCursorPosX((railSize.x - navTotalWidth) / 2.0f);

            auto BottomTab = [&](const char* label, int id) {
                bool active = (active_tab == id);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.48f, 0.17f, 0.90f, 0.15f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.48f, 0.17f, 0.90f, 0.30f));

                if (active) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.9f, 1.0f, 1.0f));
                else ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.6f, 0.8f, 1.0f));

                if (ImGui::Button(label, ImVec2(110, 40))) active_tab = id;

                if (active) {
                    // Refined Underglow
                    for(int i=1; i<=4; i++)
                        drawList->AddRectFilled(ImVec2(cursorPos.x + 30 - i, cursorPos.y + 36), ImVec2(cursorPos.x + 80 + i, cursorPos.y + 39), IM_COL32(0, 180, 255, 60/i), 2.0f);
                    drawList->AddRectFilled(ImVec2(cursorPos.x + 35, cursorPos.y + 36), ImVec2(cursorPos.x + 75, cursorPos.y + 38), IM_COL32(0, 220, 255, 255), 2.0f);
                }

                ImGui::PopStyleColor(4);
                ImGui::SameLine(0, 10.0f);
            };

            if (hasValidKey) {
                BottomTab(Lang.TabVisuals, 0);
                BottomTab(Lang.Tabprecision_calibration, 1);
                BottomTab(Lang.TabMacro, 2);
                BottomTab(Lang.TabLoot, 3);
                BottomTab(Lang.TabRadar, 4);
                if (isAdmin) BottomTab(Lang.TabAdmin, 6);
            }
            BottomTab(Lang.TabSettings, 5);

            ImGui::EndChild(); // Rail
            ImGui::PopStyleColor(); // ChildBg

            ImGui::PopStyleVar(2); // ScrollbarSize, ScrollbarRounding
            ImGui::PopStyleColor(4); // Scrollbar Colors

            ImGui::PopStyleVar(2); // WindowPadding, WindowRounding
            ImGui::End();
        }
}
