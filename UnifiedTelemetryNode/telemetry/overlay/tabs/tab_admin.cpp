#include "../overlay_menu.hpp"
#include "../../sdk/context.hpp"
#include "../../protec/skCrypt.h"
#include <vector>
#include <mutex>

void OverlayMenu::RenderTabAdmin(ImVec2 windowSize) {
    extern std::vector<DebugActorData> G_DebugActors;
    extern std::mutex G_DebugActorsMutex;

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), skCrypt("ADMIN SYSTEM EXPLORER - LIVE ACTOR BUFFER"));
    ImGui::Separator();

    ImGui::Checkbox(skCrypt("SHOW DEBUG ACTOR ESP IN WORLD"), &debug_actor_esp);
    ImGui::SameLine();
    ImGui::TextDisabled(skCrypt("(Use this to find class names for missing items)"));
    ImGui::Spacing();

    if (ImGui::BeginTable(skCrypt("##DebugActorsTable"), 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, -1))) {
        ImGui::TableSetupColumn(skCrypt("Memory Address"), ImGuiTableColumnFlags_WidthFixed, 140.0f);
        ImGui::TableSetupColumn(skCrypt("Class Name"), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(skCrypt("World Position"), ImGuiTableColumnFlags_WidthFixed, 260.0f);
        ImGui::TableSetupColumn(skCrypt("Distance"), ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        std::lock_guard<std::mutex> lock(G_DebugActorsMutex);
        for (const auto& actor : G_DebugActors) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text(skCrypt("0x%llX"), actor.Address);
            
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(actor.ClassName.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(skCrypt("%.1f, %.1f, %.1f"), actor.Position.x, actor.Position.y, actor.Position.z);

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(skCrypt("%.1fm"), actor.Distance);
        }
        ImGui::EndTable();
    }
}
