#pragma once

namespace OverlayHotkeys {

bool ConsumePressed(int vk, bool& wasDown);
void DrawKeyBind(const char* label, int* key, int*& waitingForKey);

} // namespace OverlayHotkeys
