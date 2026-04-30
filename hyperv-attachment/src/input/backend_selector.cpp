#include "backend_selector.h"
#include "diagnostics_backend.h"
#include "input_diagnostics.h"
#include "ps2_backend.h"
#include "virtual_hid_backend.h"

namespace input::backend_selector {
    namespace {
        Ps2Backend g_ps2_backend;
        VirtualHidBackend g_virtual_hid_backend;
        DiagnosticsBackend g_diagnostics_backend;
        InputBackend* g_selected_backend = nullptr;
        bool g_initialized = false;
        bool g_activated = false;

        std::uint64_t available_mask()
        {
            std::uint64_t mask = BackendMaskDiagnostics;
            if (g_ps2_backend.is_available()) {
                mask |= BackendMaskPs2;
            }
            if (g_virtual_hid_backend.is_available()) {
                mask |= BackendMaskVirtualHid;
            }
            return mask;
        }

        std::uint64_t activated_mask()
        {
            std::uint64_t mask = BackendMaskDiagnostics;
            if (g_activated && g_selected_backend != nullptr) {
                if (g_selected_backend->kind() == BackendKind::Ps2) {
                    mask |= BackendMaskPs2;
                } else if (g_selected_backend->kind() == BackendKind::VirtualHid) {
                    mask |= BackendMaskVirtualHid;
                }
            }
            return mask;
        }

        InputBackend* choose_backend()
        {
            if (g_virtual_hid_backend.is_available()) {
                return &g_virtual_hid_backend;
            }

            if (g_ps2_backend.is_available()) {
                return &g_ps2_backend;
            }

            return &g_diagnostics_backend;
        }

        void publish_state()
        {
            diagnostics::SetAvailableMask(available_mask());
            diagnostics::SetActivatedMask(activated_mask());
            diagnostics::SetSelectedBackend(g_selected_backend ? g_selected_backend->kind() : BackendKind::None);
        }
    }

    void Initialize()
    {
        diagnostics::Initialize();
        diagnostics::Increment(diagnostics::Counter::SelectorInit);

        g_ps2_backend.initialize();
        g_virtual_hid_backend.initialize();
        g_diagnostics_backend.initialize();
        g_selected_backend = choose_backend();
        g_initialized = true;
        g_activated = false;
        publish_state();
    }

    bool Activate()
    {
        if (!g_initialized) {
            Initialize();
        }

        diagnostics::Increment(diagnostics::Counter::SelectorActivate);
        g_selected_backend = choose_backend();

        if (g_selected_backend == nullptr || !g_selected_backend->activate()) {
            g_selected_backend = &g_diagnostics_backend;
            g_activated = false;
            publish_state();
            return false;
        }

        g_activated = true;
        publish_state();
        return true;
    }

    bool InjectMouse(const std::int32_t dx, const std::int32_t dy, const std::uint8_t buttons)
    {
        diagnostics::Increment(diagnostics::Counter::HypercallInject);

        if (!g_initialized) {
            Initialize();
        }

        if (!g_activated && !Activate()) {
            diagnostics::Increment(diagnostics::Counter::HypercallInjectFail);
            return false;
        }

        const MouseMovement movement = { dx, dy, buttons };
        const bool ok = g_selected_backend != nullptr && g_selected_backend->inject_mouse(movement);
        diagnostics::Increment(ok ? diagnostics::Counter::HypercallInjectSuccess : diagnostics::Counter::HypercallInjectFail);
        publish_state();
        return ok;
    }

    bool HandleIo(IoOperation& operation)
    {
        diagnostics::Increment(diagnostics::Counter::IoExit);

        if (!g_initialized || !g_activated || g_selected_backend == nullptr) {
            diagnostics::Increment(diagnostics::Counter::IoExitForwarded);
            return false;
        }

        const bool handled = g_selected_backend->handle_io(operation);
        diagnostics::Increment(handled ? diagnostics::Counter::IoExitHandled : diagnostics::Counter::IoExitForwarded);
        return handled;
    }

    BackendKind SelectedKind()
    {
        return g_selected_backend ? g_selected_backend->kind() : BackendKind::None;
    }

    input_diagnostics_snapshot_t Diagnostics()
    {
        publish_state();
        return diagnostics::Snapshot();
    }
}
