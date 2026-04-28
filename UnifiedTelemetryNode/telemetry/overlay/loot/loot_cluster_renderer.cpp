#include "loot_cluster_renderer.hpp"

#include "../../imgui/imgui.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr float kClusterScreenRadius = 42.0f;
constexpr int kTwoColumnThreshold = 8;
constexpr float kGroupIconGap = 4.0f;
constexpr float kColumnGap = 6.0f;
constexpr float kPanelPadding = 6.0f;
constexpr float kDistanceGap = 3.0f;

float ScreenDistance(const Vector2& a, const Vector2& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt((dx * dx) + (dy * dy));
}

float ClampFloat(float value, float minValue, float maxValue) {
    if (maxValue < minValue) return minValue;
    return (std::max)(minValue, (std::min)(value, maxValue));
}

float DistanceScale(float distance) {
    if (distance <= 25.0f) return 1.0f;
    if (distance >= 100.0f) return 0.68f;

    const float t = (distance - 25.0f) / 75.0f;
    return 1.0f - (t * 0.32f);
}

std::string ShortFallbackName(const std::string& id) {
    std::string name = id;
    const char* prefixes[] = {
        "Item_Attach_Weapon_",
        "Item_Weapon_",
        "Item_Ammo_",
        "Item_Heal_",
        "Item_Boost_",
        "Item_"
    };

    for (const char* prefix : prefixes) {
        const std::string p = prefix;
        if (name.find(p) == 0) {
            name.erase(0, p.size());
            break;
        }
    }

    if (name.size() > 2 && name.compare(name.size() - 2, 2, "_C") == 0) {
        name.resize(name.size() - 2);
    }
    for (char& ch : name) {
        if (ch == '_') ch = ' ';
    }
    if (name.size() > 22) {
        name.resize(19);
        name += "...";
    }
    return name;
}

void DrawTextShadow(ImDrawList* draw, const ImVec2& pos, ImU32 color, const char* text, float fontSize) {
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 220), text);
    draw->AddText(ImGui::GetFont(), fontSize, pos, color, text);
}

void DrawScaledIcon(ImDrawList* draw, TextureInfo* icon, const ImVec2& center,
                    float targetSize, ImU32 tint = IM_COL32(255, 255, 255, 255)) {
    if (!icon || !icon->SRV || icon->Width <= 0 || icon->Height <= 0) return;

    float width = targetSize;
    float height = targetSize;
    const float aspect = static_cast<float>(icon->Width) / static_cast<float>(icon->Height);
    if (aspect > 1.0f) {
        height = targetSize / aspect;
    } else {
        width = targetSize * aspect;
    }

    draw->AddImage(icon->SRV,
        ImVec2(center.x - width * 0.5f, center.y - height * 0.5f),
        ImVec2(center.x + width * 0.5f, center.y + height * 0.5f),
        ImVec2(0, 0), ImVec2(1, 1), tint);
}

void DrawDistance(ImDrawList* draw, const ImVec2& center, float y, float distance, ImU32 color, float fontSize) {
    char distBuf[32] = {};
    sprintf_s(distBuf, "[%dm]", static_cast<int>(distance));
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, distBuf);
    DrawTextShadow(draw, ImVec2(center.x - textSize.x * 0.5f, y), color, distBuf, fontSize);
}

void DrawSingle(ImDrawList* draw, const LootClusterRenderer::Entry& entry,
                const LootClusterRenderer::Settings& settings) {
    const ImVec2 screen(entry.Screen.x, entry.Screen.y);
    const float baseSize = entry.IconSize > 0.0f ? entry.IconSize :
        (entry.Groupable ? settings.ItemIconSize : settings.SpecialIconSize);
    const float iconSize = baseSize * DistanceScale(entry.Distance);

    if (entry.Icon && entry.Icon->SRV) {
        DrawScaledIcon(draw, entry.Icon, ImVec2(screen.x, screen.y - iconSize * 0.5f), iconSize);
        DrawDistance(draw, screen, screen.y + 2.0f, entry.Distance, entry.Color, settings.DistanceFontSize);
        return;
    }

    const std::string label = ShortFallbackName(entry.Name);
    char itemText[128] = {};
    sprintf_s(itemText, sizeof(itemText), "%s [%dm]", label.c_str(), static_cast<int>(entry.Distance));
    const ImVec2 textSize = ImGui::GetFont()->CalcTextSizeA(settings.DistanceFontSize, FLT_MAX, 0.0f, itemText);
    DrawTextShadow(draw, ImVec2(screen.x - textSize.x * 0.5f, screen.y), entry.Color, itemText, settings.DistanceFontSize);
}

bool IsNearGroup(const LootClusterRenderer::Entry& entry,
                 const std::vector<LootClusterRenderer::Entry>& group) {
    for (const auto& other : group) {
        if (ScreenDistance(entry.Screen, other.Screen) <= kClusterScreenRadius) return true;
    }
    return false;
}

void SortGroup(std::vector<LootClusterRenderer::Entry>& group) {
    std::stable_sort(group.begin(), group.end(), [](const auto& a, const auto& b) {
        if (a.Important != b.Important) return a.Important > b.Important;
        return a.Distance < b.Distance;
    });
}

