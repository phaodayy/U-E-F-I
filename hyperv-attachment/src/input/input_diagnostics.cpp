#include "input_diagnostics.h"
#include "../crt/crt.h"

namespace input::diagnostics {
    namespace {
        input_diagnostics_snapshot_t g_snapshot = {};

        std::uint64_t backend_value(const BackendKind kind)
        {
            return static_cast<std::uint64_t>(kind);
        }
    }

    void Initialize()
    {
        crt::set_memory(&g_snapshot, 0, sizeof(g_snapshot));
        g_snapshot.version = 1;
        g_snapshot.selected_backend = backend_value(BackendKind::None);
    }

    void Increment(const Counter counter)
    {
        switch (counter) {
        case Counter::SelectorInit: ++g_snapshot.selector_init_count; break;
        case Counter::SelectorActivate: ++g_snapshot.selector_activate_count; break;
        case Counter::BackendSwitch: ++g_snapshot.backend_switch_count; break;
        case Counter::HypercallInject: ++g_snapshot.hypercall_inject_count; break;
        case Counter::HypercallInjectSuccess: ++g_snapshot.hypercall_inject_success_count; break;
        case Counter::HypercallInjectFail: ++g_snapshot.hypercall_inject_fail_count; break;
        case Counter::IoExit: ++g_snapshot.io_exit_count; break;
        case Counter::IoExitHandled: ++g_snapshot.io_exit_handled_count; break;
        case Counter::IoExitForwarded: ++g_snapshot.io_exit_forwarded_count; break;
        case Counter::Ps2StatusRead: ++g_snapshot.ps2_status_read_count; break;
        case Counter::Ps2DataRead: ++g_snapshot.ps2_data_read_count; break;
        case Counter::Ps2StatusWrite: ++g_snapshot.ps2_status_write_count; break;
        case Counter::Ps2DataWrite: ++g_snapshot.ps2_data_write_count; break;
        case Counter::Ps2AckQueued: ++g_snapshot.ps2_ack_queued_count; break;
        case Counter::Ps2MovementPacket: ++g_snapshot.ps2_movement_packet_count; break;
        case Counter::Ps2OutputQueued: ++g_snapshot.ps2_output_queued_count; break;
        case Counter::Ps2OutputPopped: ++g_snapshot.ps2_output_popped_count; break;
        case Counter::Ps2OutputExpired: ++g_snapshot.ps2_output_expired_count; break;
        case Counter::VirtualHidInjectAttempt: ++g_snapshot.virtual_hid_inject_attempt_count; break;
        default: break;
        }
    }

    void SetSelectedBackend(const BackendKind kind)
    {
        const std::uint64_t value = backend_value(kind);
        if (g_snapshot.selected_backend != value) {
            g_snapshot.selected_backend = value;
            Increment(Counter::BackendSwitch);
        }
    }

    void SetAvailableMask(const std::uint64_t mask)
    {
        g_snapshot.available_backend_mask = mask;
    }

    void SetActivatedMask(const std::uint64_t mask)
    {
        g_snapshot.activated_backend_mask = mask;
    }

    void SetLastError(const std::uint64_t code)
    {
        g_snapshot.last_error_code = code;
    }

    input_diagnostics_snapshot_t Snapshot()
    {
        return g_snapshot;
    }
}
