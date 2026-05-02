#include "radar_overlay_renderer.hpp"

#include "../core/colors.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../translation/translation.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include "../../sdk/core/context.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>

namespace {

ImU32 GetTeamColor(const OverlayMenu& menu, int teamID) {
    if (menu.team_color_custom) {
        const int slot = teamID < 0 ? 0 : (teamID % 4);
        const float* color = menu.team_custom_colors[slot];
        return ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3]));
    }
    return telemetryColors::GetTeamColor(teamID);
}

int AlphaByte(float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    return static_cast<int>(alpha * 255.0f);
}

ImU32 WithAlpha(ImU32 color, float alphaMult) {
    const int alpha = static_cast<int>(((color >> 24) & 0xFF) * std::clamp(alphaMult, 0.0f, 1.0f));
    return (color & 0x00FFFFFF) | (alpha << 24);
}

ImVec2 TextSize(const char* text, float fontSize) {
    return ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
}

float NormalizeAngle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle > 360.0f) angle -= 360.0f;
    return angle;
}

ImVec2 GetViewPos(const ImVec2& pos, float angle, float distance) {
    constexpr float kDegToRad = 0.01745329251994f;
    const float radians = angle * kDegToRad;
    return ImVec2(pos.x + std::cos(radians) * distance,
        pos.y + std::sin(radians) * distance);
}

void DrawOutlinedText(ImDrawList* draw, const ImVec2& pos, ImU32 color,
                      const char* text, float fontSize, int outlineAlpha = 175) {
    const ImU32 outline = IM_COL32(0, 0, 0, outlineAlpha);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), outline, text);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x - 1.0f, pos.y + 1.0f), outline, text);
    draw->AddText(ImGui::GetFont(), fontSize, pos, color, text);
}

void DrawTeamMarker(ImDrawList* draw, float x, float y, int teamID,
                    ImU32 color, float radius, float fillAlpha = 0.86f,
                    float fontSize = 0.0f) {
    draw->AddCircle(ImVec2(x, y), radius + 1.8f, IM_COL32(0, 0, 0, 105), 20, 1.0f);
    draw->AddCircleFilled(ImVec2(x, y), radius, WithAlpha(color, fillAlpha), 20);
    draw->AddCircle(ImVec2(x, y), radius, IM_COL32(0, 0, 0, 205), 20, 1.0f);
    draw->AddCircle(ImVec2(x, y), radius - 1.0f, IM_COL32(255, 255, 255, 60), 20, 0.5f);

    if (teamID <= 0) return;
    char teamText[16];
    sprintf_s(teamText, "%d", teamID % 100);
    const float textFontSize = fontSize > 0.0f ? fontSize : std::clamp(radius * 1.35f, 8.0f, 14.0f);
    const ImVec2 textSize = TextSize(teamText, textFontSize);
    const ImVec2 textPos(x - textSize.x * 0.5f, y - textSize.y * 0.5f);
    DrawOutlinedText(draw, textPos, IM_COL32(255, 255, 255, 255), teamText, textFontSize, 220);
}

void DrawBigMapDirection(ImDrawList* draw, const OverlayMenu& menu, const PlayerData& player,
                         float x, float y) {
    if (!menu.bigmap_show_direction) return;

    float yaw = 0.0f;
    if (player.HasAimYaw) {
        yaw = player.AimYaw;
    } else {
        const float vx = player.Velocity.x;
        const float vy = player.Velocity.y;
        if ((vx * vx) + (vy * vy) < 225.0f) return;
        yaw = std::atan2(vy, vx) * 57.2957795f;
    }
    yaw = NormalizeAngle(yaw);

    const ImVec2 center(x, y);
    const ImVec2 tip = GetViewPos(center, yaw, menu.bigmap_marker_size + 13.5f);
    const ImVec2 left = GetViewPos(center, NormalizeAngle(yaw - 21.0f), menu.bigmap_marker_size + 4.5f);
    const ImVec2 right = GetViewPos(center, NormalizeAngle(yaw + 21.0f), menu.bigmap_marker_size + 4.5f);

    draw->AddTriangleFilled(ImVec2(tip.x + 1.0f, tip.y + 1.0f),
        ImVec2(left.x + 1.0f, left.y + 1.0f),
        ImVec2(right.x + 1.0f, right.y + 1.0f),
        IM_COL32(0, 0, 0, 120));
    draw->AddTriangleFilled(tip, left, right, IM_COL32(255, 255, 255, 225));
    draw->AddTriangle(tip, left, right, IM_COL32(0, 0, 0, 210), 1.4f);

    for (int step = 0; step <= 40; step += 4) {
        const ImVec2 p1 = GetViewPos(center, NormalizeAngle(yaw - static_cast<float>(step)),
            menu.bigmap_marker_size + 4.0f);
        const ImVec2 p2 = GetViewPos(center, NormalizeAngle(yaw + static_cast<float>(step)),
            menu.bigmap_marker_size + 4.0f);
        draw->AddLine(p1, ImVec2(p1.x + 0.5f, p1.y + 0.5f), IM_COL32(255, 255, 255, 205), 2.0f);
        draw->AddLine(p2, ImVec2(p2.x + 0.5f, p2.y + 0.5f), IM_COL32(255, 255, 255, 205), 2.0f);
    }
}

