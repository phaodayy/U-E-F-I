#include "overlay_hotkeys.hpp"

#include "../../imgui/imgui.h"
#include "../../sdk/memory/memory.hpp"
#include <protec/skCrypt.h>
#include <string>
#include <windows.h>

namespace {

std::string KeyName(int vk) {
    if (vk <= 0) return skCrypt("NONE");
    if (vk == VK_LBUTTON) return skCrypt("MOUSE LEFT");
    if (vk == VK_RBUTTON) return skCrypt("MOUSE RIGHT");
    if (vk == VK_MBUTTON) return skCrypt("MOUSE MIDDLE");
    if (vk == VK_XBUTTON1) return skCrypt("MOUSE X1");
    if (vk == VK_XBUTTON2) return skCrypt("MOUSE X2");
    if (vk >= VK_F1 && vk <= VK_F24) return std::string(skCrypt("F")) + std::to_string(vk - VK_F1 + 1);
    if (vk >= '0' && vk <= '9') return std::string(1, static_cast<char>(vk));
    if (vk >= 'A' && vk <= 'Z') return std::string(1, static_cast<char>(vk));

    char keyName[64] = {};
    const LONG scan = static_cast<LONG>(MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) << 16);
    if (scan != 0 && GetKeyNameTextA(scan, keyName, sizeof(keyName)) > 0) {
        return keyName;
    }

    return std::string(skCrypt("KEY ")) + std::to_string(vk);
}

} // namespace

namespace OverlayHotkeys {

bool ConsumePressed(int vk, bool& wasDown) {
    if (vk <= 0 || vk > 0xFE) {
        wasDown = false;
        return false;
    }

    const bool down = telemetryMemory::IsKeyDown(vk);
    const bool pressed = down && !wasDown;
    wasDown = down;
    return pressed;
}

void DrawKeyBind(const char* label, int* key, int*& waitingForKey) {
    if (!key) return;

    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine();

    const bool waiting = waitingForKey == key;
    const std::string buttonText = waiting ? std::string(skCrypt("Press key...")) : KeyName(*key);
    if (ImGui::Button(buttonText.c_str(), ImVec2(118.0f, 26.0f))) {
        waitingForKey = key;
    }

    ImGui::SameLine();
    if (ImGui::SmallButton(skCrypt("Clear"))) {
        if (waitingForKey == key) waitingForKey = nullptr;
        *key = 0;
    }

    ImGui::PopID();
}

} // namespace OverlayHotkeys
