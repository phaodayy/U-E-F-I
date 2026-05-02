#include "fname.hpp"
#include "context.hpp"
#include "offsets.hpp"
#include <iostream>
#include <intrin.h>
#include <map>

namespace FNameUtils {

static uint64_t g_FNameState[7] = {0};
static bool g_StateInitialized = false;

uint64_t decrypt_xor_keys(const uint32_t key, const uint64_t* state)
{
    const uint64_t hash = 0x2545F4914F6CDD1DULL * (key ^ ((key ^ (key >> 15)) >> 12) ^ (key << 25));
    const uint64_t idx = hash % 7;
    uint64_t val = state[idx];
    const uint32_t hi = (uint32_t)(hash >> 32);
    const uint32_t m7 = (uint32_t)idx % 7;

    auto lfsr1 = [](uint64_t x) -> uint64_t {
        return (x >> 1) ^ (((x >> 1) ^ (2 * x)) & 0xAAAAAAAAAAAAAAAAULL);
    };

    auto rotr63 = [](uint64_t x, uint32_t r) -> uint64_t {
        uint8_t s = (uint8_t)(r % 0x3F) + 1;
        return (x >> s) | (x << (64 - s));
    };

    auto rotl63 = [](uint64_t x, uint32_t r) -> uint64_t {
        uint8_t s = (uint8_t)(r % 0x3F) + 1;
        return (x << s) | (x >> (64 - s));
    };

    if (m7 == 0) {
        val = lfsr1(val + hi - 1);
    }
    else if (m7 == 1 || m7 == 3) {
    }
    else if (m7 == 2) {
        val ^= (hi + idx);
    }
    else if (m7 == 4) {
        val = rotl63(~val, hi + (uint32_t)idx);
    }
    else if (m7 == 5) {
        val = rotr63(val + hi + 2 * idx, hi + (uint32_t)idx);
    }
    else if (m7 == 6) {
        val = lfsr1(rotr63(val, hi + 2 * (uint32_t)idx));
    }

    return val ^ key;
}

std::string GetNameFast(int32_t index)
{
    if (index < 0)
        return {};

    uint64_t base = VDataContext::GetBaseAddress();
    if (!base)
        return {};

    if (!g_StateInitialized) {
        uint64_t stateAddr = base + VDataOffsets::FNameState() + VDataOffsets::FNameKey();
        if (VDataContext::ReadMemory(stateAddr, g_FNameState, sizeof(g_FNameState))) {
            g_StateInitialized = true;
        } else {
            return {};
        }
    }

    uint64_t poolBase = base + VDataOffsets::FNamePool();

    uint32_t block  = static_cast<uint32_t>(index >> 16);
    uint32_t offset = static_cast<uint32_t>(index & 0xFFFF);

    uint64_t blockPtr =
        VDataContext::Read<uint64_t>(poolBase + 0x10 + 8ull * block);
    if (!blockPtr)
        return {};

    uint64_t entryPtr = blockPtr + 4ull * offset;

    uint32_t key = VDataContext::Read<uint32_t>(entryPtr);
    uint16_t header = VDataContext::Read<uint16_t>(entryPtr + 4);
    uint16_t len    = header >> 1;

    if ((header & 1u) || len == 0 || len >= 1024)
        return {};

    char buf[1024]{};
    if (!VDataContext::ReadMemory(entryPtr + 6, buf, len))
        return {};

    uint64_t decryptedKey = decrypt_xor_keys(key, g_FNameState);
    uint32_t xorKey = static_cast<uint32_t>(decryptedKey);

    for (uint16_t i = 0; i < len; i++) {
        buf[i] ^= len ^ *((uint8_t*)&xorKey + (i & 3));
    }

    buf[len] = '\0';
    return std::string(buf);
}

std::string GetCharacterName(const std::string& in)
{
    if (in.find("Training") != std::string::npos) return "Bot Lobby";
    if (in.find("BountyHunter_PC_C") != std::string::npos) return "Fade";
    if (in.find("Stealth_PC_C") != std::string::npos) return "Yoru";
    if (in.find("Pandemic_PC_C") != std::string::npos) return "Viper";
    if (in.find("Hunter_PC_C") != std::string::npos) return "Sova";
    if (in.find("Guide_PC_C") != std::string::npos) return "Skye";
    if (in.find("Thorne_PC_C") != std::string::npos) return "Sage";
    if (in.find("Vampire_PC_C") != std::string::npos) return "Reyna";
    if (in.find("Clay_PC_C") != std::string::npos) return "Raze";
    if (in.find("Phoenix_PC_C") != std::string::npos) return "Phoenix";
    if (in.find("Wraith_PC_C") != std::string::npos) return "Omen";
    if (in.find("Sprinter_PC_C") != std::string::npos) return "Neon";
    if (in.find("Killjoy_PC_C") != std::string::npos) return "Killjoy";
    if (in.find("Grenadier_PC_C") != std::string::npos) return "Kayo";
    if (in.find("Terra_PC_C") != std::string::npos) return "Waylay";
    if (in.find("Cashew_PC_C") != std::string::npos) return "Tejo";
    if (in.find("Wushu_PC_C") != std::string::npos) return "Jett";
    if (in.find("Gumshoe_PC_C") != std::string::npos) return "Cypher";
    if (in.find("Deadeye_PC_C") != std::string::npos) return "Chamber";
    if (in.find("Sarge_PC_C") != std::string::npos) return "Brimstone";
    if (in.find("Breach_PC_C") != std::string::npos) return "Breach";
    if (in.find("Rift_TargetingForm_PC_C") != std::string::npos) return "Astra";
    if (in.find("Rift_PC_C") != std::string::npos) return "Astra";
    if (in.find("Mage_PC_C") != std::string::npos) return "Harbor";
    if (in.find("AggroBot_PC_C") != std::string::npos) return "Gekko";
    if (in.find("Cable_PC_C") != std::string::npos) return "DeadLock";
    if (in.find("Sequoia_PC_C") != std::string::npos) return "Iso";
    if (in.find("Smonk_PC_C") != std::string::npos) return "Clove";
    if (in.find("Nox_PC_C") != std::string::npos) return "Vyse";
    return "";
}

bool IsA(uint64_t actor, const std::string &class_name)
{
    if (!actor || class_name.empty())
        return false;

    uint64_t current = VDataContext::Read<uint64_t>(actor + 0x10);
    while (current) {
        int32_t fnameIndex = VDataContext::Read<int32_t>(current + 0x18);
        std::string name = GetNameFast(fnameIndex);
        if (name.find(class_name) != std::string::npos)
            return true;
        current = VDataContext::Read<uint64_t>(current + 0x48);
    }
    return false;
}

} 