void DrawBigMapName(ImDrawList* draw, const OverlayMenu& menu, const PlayerData& player,
                    float x, float y) {
    if (!menu.bigmap_show_names || player.Name.empty()) return;

    const char* name = player.Name.c_str();
    const float fontSize = std::clamp(menu.bigmap_name_font_size, 9.0f, 22.0f);
    const ImVec2 textSize = TextSize(name, fontSize);
    const ImVec2 textPos(x - textSize.x * 0.5f, y + menu.bigmap_marker_size + 4.0f);

    if (menu.bigmap_name_background && menu.bigmap_name_bg_alpha > 0.01f) {
        const ImVec2 bgMin(textPos.x - 4.0f, textPos.y - 1.0f);
        const ImVec2 bgMax(textPos.x + textSize.x + 4.0f, textPos.y + textSize.y + 2.0f);
        draw->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, AlphaByte(menu.bigmap_name_bg_alpha)), 3.0f);
        draw->AddRect(bgMin, bgMax, IM_COL32(255, 255, 255, 24), 3.0f, 0, 1.0f);
    }

    DrawOutlinedText(draw, textPos, IM_COL32(255, 255, 255, 240), name, fontSize, 155);
}

void DrawBigMapMarker(ImDrawList* draw, const OverlayMenu& menu, const PlayerData& player,
                      float x, float y) {
    DrawBigMapDirection(draw, menu, player, x, y);
    DrawTeamMarker(draw, x, y, player.TeamID, GetTeamColor(menu, player.TeamID),
        menu.bigmap_marker_size, menu.bigmap_marker_alpha, menu.bigmap_name_font_size * 0.82f);
    DrawBigMapName(draw, menu, player, x, y);
}

struct BigMapRect {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

bool IsValidBigMapRect(const BigMapRect& rect) {
    return std::isfinite(rect.left) && std::isfinite(rect.top) &&
        std::isfinite(rect.right) && std::isfinite(rect.bottom) &&
        rect.right > rect.left + 100.0f &&
        rect.bottom > rect.top + 100.0f;
}

ImVec2 ProjectBigMapScreen(const BigMapRect& rect, const Vector3& position) {
    const float mapWorldSize = (std::isfinite(G_Radar.MapWorldSize) && G_Radar.MapWorldSize > 1000.0f)
        ? G_Radar.MapWorldSize
        : 408000.0f;
    const float zoom = (std::isfinite(G_Radar.WorldMapZoomFactor) && G_Radar.WorldMapZoomFactor > 0.001f)
        ? G_Radar.WorldMapZoomFactor
        : 1.0f;
    float mapSizeFactored = G_Radar.MapSizeFactored;
    if (!std::isfinite(mapSizeFactored) || mapSizeFactored <= 1000.0f) {
        mapSizeFactored = mapWorldSize / zoom;
    }

    Vector3 worldCenter = G_Radar.WorldCenterLocation;
    if (!std::isfinite(worldCenter.x) || !std::isfinite(worldCenter.y) ||
        (std::fabs(worldCenter.x) < 1.0f && std::fabs(worldCenter.y) < 1.0f)) {
        worldCenter = {
            mapWorldSize * (1.0f + G_Radar.WorldMapPosition.x),
            mapWorldSize * (1.0f + G_Radar.WorldMapPosition.y),
            0.0f
        };
    }

    const Vector3 worldLocation = {
        position.x + G_Radar.WorldOriginLocation.x,
        position.y + G_Radar.WorldOriginLocation.y,
        0.0f
    };
    const float radarPosX = worldLocation.x - worldCenter.x;
    const float radarPosY = worldLocation.y - worldCenter.y;
    const float mapPixelRadius = (std::max)(1.0f,
        (std::min)(rect.right - rect.left, rect.bottom - rect.top) * 0.5f);
    const float x = rect.left + (rect.right - rect.left) * 0.5f +
        (radarPosX / mapSizeFactored) * mapPixelRadius;
    const float y = rect.top + (rect.bottom - rect.top) * 0.5f +
        (radarPosY / mapSizeFactored) * mapPixelRadius;
    return ImVec2(x, y);
}

ImVec2 ClampToBigMap(const ImVec2& point, const BigMapRect& rect) {
    return ImVec2(
        std::clamp(point.x, rect.left + 2.0f, rect.right - 2.0f),
        std::clamp(point.y, rect.top + 2.0f, rect.bottom - 2.0f));
}

bool WorldToBigMapScreen(const BigMapRect& rect, const Vector3& position,
                         float& outX, float& outY) {
    const ImVec2 point = ProjectBigMapScreen(rect, position);
    outX = point.x;
    outY = point.y;
    return outX >= rect.left && outX <= rect.right && outY >= rect.top && outY <= rect.bottom;
}

ImVec2 BigMapRayEnd(const BigMapRect& rect, const PlayerData& player, const ImVec2& start,
                    float yaw, float lengthMeters, float minPixelLength) {
    constexpr float kDegToRad = 0.01745329251994f;
    const float radians = NormalizeAngle(yaw) * kDegToRad;
    const float worldLength = lengthMeters * 100.0f;
    const Vector3 endWorld = player.Position + Vector3{
        static_cast<float>(std::cos(radians) * worldLength),
        static_cast<float>(std::sin(radians) * worldLength),
        0.0f
    };

    ImVec2 end = ProjectBigMapScreen(rect, endWorld);
    const float dx = end.x - start.x;
    const float dy = end.y - start.y;
    const float pixelLength = std::sqrt((dx * dx) + (dy * dy));
    if (pixelLength < minPixelLength) {
        end = GetViewPos(start, yaw, minPixelLength);
    }
    return ClampToBigMap(end, rect);
}

void DrawBigMapAimRay(ImDrawList* draw, const OverlayMenu& menu, const PlayerData& player,
                      const BigMapRect& rect, const ImVec2& marker) {
    if (player.IsTeammate || !player.HasAimYaw) return;

    const float yaw = NormalizeAngle(player.AimYaw + menu.radar_rotation_offset);
    const float rayWidth = std::clamp(menu.minimap_ray_width, 0.5f, 4.0f);

    if (menu.minimap_show_direction) {
        const ImVec2 viewEnd = BigMapRayEnd(rect, player, marker, yaw,
            menu.minimap_view_ray_length, menu.bigmap_marker_size + 22.0f);
        ImU32 viewCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
            menu.view_direction_color[0], menu.view_direction_color[1],
            menu.view_direction_color[2], menu.view_direction_color[3]));
        draw->AddLine(ImVec2(marker.x + 1.0f, marker.y + 1.0f),
            ImVec2(viewEnd.x + 1.0f, viewEnd.y + 1.0f),
            IM_COL32(0, 0, 0, 130), rayWidth + 1.6f);
        draw->AddLine(marker, viewEnd, WithAlpha(viewCol, 0.76f), rayWidth);
    }

    if (!menu.minimap_fire_trace || player.LastShotTimeMs == 0) return;

    const uint64_t now = GetTickCount64();
    const float ageMs = static_cast<float>(now - player.LastShotTimeMs);
    const float flashMs = (std::max)(80.0f, menu.minimap_fire_flash_ms);
    if (ageMs > flashMs) return;

    const float pulse = 1.0f - (ageMs / flashMs);
    const float alpha = std::clamp(0.25f + pulse * 0.75f, 0.0f, 1.0f);
    const ImVec2 fireEnd = BigMapRayEnd(rect, player, marker, yaw,
        menu.minimap_fire_ray_length, menu.bigmap_marker_size + 92.0f);
    ImU32 fireCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
        menu.aim_warning_color[0], menu.aim_warning_color[1],
        menu.aim_warning_color[2], menu.aim_warning_color[3]));

    draw->AddLine(ImVec2(marker.x + 1.0f, marker.y + 1.0f),
        ImVec2(fireEnd.x + 1.0f, fireEnd.y + 1.0f),
        IM_COL32(0, 0, 0, AlphaByte(0.66f * alpha)), rayWidth + 4.2f);
    draw->AddLine(marker, fireEnd, WithAlpha(fireCol, alpha * 0.42f), rayWidth + 3.0f);
    draw->AddLine(marker, fireEnd, WithAlpha(fireCol, alpha), rayWidth + 1.0f);
    draw->AddCircleFilled(fireEnd, 2.8f + pulse * 3.6f, WithAlpha(fireCol, alpha), 18);
}

