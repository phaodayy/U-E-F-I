#pragma once

#include "../core/overlay_menu.hpp"
#include <string>

namespace LootIconResolver {
TextureInfo* GetItemIcon(const std::string& itemName);
}
