#pragma once

#include "../core/overlay_menu.hpp"
#include <vector>

namespace RadarOverlayRenderer {

void Draw(ImDrawList* draw, OverlayMenu& menu, const std::vector<PlayerData>& players);

} // namespace RadarOverlayRenderer