void DrawScaledIcon(ImDrawList* draw, TextureInfo* icon, const ImVec2& center,
                    float targetSize, ImU32 tint = IM_COL32(255, 255, 255, 255),
                    bool important = false) {
    if (!icon || !icon->SRV || icon->Width <= 0 || icon->Height <= 0) return;

    OverlayAssetAnimation::DrawOptions anim{};
    anim.important = important;
    anim.strength = important ? 1.18f : 0.86f;
    OverlayAssetAnimation::DrawAnimatedImage(draw, icon, center, targetSize, tint, anim);
}

bool ShouldDrawBigMapVehicle(const OverlayMenu& menu, const std::string& name) {
    const VehicleResolver::VehicleInfo info = VehicleResolver::Resolve(name);
    switch (info.CategoryId) {
    case VehicleResolver::Category::Uaz: return menu.loot_vehicle_uaz;
    case VehicleResolver::Category::Dacia: return menu.loot_vehicle_dacia;
    case VehicleResolver::Category::Buggy: return menu.loot_vehicle_buggy;
    case VehicleResolver::Category::Bike: return menu.loot_vehicle_bike;
    case VehicleResolver::Category::Boat: return menu.loot_vehicle_boat;
    case VehicleResolver::Category::Brdm: return menu.loot_vehicle_brdm;
    case VehicleResolver::Category::Scooter: return menu.loot_vehicle_scooter;
    case VehicleResolver::Category::Snow: return menu.loot_vehicle_snow;
    case VehicleResolver::Category::Tuk: return menu.loot_vehicle_tuk;
    case VehicleResolver::Category::Bus: return menu.loot_vehicle_bus;
    case VehicleResolver::Category::Truck: return menu.loot_vehicle_truck;
    case VehicleResolver::Category::Train: return menu.loot_vehicle_train;
    case VehicleResolver::Category::Mirado: return menu.loot_vehicle_mirado;
    case VehicleResolver::Category::Pickup: return menu.loot_vehicle_pickup;
    case VehicleResolver::Category::Rony: return menu.loot_vehicle_rony;
    case VehicleResolver::Category::Blanc: return menu.loot_vehicle_blanc;
    case VehicleResolver::Category::Air: return menu.loot_vehicle_air;
    default: return true;
    }
}

