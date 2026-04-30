#pragma once

#include <Windows.h>

class SendInputMacro {
public:
    static bool Move(long x, long y) {
        if (x == 0 && y == 0) return true;

        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = x;
        input.mi.dy = y;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        return SendInput(1, &input, sizeof(INPUT)) == 1;
    }

    static bool Down() {
        return Button(MOUSEEVENTF_LEFTDOWN);
    }

    static bool Up() {
        return Button(MOUSEEVENTF_LEFTUP);
    }

    static bool Click() {
        const bool downOk = Down();
        const bool upOk = Up();
        return downOk && upOk;
    }

private:
    static bool Button(DWORD flag) {
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = flag;
        return SendInput(1, &input, sizeof(INPUT)) == 1;
    }
};
