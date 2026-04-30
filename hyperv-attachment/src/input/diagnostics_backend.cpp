#include "diagnostics_backend.h"
#include "input_diagnostics.h"

namespace input {
    BackendKind DiagnosticsBackend::kind() const
    {
        return BackendKind::Diagnostics;
    }

    const char* DiagnosticsBackend::name() const
    {
        return "diagnostics";
    }

    void DiagnosticsBackend::initialize()
    {
    }

    bool DiagnosticsBackend::is_available() const
    {
        return true;
    }

    bool DiagnosticsBackend::activate()
    {
        return true;
    }

    bool DiagnosticsBackend::inject_mouse(const MouseMovement& movement)
    {
        (void)movement;
        diagnostics::SetLastError(0x44494147);
        return false;
    }

    bool DiagnosticsBackend::handle_io(IoOperation& operation)
    {
        (void)operation;
        return false;
    }
}
