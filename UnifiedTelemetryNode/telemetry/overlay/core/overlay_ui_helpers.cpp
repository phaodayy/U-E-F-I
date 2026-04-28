#include "overlay_menu.hpp"
#include "../translation/translation.hpp"
void OverlayMenu::BeginGlassCard(const char* id, const char* title, ImVec2 size) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    // Card Background
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(15, 30, 60, 100), 12.0f);
    // Card Inner Glow Border
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 180, 255, 30), 12.0f, 0, 1.0f);
    // Top Highlight Bar
    drawList->AddLine(ImVec2(pos.x + 20, pos.y), ImVec2(pos.x + size.x - 20, pos.y), IM_COL32(0, 200, 255, 120), 2.0f);

    ImGui::BeginChild(id, size, false, ImGuiWindowFlags_NoBackground);
    ImGui::SetCursorPos(ImVec2(12, 8));
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), title);
    ImGui::Separator();
    ImGui::Spacing();
}

void OverlayMenu::DrawDisplayOnlyOption(const char* label) {
    auto Lang = Translation::Get();
    bool previewEnabled = true;
    ImGui::BeginDisabled(true);
    ImGui::Checkbox(label, &previewEnabled);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%s", Lang.DisplayOnlyNote);
    }
}