void DrawBigMapIcon(ImDrawList* draw, float x, float y, TextureInfo* icon,
                    float size, ImU32 tint, ImU32 fallbackColor, bool important = false) {
    const ImVec2 center(x, y);
    draw->AddCircleFilled(ImVec2(x + 1.0f, y + 1.0f), size * 0.48f, IM_COL32(0, 0, 0, 55), 18);
    if (icon && icon->SRV) {
        DrawScaledIcon(draw, icon, center, size, tint, important);
    } else {
        draw->AddCircleFilled(center, size * 0.34f, fallbackColor, 18);
        draw->AddCircle(center, size * 0.36f, IM_COL32(0, 0, 0, 170), 18, 1.0f);
    }
}

void DrawBigMapItem(ImDrawList* draw, const OverlayMenu& menu, const ItemData& item,
                    float x, float y) {
    TextureInfo* icon = nullptr;
    ImU32 tint = IM_COL32(255, 255, 255, 235);
    ImU32 fallback = IM_COL32(255, 255, 255, 220);
    float size = menu.bigmap_icon_size;
    bool important = false;

    if (item.RenderType == ItemRenderType::Vehicle) {
        if (!menu.bigmap_show_vehicles || !menu.esp_vehicles ||
            item.Distance > static_cast<float>(menu.vehicle_max_dist)) return;
        const std::string vehicleName = item.ClassName.empty() ? item.Name : item.ClassName;
        if (!ShouldDrawBigMapVehicle(menu, vehicleName)) return;
        icon = OverlayTextures::GetVehicleIcon(vehicleName);
        if (!icon || !icon->SRV) icon = OverlayTextures::GetMapIcon(skCrypt("indicator_onscreen_status_vehicle"));
        tint = IM_COL32(115, 205, 250, 240);
        fallback = IM_COL32(85, 210, 255, 230);
    } else if (item.RenderType == ItemRenderType::AirDrop) {
        if (!menu.bigmap_show_airdrops || !menu.esp_airdrops) return;
        icon = OverlayTextures::GetMapIcon(skCrypt("Carapackage_RedBox_C"));
        size *= 1.15f;
        fallback = IM_COL32(255, 70, 70, 235);
        important = true;
    } else if (item.RenderType == ItemRenderType::DeadBox) {
        if (!menu.bigmap_show_deadboxes || !menu.esp_deadboxes) return;
        icon = OverlayTextures::GetMapIcon(skCrypt("dead"));
        size *= 0.90f;
        tint = IM_COL32(255, 190, 90, 235);
        fallback = IM_COL32(255, 150, 55, 230);
        important = true;
    } else {
        return;
    }

    DrawBigMapIcon(draw, x, y, icon, size, tint, fallback, important);
}

void DrawBigMapItems(ImDrawList* draw, const OverlayMenu& menu, const BigMapRect& rect) {
    if (!menu.bigmap_show_vehicles && !menu.bigmap_show_airdrops && !menu.bigmap_show_deadboxes) return;

    std::vector<ItemData> items;
    {
        std::lock_guard<std::mutex> lock(CachedItemsMutex);
        items = CachedItems;
    }

    struct SeenIcon {
        ItemRenderType type = ItemRenderType::Loot;
        float x = 0.0f;
        float y = 0.0f;
    };
    std::vector<SeenIcon> seen;
    for (const auto& item : items) {
        float x = 0.0f;
        float y = 0.0f;
        if (!WorldToBigMapScreen(rect, item.Position, x, y)) continue;

        bool duplicate = false;
        for (const auto& prev : seen) {
            if (prev.type != item.RenderType) continue;
            const float dx = prev.x - x;
            const float dy = prev.y - y;
            if ((dx * dx) + (dy * dy) <= 14.0f * 14.0f) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        DrawBigMapItem(draw, menu, item, x, y);
        seen.push_back({ item.RenderType, x, y });
    }
}

void DrawLegendRow(ImDrawList* draw, float x, float y, ImU32 color, const char* label,
                   float fontSize, bool line = false) {
    if (line) {
        draw->AddLine(ImVec2(x - 5.0f, y + 6.0f), ImVec2(x + 9.0f, y + 6.0f),
            IM_COL32(0, 0, 0, 135), 3.2f);
        draw->AddLine(ImVec2(x - 5.0f, y + 6.0f), ImVec2(x + 9.0f, y + 6.0f),
            color, 1.8f);
    } else {
        draw->AddCircleFilled(ImVec2(x + 2.0f, y + 6.0f), 4.0f, color, 16);
        draw->AddCircle(ImVec2(x + 2.0f, y + 6.0f), 4.5f, IM_COL32(0, 0, 0, 130), 16, 1.0f);
    }
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 16.0f, y), IM_COL32(235, 245, 255, 230), label);
}

