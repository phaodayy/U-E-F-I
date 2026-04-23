#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <intrin.h>

// [HYPERVISOR INTEGRATION] Include project headers
#include "../PUBG-2/pubg/sdk/memory.hpp"

#pragma comment(lib, "psapi.lib")

class Scanner {
public:
    static uint64_t FindPattern(const char* pattern, uint64_t start, uint64_t size) {
        std::vector<uint8_t> buffer(0x1000); 
        auto parsePattern = [](const char* pattern) {
            std::vector<int> bytes;
            char* ptr = const_cast<char*>(pattern);
            while (*ptr) {
                if (*ptr == '?') {
                    bytes.push_back(-1);
                    if (*(ptr + 1) == '?') ptr++;
                } else if (isxdigit(*ptr)) {
                    bytes.push_back(strtol(ptr, &ptr, 16));
                    continue;
                }
                ptr++;
            }
            return bytes;
        };

        auto patternBytes = parsePattern(pattern);
        size_t pSize = patternBytes.size();

        for (uint64_t offset = 0; offset < size - pSize; offset += (0x1000 - pSize)) {
            if (!PubgMemory::ReadMemory(start + offset, buffer.data(), 0x1000)) continue;

            for (size_t i = 0; i < 0x1000 - pSize; ++i) {
                bool found = true;
                for (size_t j = 0; j < pSize; ++j) {
                    if (patternBytes[j] != -1 && buffer[i + j] != patternBytes[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) return start + offset + i;
            }
        }
        return 0;
    }

    static uint64_t GetRelative(uint64_t addr, int offset, int size) {
        if (!addr) return 0;
        int32_t rel = PubgMemory::Read<int32_t>(addr + offset);
        return addr + size + rel;
    }
};

int main() {
    std::cout << "[*] Master Scanner v2.4 (Production Sync)\n";
    
    if (!PubgMemory::InitializeHyperInterface()) {
        std::cout << "[-] Hypervisor Bridge FAILED!\n";
        system("pause"); return 1;
    }

    std::cout << "[*] Waiting for game...\n";
    while (true) {
        if (PubgMemory::AttachToGameStealthily()) {
            if (PubgMemory::g_BaseAddress != 0 && PubgMemory::g_BaseAddress < 0xFFFF000000000000) break;
        }
        std::cout << "."; Sleep(1000);
    }
    
    uint64_t base = PubgMemory::g_BaseAddress;
    uint64_t size = 0x10000000;
    std::map<std::string, uint64_t> results;

    std::cout << "\n[+] Hyper-Link Established. Base: 0x" << std::hex << base << "\n\n";

    // --- 1. CORE RVAs (User Verified) ---
    auto scan = [&](std::string name, const char* pat, int off, int inst) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint64_t rva = Scanner::GetRelative(addr, off, inst) - base;
            results[name] = rva;
            std::cout << "[SCAN] " << name << ": 0x" << std::hex << rva << "\n";
        } else {
            std::cout << "[FAIL] Could not find " << name << " pattern.\n";
        }
    };

