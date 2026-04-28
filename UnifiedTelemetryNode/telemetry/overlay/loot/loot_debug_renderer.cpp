#include "loot_debug_renderer.hpp"

#include <protec/skCrypt.h>
#include <cstdio>

namespace {

const char* TypeName(ItemRenderType type) {
    switch (type) {
    case ItemRenderType::Loot: return "Loot";
    case ItemRenderType::Vehicle: return "Vehicle";
    case ItemRenderType::AirDrop: return "AirDrop";
    case ItemRenderType::DeadBox: return "DeadBox";
    case ItemRenderType::Projectile: return "Projectile";
    default: return "Unknown";
    }
}

} // namespace

namespace LootDebugRenderer {

void Draw(ImDrawList* draw, const ItemData& item, const Vector2& screen,
          const std::string& resolvedIcon, bool duplicate, bool selected) {
    if (!draw) return;

    char line1[256] = {};
    char line2[256] = {};
    sprintf_s(line1, skCrypt("%s | %.1fm | %s"), TypeName(item.RenderType),
        item.Distance, selected ? "DRAW" : "SKIP");
    sprintf_s(line2, skCrypt("raw:%s\nicon:%s%s"), item.Name.c_str(),
        resolvedIcon.c_str(), duplicate ? "\nDUPLICATE" : "");

    const ImVec2 pos(screen.x + 8.0f, screen.y + 8.0f);
    const float fontSize = 11.0f;
    const ImVec2 s1 = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, line1);
    const ImVec2 s2 = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, line2);
    const float width = (s1.x > s2.x ? s1.x : s2.x) + 10.0f;
    const float height = s1.y + s2.y + 16.0f;

    const ImU32 bg = duplicate ? IM_COL32(120, 30, 0, 185) : IM_COL32(8, 18, 32, 185);
    const ImU32 border = selected ? IM_COL32(0, 220, 255, 210) : IM_COL32(220, 170, 0, 190);
    draw->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height), bg, 4.0f);
    draw->AddRect(pos, ImVec2(pos.x + width, pos.y + height), border, 4.0f, 0, 1.0f);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 5.0f, pos.y + 4.0f),
        IM_COL32(170, 235, 255, 255), line1);
    draw->AddText(ImGui::GetFont(), fontSize, ImVec2(pos.x + 5.0f, pos.y + s1.y + 8.0f),
        IM_COL32(235, 240, 245, 235), line2);
}

} // namespace LootDebugRenderer
