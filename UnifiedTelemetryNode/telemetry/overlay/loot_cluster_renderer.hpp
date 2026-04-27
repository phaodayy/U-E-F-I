#pragma once

#include "overlay_menu.hpp"
#include <string>
#include <vector>

namespace LootClusterRenderer {

struct Entry {
    std::string Name;
    Vector2 Screen;
    float Distance = 0.0f;
    ImU32 Color = 0;
    TextureInfo* Icon = nullptr;
    bool Groupable = true;
    bool Important = false;
};

void Draw(ImDrawList* draw, const std::vector<Entry>& entries);

} // namespace LootClusterRenderer