    auto scanDisp = [&](std::string name, const char* pat, int off) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint32_t disp = PubgMemory::Read<uint32_t>(addr + off);
            results[name] = disp;
            std::cout << "[DISP] " << name << ": 0x" << std::hex << disp << "\n";
        } else {
            std::cout << "[FAIL] Could not find " << name << " pattern.\n";
        }
    };

    auto scanByte = [&](std::string name, const char* pat, int off) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint8_t val = PubgMemory::Read<uint8_t>(addr + off);
            results[name] = val;
            std::cout << "[BYTE] " << name << ": 0x" << std::hex << (int)val << "\n";
        } else {
            std::cout << "[FAIL] Could not find " << name << " pattern.\n";
        }
    };

    scan("XenuineDecrypt", "48 8B 05 ? ? ? ? 49 8B D0 8B CE FF D0 E9", 3, 7);
    scan("UWorld", "48 8B 05 ? ? ? ? 33 DB 48 39 1D", 3, 7);
    scan("GNames", "48 8D 0D ? ? ? ? 48 83 3D ? ? ? ? 00 75 ? 48 8B D1 B9 ? ? ? ? 48 8B 05 ? ? ? ? FF D0 EB ? 8B C1 35 ? ? ? ? 05", 3, 7);
    scan("GObjects", "4C 8B 1D ? ? ? ? 4C 8B B4 24", 3, 7);
    scan("PhysxSDK", "48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 89", 3, 7);

    // --- 1.5. UNREAL CORE ENUMERATION ---
    scanDisp("CurrentLevel", "48 8B 8F ?? ?? ?? ?? BE ?? ?? ?? ?? BB ?? ?? ?? ?? 41 BF", 3);
    scanDisp("Actors", "49 8B 06 48 8B 50 ?? 48 8B 58 ?? 48 85 DB 74 03 FF 43 08", 6);
    scanDisp("GameInstance", "4C 8B 05 ?? ?? ?? ?? 48 8B 8F ?? ?? ?? ?? 4D 85 C0 75 1A", 10);
    scanDisp("AcknowledgedPawn", "48 8B 8F ?? ?? ?? ?? 48 85 C9 75 34 48 8B 9F ?? ?? ?? ?? 48 85 DB", 14);
    scanDisp("GameState", "49 8B 06 4C 8B 90 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 4C 3B D0 0F 84", 6);
    scanDisp("PlayerCameraManager", "48 83 BB ?? ?? ?? ?? 00 74 29 48 8B CF E8 ?? ?? ?? ?? 84 C0 74 1D 48 8B 8B", 3);
    scanDisp("CameraCacheRotation", "48 8B 86 ?? ?? ?? ?? 48 89 45 D0 48 85 C0 74 03 FF 40 08", 3);
    scanDisp("CameraCacheLocation", "48 8B 8E ?? ?? ?? ?? 48 89 4D D8 48 8B 8E ?? ?? ?? ?? 48 89 4D E0", 3);
    scanDisp("CameraCacheFOV", "0F 2F 05 ?? ?? ?? ?? 77 08 F3 0F 10 81 ?? ?? ?? ?? C3", 12);
    scanDisp("SpectatorPawn", "F3 0F 10 8E ?? ?? ?? ?? EB 08 F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06", 4);
    scanDisp("CharacterName", "48 8B 85 ?? ?? ?? ?? 8B ?? ?? ?? 48 85 C0 74 1C 48 8B 08", 3);
    scanDisp("CharacterClanInfo", "48 8B 85 ?? ?? 00 00 48 89 45 98 48 89 85 A0 00 00 00 48 8B 85 ?? ?? 00 00", 19);
    scanDisp("DefaultFOV", "48 8B 85 ?? ?? 00 00 48 89 45 98 48 89 85 A0 00 00 00 48 8B 85 ?? ?? 00 00", 3);
    scanDisp("CharacterMovement", "C6 87 ?? ?? ?? ?? 00 48 8B 8F ?? ?? ?? ?? 48 83 3D ?? ?? ?? ?? 00", 10);
    scanDisp("EquippedWeapons", "48 8B 8D ?? ?? ?? ?? 48 33 CC E8 ?? ?? ?? ?? 4C 8D 9C 24", 3);
    scanDisp("CharacterState", "44 88 BF ?? ?? ?? ?? 8B 8F ?? ?? ?? ?? 41 B8 FE FF FF FF", 9);
    scanDisp("CurrentAmmoData", "44 89 BF 2C 0E 00 00 44 89 BF 68 0E 00 00 44 88 BF 64 0B 00 00", 10);

    // --- 2. MEMBER OFFSETS ---
    scanDisp("TeamNumber", "8B 81 ?? ?? ?? ?? 8D 98 ?? ?? ?? ?? 3D ?? ?? ?? ?? 0F 4C", 2); 
    scanDisp("BoneArray", "48 8B 81 ?? ?? ?? ?? 48 85 C0 74 04 8B 40 ?? C3 C3 CC CC CC 48 8B C4 48 89 58 08", 3);
    scanDisp("WeaponProcessor", "8B EC 48 83 EC 50 48 8B F9 48 8B 91 ?? ?? ?? ??", 12);
    scanDisp("BoneCount", "48 83 EC 28 48 8B 81 ?? ?? ?? ?? 49 39", 7);
    scanDisp("Eyes", "F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06 7C 0B 45 8A F3", 4);
    scanByte("GNamesPtr", "49 8B ?? ?? B8 ?? ?? ?? ?? 41 F7 ?? 45 8B ?? 45 8D", 3);
    scanDisp("ChunkSize", "41 69 ?? ?? ?? ?? ?? 44 2B E8 4D 85 C0", 3);

    // --- 3. DECRYPTION KEYs (Verified Offsets) ---
    uint64_t addrNameDec = Scanner::FindPattern("41 8B ?? ?? BB ?? ?? ?? ?? 33 ?? 8B ?? C1 ?? 17", base, size);
    if (addrNameDec) {
        results["DecryptNameIndexXorKey1"] = PubgMemory::Read<uint32_t>(addrNameDec + 5);
        results["DecryptNameIndexRval"] = PubgMemory::Read<uint8_t>(addrNameDec + 14);
        std::cout << "[KEY]  DecryptNameIndexXorKey1: 0x" << std::hex << results["DecryptNameIndexXorKey1"] << "\n";
        std::cout << "[KEY]  DecryptNameIndexRval: 0x" << std::hex << (int)results["DecryptNameIndexRval"] << "\n";
    }

    uint64_t addr23 = Scanner::FindPattern("BE ? ? ? ? 41 23 C6 C1 E1 19 0B C1 33 D0 41 BF", base, size);
    if (addr23) {
        results["DecryptNameIndexXorKey3"] = PubgMemory::Read<uint32_t>(addr23 + 1);
        results["DecryptNameIndexXorKey2"] = PubgMemory::Read<uint32_t>(addr23 + 17);
        std::cout << "[KEY]  DecryptNameIndexXorKey3: 0x" << std::hex << results["DecryptNameIndexXorKey3"] << "\n";
        std::cout << "[KEY]  DecryptNameIndexXorKey2: 0x" << std::hex << results["DecryptNameIndexXorKey2"] << "\n";
    }

    uint64_t addrH12 = Scanner::FindPattern("C7 45 B0 ?? ?? ?? ?? 33 D2 C7 45 B4 ?? ?? ?? ?? C7 45 B8 ?? ?? ?? ??", base, size);
    if (addrH12) {
        results["HealthKey1"] = PubgMemory::Read<uint32_t>(addrH12 + 12);
        results["HealthKey2"] = PubgMemory::Read<uint32_t>(addrH12 + 19);
        std::cout << "[KEY]  HealthKey1: 0x" << std::hex << results["HealthKey1"] << "\n";
        std::cout << "[KEY]  HealthKey2: 0x" << std::hex << results["HealthKey2"] << "\n";
    }

    // --- 3. AUTO-UPDATE pubg_config.hpp ---
    std::string configPath = "../PUBG-2/.shared/pubg_config.hpp";
    std::ifstream inFile(configPath);
    if (inFile.is_open()) {
        std::vector<std::string> lines;
        std::string line;
        int updates = 0;
        while (std::getline(inFile, line)) {
            bool matched = false;
            for (auto const& [name, val] : results) {
                if (line.find("inline uint64_t " + name + " =") != std::string::npos ||
                    line.find("inline uint32_t " + name + " =") != std::string::npos) 
                {
                    char newline[512];
                    if (val > 0xFFFFFFFF)
                        sprintf_s(newline, "        inline uint64_t %s = 0x%llX;", name.c_str(), val);
                    else
                        sprintf_s(newline, "        inline uint32_t %s = 0x%X;", name.c_str(), (uint32_t)val);
                    lines.push_back(newline);
                    matched = true;
                    updates++;
                    break;
                }
            }
            if (!matched) lines.push_back(line);
        }
        inFile.close();
        if (updates > 0) {
            std::ofstream outFile(configPath);
            for (const auto& l : lines) outFile << l << "\n";
            outFile.close();
            std::cout << "\n[+] Synchronized " << updates << " values to pubg_config.hpp\n";
        }
    }

    std::cout << "\n[*] Offset scanning complete. System ready.\n";
    system("pause");
    return 0;
}
