#include "../core/overlay_menu.hpp"
#include "../../sdk/Utils/MacroEngine.h"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>
#include <vector>
#include <mutex>

void OverlayMenu::RenderTabAdmin(ImVec2 windowSize) {
    extern std::vector<DebugActorData> G_DebugActors;
    extern std::mutex G_DebugActorsMutex;

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), skCrypt("MORTAR AIMBOT - LIVE DEBUG INFO"));
    ImGui::Separator();
    
    bool isMortarActive = (G_LocalMortarEntity > 0x1000000) || (MacroEngine::current_weapon_name.find(skCrypt("mortar")) != std::string::npos);
    ImGui::Text("Is Mortar Active: "); ImGui::SameLine();
    if (isMortarActive) ImGui::TextColored(ImVec4(0, 1, 0, 1), "TRUE"); else ImGui::TextColored(ImVec4(1, 0, 0, 1), "FALSE");
    
    ImGui::Text("G_LocalPawn Address: 0x%llX", G_LocalPawn);
    
    std::string localPawnName = "None";
    if (G_LocalPawn > 0x1000000) {
        localPawnName = FNameUtils::GetNameFast(telemetryOffsets::DecryptCIndex(telemetryMemory::Read<uint32_t>(G_LocalPawn + telemetryOffsets::ObjID)));
    }
    ImGui::Text("G_LocalPawn Name: %s", localPawnName.c_str());
    
    ImGui::Text("G_LocalMortarEntity: 0x%llX", G_LocalMortarEntity);
    ImGui::Text("Mortar Pitch (X): %.2f", G_LocalMortarRotation.x);
    ImGui::Text("MacroEngine Weapon: %s", MacroEngine::current_weapon_name.c_str());
    ImGui::Spacing();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), skCrypt("ADMIN SYSTEM EXPLORER - LIVE ACTOR BUFFER"));
    ImGui::Separator();

    ImGui::Checkbox(skCrypt("SHOW DEBUG ACTOR ESP IN WORLD"), &debug_actor_esp);
    ImGui::SameLine();
    ImGui::TextDisabled(skCrypt("(Use this to find class names for missing items)"));
    ImGui::Checkbox(skCrypt("Map Mesh Debug ESP"), &debug_map_mesh_esp);
    ImGui::SameLine();
    ImGui::TextDisabled(skCrypt("(Draw ClonedMapMeshes wireframe for nade collision checks)"));
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
