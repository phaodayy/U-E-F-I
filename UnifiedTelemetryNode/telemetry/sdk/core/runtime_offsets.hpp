#pragma once

#include <cstdint>

namespace telemetryRuntimeOffsets {
    struct RuntimeScanReport {
        bool Scanned = false;
        bool AppliedRuntimeOffsets = false;
        uint32_t RequiredTotal = 0;
        uint32_t RequiredFound = 0;
        uint32_t Found = 0;
        uint32_t Applied = 0;
    };

    bool ApplyRuntimeScan(uint64_t baseAddress);
    void InvalidateRuntimeScanCache(uint64_t baseAddress = 0);
    const RuntimeScanReport& GetLastReport();
}