Vector2 GroupCenter(const std::vector<LootClusterRenderer::Entry>& group) {
    Vector2 center{};
    for (const auto& item : group) {
        center.x += item.Screen.x;
        center.y += item.Screen.y;
    }
    center.x /= static_cast<float>(group.size());
    center.y /= static_cast<float>(group.size());
    return center;
}

float GroupDistance(const std::vector<LootClusterRenderer::Entry>& group) {
    float distance = group.front().Distance;
    for (const auto& item : group) {
        distance = (std::min)(distance, item.Distance);
    }
    return distance;
}

void DrawIconCell(ImDrawList* draw, const LootClusterRenderer::Entry& entry,
                  const ImVec2& center, float iconSize) {
    if (entry.Important) {
        draw->AddCircleFilled(center, iconSize * 0.58f, entry.Color & IM_COL32(255, 255, 255, 90));
    }

    if (entry.Icon && entry.Icon->SRV) {
        DrawScaledIcon(draw, entry.Icon, center, iconSize);
    } else {
        draw->AddCircleFilled(center, iconSize * 0.25f, entry.Color);
    }
}

void DrawIconGroup(ImDrawList* draw, std::vector<LootClusterRenderer::Entry> group,
                   const LootClusterRenderer::Settings& settings) {
    SortGroup(group);

    const int columns = static_cast<int>(group.size()) > kTwoColumnThreshold ? 2 : 1;
    const int rowsPerColumn = (static_cast<int>(group.size()) + columns - 1) / columns;
    const float distance = GroupDistance(group);
    const float iconSize = settings.GroupIconSize * DistanceScale(distance);
    const float cellSize = iconSize + kGroupIconGap;

    char distBuf[32] = {};
    sprintf_s(distBuf, "[%dm]", static_cast<int>(distance));
    const ImVec2 distSize = ImGui::GetFont()->CalcTextSizeA(settings.DistanceFontSize, FLT_MAX, 0.0f, distBuf);
    const float iconAreaWidth = (columns * cellSize) + ((columns - 1) * kColumnGap);
    const float panelWidth = iconAreaWidth + (kPanelPadding * 2.0f);
    const float panelHeight = (rowsPerColumn * cellSize) + distSize.y + kDistanceGap + (kPanelPadding * 2.0f);
    const Vector2 center = GroupCenter(group);

    ImVec2 panelMin(center.x - panelWidth * 0.5f, center.y - panelHeight - 10.0f);
    if (panelMin.y < 8.0f) panelMin.y = center.y + 12.0f;

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    if (display.x > panelWidth && display.y > panelHeight) {
        panelMin.x = ClampFloat(panelMin.x, 8.0f, display.x - panelWidth - 8.0f);
        panelMin.y = ClampFloat(panelMin.y, 8.0f, display.y - panelHeight - 8.0f);
    }

    const ImVec2 panelMax(panelMin.x + panelWidth, panelMin.y + panelHeight);
    draw->AddRectFilled(panelMin, panelMax, IM_COL32(5, 12, 20, 215), 6.0f);
    draw->AddRect(panelMin, panelMax, IM_COL32(0, 190, 255, 110), 6.0f, 0, 1.1f);

    for (int column = 0; column < columns; ++column) {
        const int start = column * rowsPerColumn;
        const int end = (std::min)(start + rowsPerColumn, static_cast<int>(group.size()));
        const float columnX = panelMin.x + kPanelPadding + column * (cellSize + kColumnGap);

        for (int i = start; i < end; ++i) {
            const int row = i - start;
            const ImVec2 iconCenter(columnX + cellSize * 0.5f,
                panelMin.y + kPanelPadding + row * cellSize + cellSize * 0.5f);
            DrawIconCell(draw, group[i], iconCenter, iconSize);
        }
    }

    if (columns == 2) {
        const float separatorX = panelMin.x + kPanelPadding + cellSize + kColumnGap * 0.5f;
        draw->AddLine(ImVec2(separatorX, panelMin.y + 6.0f),
            ImVec2(separatorX, panelMax.y - 6.0f), IM_COL32(0, 190, 255, 45), 1.0f);
    }

    DrawDistance(draw, ImVec2(panelMin.x + panelWidth * 0.5f, 0.0f),
        panelMax.y - kPanelPadding - distSize.y, distance, IM_COL32(220, 232, 245, 230), settings.DistanceFontSize);
}

} // namespace

namespace LootClusterRenderer {

void Draw(ImDrawList* draw, const std::vector<Entry>& entries, const Settings& settings) {
    std::vector<bool> used(entries.size(), false);

    for (size_t i = 0; i < entries.size(); ++i) {
        if (used[i]) continue;
        used[i] = true;

        if (!entries[i].Groupable) {
            DrawSingle(draw, entries[i], settings);
            continue;
        }

        std::vector<Entry> group;
        group.push_back(entries[i]);

        bool expanded = true;
        while (expanded) {
            expanded = false;
            for (size_t j = i + 1; j < entries.size(); ++j) {
                if (used[j] || !entries[j].Groupable) continue;
                if (!IsNearGroup(entries[j], group)) continue;

                used[j] = true;
                group.push_back(entries[j]);
                expanded = true;
            }
        }

        if (group.size() <= 1) {
            DrawSingle(draw, group.front(), settings);
        } else {
            DrawIconGroup(draw, std::move(group), settings);
        }
    }
}

} // namespace LootClusterRenderer
