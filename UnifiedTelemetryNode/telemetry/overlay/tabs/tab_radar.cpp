#include "../core/overlay_menu.hpp"
#include "../translation/translation.hpp"
#include "../../sdk/core/context.hpp"
#include <protec/skCrypt.h>

void OverlayMenu::RenderTabRadar(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60.0f;

    auto SliderFloatFull = [](const char* label, float* value, float min, float max, const char* format) {
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat(label, value, min, max, format);
    };

    ImGui::Columns(4, skCrypt("MapColumns4"), false);
    ImGui::SetColumnWidth(0, totalWidth / 4.0f);
    ImGui::SetColumnWidth(1, totalWidth / 4.0f);
    ImGui::SetColumnWidth(2, totalWidth / 4.0f);
    ImGui::SetColumnWidth(3, totalWidth / 4.0f);

    BeginGlassCard(skCrypt("##MapCore"), Lang.TabRadar, ImVec2(totalWidth / 4.0f - 15.0f, 0));
    ImGui::Checkbox(Lang.RadarEnable, &radar_enabled);
    ImGui::Checkbox(Lang.MiniMap, &minimap_enabled);
    ImGui::Checkbox(Lang.BigMap, &bigmap_enabled);
    ImGui::Checkbox(Lang.ShowCrosshair, &show_radar_center);
    ImGui::Separator();
    ImGui::Checkbox(Lang.ItemsVehicles, &esp_vehicles);
    ImGui::Checkbox(Lang.ShowNeutralTargets, &esp_airdrops);
    ImGui::Checkbox(Lang.ShowDeathboxes, &esp_deadboxes);
    ImGui::Checkbox(Lang.ShowcaseRadarLegend, &bigmap_show_legend);
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##MapMini"), Lang.MiniMapConfig, ImVec2(totalWidth / 4.0f - 15.0f, 0));
    ImGui::Checkbox(Lang.EnableMiniMap, &minimap_enabled);
    ImGui::Checkbox(skCrypt("Radar Center"), &show_radar_center);
    SliderFloatFull(Lang.RadarDotSize, &radar_dot_size, 1.0f, 18.0f, skCrypt("%.1f px"));
    SliderFloatFull(Lang.RadarZoom, &radar_zoom_multiplier, 0.5f, 5.0f, skCrypt("%.1f x"));
    SliderFloatFull(Lang.RadarOffsetX, &radar_offset_x, -250.0f, 250.0f, skCrypt("%.0f px"));
    SliderFloatFull(Lang.RadarOffsetY, &radar_offset_y, -250.0f, 250.0f, skCrypt("%.0f px"));
    ImGui::Separator();
    ImGui::Checkbox(Lang.ViewRay, &minimap_show_direction);
    ImGui::Checkbox(Lang.FireTrace, &minimap_fire_trace);
    ImGui::PushID(skCrypt("ViewRayLen"));
    SliderFloatFull(Lang.ViewRay, &minimap_view_ray_length, 10.0f, 180.0f, skCrypt("%.0f m"));
    ImGui::PopID();
    ImGui::PushID(skCrypt("FireRayLen"));
    SliderFloatFull(Lang.FireTrace, &minimap_fire_ray_length, 40.0f, 500.0f, skCrypt("%.0f m"));
    ImGui::PopID();
    SliderFloatFull(skCrypt("Fire Flash"), &minimap_fire_flash_ms, 80.0f, 1200.0f, skCrypt("%.0f ms"));
    SliderFloatFull(Lang.RayWidth, &minimap_ray_width, 0.5f, 4.0f, skCrypt("%.1f px"));
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##MapBig"), Lang.ShowcaseMapLayers, ImVec2(totalWidth / 4.0f - 15.0f, 0));
    ImGui::Checkbox(Lang.EnableBigMap, &bigmap_enabled);
    ImGui::Checkbox(Lang.NameRadar, &bigmap_show_names);
    ImGui::Checkbox(Lang.DirectionRadar, &bigmap_show_direction);
    ImGui::Checkbox(Lang.NameBackground, &bigmap_name_background);
    ImGui::Checkbox(Lang.ShowcaseRadarLegend, &bigmap_show_legend);
    ImGui::Separator();
    ImGui::Checkbox(Lang.VehicleRadar, &bigmap_show_vehicles);
    ImGui::Checkbox(Lang.AirdropRadar, &bigmap_show_airdrops);
    ImGui::Checkbox(Lang.DeadBoxRadar, &bigmap_show_deadboxes);
    ImGui::Separator();
    SliderFloatFull(skCrypt("BigMap Marker"), &bigmap_marker_size, 3.0f, 18.0f, skCrypt("%.1f px"));
    SliderFloatFull(skCrypt("BigMap Marker Alpha"), &bigmap_marker_alpha, 0.25f, 1.0f, skCrypt("%.2f"));
    SliderFloatFull(skCrypt("BigMap Icon Size"), &bigmap_icon_size, 10.0f, 42.0f, skCrypt("%.0f px"));
    SliderFloatFull(skCrypt("BigMap Name Size"), &bigmap_name_font_size, 9.0f, 22.0f, skCrypt("%.1f px"));
    SliderFloatFull(skCrypt("BigMap BG Alpha"), &bigmap_name_bg_alpha, 0.0f, 0.70f, skCrypt("%.2f"));
    ImGui::Separator();
    SliderFloatFull(skCrypt("BigMap X"), &bigmap_offset_x, -400.0f, 400.0f, skCrypt("%.0f px"));
    SliderFloatFull(skCrypt("BigMap Y"), &bigmap_offset_y, -400.0f, 400.0f, skCrypt("%.0f px"));
    SliderFloatFull(skCrypt("BigMap Scale"), &bigmap_screen_scale, 0.70f, 1.25f, skCrypt("%.2f x"));
    ImGui::EndChild();

    ImGui::NextColumn();
    BeginGlassCard(skCrypt("##MapShare"), Lang.HeaderShareRadar, ImVec2(totalWidth / 4.0f - 15.0f, 0));
    ImGui::Checkbox(Lang.RadarShare, &share_radar);
    ImGui::TextDisabled("%s: %s", Lang.LiveSharing, share_radar ? Lang.StatusOnline : Lang.StatusOffline);
    ImGui::TextDisabled("%s", Lang.RadarIP);
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText(skCrypt("##RadarIP"), share_radar_ip, sizeof(share_radar_ip));
    ImGui::Separator();
    ImGui::TextDisabled("%s: %s", Lang.BigMap, G_Radar.IsWorldMapVisible ? skCrypt("HUD") : skCrypt("Fallback"));
    ImGui::TextDisabled(skCrypt("Map: %.0f | Zoom: %.2f"),
        G_Radar.MapWorldSize, G_Radar.WorldMapZoomFactor > 0.001f ? G_Radar.WorldMapZoomFactor : 1.0f);
    ImGui::TextDisabled(skCrypt("Center: %.0f %.0f"),
        G_Radar.WorldCenterLocation.x, G_Radar.WorldCenterLocation.y);
    ImGui::EndChild();

    ImGui::Columns(1);
}
