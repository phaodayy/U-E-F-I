#pragma once
#include <windows.h>

class UsermodeMouse {
public:
    static void Move(int x, int y) {
        if (x == 0 && y == 0) return;
        INPUT inp = { 0 };
        inp.type = INPUT_MOUSE;
        inp.mi.dx = x;
        inp.mi.dy = y;
        inp.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &inp, sizeof(INPUT));
    }

    static void Click() {
        INPUT inp[2] = { 0 };
        inp[0].type = INPUT_MOUSE;
        inp[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        inp[1].type = INPUT_MOUSE;
        inp[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(2, inp, sizeof(INPUT));
    }
    
    static void PressTheLeft() {
        INPUT inp = { 0 };
        inp.type = INPUT_MOUSE;
        inp.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &inp, sizeof(INPUT));
    }
    
    static void PopUpTheLeft() {
        INPUT inp = { 0 };
        inp.type = INPUT_MOUSE;
        inp.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &inp, sizeof(INPUT));
    }
};
