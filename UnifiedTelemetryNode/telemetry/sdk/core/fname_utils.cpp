#include "fname.hpp"
using namespace FNames;
#include "context.hpp"
#include "offsets.hpp"
#include "telemetry_decrypt.hpp"
#include <iostream>

namespace FNameUtils {

static uint32_t g_FNameKey = 0;

uint32_t GetKey()
{
    // No longer required in modern telemetry, names are not XOR encrypted.
    return 1;
}

static uint64_t CachedPoolBase = 0;

std::string GetNameFast(int32_t index)
{
    if (index < 0) return "";

    if (!CachedPoolBase) {
        uint64_t base = telemetryContext::GetBaseAddress();
        if (!base) return "";

        uint64_t tmp = 0;
        if (!telemetryContext::ReadMemory(base + telemetryOffsets::GNames, &tmp, sizeof(uint64_t))) return "";
        
        uint64_t v11_raw = telemetryDecrypt::Xe(tmp);
        uint64_t v11 = 0;
        if (!telemetryContext::ReadMemory(v11_raw + telemetryOffsets::GNamesPtr, &v11, sizeof(uint64_t))) return "";
        
        CachedPoolBase = telemetryDecrypt::Xe(v11);
    }
    
    if (!CachedPoolBase) return "";

    // Modern telemetry GNames formula (ChunkSize = 0x3E4C by default, check offsets.hpp)
    uint64_t fNamePtr = telemetryContext::Read<uint64_t>(CachedPoolBase + ((static_cast<int>(index / telemetryOffsets::ChunkSize)) * 8));
    if (!fNamePtr) return "";

    uint64_t fName = telemetryContext::Read<uint64_t>(fNamePtr + ((static_cast<int>(index % telemetryOffsets::ChunkSize)) * 8));
    if (!fName) return "";

    char names_c[64] = { 0 };
    if (!telemetryContext::ReadMemory(fName + 0x10, names_c, sizeof(names_c) - 1)) return "";

    std::string name(names_c);
    return name;
}

bool IsA(uint64_t actor, const std::string &class_name)
{
    if (!actor || class_name.empty())
        return false;

    uint64_t current = telemetryContext::Read<uint64_t>(actor + 0x10);
    while (current) {
        int32_t fnameIndex = telemetryContext::Read<int32_t>(current + 0x18);
        std::string name = GetNameFast(fnameIndex);
        if (name.find(class_name) != std::string::npos)
            return true;
        current = telemetryContext::Read<uint64_t>(current + 0x48);
    }
    return false;
}

} 
