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
#include "../UnifiedTelemetryNode/telemetry/sdk/memory/memory.hpp"

namespace MemoryManager {
    enum class ScanMode {
        Live,
        Dump
    };

    ScanMode currentMode = ScanMode::Live;
    std::vector<uint8_t> dumpBuffer;
    uint64_t dumpBase = 0;

    bool Initialize(ScanMode mode, const std::string& dumpPath = "", uint64_t base = 0) {
        currentMode = mode;
        if (mode == ScanMode::Live) {
            if (!telemetryMemory::InitializeHyperInterface()) return false;
            return true;
        } else {
            std::ifstream file(dumpPath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cout << "[-] Failed to open dump file: " << dumpPath << "\n";
                return false;
            }
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            dumpBuffer.resize(size);
            if (!file.read((char*)dumpBuffer.data(), size)) {
                std::cout << "[-] Failed to read dump file.\n";
                return false;
            }
            dumpBase = base;
            std::cout << "[+] Loaded dump: " << size << " bytes at base 0x" << std::hex << base << "\n";
            return true;
        }
    }

    bool Read(uint64_t address, void* buffer, size_t size) {
        if (currentMode == ScanMode::Live) {
            return telemetryMemory::ReadMemory(address, buffer, size);
        } else {
            if (address < dumpBase || address + size > dumpBase + dumpBuffer.size()) return false;
            memcpy(buffer, dumpBuffer.data() + (address - dumpBase), size);
            return true;
        }
    }

    template <typename T>
    T ReadType(uint64_t address) {
        T buffer = {};
        Read(address, &buffer, sizeof(T));
        return buffer;
    }
}

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
            if (!MemoryManager::Read(start + offset, buffer.data(), 0x1000)) continue;

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
        int32_t rel = MemoryManager::ReadType<int32_t>(addr + offset);
        return addr + size + rel;
    }
};

