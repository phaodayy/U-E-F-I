#pragma once

#include <Windows.h>

class SendInputMacro {
public:
    static DWORD LastErrorCode() {
        return last_error;
    }

    static bool Move(long x, long y) {
        if (x == 0 && y == 0) return true;

        last_error = 0;
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dx = x;
        input.mi.dy = y;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        const bool ok = SendInput(1, &input, sizeof(INPUT)) == 1;
        if (!ok) last_error = GetLastError();
        return ok;
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
    static inline DWORD last_error = 0;

    static bool Button(DWORD flag) {
        last_error = 0;
        INPUT input{};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = flag;
        const bool ok = SendInput(1, &input, sizeof(INPUT)) == 1;
        if (!ok) last_error = GetLastError();
        return ok;
    }
};
