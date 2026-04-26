#pragma once
// ============================================================
// Ring -1 Hypervisor Mouse Client
// evasion_calibrationes all Hook/HID/UserMode level checks
// Communicates with Hyper-reV via Hypercall (Ring -1)
// ============================================================

#include "hypercall_bridge.hpp"
#include <Windows.h>

namespace VMouseClient {

inline bool Initialize() {
    // Hypercall context is initialized by telemetryHyperCall::Init()
    return true;
}

// Move mouse by (dx, dy) pixels with button flags. Humanized micro-steps algorithm.
inline bool Move(int dx, int dy, unsigned short flags = 0) {
    // If movement is zero, just send the flags (click)
    if (dx == 0 && dy == 0) {
        telemetryHyperCall::InjectMouseMovement(0, 0, flags);
        return true;
    }

    // Dựa theo phân tích Heuristics V4: Split thành các packet ngẫu nhiên, không để max 127
    while (dx != 0 || dy != 0) {
        long step_x = 0;
        long step_y = 0;

        // Mô phỏng cảm biến quang học chuột: Tách thành các bước nhỏ ngẫu nhiên từ 3-15 pixels.
        if (dx != 0) {
            int sign_x = (dx > 0) ? 1 : -1;
            int abs_x = abs(dx);
            int sub_x = (abs_x > 15) ? (rand() % 12 + 4) : abs_x;
            step_x = sub_x * sign_x;
        }

        if (dy != 0) {
            int sign_y = (dy > 0) ? 1 : -1;
            int abs_y = abs(dy);
            int sub_y = (abs_y > 15) ? (rand() % 12 + 4) : abs_y;
            step_y = sub_y * sign_y;
        }
        
        // Gửi trực tiếp xuống Ring -1 (Evasion CP)
        // We only send flags in the FIRST packet of a movement chain or alone.
        telemetryHyperCall::InjectMouseMovement(step_x, step_y, flags);
        flags = 0; // Clear flags for subsequent steps
        
        dx -= step_x;
        dy -= step_y;
    }
    return true;
}

inline void Shutdown() {
    // No handles to close - hypervisor layer
}

} // namespace VMouseClient