void DrawBigMapLegend(ImDrawList* draw, const OverlayMenu& menu, const BigMapRect& rect) {
    if (!menu.bigmap_show_legend) return;

    auto Lang = Translation::Get();
    const float fontSize = 12.0f;
    const float rowHeight = 16.0f;
    const float width = 142.0f;
    float rows = 1.0f;
    rows += menu.bigmap_show_vehicles ? 1.0f : 0.0f;
    rows += menu.bigmap_show_airdrops ? 1.0f : 0.0f;
    rows += menu.bigmap_show_deadboxes ? 1.0f : 0.0f;
    rows += menu.minimap_show_direction ? 1.0f : 0.0f;
    rows += menu.minimap_fire_trace ? 1.0f : 0.0f;
    const float height = 18.0f + rows * rowHeight;
    const ImVec2 boxMin(rect.left + 12.0f, rect.top + 12.0f);
    const ImVec2 boxMax(boxMin.x + width, boxMin.y + height);

    draw->AddRectFilled(boxMin, boxMax, IM_COL32(10, 18, 26, 118), 5.0f);
    draw->AddRect(boxMin, boxMax, IM_COL32(95, 190, 255, 92), 5.0f, 0, 1.0f);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(boxMin.x + 10.0f, boxMin.y + 6.0f),
        IM_COL32(125, 220, 255, 235), Lang.MapLegend);

    float y = boxMin.y + 24.0f;
    DrawLegendRow(draw, boxMin.x + 13.0f, y, IM_COL32(255, 80, 80, 230), Lang.Enemy, fontSize);
    y += rowHeight;
    if (menu.bigmap_show_vehicles) {
        DrawLegendRow(draw, boxMin.x + 13.0f, y, IM_COL32(85, 210, 255, 230), Lang.Vehicle, fontSize);
        y += rowHeight;
    }
    if (menu.bigmap_show_airdrops) {
        DrawLegendRow(draw, boxMin.x + 13.0f, y, IM_COL32(255, 70, 70, 235), Lang.ShowAirdrops, fontSize);
        y += rowHeight;
    }
    if (menu.bigmap_show_deadboxes) {
        DrawLegendRow(draw, boxMin.x + 13.0f, y, IM_COL32(255, 150, 55, 230), Lang.ShowDeathboxes, fontSize);
        y += rowHeight;
    }
    if (menu.minimap_show_direction) {
        ImU32 viewCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
            menu.view_direction_color[0], menu.view_direction_color[1],
            menu.view_direction_color[2], menu.view_direction_color[3]));
        DrawLegendRow(draw, boxMin.x + 13.0f, y, WithAlpha(viewCol, 0.86f), Lang.ViewRay, fontSize, true);
        y += rowHeight;
    }
    if (menu.minimap_fire_trace) {
        ImU32 fireCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
            menu.aim_warning_color[0], menu.aim_warning_color[1],
            menu.aim_warning_color[2], menu.aim_warning_color[3]));
        DrawLegendRow(draw, boxMin.x + 13.0f, y, WithAlpha(fireCol, 0.92f), Lang.FireTrace, fontSize, true);
    }
}

ImVec2 MiniMapCenter(float screenWidth, float screenHeight, bool expanded) {
    if (!expanded) {
        if (screenWidth == 1280.0f && screenHeight == 720.0f) return ImVec2(0.9136f, 0.8546f);
        if (screenWidth == 1280.0f && screenHeight == 768.0f) return ImVec2(0.9078f, 0.8544f);
        if (screenWidth == 1280.0f && screenHeight == 800.0f) return ImVec2(0.9038f, 0.8546f);
        if (screenWidth == 1360.0f && screenHeight == 768.0f) return ImVec2(0.9136f, 0.8548f);
        if (screenWidth == 1366.0f && screenHeight == 768.0f) return ImVec2(0.9136f, 0.8548f);
        if (screenWidth == 1440.0f && screenHeight == 900.0f) return ImVec2(0.9040f, 0.8546f);
        if (screenWidth == 1600.0f && screenHeight == 900.0f) return ImVec2(0.9138f, 0.8546f);
        if (screenWidth == 1680.0f && screenHeight == 1050.0f) return ImVec2(0.9038f, 0.8549f);
        if (screenWidth == 1728.0f && screenHeight == 1080.0f) return ImVec2(0.9063f, 0.8539f);
        if (screenWidth == 1920.0f && screenHeight == 1080.0f) return ImVec2(0.9140f, 0.8550f);
        if (screenWidth == 1920.0f && screenHeight == 1200.0f) return ImVec2(0.90478f, 0.8535f);
        if (screenWidth == 2560.0f && screenHeight == 1080.0f) return ImVec2(0.936585f, 0.8519f);
        if (screenWidth == 2560.0f && screenHeight == 1440.0f) return ImVec2(0.91424f, 0.85559f);
        if (screenWidth == 2560.0f && screenHeight == 1600.0f) return ImVec2(0.915490f, 0.852890f);
        if (screenWidth == 3440.0f && screenHeight == 1440.0f) return ImVec2(0.937135f, 0.8525f);
        if (screenWidth == 3840.0f && screenHeight == 2160.0f) return ImVec2(0.914402f, 0.8554f);
        return ImVec2(0.914402f, 0.8554f);
    }

    if (screenWidth == 1280.0f && screenHeight == 720.0f) return ImVec2(0.861323f, 0.761643f);
    if (screenWidth == 1280.0f && screenHeight == 768.0f) return ImVec2(0.851852f, 0.761865f);
    if (screenWidth == 1280.0f && screenHeight == 800.0f) return ImVec2(0.8456486f, 0.761643f);
    if (screenWidth == 1360.0f && screenHeight == 768.0f) return ImVec2(0.8613228f, 0.761821f);
    if (screenWidth == 1366.0f && screenHeight == 768.0f) return ImVec2(0.8613428f, 0.761821f);
    if (screenWidth == 1440.0f && screenHeight == 900.0f) return ImVec2(0.8455022f, 0.761643f);
    if (screenWidth == 1600.0f && screenHeight == 900.0f) return ImVec2(0.8615114f, 0.761643f);
    if (screenWidth == 1680.0f && screenHeight == 1050.0f) return ImVec2(0.8469486f, 0.761910f);
    if (screenWidth == 1728.0f && screenHeight == 1080.0f) return ImVec2(0.8493f, 0.7607f);
    if (screenWidth == 1920.0f && screenHeight == 1080.0f) return ImVec2(0.8617f, 0.762f);
    if (screenWidth == 1920.0f && screenHeight == 1200.0f) return ImVec2(0.84677f, 0.762285f);
    if (screenWidth == 2560.0f && screenHeight == 1080.0f) return ImVec2(0.897178f, 0.760035f);
    if (screenWidth == 2560.0f && screenHeight == 1440.0f) return ImVec2(0.862473f, 0.762785f);
    if (screenWidth == 2560.0f && screenHeight == 1600.0f) return ImVec2(0.863273f, 0.759935f);
    if (screenWidth == 3440.0f && screenHeight == 1440.0f) return ImVec2(0.897929f, 0.759484f);
    if (screenWidth == 3840.0f && screenHeight == 2160.0f) return ImVec2(0.862393f, 0.762185f);
    return ImVec2(0.862393f, 0.762185f);
}

