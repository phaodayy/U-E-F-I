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

// Direct injection without micro-stepping (used for PID/Aimbot)
inline bool MoveDirect(int dx, int dy, unsigned short flags = 0) {
    return telemetryHyperCall::InjectMouseMovement(dx, dy, flags) != 0;
}

// Move mouse by (dx, dy) pixels with button flags. Humanized micro-steps algorithm.
inline bool Move(int dx, int dy, unsigned short flags = 0) {
    // If movement is small or zero, just send it directly
    if (abs(dx) <= 15 && abs(dy) <= 15) {
        return MoveDirect(dx, dy, flags);
    }
    
    // For large movements, split into packets to mimic optical sensor behavior
    int remaining_x = dx;
    int remaining_y = dy;

    while (remaining_x != 0 || remaining_y != 0) {
        long step_x = 0;
        long step_y = 0;

        if (remaining_x != 0) {
            int sign_x = (remaining_x > 0) ? 1 : -1;
            int abs_x = abs(remaining_x);
            int sub_x = (abs_x > 15) ? (rand() % 12 + 4) : abs_x;
            step_x = sub_x * sign_x;
        }

        if (remaining_y != 0) {
            int sign_y = (remaining_y > 0) ? 1 : -1;
            int abs_y = abs(remaining_y);
            int sub_y = (abs_y > 15) ? (rand() % 12 + 4) : abs_y;
            step_y = sub_y * sign_y;
        }
        
        if (!MoveDirect(static_cast<int>(step_x), static_cast<int>(step_y), flags)) {
            return false;
        }
        flags = 0; 
        
        remaining_x -= (int)step_x;
        remaining_y -= (int)step_y;
    }
    return true;
}

inline void Shutdown() {
    // No handles to close - hypervisor layer
}

} // namespace VMouseClient
