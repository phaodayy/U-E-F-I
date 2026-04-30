#include "ps2_backend.h"
#include "input_diagnostics.h"
#include "../arch/mouse_ps2.h"

namespace input {
    BackendKind Ps2Backend::kind() const
    {
        return BackendKind::Ps2;
    }

    const char* Ps2Backend::name() const
    {
        return "ps2";
    }

    void Ps2Backend::initialize()
    {
        arch::mouse_ps2::Initialize();
        initialized_ = true;
        active_ = false;
    }

    bool Ps2Backend::is_available() const
    {
        return initialized_;
    }

    bool Ps2Backend::activate()
    {
        if (!initialized_) {
            initialize();
        }

        active_ = true;
        return true;
    }

    bool Ps2Backend::inject_mouse(const MouseMovement& movement)
    {
        if (!active_) {
            diagnostics::SetLastError(0x50533201);
            return false;
        }

        return arch::mouse_ps2::InjectMovementViaController(movement.dx, movement.dy, movement.buttons);
    }

    bool Ps2Backend::handle_io(IoOperation& operation)
    {
        if (!active_ || !operation.is_byte_access || operation.is_string || operation.is_rep) {
            return false;
        }

        if (operation.port == arch::mouse_ps2::PS2_DATA_PORT) {
            if (operation.is_in) {
                if (!arch::mouse_ps2::HasPendingOutput()) {
                    return false;
                }

                operation.result_al = arch::mouse_ps2::HandleReadData();
                diagnostics::Increment(diagnostics::Counter::Ps2DataRead);
                return true;
            }

            bool handled = arch::mouse_ps2::HandleWriteData(operation.al);
            if (handled) diagnostics::Increment(diagnostics::Counter::Ps2DataWrite);
            return handled;
        }

        if (operation.port == arch::mouse_ps2::PS2_STATUS_PORT) {
            if (operation.is_in) {
                if (!arch::mouse_ps2::HasPendingOutput()) {
                    return false;
                }

                operation.result_al = arch::mouse_ps2::HandleReadStatus();
                diagnostics::Increment(diagnostics::Counter::Ps2StatusRead);
                return true;
            }

            bool handled = arch::mouse_ps2::HandleWriteCommand(operation.al);
            if (handled) diagnostics::Increment(diagnostics::Counter::Ps2StatusWrite);
            return handled;
        }

        return false;
    }
}
