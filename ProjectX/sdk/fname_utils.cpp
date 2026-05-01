#include "fname.hpp"
#include "context.hpp"
#include "offsets.hpp"
#include <iostream>

namespace FNameUtils {

static uint32_t g_FNameKey = 0;

uint32_t GetKey()
{
    if (g_FNameKey)
        return g_FNameKey;

    constexpr const char* plaintext = "ByteProperty";
    constexpr uint16_t len = 12;
    constexpr int32_t index = 3;

    uint64_t base = ProjectXContext::GetBaseAddress();
    if (!base) {
        return 0;
    }

    uint64_t poolBase = base + ProjectXOffsets::FNamePool();

    uint32_t block  = static_cast<uint32_t>(index >> 16);
    uint32_t offset = static_cast<uint32_t>(index & 0xFFFF);

    uint64_t dataPtr =
        ProjectXContext::Read<uint64_t>(poolBase + 0x10 + 8ull * block);
    if (!dataPtr) {
        return 0;
    }
    dataPtr += 4ull * offset;

    uint16_t header   = ProjectXContext::Read<uint16_t>(dataPtr + 4);
    uint16_t read_len = header >> 1;
    
    if ((header & 1u) || read_len != len) {
        return 0;
    }
    
    char enc[len]{};
    if (!ProjectXContext::ReadMemory(dataPtr + 6, enc, len)) {
        return 0;
    }

    uint32_t key = 0;
    for (int i = 0; i < 4; ++i)
        reinterpret_cast<char*>(&key)[i] = enc[i] ^ plaintext[i] ^ len;

    g_FNameKey = key;
    // std::cout << "[DEBUG] FName Key resolved: " << std::hex << key << std::dec << std::endl;
    return key;
}

std::string GetNameFast(int32_t index)
{
    if (index < 0)
        return {};

    uint32_t key = GetKey();
    if (!key)
        return {};

    uint64_t base = ProjectXContext::GetBaseAddress();
    if (!base)
        return {};

    uint64_t poolBase = base + ProjectXOffsets::FNamePool();

    uint32_t block  = static_cast<uint32_t>(index >> 16);
    uint32_t offset = static_cast<uint32_t>(index & 0xFFFF);

    uint64_t blockPtr =
        ProjectXContext::Read<uint64_t>(poolBase + 0x10 + 8ull * block);
    if (!blockPtr)
        return {};

    uint64_t entryPtr = blockPtr + 4ull * offset;

    uint16_t header = ProjectXContext::Read<uint16_t>(entryPtr + 4);
    uint16_t len    = header >> 1;

    if ((header & 1u) || len == 0 || len >= 1024)
        return {};

    char buf[1024]{};
    if (!ProjectXContext::ReadMemory(entryPtr + 6, buf, len))
        return {};
    buf[len] = '\0';

    FNameEntry entry{};
    entry.NameEntryHeader.dwSTRLen = len;
    entry.NameEntryHeader.bWideSTR = 0;
    std::memcpy(entry.first_char_encrypted, buf, len);

    std::string result = FName::DecryptName(entry, key);
    
    /*
    static int fnameLogCount = 0;
    if (fnameLogCount < 10) { // Log first few names to verify
        std::cout << "[DEBUG] FName(" << index << ") decrypted: " << result << std::endl;
        fnameLogCount++;
    }
    */

    return result;
}

bool IsA(uint64_t actor, const std::string &class_name)
{
    if (!actor || class_name.empty())
        return false;

    uint64_t current = ProjectXContext::Read<uint64_t>(actor + 0x10);
    while (current) {
        int32_t fnameIndex = ProjectXContext::Read<int32_t>(current + 0x18);
        std::string name = GetNameFast(fnameIndex);
        if (name.find(class_name) != std::string::npos)
            return true;
        current = ProjectXContext::Read<uint64_t>(current + 0x48);
    }
    return false;
}

} 

