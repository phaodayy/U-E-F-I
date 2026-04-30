#pragma once

#include <cstdint>
#include <hypercall/hypercall_def.h>
#include "input_backend.h"

namespace input::diagnostics {
    enum class Counter : std::uint8_t {
        SelectorInit,
        SelectorActivate,
        BackendSwitch,
        HypercallInject,
        HypercallInjectSuccess,
        HypercallInjectFail,
        IoExit,
        IoExitHandled,
        IoExitForwarded,
        Ps2StatusRead,
        Ps2DataRead,
        Ps2StatusWrite,
        Ps2DataWrite,
        Ps2AckQueued,
        Ps2MovementPacket,
        Ps2OutputQueued,
        Ps2OutputPopped,
        Ps2OutputExpired,
        VirtualHidInjectAttempt
    };

    void Initialize();
    void Increment(Counter counter);
    void SetSelectedBackend(BackendKind kind);
    void SetAvailableMask(std::uint64_t mask);
    void SetActivatedMask(std::uint64_t mask);
    void SetLastError(std::uint64_t code);
    input_diagnostics_snapshot_t Snapshot();
}
