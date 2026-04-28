#pragma once

class OverlayMenu;

namespace OverlayPresets {

enum class Preset {
    Clean,
    Loot,
    Combat,
    Debug
};

void Apply(OverlayMenu& menu, Preset preset);
const char* Name(Preset preset);

} // namespace OverlayPresets
