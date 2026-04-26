#pragma once

#include "overlay_menu.hpp"

#include <cstdlib>
#include <cstdint>
#include <windows.h>
#include "../sdk/hyper_process.hpp"
#include "../../protec/skCrypt.h"

// Legacy filename kept for project compatibility. This adapter only consumes
// bridge hosts that are explicitly registered by a trusted local bridge.
namespace VisualizationBridgeDiscovery {

constexpr uint32_t kRegistryMagic = 0x42565455; // UTVB
constexpr uint32_t kRegistryVersion = 1;
constexpr DWORD kFlagClearBeforeRender = 1 << 0;
constexpr DWORD kFlagPresentAfterRender = 1 << 1;

struct RegistryBlock {
    uint32_t magic = 0;
    uint32_t version = 0;
    uint64_t hwnd = 0;
    uint32_t owner_pid = 0;
    uint32_t flags = 0;
};

inline bool AssignWindow(VisualizationBridgeHost& bridge, uint64_t raw_hwnd, uint32_t expected_owner_pid = 0) {
    if (raw_hwnd == 0) return false;

    HWND hwnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(raw_hwnd));
    if (!IsWindow(hwnd)) return false;

    DWORD owner_pid = 0;
    GetWindowThreadProcessId(hwnd, &owner_pid);
    if (expected_owner_pid != 0 && owner_pid != expected_owner_pid) return false;

    bridge.hwnd = hwnd;
    return true;
}

inline VisualizationBridgeHost FromEnvironment() {
    VisualizationBridgeHost bridge = {};
    char value[64] = {};
    DWORD len = GetEnvironmentVariableA("UTN_VISUALIZATION_HWND", value, static_cast<DWORD>(sizeof(value)));
    if (len == 0 || len >= sizeof(value)) return bridge;

    char* end = nullptr;
    unsigned long long raw = std::strtoull(value, &end, 0);
    if (end == value || raw == 0) return bridge;

    AssignWindow(bridge, raw);
    return bridge;
}

inline VisualizationBridgeHost FromSharedRegistry() {
    VisualizationBridgeHost bridge = {};
    HANDLE mapping = OpenFileMappingA(FILE_MAP_READ, FALSE, "Local\\UTNVisualizationBridge");
    if (!mapping) return bridge;

    auto* mapped = static_cast<const RegistryBlock*>(
        MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, sizeof(RegistryBlock)));
    if (mapped) {
        RegistryBlock block = *mapped;
        UnmapViewOfFile(mapped);

        if (block.magic == kRegistryMagic &&
            block.version == kRegistryVersion &&
            AssignWindow(bridge, block.hwnd, block.owner_pid)) {
            bridge.clear_before_render = (block.flags & kFlagClearBeforeRender) != 0;
            bridge.present_after_render = (block.flags & kFlagPresentAfterRender) != 0;
        }
    }

    CloseHandle(mapping);
    return bridge;
}

inline VisualizationBridgeHost FromDiscordFallback() {
    VisualizationBridgeHost bridge = {};
    
    // Yêu cầu bắt buộc: Súc sạo hoàn toàn từ luồng Hypervisor (Bỏ FindWindowA)
    std::cout << skCrypt("[*] Waiting for Bridge Link (Discord via Hypervisor GhostWalk)...\n");
    std::cout << skCrypt("[!] Make sure Discord overlay is ENABLED and PUBG is in BORDERLESS mode.\n");

    HWND overlay = nullptr;
    while (!overlay) {
        // [HYPERVISOR] Bước 1: Quét EPROCESS từ không gian nhân (Kernel) để lấy PIDs của Discord
        std::vector<uint32_t> discord_pids = telemetryHyperProcess::FindAllPidsByGhostWalk(skCrypt("Discord"));
        
        if (!discord_pids.empty()) {
            // [USERMODE-SAFE] Bước 2: Liệt kê HWND thô và khớp với PID an toàn đã tìm từ hypervisor
            HWND hwnd = GetTopWindow(NULL);
            while (hwnd) {
                DWORD pid = 0;
                GetWindowThreadProcessId(hwnd, &pid);
                
                bool is_discord = false;
                for (uint32_t d_pid : discord_pids) {
                    if (pid == d_pid) {
                        is_discord = true;
                        break;
                    }
                }
                
                if (is_discord) {
                    char className[256];
                    GetClassNameA(hwnd, className, sizeof(className));
                    if (strcmp(className, skCrypt("Chrome_WidgetWin_1")) == 0) {
                        // Tìm thấy đúng cửa sổ Overlay của Discord!
                        overlay = hwnd;
                        break;
                    }
                }
                hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
            }
        }

        if (!overlay) {
            Sleep(1000);
        }
    }
    
    std::cout << skCrypt("[+] Found passive host HWND via Hypervisor PID match.\n");
    bridge.hwnd = overlay;
    bridge.clear_before_render = false;
    bridge.present_after_render = true;
    
    return bridge;
}

inline VisualizationBridgeHost ResolveHost() {
    VisualizationBridgeHost bridge = FromEnvironment();
    if (bridge.hwnd) return bridge;

    bridge = FromSharedRegistry();
    if (bridge.hwnd) return bridge;

    return FromDiscordFallback();
}

} // namespace VisualizationBridgeDiscovery
