#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include <protec/skCrypt.h>

void OverlayMenu::RenderTabRadar(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(3, skCrypt("MapColumns"), false);
    ImGui::SetColumnWidth(0, totalWidth / 3.0f);
    ImGui::SetColumnWidth(1, totalWidth / 3.0f);
    ImGui::SetColumnWidth(2, totalWidth / 3.0f);

    // Col 1: Config
    BeginGlassCard(skCrypt("##MapCol1"), Lang.TabRadar, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::Checkbox(Lang.RadarEnable, &radar_enabled);
    ImGui::Checkbox(Lang.ItemsVehicles, &esp_vehicles);
    ImGui::Checkbox(Lang.ShowNeutralTargets, &esp_airdrops);
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseMapProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseMapLayers);
    DrawDisplayOnlyOption(Lang.ShowcaseSharedRadarProfile);
    DrawDisplayOnlyOption(Lang.ShowcaseRadarPins);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 2: Position/Zoom
    BeginGlassCard(skCrypt("##MapCol2"), Lang.MiniMapConfig, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::SliderFloat(Lang.RadarDotSize, &radar_dot_size, 1.0f, 10.0f, skCrypt("%.1f px"));
    ImGui::SliderFloat(Lang.RadarZoom, &radar_zoom_multiplier, 0.5f, 5.0f, skCrypt("%.1f x"));
    ImGui::Separator();
    DrawDisplayOnlyOption(Lang.ShowcaseRadarLegend);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 3: Share
    BeginGlassCard(skCrypt("##MapCol3"), Lang.HeaderShareRadar, ImVec2(totalWidth / 3.0f - 20, 0));
    ImGui::Checkbox(Lang.RadarShare, &share_radar);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText(skCrypt("##RadarIP"), share_radar_ip, sizeof(share_radar_ip));
    ImGui::TextDisabled("%s: %s", Lang.LiveSharing, share_radar ? Lang.StatusOnline : Lang.StatusOffline);
    ImGui::EndChild();

    ImGui::Columns(1);
}
