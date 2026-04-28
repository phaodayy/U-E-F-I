#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>

void OverlayMenu::RenderTabRadar(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 70.0f;
    if (totalWidth < 420.0f) totalWidth = 420.0f;

    auto SliderFloatFull = [](const char* label, float* value, float min, float max, const char* format) {
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat(label, value, min, max, format);
    };

    BeginGlassCard(skCrypt("##MapRowCore"), Lang.TabRadar, ImVec2(totalWidth, 118.0f));
    ImGui::Columns(4, skCrypt("##MapCoreColumns"), false);
    ImGui::Checkbox(Lang.RadarEnable, &radar_enabled);
    ImGui::Checkbox(skCrypt("MiniMap"), &minimap_enabled);
    ImGui::NextColumn();
    ImGui::Checkbox(skCrypt("BigMap (M)"), &bigmap_enabled);
    ImGui::Checkbox(Lang.ShowCrosshair, &show_radar_center);
    ImGui::NextColumn();
    ImGui::Checkbox(Lang.ItemsVehicles, &esp_vehicles);
    ImGui::Checkbox(Lang.ShowNeutralTargets, &esp_airdrops);
    ImGui::NextColumn();
    ImGui::Checkbox(Lang.ShowDeathboxes, &esp_deadboxes);
    ImGui::Checkbox(Lang.ShowcaseRadarLegend, &bigmap_show_legend);
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::Spacing();

    BeginGlassCard(skCrypt("##MapRowMini"), Lang.MiniMapConfig, ImVec2(totalWidth, 168.0f));
    ImGui::Columns(3, skCrypt("##MapMiniColumns"), false);
    ImGui::Checkbox(skCrypt("Enable MiniMap"), &minimap_enabled);
    ImGui::Checkbox(skCrypt("Radar Center"), &show_radar_center);
    SliderFloatFull(Lang.RadarDotSize, &radar_dot_size, 1.0f, 18.0f, skCrypt("%.1f px"));
    ImGui::NextColumn();
    SliderFloatFull(Lang.RadarZoom, &radar_zoom_multiplier, 0.5f, 5.0f, skCrypt("%.1f x"));
    SliderFloatFull(Lang.RadarOffsetX, &radar_offset_x, -250.0f, 250.0f, skCrypt("%.0f px"));
    SliderFloatFull(Lang.RadarOffsetY, &radar_offset_y, -250.0f, 250.0f, skCrypt("%.0f px"));
    ImGui::NextColumn();
    ImGui::Checkbox(skCrypt("View Ray"), &minimap_show_direction);
    ImGui::Checkbox(skCrypt("Fire Trace"), &minimap_fire_trace);
    SliderFloatFull(skCrypt("View Ray Len"), &minimap_view_ray_length, 10.0f, 180.0f, skCrypt("%.0f m"));
    SliderFloatFull(skCrypt("Fire Ray Len"), &minimap_fire_ray_length, 40.0f, 500.0f, skCrypt("%.0f m"));
    SliderFloatFull(skCrypt("Fire Flash"), &minimap_fire_flash_ms, 80.0f, 1200.0f, skCrypt("%.0f ms"));
    SliderFloatFull(skCrypt("Ray Width"), &minimap_ray_width, 1.0f, 5.0f, skCrypt("%.1f px"));
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::Spacing();

    BeginGlassCard(skCrypt("##MapRowBig"), Lang.ShowcaseMapLayers, ImVec2(totalWidth, 214.0f));
    ImGui::Columns(3, skCrypt("##MapBigColumns"), false);
    ImGui::Checkbox(skCrypt("Enable BigMap"), &bigmap_enabled);
    ImGui::Checkbox(skCrypt("Name Radar"), &bigmap_show_names);
    ImGui::Checkbox(skCrypt("Direction Radar"), &bigmap_show_direction);
    ImGui::Checkbox(skCrypt("Name Background"), &bigmap_name_background);
    ImGui::Checkbox(Lang.ShowcaseRadarLegend, &bigmap_show_legend);
    ImGui::NextColumn();
    ImGui::Checkbox(skCrypt("Vehicle Radar"), &bigmap_show_vehicles);
    ImGui::Checkbox(skCrypt("Airdrop Radar"), &bigmap_show_airdrops);
    ImGui::Checkbox(skCrypt("DeadBox Radar"), &bigmap_show_deadboxes);
    ImGui::Checkbox(Lang.ItemsVehicles, &esp_vehicles);
    ImGui::Checkbox(Lang.ShowNeutralTargets, &esp_airdrops);
    ImGui::NextColumn();
    SliderFloatFull(skCrypt("BigMap Marker"), &bigmap_marker_size, 3.0f, 18.0f, skCrypt("%.1f px"));
    SliderFloatFull(skCrypt("BigMap Marker Alpha"), &bigmap_marker_alpha, 0.25f, 1.0f, skCrypt("%.2f"));
    SliderFloatFull(skCrypt("BigMap Icon Size"), &bigmap_icon_size, 10.0f, 42.0f, skCrypt("%.0f px"));
    SliderFloatFull(skCrypt("BigMap Name Size"), &bigmap_name_font_size, 9.0f, 22.0f, skCrypt("%.1f px"));
    SliderFloatFull(skCrypt("BigMap BG Alpha"), &bigmap_name_bg_alpha, 0.0f, 0.70f, skCrypt("%.2f"));
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::Spacing();

    BeginGlassCard(skCrypt("##MapRowShare"), Lang.HeaderShareRadar, ImVec2(totalWidth, 116.0f));
    ImGui::Columns(3, skCrypt("##MapShareColumns"), false);
    ImGui::Checkbox(Lang.RadarShare, &share_radar);
    ImGui::TextDisabled("%s: %s", Lang.LiveSharing, share_radar ? Lang.StatusOnline : Lang.StatusOffline);
    ImGui::NextColumn();
    ImGui::TextDisabled("%s", Lang.RadarIP);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText(skCrypt("##RadarIP"), share_radar_ip, sizeof(share_radar_ip));
    ImGui::NextColumn();
    ImGui::TextDisabled(skCrypt("BigMap: %s"), G_Radar.IsWorldMapVisible ? skCrypt("HUD") : skCrypt("Fallback"));
    ImGui::TextDisabled(skCrypt("Map: %.0f | Zoom: %.2f"),
        G_Radar.MapWorldSize, G_Radar.WorldMapZoomFactor > 0.001f ? G_Radar.WorldMapZoomFactor : 1.0f);
    ImGui::TextDisabled(skCrypt("Center: %.0f %.0f"),
        G_Radar.WorldCenterLocation.x, G_Radar.WorldCenterLocation.y);
    ImGui::Columns(1);
    ImGui::EndChild();
}
