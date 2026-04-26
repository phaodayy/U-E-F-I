#pragma once
#include "memory.hpp"
#include "offsets.hpp"
#include <algorithm>

namespace telemetryHealth {
    inline float DecryptHealth(uint64_t pawn) {
        // 1. Read decryption parameters (Kaka-Style)
        uint8_t bEncrypted = telemetryMemory::Read<uint8_t>(pawn + telemetryOffsets::Health5);   
        uint8_t extraOffset = telemetryMemory::Read<uint8_t>(pawn + telemetryOffsets::Health3);  
        uint8_t decryptIndex = telemetryMemory::Read<uint8_t>(pawn + telemetryOffsets::Health6); 

        float healthValue = 0.0f;
        // 2. Read raw health with dynamic offset
        uint64_t finalHealthAddr = pawn + telemetryOffsets::Health4 + (bEncrypted != 0 ? extraOffset : 0);
        if (!telemetryMemory::ReadMemory(finalHealthAddr, &healthValue, sizeof(float))) return 0.0f;

        // 3. Rolling XOR Byte-by-byte (64-byte seed table)
        if (bEncrypted != 0) {
            uint32_t decryptionTable[16] = {
                telemetryOffsets::HealthKey0, telemetryOffsets::HealthKey1, telemetryOffsets::HealthKey2, telemetryOffsets::HealthKey3,
                telemetryOffsets::HealthKey4, telemetryOffsets::HealthKey5, telemetryOffsets::HealthKey6, telemetryOffsets::HealthKey7,
                telemetryOffsets::HealthKey8, telemetryOffsets::HealthKey9, telemetryOffsets::HealthKey10, telemetryOffsets::HealthKey11,
                telemetryOffsets::HealthKey12, telemetryOffsets::HealthKey13, telemetryOffsets::HealthKey14, telemetryOffsets::HealthKey15
            };

            unsigned char* ptr = reinterpret_cast<unsigned char*>(&healthValue);
            unsigned char* keysPtr = reinterpret_cast<unsigned char*>(decryptionTable);
            
            for (unsigned int i = 0; i < 4; ++i) {
                // Kaka health decryption logic
                ptr[i] ^= keysPtr[(i + decryptIndex) & 0x3F];
            }
        }

        // 4. Validate range (telemetry Max Health is usually 100 or 105)
        if (healthValue < 0.0f || healthValue > 125.0f) return 0.0f;
        return healthValue;
    }
}