float MiniMapDiv(float screenWidth, float screenHeight, bool expanded) {
    if (!expanded) {
        if (screenWidth == 1728.0f && screenHeight == 1080.0f) return 122.0f;
    } else if (screenWidth == 1728.0f && screenHeight == 1080.0f) {
        return 132.0f;
    }

    if (screenWidth == 1280.0f && screenHeight == 720.0f) return 86.0f;
    if (screenWidth == 1280.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1280.0f && screenHeight == 800.0f) return 96.0f;
    if (screenWidth == 1360.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1366.0f && screenHeight == 768.0f) return 92.0f;
    if (screenWidth == 1440.0f && screenHeight == 900.0f) return 104.0f;
    if (screenWidth == 1600.0f && screenHeight == 900.0f) return 104.0f;
    if (screenWidth == 1680.0f && screenHeight == 1050.0f) return 126.0f;
    if (screenWidth == 1920.0f && screenHeight == 1080.0f) return 126.0f;
    if (screenWidth == 1920.0f && screenHeight == 1200.0f) return 146.0f;
    if (screenWidth == 2560.0f && screenHeight == 1080.0f) return 128.0f;
    if (screenWidth == 2560.0f && screenHeight == 1440.0f) return 175.0f;
    if (screenWidth == 2560.0f && screenHeight == 1600.0f) return 172.0f;
    if (screenWidth == 3440.0f && screenHeight == 1440.0f) return 158.0f;
    if (screenWidth == 3840.0f && screenHeight == 2160.0f) return 258.0f;
    return 258.0f;
}

float CurrentMiniMapViewScale() {
    float scale = G_Radar.CurrentMinimapViewScale;
    if (!std::isfinite(scale) || scale < 0.01f || scale > 20.0f) {
        return 1.0f;
    }
    return std::clamp(scale, 0.65f, 2.50f);
}

float ExpandedMiniMapScale(float rawScale, bool expanded) {
    if (!expanded) return 1.0f;
    constexpr float kDefaultExpandedScale = 1.7692308f;
    return std::clamp((std::max)(rawScale, kDefaultExpandedScale), 1.0f, 2.50f);
}

float MiniMapWorldRange(float sizeScale, bool expanded) {
    if (!expanded) return 20000.0f;
    const float overviewScale = 1.0f + (std::max)(0.0f, sizeScale - 1.0f) * 2.0f;
    const float range = 20000.0f * overviewScale;
    return std::clamp(range, 12000.0f, 90000.0f);
}

bool HasMiniMapProfile(float screenWidth, float screenHeight) {
    return (screenWidth == 1280.0f && screenHeight == 720.0f) ||
        (screenWidth == 1280.0f && screenHeight == 768.0f) ||
        (screenWidth == 1280.0f && screenHeight == 800.0f) ||
        (screenWidth == 1360.0f && screenHeight == 768.0f) ||
        (screenWidth == 1366.0f && screenHeight == 768.0f) ||
        (screenWidth == 1440.0f && screenHeight == 900.0f) ||
        (screenWidth == 1600.0f && screenHeight == 900.0f) ||
        (screenWidth == 1680.0f && screenHeight == 1050.0f) ||
        (screenWidth == 1728.0f && screenHeight == 1080.0f) ||
        (screenWidth == 1920.0f && screenHeight == 1080.0f) ||
        (screenWidth == 1920.0f && screenHeight == 1200.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1080.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1440.0f) ||
        (screenWidth == 2560.0f && screenHeight == 1600.0f) ||
        (screenWidth == 3440.0f && screenHeight == 1440.0f) ||
        (screenWidth == 3840.0f && screenHeight == 2160.0f);
}

bool ShouldDrawPlayer(const OverlayMenu& menu, const PlayerData& player) {
    return player.IsTeammate ? menu.esp_show_teammates : menu.esp_show_enemies;
}

ImVec2 ClampToMiniMap(const ImVec2& point, float left, float top, float right, float bottom) {
    return ImVec2(
        std::clamp(point.x, left + 2.0f, right - 2.0f),
        std::clamp(point.y, top + 2.0f, bottom - 2.0f));
}

