#pragma once

#include "input_backend.h"
#include <hypercall/hypercall_def.h>

namespace input::backend_selector {
    void Initialize();
    bool Activate();
    bool InjectMouse(std::int32_t dx, std::int32_t dy, std::uint8_t buttons);
    bool HandleIo(IoOperation& operation);
    BackendKind SelectedKind();
    input_diagnostics_snapshot_t Diagnostics();
}
