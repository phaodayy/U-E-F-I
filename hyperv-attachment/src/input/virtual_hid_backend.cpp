#include "virtual_hid_backend.h"
#include "input_diagnostics.h"

namespace input {
    BackendKind VirtualHidBackend::kind() const
    {
        return BackendKind::VirtualHid;
    }

    const char* VirtualHidBackend::name() const
    {
        return "virtual-hid";
    }

    void VirtualHidBackend::initialize()
    {
    }

    bool VirtualHidBackend::is_available() const
    {
        return false;
    }

    bool VirtualHidBackend::activate()
    {
        diagnostics::SetLastError(0x48494401);
        return false;
    }

    bool VirtualHidBackend::inject_mouse(const MouseMovement& movement)
    {
        (void)movement;
        diagnostics::Increment(diagnostics::Counter::VirtualHidInjectAttempt);
        diagnostics::SetLastError(0x48494402);
        return false;
    }

    bool VirtualHidBackend::handle_io(IoOperation& operation)
    {
        (void)operation;
        return false;
    }
}