ImVec2 MiniMapRayEnd(const ImVec2& start, float yaw, float lengthMeters, float mapDiv, float worldRange,
                     float left, float top, float right, float bottom) {
    constexpr float kDegToRad = 0.01745329251994f;
    const float radians = NormalizeAngle(yaw) * kDegToRad;
    const float worldLength = lengthMeters * 100.0f;
    const ImVec2 end(
        start.x + std::cos(radians) * (worldLength / worldRange) * mapDiv,
        start.y + std::sin(radians) * (worldLength / worldRange) * mapDiv);
    return ClampToMiniMap(end, left, top, right, bottom);
}

void DrawMiniMapAimRay(ImDrawList* draw, const OverlayMenu& menu, const PlayerData& player,
                       const ImVec2& marker, float mapDiv, float worldRange,
                       float left, float top, float right, float bottom) {
    if (player.IsTeammate || !player.HasAimYaw) return;

    const float yaw = NormalizeAngle(player.AimYaw + menu.radar_rotation_offset);
    const float rayWidth = std::clamp(menu.minimap_ray_width, 0.5f, 4.0f);

    if (menu.minimap_show_direction) {
        const ImVec2 viewEnd = MiniMapRayEnd(marker, yaw, menu.minimap_view_ray_length,
            mapDiv, worldRange, left, top, right, bottom);
        ImU32 viewCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
            menu.view_direction_color[0], menu.view_direction_color[1],
            menu.view_direction_color[2], menu.view_direction_color[3]));
        draw->AddLine(ImVec2(marker.x + 1.0f, marker.y + 1.0f),
            ImVec2(viewEnd.x + 1.0f, viewEnd.y + 1.0f),
            IM_COL32(0, 0, 0, 115), rayWidth + 1.4f);
        draw->AddLine(marker, viewEnd, WithAlpha(viewCol, 0.70f), rayWidth);
    }

    if (!menu.minimap_fire_trace || player.LastShotTimeMs == 0) return;

    const uint64_t now = GetTickCount64();
    const float ageMs = static_cast<float>(now - player.LastShotTimeMs);
    const float flashMs = (std::max)(80.0f, menu.minimap_fire_flash_ms);
    if (ageMs > flashMs) return;

    const float pulse = 1.0f - (ageMs / flashMs);
    const float alpha = std::clamp(0.25f + pulse * 0.75f, 0.0f, 1.0f);
    const ImVec2 fireEnd = MiniMapRayEnd(marker, yaw, menu.minimap_fire_ray_length,
        mapDiv, worldRange, left, top, right, bottom);
    ImU32 fireCol = ImGui::ColorConvertFloat4ToU32(ImVec4(
        menu.aim_warning_color[0], menu.aim_warning_color[1],
        menu.aim_warning_color[2], menu.aim_warning_color[3]));

    draw->AddLine(ImVec2(marker.x + 1.0f, marker.y + 1.0f),
        ImVec2(fireEnd.x + 1.0f, fireEnd.y + 1.0f),
        IM_COL32(0, 0, 0, AlphaByte(0.62f * alpha)), rayWidth + 4.0f);
    draw->AddLine(marker, fireEnd, WithAlpha(fireCol, alpha * 0.40f), rayWidth + 3.0f);
    draw->AddLine(marker, fireEnd, WithAlpha(fireCol, alpha), rayWidth + 0.9f);
    draw->AddCircleFilled(fireEnd, 2.3f + pulse * 2.7f, WithAlpha(fireCol, alpha), 16);
}

} // namespace