int main() {
    std::cout << "================================================\n";
    std::cout << "      Master Scanner v3.0 (Dual-Mode Edition)\n";
    std::cout << "================================================\n\n";

    std::cout << "[1] Live Game Scan (Hyper-V)\n";
    std::cout << "[2] File Dump Scan\n";
    std::cout << "\nChoose mode: ";
    
    int choice;
    std::cin >> choice;
    uint64_t base = 0x140000000;
    
    if (choice == 1) {
        if (!MemoryManager::Initialize(MemoryManager::ScanMode::Live)) {
            std::cout << "[-] Hypervisor Bridge FAILED!\n";
            system("pause"); return 1;
        }

        std::cout << "[*] Waiting for game...\n";
        while (true) {
            if (telemetryMemory::AttachToGameStealthily()) {
                if (telemetryMemory::g_BaseAddress != 0 && telemetryMemory::g_BaseAddress < 0xFFFF000000000000) {
                    std::cout << "\n[+] Game Found! PID: " << std::dec << telemetryMemory::g_ProcessId 
                              << " | Base: 0x" << std::hex << telemetryMemory::g_BaseAddress << "\n";
                    break;
                }
            }
            std::cout << "."; Sleep(1000);
        }
        base = telemetryMemory::g_BaseAddress;
        std::cout << "[+] Hyper-Link Established.\n\n";
    } else {
        std::string dumpPath = "Process-Dumper/bin/dump_TslGame.exe";
        std::cout << "[*] Auto-loading Dump: " << dumpPath << "\n";
        
        if (!MemoryManager::Initialize(MemoryManager::ScanMode::Dump, dumpPath, base)) {
            return 1;
        }
    }
    
    uint64_t size = 0x10000000;
    std::map<std::string, uint64_t> results;

    // --- 1. CORE RVAs ---
    auto scan = [&](std::string name, const char* pat, int off, int inst) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint64_t rva = Scanner::GetRelative(addr, off, inst) - base;
            results[name] = rva;
            std::cout << "[SCAN] " << name << ": 0x" << std::hex << rva << "\n";
        } else {
            results[name] = 0;
            std::cout << name << ": NULL\n";
        }
    };

    auto scanDisp = [&](std::string name, const char* pat, int off) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint32_t disp = MemoryManager::ReadType<uint32_t>(addr + off);
            results[name] = disp;
            std::cout << "[DISP] " << name << ": 0x" << std::hex << disp << "\n";
        } else {
            results[name] = 0;
            std::cout << name << ": NULL\n";
        }
    };

    auto scanByte = [&](std::string name, const char* pat, int off) {
        uint64_t addr = Scanner::FindPattern(pat, base, size);
        if (addr) {
            uint8_t val = MemoryManager::ReadType<uint8_t>(addr + off);
            results[name] = val;
            std::cout << "[BYTE] " << name << ": 0x" << std::hex << (int)val << "\n";
        } else {
            results[name] = 0;
            std::cout << name << ": NULL\n";
        }
    };

    scan("XenuineDecrypt", "48 8B 05 ? ? ? ? 49 8B D0 8B CE FF D0 E9", 3, 7);
    scan("UWorld", "48 8B 05 ? ? ? ? 33 DB 48 39 1D", 3, 7);
    scan("GNames", "48 8D 0D ? ? ? ? 48 83 3D ? ? ? ? 00 75 ? 48 8B D1 B9 ? ? ? ? 48 8B 05 ? ? ? ? FF D0 EB ? 8B C1 35 ? ? ? ? 05", 3, 7);
    scan("GObjects", "4C 8B 1D ? ? ? ? 4C 8B B4 24", 3, 7);
    scan("PhysxSDK", "48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 89", 3, 7);

    // --- 1.5. UNREAL CORE ENUMERATION ---
    scanDisp("CurrentLevel", "48 8B 8F ?? ?? ?? ?? BE ?? ?? ?? ?? BB ?? ?? ?? ?? 41 BF", 3);
    scanByte("Actors", "49 8B 06 48 8B 50 ?? 48 8B 58 ?? 48 85 DB 74 03 FF 43 08", 6);
    scanDisp("GameInstance", "4C 8B 05 ?? ?? ?? ?? 48 8B 8F ?? ?? ?? ?? 4D 85 C0 75 1A", 10);
    scanDisp("AcknowledgedPawn", "48 8B 8F ?? ?? ?? ?? 48 85 C9 75 34 48 8B 9F ?? ?? ?? ?? 48 85 DB", 15);
    scanDisp("LocalPlayer", "48 8B 8F ?? ?? ?? ?? 48 85 C9 74 4C 48 8B 89 F0 00 00 00", 15);
    scanByte("PlayerController", "48 8B ?? 38 48 85 C0 74 25 48 8B 53 30 48 85 D2 74", 3);
    scanDisp("GameState", "49 8B 06 4C 8B 90 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 4C 3B D0 0F 84", 6);
    scanDisp("PlayerArray", "F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 49 8B 8F ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0", 16);
    scanDisp("Mesh", "F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 4C 8B B7 ?? ?? ?? ??", 16);
    scanDisp("Pawn", "8B 86 40 04 00 00 89 87 40 04 00 00 8B 86 44 04 00 00 89 87 44 04 00 00 8B 86 48 04 00 00", 26);
    scanDisp("ComponentToWorld", "4C 8D 3C C3 49 3B DF 74 5D 48 8B 33 4C 8B 96 ?? ?? ?? ??", 15);
    scanDisp("VehicleMovement", "F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 48 8B 9F ?? ?? ?? ?? 48 85 DB", 16);
    scanDisp("LastSubmitTime", "C6 44 24 30 01 48 8B 07 4C 8B 88 ?? ?? ?? ?? 4C 63 87", 11);
    scanDisp("PlayerState", "48 85 C0 0F 85 ?? ?? ?? ?? 48 8B 96 ?? ?? ?? ?? 41 BF", 12);
    scanDisp("MyHUD", "57 41 56 41 57 48 83 EC 30 48 8B 99 ?? ?? ?? ?? 8B F2", 12);
    scanDisp("RootComponent", "41 8B 04 37 39 05 ?? ?? ?? ?? 0F 8F ?? ?? ?? ?? 48 8B 87 ?? ?? ?? ??", 19);
    scanDisp("ViewTarget", "0F 11 87 50 10 00 00 0F 10 48 30 0F 11 8F 60 10", 3);
    scanDisp("PlayerCameraManager", "48 83 BB ?? ?? ?? ?? 00 74 29 48 8B CF E8 ?? ?? ?? ?? 84 C0 74 1D 48 8B 8B", 3);
    scanDisp("CameraCacheRotation", "48 8B 86 ?? ?? ?? ?? 48 89 45 D0 48 85 C0 74 03 FF 40 08", 3);
    scanDisp("CameraCacheLocation", "48 8B 8E ?? ?? ?? ?? 48 89 4D D8 48 8B 8E ?? ?? ?? ?? 48 89 4D E0", 3);
    scanDisp("CameraCacheFOV", "F3 0F 10 81 ?? ?? ?? ?? C3 CC CC 48 83", 4);
    scanDisp("SpectatorPawn", "F3 0F 10 8E ?? ?? ?? ?? EB 08 F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06", 4);
    scanDisp("CharacterName", "48 8B 85 ?? ?? ?? ?? 8B ?? ?? ?? 48 85 C0 74 1C 48 8B 08", 3);
    scanDisp("CharacterClanInfo", "48 8B 9D 30 08 00 00 48 8B 85 ?? ?? 00 00 48 89 45 98 48 89 85", 10);
    scanDisp("DefaultFOV", "48 8B 85 ?? ?? 00 00 48 89 45 80 48 89 85 98 00 00 00 48 8B 9D", 3);
    scanDisp("CharacterMovement", "C6 87 ?? ?? ?? ?? 00 48 8B 8F ?? ?? ?? ?? 48 83 3D ?? ?? ?? ?? 00", 10);
    scanDisp("EquippedWeapons", "48 8B 8D ?? ?? ?? ?? 48 33 CC E8 ?? ?? ?? ?? 4C 8D 9C 24", 3);
    scanDisp("CharacterState", "44 88 BF ?? ?? ?? ?? 8B 8F ?? ?? ?? ?? 41 B8 FE FF FF FF", 9);
    scanDisp("CurrentAmmoData", "44 89 BF 2C 0E 00 00 44 89 BF 68 0E 00 00 44 88 BF 64 0B 00 00", 10);
    scanDisp("FeatureRepObject", "48 83 EC 30 48 8B 99 ?? ?? ?? ?? 0F 29 70 E8 0F 28 F1 48 63 81", 7);
    scanDisp("NumAliveTeams", "48 63 FA 48 8B D9 48 8B 89 ?? ?? ?? ?? 41 83 F8 10 77 14", 9);
    scanDisp("HeaFlag", "40 53 48 83 EC 20 80 B9 ?? ?? ?? ?? 03 48 8B D9 0F 84", 8);
    scanDisp("InventoryFacade", "48 83 3D ?? ?? ?? ?? 00 48 8B 8B ?? ?? ?? ?? 75 16", 11);
    scanDisp("InventoryItems", "48 8B 8B 30 06 00 00 BD 01 00 00 00 48 85 C9 0F 85 ?? ?? ?? ?? 48 8B BB", 24);
    scanDisp("ItemPackageItems", "48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B B9 ?? ?? ?? ?? 41 BE", 13);
    scanDisp("WeaponProcessor", "48 8B F9 48 8B 91 ?? ?? ?? ?? 45 33 FF 48 85 D2 0F 84", 6);
    scanDisp("TrajectoryConfig", "48 8B 1E 48 8B 03 48 8B B8 ?? ?? ?? ??", 9);
    scanDisp("BlueZonePosition", "F3 0F 10 85 ?? ?? ?? ?? 48 63 C8 48 8D 44 24 50", 4);
    scanDisp("SafetyZonePosition", "F3 0F 11 44 24 30 F3 0F 10 85 ?? ?? ?? ?? 49 03 0E 48 89 44 24 28", 10);
    scanDisp("Health1", "8A 83 3C 0A 00 00 0A C2 88 83 3C 0A 00 00 48 89 9B 80 0A", 2);
    scanDisp("Health2", "48 8B 80 ?? ?? ?? ?? 48 85 C0 0F 84 ?? ?? ?? ?? F2 0F 10 80 ?? ?? ?? ?? F2 0F 11 44 24 20 8B 80", 32);
    scanDisp("Health3", "8A 88 92 05 00 00 80 E1 01 48 8B 44 24 30 88 48 28 48 8B 07 8B 88", 22);
    scanDisp("Health4", "FF 90 80 01 00 00 84 C0 74 10 48 8B 8F ?? ?? ?? ?? 48 85 C9", 13);
    scanDisp("Health5", "44 38 B7 25 0A 00 00 0F 85", 3);
    scanDisp("Health6", "48 8B B1 20 0A 00 00 45 33 F6", 3);
    scanDisp("GroggyHealth", "4C 89 BF A0 14 00 00 48 8D 8F ?? ?? ?? ?? E8", 10);
 
    // --- 2. MEMBER OFFSETS ---
    scanDisp("TeamNumber", "8B 81 ?? ?? ?? ?? 8D 98 ?? ?? ?? ?? 3D ?? ?? ?? ?? 0F 4C", 2); 
    scanDisp("BoneArray", "48 8B 81 ?? ?? ?? ?? 48 85 C0 74 04 8B 40 ?? C3 C3 CC CC CC 48 8B C4 48 89 58 08", 3);
    scanDisp("BoneCount", "48 83 EC 28 48 8B 81 ?? ?? ?? ?? 49 39", 7);
    scanDisp("Eyes", "F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06 7C 0B 45 8A F3", 4);
    scanByte("GNamesPtr", "49 8B ?? ?? B8 ?? ?? ?? ?? 41 F7 ?? 45 8B ?? 45 8D", 3);
    scanDisp("ChunkSize", "41 69 ?? ?? ?? ?? ?? 44 2B E8 4D 85 C0", 3);
 
    // --- 3. RADAR & MINIMAP ---
    scanDisp("Minimap", "8B B1 ?? ?? ?? ?? 48 8B 81 ?? ?? ?? ?? 33 FF 48 85 C0", 9);
    scanDisp("CurrentMinimapViewScale", "4C 89 9B 9C 04 00 00 89 B3 A4 04 00 00 89 B3 A8 04 00 00", 9);
    scanDisp("ScreenSize", "4C 89 9B 9C 04 00 00 89 B3 A4 04 00 00 89 B3 A8 04 00 00", 15);
    scanDisp("ScreenPosX", "41 8B 87 B8 04 00 00 41 89 86 B8 04 00 00 41 8B 87 BC 04 00 00", 3);
    scanDisp("ScreenPosY", "41 8B 87 B8 04 00 00 41 89 86 B8 04 00 00 41 8B 87 BC 04 00 00", 17);
    scanDisp("SelectMinimapSizeIndex", "8B 83 C4 05 00 00 89 87 C4 05 00 00 48 8B 83 C8 05 00 00", 15);
 
    // --- 4. CHARACTER & VISUALS ---
    scanDisp("Mesh3P", "E8 ?? ?? ?? ?? 83 F8 01 0F 84 ?? ?? ?? ?? 49 8B 8F ?? ?? ?? ??", 17);
    scanDisp("ComponentLocation", "40 80 FD 03 0F 84 ?? ?? ?? ?? F3 0F 10 98 ?? ?? ?? ??", 14);
    scanDisp("StaticMesh", "41 0F B7 40 53 48 83 EC 20 4C 8B 81 ?? ?? ?? ??", 12);
    scanDisp("WorldToMap", "F3 0F 10 A7 ?? ?? ?? ?? F3 0F 11 A4 24 90 00 00 00 F3 0F 10 AF ?? ?? ?? ??", 21);
    scanDisp("LastRenderTimeOnScreen", "F3 0F 10 97 ?? ?? ?? ?? F3 0F 10 8F ?? ?? ?? ?? F3 0F 5C CA", 12);
    scanDisp("LastTeamNum", "F7 47 08 00 00 00 20 75 30 8B 83 ?? ?? ?? ?? 41 39", 11);

    scanDisp("SpectatedCount", "4C 8D 4C 24 30 45 33 C0 41 8B 97 ?? ?? ?? ?? 48 8D 4C 24 40", 11);
 
    // --- 5. WEAPONS & BALLISTICS ---
    scanDisp("CurrentWeaponIndex", "48 83 EC 20 33 FF 4C 8B FA 80 B9 ?? ?? ?? ?? FF", 11);
    scanDisp("WeaponTrajectoryData", "48 8D 8F A8 11 00 00 E8 ?? ?? ?? ??", 3);
    scanDisp("TrajectoryGravityZ", "44 89 8C 24 6C 10 00 00 48 8B 84 24 68 10 00 00", 4);
    scanDisp("FiringAttachPoint", "88 87 55 08 00 00 8B 86 ?? ?? ?? ?? 89 87", 8);
    scanDisp("ScopingAttachPoint", "0F 28 D6 41 8B D5 E8 ?? ?? ?? ?? 48 8B 8F ?? ?? ?? ??", 14);
    scanDisp("RecoilValueVector", "8B 9D ?? ?? ?? ?? 44 8B 85 ?? ?? ?? ?? 48 8D 8D", 9);
    scanDisp("VerticalRecovery", "44 89 A7 D0 10 00 00 44 89 A7 D8 10 00 00 44 89 A7 E0 10", 10);
    scanDisp("AttachedItems", "88 87 55 08 00 00 8B 86 ?? ?? ?? ?? 89 87", 8);
 
    // --- 6. ITEMS & INVENTORY ---
    scanDisp("Inventory", "F7 40 08 00 00 00 20 75 12 48 8B 83 ?? ?? ?? ??", 12);
    scanDisp("InventoryItemCount", "48 8B BB ?? ?? ?? ?? 8B B3 ?? ?? ?? ?? 85 F6 0F 85", 9);
    scanDisp("Equipment", "F7 40 08 00 00 00 20 75 12 48 8B 83 ?? ?? ?? ??", 12); 
    scanDisp("ItemsArray", "48 8B 91 ?? ?? ?? ?? 48 8B D9 E8 ?? ?? ?? ?? 48 8B 93 ?? ?? ?? ??", 19);
    scanDisp("ItemID", "F2 0F 10 87 ?? ?? ?? ?? F2 0F 11 03 8B 87 ?? ?? ?? ??", 14);
    scanDisp("ItemTable", "4C 8B 8D C0 00 00 00 88 44 24 40 48 8B 85 ?? ?? ?? ??", 14);
 
    // --- 7. INPUT & STATE ---
    scanDisp("PlayerInput", "48 8B 05 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 48 81 C1", 10);
    scanDisp("InputYawScale", "41 0F 28 C3 F3 0F 58 83 ?? ?? ?? ?? F3 0F 11 83 ?? ?? ?? ??", 8);
    scanDisp("bIsScoping_CP", "38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ??", 10);
    scanDisp("bIsReloading_CP", "38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ??", 18);
 
    // --- 8. VEHICLES ---
    scanDisp("VehicleFuel", "F3 0F 10 91 E4 02 00 00 F3 0F 10 05 ?? ?? ?? ?? 0F 28 CA 0F 54 0D ?? ?? ?? ?? 0F 2F C1 73 15 F3 0F 10 81 E0 02 00 00", 31);
    scanDisp("VehicleFuelMax", "F3 0F 10 91 E4 02 00 00 F3 0F 10 05 ?? ?? ?? ?? 0F 28 CA 0F 54 0D", 4);
    scanDisp("VehicleHealth", "F3 0F 10 A3 D8 02 00 00", 4);
    scanDisp("VehicleHealthMax", "89 87 ?? ?? ?? ?? 45 33 E4 44 88 A7 ?? ?? ?? ??", 12);
    scanDisp("SeatIndex", "89 B5 C8 03 00 00 48 8B 83 ?? ?? ?? ?? 8B 48 08", 9);
 
    // --- 9. ENGINE & WORLD UTILS ---
    scanDisp("ActorsForGC", "40 88 B7 CB 07 00 00 40 88 B7 D0 07 00 00", 10);
    scanDisp("SafetyZoneRadius", "F3 0F 10 93 ?? ?? ?? ?? 48 8D 54 24 20 0F 28 DA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? F3 0F 10 83 ?? ?? ?? ??", 35);
    scanDisp("BlueZoneRadius", "F3 0F 10 93 ?? ?? ?? ?? 48 8D 54 24 20 0F 28 DA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? F3 0F 10 8B ?? ?? ?? ??", 48);
    scanDisp("ControlRotation", "85 C0 0F 84 ?? ?? ?? ?? F3 0F 10 90 0C 04 00 00", 12);
    scanDisp("bShowMouseCursor", "F6 83 58 06 00 00 02 75 28 48 8B", 2);
    scanDisp("TimeSeconds", "F3 0F 10 81 ?? ?? ?? ?? F3 0F 5C 81 ?? ?? ?? ?? 0F 28 C8 0F 5A C1", 4);
 
    // --- 10. PLAYER METRICS & SOCIAL ---
    scanDisp("AccountId", "F3 0F 11 87 10 08 00 00", 4);
    scanDisp("SquadMemberIndex", "F7 43 08 00 00 00 20 75 0B 8B 87 ?? ?? ?? ??", 11);
    scanDisp("PlayerStatistics", "48 8B 8F 10 0A 00 00 48 85 C9", 3);
    scanDisp("DamageDealtOnEnemy", "4C 89 81 ?? ?? ?? ?? 48 89 B1 ?? ?? ?? ?? 44 89 81 0C 08", 10);
    scanDisp("SurvivaLlevel", "F3 0F 11 B6 CC 0C 00 00 F3 0F 5C F7 48", 4);
 
    // --- 11. NETWORK & UI ---
    scanDisp("ping", "F3 0F 58 97 ?? ?? ?? ?? F3 0F 11 97 ?? ?? ?? ??", 12);
    scanDisp("Durability", "48 8B 06 48 8B CE FF 90 30 01 00 00 F3 0F 10 80 ?? ?? ?? ??", 14);
 
    // --- 12. COMBAT ADVANCED ---
    scanDisp("ElapsedCookingTime", "4C 89 A7 ?? ?? ?? ?? 44 89 A7", 3);
    scanDisp("LeanLeftAlpha_CP", "F3 0F 11 87 ?? ?? ?? ?? F3 0F 11 97 ?? ?? ?? ?? 44 88 A7", 12);
    scanDisp("LeanRightAlpha_CP", "FF 90 ?? ?? ?? ?? 48 8B 08 48 89 8F ?? ?? ?? ??", 12);
 
    // --- 13. REPLICATION & ANIMATION ---
    scanDisp("VehicleRiderComponent", "89 8F 20 19 00 00 48 C7 87", 2);
    scanDisp("ReplicatedMovement", "48 8B 4D 58 E8 ?? ?? ?? ?? 48 83 C4 20 5D C3 48 8B 8A ?? ?? ?? ??", 17);
    scanDisp("MatchId", "BE 00 00 00 20 85 70 08 0F 85 ?? ?? ?? ?? 48 8B 97 ?? ?? ?? ??", 17);
 
    // --- 14. CORE ENGINE OVERRIDES ---
    scanDisp("PlayerName", "44 39 BB 28 04 00 00 0F 8E ?? ?? ?? ?? 33 F6 48 8B 15 ?? ?? ?? ?? 48 8B 83 ?? ?? ?? ??", 25);
    scan("LocalPlayers", "48 83 3D ?? ?? ?? ?? 00 74 ?? 48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 50", 13, 17);
 
    // --- 15. DYNAMIC METADATA & GADGETS ---
    scanByte("ObjID", "41 8B ?? ?? BB ?? ?? ?? ?? 33 ?? 8B ?? C1 ?? 17", 3);
    scanDisp("SurvivalTier", "8B 86 40 04 00 00 89 87 40 04 00 00 8B 86 44 04 00 00", 2);
    scanDisp("DurabilityMax", "8B 82 14 03 00 00 89 81 14 03 00 00 8B 82 18 03 00 00", 2);
    scanDisp("BallisticCurve", "48 8B B8 28 00 00 00 E8 ?? ?? ?? ?? 48 2B E0 33 C9", 3);
    scanDisp("FloatCurves", "5D C3 CC 48 8B 8A 38 00 00 00 E9", 6);
    scanDisp("WeaponConfig_WeaponClass", "90 49 8B 06 4C 8B 90 78 08 00 00", 12);
    scanDisp("ControlRotation_CP", "80 3B 00 0F 85 ?? ?? ?? ?? 48 8B 8F C8 0B 00 00", 12);

    scanDisp("SafetyZoneRadius", "F3 0F 10 ?? 14 01 00 00", 4);
    scanDisp("BlueZoneRadius", "F3 0F 10 ?? 18 01 00 00", 4); 
    scanDisp("TimeSeconds", "F3 0F 10 83 ?? ?? 00 00 F3 0F 58 83 ?? ?? 00 00", 4);

    scanDisp("VerticalRecovery", "44 89 A7 D0 10 00 00 4C 89 A7 D8 10 00 00 44 89 A7 E0 10", 10);
 
    uint64_t addrSpoof = Scanner::FindPattern("48 85 C0 74 09 83 38 00 C6 45 77 01 77 04 C6 45 77", base, size);
    if (addrSpoof) results["SPOOFCALL_GADGET"] = addrSpoof - base;
 
    uint64_t addrLineTrace = Scanner::FindPattern("E8 ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 85 ?? 74 ?? 48 8B ?? ?? ?? ?? 44 8A ?? ?? 00 00 00", base, size);
    if (addrLineTrace) results["LineTraceSingle"] = (Scanner::GetRelative(addrLineTrace, 1, 5)) - base;
 
    uint64_t addrHook = Scanner::FindPattern("48 8D 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 FF 50", base, size);
    if (addrHook) results["HOOK"] = (Scanner::GetRelative(addrHook, 3, 7)) - base;
 
    uint64_t addrHook2 = Scanner::FindPattern("41 3B D8 7D 7C 48 89 54 24 20 4D 85 C9 75 13 B9", base, size);
    if (addrHook2) results["HOOK_TWO"] = (addrHook2 + 15) - base;
 
    // --- 16. DECRYPTION KEYs (Verified Offsets) ---
    uint64_t addrNameDec = Scanner::FindPattern("41 8B ?? ?? BB ?? ?? ?? ?? 33 ?? 8B ?? C1 ?? 01", base, size);
    if (addrNameDec) {
        results["DecryptNameIndexXorKey1"] = MemoryManager::ReadType<uint32_t>(addrNameDec + 5);
        results["DecryptNameIndexRval"] = MemoryManager::ReadType<uint8_t>(addrNameDec + 15);
        results["DecryptNameIndexRor"] = results["DecryptNameIndexRval"]; // Logic uses ROR with Rval
        std::cout << "[KEY]  DecryptNameIndexXorKey1: 0x" << std::hex << results["DecryptNameIndexXorKey1"] << "\n";
        std::cout << "[KEY]  DecryptNameIndexRval: 0x" << std::hex << (int)results["DecryptNameIndexRval"] << "\n";
    }
 
    uint64_t addr23 = Scanner::FindPattern("BE ?? ?? ?? ?? 41 23 C6 C1 E1 ?? 0B C1 33 D0 41 BF", base, size);
    if (addr23) {
        results["DecryptNameIndexXorKey3"] = MemoryManager::ReadType<uint32_t>(addr23 + 1);
        results["DecryptNameIndexDval"] = MemoryManager::ReadType<uint8_t>(addr23 + 10);
        results["DecryptNameIndexSval"] = 32 - (uint32_t)results["DecryptNameIndexDval"];
        
        // Find XorKey2 (follows the block)
        uint64_t addrKey2 = Scanner::FindPattern("41 BF ?? ?? ?? ?? 41 33 D7", addr23, 128);
        if (addrKey2) {
            results["DecryptNameIndexXorKey2"] = MemoryManager::ReadType<uint32_t>(addrKey2 + 2);
        }
        
        std::cout << "[KEY]  DecryptNameIndexXorKey3: 0x" << std::hex << results["DecryptNameIndexXorKey3"] << "\n";
        std::cout << "[KEY]  DecryptNameIndexDval: 0x" << std::hex << (int)results["DecryptNameIndexDval"] << "\n";
        std::cout << "[KEY]  DecryptNameIndexXorKey2: 0x" << std::hex << results["DecryptNameIndexXorKey2"] << "\n";
    }
 
    uint64_t addrH12 = Scanner::FindPattern("C7 45 B0 ?? ?? ?? ?? 33 D2 C7 45 B4 ?? ?? ?? ?? C7 45 B8 ?? ?? ?? ??", base, size);
    if (addrH12) {
        results["HealthKey1"] = MemoryManager::ReadType<uint32_t>(addrH12 + 12);
        results["HealthKey2"] = MemoryManager::ReadType<uint32_t>(addrH12 + 19);
        std::cout << "[KEY]  HealthKey1: 0x" << std::hex << results["HealthKey1"] << "\n";
        std::cout << "[KEY]  HealthKey2: 0x" << std::hex << results["HealthKey2"] << "\n";
    }

    // --- 3. AUTO-UPDATE telemetry_config.hpp ---
    std::string configPath = "../UnifiedTelemetryNode/.shared/telemetry_config.hpp";
    std::vector<std::string> lines;
    std::ifstream inFile(configPath);
    if (!inFile.is_open()) {
        configPath = "../../UnifiedTelemetryNode/.shared/telemetry_config.hpp";
        inFile.open(configPath);
    }
 
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            bool matched = false;
            for (auto const& [name, val] : results) {
                if (line.find("inline SecureOffset " + name + " =") != std::string::npos ||
                    line.find("inline SecureOffset32 " + name + " =") != std::string::npos ||
                    line.find("inline uint64_t " + name + " =") != std::string::npos ||
                    line.find("inline uint32_t " + name + " =") != std::string::npos) 
                {
                    char newline[512];
                    if (val > 0xFFFFFFFF)
                        sprintf_s(newline, "        inline SecureOffset %s = 0x%llX;", name.c_str(), val);
                    else
                        sprintf_s(newline, "        inline SecureOffset32 %s = 0x%X;", name.c_str(), (uint32_t)val);
                    lines.push_back(newline);
                    matched = true;
                    break;
                }
            }
            if (!matched) lines.push_back(line);
        }
        inFile.close();
        
        // Ghi đè kết quả mới vào telemetry_config.hpp
        std::ofstream outFile(configPath);
        if (outFile.is_open()) {
            for (const auto& l : lines) outFile << l << "\n";
            outFile.close();
            std::cout << "[+] Template sync: Successfully updated telemetry_config.hpp\n";
        }
    } else {
        std::cout << "\n[!] WARNING: Could not find telemetry_config.hpp for mirroring. Using raw fallback.\n";
    }
 
    // --- 4. EXPORT TO PUBG_Offsets.h ---
    std::ofstream hFile("bin/PUBG_Offsets.h");
    if (hFile.is_open()) {
        hFile << "#pragma once\n#include <cstdint>\n\n// Standalone offsets mirroring project structure\nnamespace offsets {\n";
        
        bool exportedAny = false;
        if (!lines.empty()) {
            for (const auto& line : lines) {
                if (line.find("inline uint") != std::string::npos || line.find("inline SecureOffset") != std::string::npos) {
                    std::string clean = line;
                    size_t first = clean.find_first_not_of(" ");
                    if (first != std::string::npos) clean = clean.substr(first);
                    
                    if (clean.find("SecureOffset") != std::string::npos) {
                        size_t pos = clean.find("SecureOffset");
                        if (clean.find("SecureOffset32") != std::string::npos)
                            clean.replace(pos, 14, "uint32_t");
                        else
                            clean.replace(pos, 12, "uint64_t");
                    }
                    hFile << "    " << clean << "\n";
                    exportedAny = true;
                }
            }
        }

        if (!exportedAny) {
            for (auto const& [name, val] : results) {
                if (val != 0) {
                    if (val > 0xFFFFFFFF)
                        hFile << "    inline uint64_t " << name << " = 0x" << std::hex << val << ";\n";
                    else
                        hFile << "    inline uint32_t " << name << " = 0x" << std::hex << (uint32_t)val << ";\n";
                }
            }
        }
        
        hFile << "}\n";
        hFile.close();
        std::cout << "[+] Exported synchronized results to bin/PUBG_Offsets.h\n";
    }
 
    std::cout << "\n[*] Offset scanning complete. System ready.\n";
    std::cout << "[!] This window will close automatically in 10 seconds...\n";
    for (int i = 10; i > 0; --i) {
        std::cout << "\rClosing in: " << i << " seconds...   " << std::flush;
        Sleep(1000);
    }
    return 0;
}
