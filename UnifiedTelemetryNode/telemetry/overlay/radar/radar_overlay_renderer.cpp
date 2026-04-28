#include "radar_overlay_renderer.hpp"

#include "../core/colors.hpp"
#include "../../sdk/context.hpp"
#include <algorithm>
#include <cmath>

namespace {

ImU32 GetTeamColor(int teamID) {
    return telemetryColors::GetTeamColor(teamID);
}

void DrawTeamMarker(ImDrawList* draw, float x, float y, int teamID,
                    ImU32 color, float radius) {
    draw->AddCircle(ImVec2(x, y), radius + 1.8f, IM_COL32(0, 0, 0, 160), 20, 1.0f);
    draw->AddCircleFilled(ImVec2(x, y), radius, color, 20);
    draw->AddCircle(ImVec2(x, y), radius - 1.0f, IM_COL32(255, 255, 255, 80), 20, 0.5f);

    if (teamID <= 0) return;
    char teamText[16];
    sprintf_s(teamText, "%d", teamID % 100);
    ImVec2 textSize = ImGui::CalcTextSize(teamText);
    draw->AddText(ImVec2(x - textSize.x * 0.5f + 1.0f, y - textSize.y * 0.5f + 1.0f),
        IM_COL32(0, 0, 0, 255), teamText);
    draw->AddText(ImVec2(x - textSize.x * 0.5f, y - textSize.y * 0.5f),
        IM_COL32(255, 255, 255, 255), teamText);
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

} // namespace

namespace RadarOverlayRenderer {

void Draw(ImDrawList* draw, OverlayMenu& menu, const std::vector<PlayerData>& players) {
    if (!draw || !menu.radar_enabled) return;

    const float screenWidth = menu.ScreenWidth;
    const float screenHeight = menu.ScreenHeight;
    const bool expandedMiniMap = G_Radar.SelectMinimapSizeIndex > 0;
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
        mapDiv = MiniMapDiv(screenWidth, screenHeight, expandedMiniMap) * menu.radar_zoom_multiplier;
        miniLeft = centerX - mapDiv;
        miniTop = centerY - mapDiv;
        miniRight = centerX + mapDiv;
        miniBottom = centerY + mapDiv;
    } else if (hasHudMiniMapRect) {
        const float radarW = G_Radar.ScreenSize;
        const float radarH = G_Radar.ScreenSizeY;
        centerX = G_Radar.ScreenPosX + radarW * 0.5f + menu.radar_offset_x;
        centerY = G_Radar.ScreenPosY + radarH * 0.5f + menu.radar_offset_y;
        mapDiv = (radarW < radarH ? radarW : radarH) * 0.5f * menu.radar_zoom_multiplier;
        miniLeft = G_Radar.ScreenPosX;
        miniTop = G_Radar.ScreenPosY;
        miniRight = G_Radar.ScreenPosX + radarW;
        miniBottom = G_Radar.ScreenPosY + radarH;
    } else {
        const ImVec2 mapCenter = MiniMapCenter(screenWidth, screenHeight, expandedMiniMap);
        centerX = screenWidth * mapCenter.x + menu.radar_offset_x;
        centerY = screenHeight * mapCenter.y + menu.radar_offset_y;
        mapDiv = MiniMapDiv(screenWidth, screenHeight, expandedMiniMap) * menu.radar_zoom_multiplier;
        miniLeft = centerX - mapDiv;
        miniTop = centerY - mapDiv;
        miniRight = centerX + mapDiv;
        miniBottom = centerY + mapDiv;
    }

    if (mapDiv < 50.0f) mapDiv = 50.0f;

    const bool keyWorldMapPressed = telemetryMemory::IsKeyDown('M');
    const bool canDrawWorldMap = (G_Radar.IsWorldMapVisible || keyWorldMapPressed) &&
        G_Radar.MapWorldSize > 1000.0f;
    bool worldMapDrawn = false;

    if (canDrawWorldMap) {
        float worldLeft = G_Radar.WorldMapX;
        float worldTop = G_Radar.WorldMapY;
        float worldRight = worldLeft + G_Radar.WorldMapWidth;
        float worldBottom = worldTop + G_Radar.WorldMapHeight;
        if (G_Radar.WorldMapWidth <= 10.0f || G_Radar.WorldMapHeight <= 10.0f) {
            const float mapSize = screenHeight;
            worldLeft = (screenWidth - mapSize) * 0.5f;
            worldTop = 0.0f;
            worldRight = worldLeft + mapSize;
            worldBottom = mapSize;
        }

        for (const auto& player : players) {
            if (!ShouldDrawPlayer(menu, player)) continue;
            const Vector3 worldLocation = {
                player.Position.x + G_Radar.WorldOriginLocation.x,
                player.Position.y + G_Radar.WorldOriginLocation.y,
                0.0f
            };
            const float radarPosX = worldLocation.x - G_Radar.WorldCenterLocation.x;
            const float radarPosY = worldLocation.y - G_Radar.WorldCenterLocation.y;
            const float mapX = worldLeft + (worldRight - worldLeft) * 0.5f +
                (radarPosX / G_Radar.MapWorldSize) * G_Radar.WorldMapZoomFactor *
                (worldRight - worldLeft) * 0.5f;
            const float mapY = worldTop + (worldBottom - worldTop) * 0.5f +
                (radarPosY / G_Radar.MapWorldSize) * G_Radar.WorldMapZoomFactor *
                (worldBottom - worldTop) * 0.5f;
            if (mapX < worldLeft || mapX > worldRight || mapY < worldTop || mapY > worldBottom) continue;
            DrawTeamMarker(draw, mapX, mapY, player.TeamID, GetTeamColor(player.TeamID), menu.radar_dot_size);
        }
        worldMapDrawn = true;
    }

    if (worldMapDrawn || (!hasHudMiniMapRect && !G_Radar.IsMiniMapVisible)) return;

    for (const auto& player : players) {
        if (!ShouldDrawPlayer(menu, player)) continue;
        const float worldRange = expandedMiniMap ? 37000.0f : 20000.0f;
        const float dx = player.Position.x - G_LocalPlayerPos.x;
        const float dy = player.Position.y - G_LocalPlayerPos.y;
        if (dx > worldRange || dx < -worldRange || dy > worldRange || dy < -worldRange) continue;

        const float finalX = roundf(dx / 20000.0f * mapDiv) + centerX;
        const float finalY = roundf(dy / 20000.0f * mapDiv) + centerY;
        const float clampedX = std::clamp(finalX, miniLeft + 3.0f, miniRight - 3.0f);
        const float clampedY = std::clamp(finalY, miniTop + 3.0f, miniBottom - 3.0f);
        DrawTeamMarker(draw, clampedX, clampedY, player.TeamID,
            GetTeamColor(player.TeamID), menu.radar_dot_size);
    }

    if (menu.show_radar_center) {
        draw->AddLine(ImVec2(centerX - 10.0f, centerY), ImVec2(centerX + 10.0f, centerY),
            IM_COL32(255, 255, 255, 255), 1.0f);
        draw->AddLine(ImVec2(centerX, centerY - 10.0f), ImVec2(centerX, centerY + 10.0f),
            IM_COL32(255, 255, 255, 255), 1.0f);
    }
}

} // namespace RadarOverlayRenderer