namespace RadarOverlayRenderer {

void Draw(ImDrawList* draw, OverlayMenu& menu, const std::vector<PlayerData>& players) {
    if (!draw || !menu.radar_enabled) return;

    const float screenWidth = menu.ScreenWidth;
    const float screenHeight = menu.ScreenHeight;
    const float miniMapViewScale = CurrentMiniMapViewScale();

    // Manual toggle for N (minimap expansion)
    static bool localNState = false;
    static bool nWasDown = false;
    bool nIsDown = (GetAsyncKeyState('N') & 0x8000);
    if (nIsDown && !nWasDown && !menu.showmenu) {
        localNState = !localNState;
    }
    nWasDown = nIsDown;

    // Auto-correction: If memory definitely says it's small, reset our local toggle to prevent "reversed" state
    const bool isLargeWidget = G_Radar.ScreenSize > 300.0f; 
    const bool isDefinitivelySmall = G_Radar.ScreenSize > 40.0f && G_Radar.ScreenSize < 280.0f;
    
    if (isDefinitivelySmall) {
        localNState = false; // Force sync if the widget is physically small
    } else if (isLargeWidget || G_Radar.SelectMinimapSizeIndex > 0) {
        localNState = true; // Force sync if the widget is physically large
    }

    const bool expandedMiniMap = localNState || G_Radar.SelectMinimapSizeIndex > 0 || miniMapViewScale > 1.05f || isLargeWidget;
    const float miniMapSizeScale = ExpandedMiniMapScale(miniMapViewScale, expandedMiniMap);
    const bool hasHudMiniMapRect = G_Radar.ScreenPosX > 1.0f && G_Radar.ScreenPosY > 1.0f &&
        G_Radar.ScreenSize > 40.0f && G_Radar.ScreenSizeY > 40.0f;
    const bool hasPaodProfile = HasMiniMapProfile(screenWidth, screenHeight);
    float centerX = 0.0f;
    float centerY = 0.0f;
    float mapDiv = 0.0f;
    float miniLeft = 0.0f;
    float miniTop = 0.0f;
    float miniRight = 0.0f;
    float miniBottom = 0.0f;

    if (hasHudMiniMapRect && hasPaodProfile) {
        const ImVec2 mapCenter = MiniMapCenter(screenWidth, screenHeight, expandedMiniMap);
        centerX = screenWidth * mapCenter.x + menu.radar_offset_x;
        centerY = screenHeight * mapCenter.y + menu.radar_offset_y;
        mapDiv = MiniMapDiv(screenWidth, screenHeight, expandedMiniMap) *
            miniMapSizeScale * menu.radar_zoom_multiplier;
        miniLeft = centerX - mapDiv;
        miniTop = centerY - mapDiv;
        miniRight = centerX + mapDiv;
        miniBottom = centerY + mapDiv;
    } else if (hasHudMiniMapRect) {
        const float radarW = G_Radar.ScreenSize * miniMapSizeScale;
        const float radarH = G_Radar.ScreenSizeY * miniMapSizeScale;
        centerX = G_Radar.ScreenPosX + G_Radar.ScreenSize * 0.5f + menu.radar_offset_x;
        centerY = G_Radar.ScreenPosY + G_Radar.ScreenSizeY * 0.5f + menu.radar_offset_y;
        mapDiv = (radarW < radarH ? radarW : radarH) * 0.5f * menu.radar_zoom_multiplier;
        miniLeft = centerX - radarW * 0.5f;
        miniTop = centerY - radarH * 0.5f;
        miniRight = centerX + radarW * 0.5f;
        miniBottom = centerY + radarH * 0.5f;
    } else {
        const ImVec2 mapCenter = MiniMapCenter(screenWidth, screenHeight, expandedMiniMap);
        centerX = screenWidth * mapCenter.x + menu.radar_offset_x;
        centerY = screenHeight * mapCenter.y + menu.radar_offset_y;
        mapDiv = MiniMapDiv(screenWidth, screenHeight, expandedMiniMap) *
            miniMapSizeScale * menu.radar_zoom_multiplier;
        miniLeft = centerX - mapDiv;
        miniTop = centerY - mapDiv;
        miniRight = centerX + mapDiv;
        miniBottom = centerY + mapDiv;
    }

    if (mapDiv < 50.0f) mapDiv = 50.0f;
    const float maxMapDiv = (std::min)(screenWidth, screenHeight) * 0.48f;
    if (mapDiv > maxMapDiv) {
        mapDiv = maxMapDiv;
        miniLeft = centerX - mapDiv;
        miniTop = centerY - mapDiv;
        miniRight = centerX + mapDiv;
        miniBottom = centerY + mapDiv;
    }
    const float worldRange = MiniMapWorldRange(miniMapSizeScale, expandedMiniMap);

    const bool canDrawWorldMap = menu.bigmap_enabled &&
        G_Radar.IsWorldMapVisible &&
        G_Radar.MapWorldSize > 1000.0f;
    bool worldMapDrawn = false;

    if (canDrawWorldMap) {
        BigMapRect bigMapRect = { 0.0f, 0.0f, screenWidth, screenHeight };

        if (IsValidBigMapRect(bigMapRect)) {
            DrawBigMapItems(draw, menu, bigMapRect);

            for (const auto& player : players) {
                if (!ShouldDrawPlayer(menu, player)) continue;
                float mapX = 0.0f;
                float mapY = 0.0f;
                if (!WorldToBigMapScreen(bigMapRect, player.Position, mapX, mapY)) continue;
                DrawBigMapAimRay(draw, menu, player, bigMapRect, ImVec2(mapX, mapY));
                DrawBigMapMarker(draw, menu, player, mapX, mapY);
            }

            DrawBigMapLegend(draw, menu, bigMapRect);
            worldMapDrawn = true;
        }
    }

    if (worldMapDrawn || (!hasHudMiniMapRect && !G_Radar.IsMiniMapVisible)) return;
    if (!menu.minimap_enabled) return;

    for (const auto& player : players) {
        if (!ShouldDrawPlayer(menu, player)) continue;
        const float dx = player.Position.x - G_LocalPlayerPos.x;
        const float dy = player.Position.y - G_LocalPlayerPos.y;
        if (dx > worldRange || dx < -worldRange || dy > worldRange || dy < -worldRange) continue;

        const float finalX = roundf(dx / worldRange * mapDiv) + centerX;
        const float finalY = roundf(dy / worldRange * mapDiv) + centerY;
        const float clampedX = std::clamp(finalX, miniLeft + 3.0f, miniRight - 3.0f);
        const float clampedY = std::clamp(finalY, miniTop + 3.0f, miniBottom - 3.0f);
        DrawMiniMapAimRay(draw, menu, player, ImVec2(clampedX, clampedY), mapDiv, worldRange,
            miniLeft, miniTop, miniRight, miniBottom);
        DrawTeamMarker(draw, clampedX, clampedY, player.TeamID,
            GetTeamColor(menu, player.TeamID), menu.radar_dot_size);
    }

    if (menu.show_radar_center) {
        draw->AddLine(ImVec2(centerX - 10.0f, centerY), ImVec2(centerX + 10.0f, centerY),
            IM_COL32(255, 255, 255, 255), 1.0f);
        draw->AddLine(ImVec2(centerX, centerY - 10.0f), ImVec2(centerX, centerY + 10.0f),
            IM_COL32(255, 255, 255, 255), 1.0f);
    }
}

} // namespace RadarOverlayRenderer
