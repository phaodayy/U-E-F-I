#pragma once

#include "../core/overlay_menu.hpp"
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
    float IconSize = 0.0f;
};

struct Settings {
    float ItemIconSize = 24.0f;
    float SpecialIconSize = 34.0f;
    float GroupIconSize = 18.0f;
    float DistanceFontSize = 12.0f;
};

void Draw(ImDrawList* draw, const std::vector<Entry>& entries, const Settings& settings = Settings{});

} // namespace LootClusterRenderer
