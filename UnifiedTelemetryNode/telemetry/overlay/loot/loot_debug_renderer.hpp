#pragma once

#include "../../imgui/imgui.h"
#include "../../sdk/core/context.hpp"
#include <string>

namespace LootDebugRenderer {

void Draw(ImDrawList* draw, const ItemData& item, const Vector2& screen,
          const std::string& resolvedIcon, bool duplicate, bool selected);

} // namespace LootDebugRenderer
