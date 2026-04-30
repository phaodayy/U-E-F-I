#include <Windows.h>
#include <cstdlib>
#include <cstdint>
#include <cstdio>

#include "telemetry/sdk/memory/hypercall_bridge.hpp"

namespace {
    const char* BackendName(const std::uint64_t backend)
    {
        switch (backend) {
        case 1: return "Ps2Backend";
        case 2: return "VirtualHidBackend";
        case 3: return "DiagnosticsBackend";
        default: return "None";
        }
    }

    void PrintDiagnostics(const input_diagnostics_snapshot_t& d)
    {
        std::printf("--------------- Diagnostics ---------------\n");
        std::printf("version:                  %llu\n", d.version);
        std::printf("selected_backend:         %s (%llu)\n", BackendName(d.selected_backend), d.selected_backend);
        std::printf("available_backend_mask:   0x%llx\n", d.available_backend_mask);
        std::printf("activated_backend_mask:   0x%llx\n", d.activated_backend_mask);
        std::printf("selector_init:            %llu\n", d.selector_init_count);
        std::printf("selector_activate:        %llu\n", d.selector_activate_count);
        std::printf("backend_switch:           %llu\n", d.backend_switch_count);
        std::printf("inject:                   %llu\n", d.hypercall_inject_count);
        std::printf("inject_success:           %llu\n", d.hypercall_inject_success_count);
        std::printf("inject_fail:              %llu\n", d.hypercall_inject_fail_count);
        std::printf("io_exit:                  %llu\n", d.io_exit_count);
        std::printf("io_exit_handled:          %llu\n", d.io_exit_handled_count);
        std::printf("io_exit_forwarded:        %llu\n", d.io_exit_forwarded_count);
        std::printf("ps2_status_read:          %llu\n", d.ps2_status_read_count);
        std::printf("ps2_data_read:            %llu\n", d.ps2_data_read_count);
        std::printf("ps2_status_write:         %llu\n", d.ps2_status_write_count);
        std::printf("ps2_data_write:           %llu\n", d.ps2_data_write_count);
        std::printf("ps2_ack_queued:           %llu\n", d.ps2_ack_queued_count);
        std::printf("ps2_movement_packet:      %llu\n", d.ps2_movement_packet_count);
        std::printf("ps2_output_queued:        %llu\n", d.ps2_output_queued_count);
        std::printf("ps2_output_popped:        %llu\n", d.ps2_output_popped_count);
        std::printf("ps2_output_expired:       %llu\n", d.ps2_output_expired_count);
        std::printf("virtual_hid_attempt:      %llu\n", d.virtual_hid_inject_attempt_count);
        std::printf("last_error_code:          0x%llx\n", d.last_error_code);
        std::printf("-------------------------------------------\n");
    }

    void QueryAndPrintDiagnostics()
    {
        input_diagnostics_snapshot_t diagnostics = {};
        if (!telemetryHyperCall::GetInputDiagnostics(&diagnostics)) {
            std::printf("[!] GetInputDiagnostics(0x280) failed.\n");
            return;
        }

        PrintDiagnostics(diagnostics);
    }
}

int main()
{
    std::printf("========================================\n");
    std::printf("   Hypervisor Mouse Backend Test v3.0\n");
    std::printf("========================================\n");

    if (!telemetryHyperCall::Init()) {
        std::printf("[!] Error: Hypervisor bridge init failed.\n");
        std::system("pause");
        return 1;
    }

    const std::uint64_t version = telemetryHyperCall::MakeHypercall(hypercall_type_t::_hc_0x100, 0, 0, 0, 0);
    if (version != 0x2323) {
        std::printf("[WARNING] Hypervisor version mismatch.\n");
        std::printf("[WARNING] Expected: 0x2323, Got: 0x%llx\n", version);
        std::printf("[TIP] Restart the PC to clear the old hypervisor image.\n");
        std::system("pause");
        return 1;
    }

    std::printf("[SUCCESS] Hypervisor v2.3 detected.\n");
    std::printf("[*] Current input backend state before movement:\n");
    QueryAndPrintDiagnostics();

    std::printf("[*] Sending movement through backend selector (0x220)...\n");
    for (int i = 0; i < 12; ++i) {
        const std::uint64_t ok = telemetryHyperCall::InjectMouseMovement(18, 0, 0);
        std::printf("    step %02d: %s\n", i + 1, ok ? "accepted" : "rejected");
        Sleep(20);
    }

    Sleep(250);
    std::printf("[*] Backend state after movement:\n");
    QueryAndPrintDiagnostics();

    std::printf("[NOTE] This test does not use mouclass hook or hypercall 0x230.\n");
    std::system("pause");
    return 0;
}
