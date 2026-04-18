#pragma once
#include "memory.hpp"
#include "offsets.hpp"
#include <algorithm>

namespace PubgHealth {
    inline float DecryptHealth(uint64_t pawn) {
        // 1. Read decryption parameters (Kaka-Style)
        uint8_t bEncrypted = PubgMemory::Read<uint8_t>(pawn + PubgOffsets::Health5);   
        uint8_t extraOffset = PubgMemory::Read<uint8_t>(pawn + PubgOffsets::Health3);  
        uint8_t decryptIndex = PubgMemory::Read<uint8_t>(pawn + PubgOffsets::Health6); 

        float healthValue = 0.0f;
        // 2. Read raw health with dynamic offset
        uint64_t finalHealthAddr = pawn + PubgOffsets::Health4 + (bEncrypted != 0 ? extraOffset : 0);
        if (!PubgMemory::ReadMemory(finalHealthAddr, &healthValue, sizeof(float))) return 0.0f;

        // 3. Rolling XOR Byte-by-byte (64-byte seed table)
        if (bEncrypted != 0) {
            uint32_t decryptionTable[16] = {
                PubgOffsets::HealthKey0, PubgOffsets::HealthKey1, PubgOffsets::HealthKey2, PubgOffsets::HealthKey3,
                PubgOffsets::HealthKey4, PubgOffsets::HealthKey5, PubgOffsets::HealthKey6, PubgOffsets::HealthKey7,
                PubgOffsets::HealthKey8, PubgOffsets::HealthKey9, PubgOffsets::HealthKey10, PubgOffsets::HealthKey11,
                PubgOffsets::HealthKey12, PubgOffsets::HealthKey13, PubgOffsets::HealthKey14, PubgOffsets::HealthKey15
            };

            unsigned char* ptr = reinterpret_cast<unsigned char*>(&healthValue);
            unsigned char* keysPtr = reinterpret_cast<unsigned char*>(decryptionTable);
            
            for (unsigned int i = 0; i < 4; ++i) {
                // Kaka health decryption logic
                ptr[i] ^= keysPtr[(i + decryptIndex) & 0x3F];
            }
        }

        // 4. Validate range (PUBG Max Health is usually 100 or 105)
        if (healthValue < 0.0f || healthValue > 125.0f) return 0.0f;
        return healthValue;
    }
}
