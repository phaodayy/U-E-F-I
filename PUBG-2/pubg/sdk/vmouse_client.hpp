#pragma once
// ============================================================
// Ring -1 Hypervisor Mouse Client
// Bypasses all Hook/HID/UserMode level checks
// Communicates with Hyper-reV via Hypercall (Ring -1)
// ============================================================

#include "hypercall_bridge.hpp"
#include <Windows.h>

namespace VMouseClient {

inline bool Initialize() {
    // Hypercall context is initialized by PubgHyperCall::Init()
    return true;
}

// Move mouse by (dx, dy) pixels. Chunks into -127..127 steps automatically.
inline bool Move(int dx, int dy) {
    bool ok = true;
    while (dx != 0 || dy != 0) {
        long step_x = max(-127, min(127, dx));
        long step_y = max(-127, min(127, dy));
        
        // Pass X,Y to Ring-1 Hypervisor directly
        PubgHyperCall::InjectMouseMovement(step_x, step_y);
        
        dx -= step_x;
        dy -= step_y;
    }
    return ok;
}

inline void Shutdown() {
    // No handles to close - hypervisor layer
}

} // namespace VMouseClient
