#pragma once
#include <windows.h>

// [SECURITY WARNING] UsermodeMouse using SendInput is DISABLED.
// BattlEye flags SendInput usage for mouse simulation.
// Use PubgMemory::MoveMouse (Hypervisor Ring -1) instead.

class UsermodeMouse {
public:
    static void Move(int x, int y) {
        // DISABLED for security. Use PubgMemory::MoveMouse.
        /*
        if (x == 0 && y == 0) return;
        INPUT inp = { 0 };
        inp.type = INPUT_MOUSE;
        inp.mi.dx = x;
        inp.mi.dy = y;
        inp.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &inp, sizeof(INPUT));
        */
    }

    static void Click() {
        // DISABLED. Clicks via SendInput are detectable.
    }

    static void PressTheLeft() {
        // DISABLED.
    }

    static void PopUpTheLeft() {
        // DISABLED.
    }
};
