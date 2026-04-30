#include "runtime_offsets.hpp"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <.shared/telemetry_config.hpp>
#include <protec/skCrypt.h>

#include "../memory/memory.hpp"
#include "telemetry_decrypt.hpp"

namespace telemetryRuntimeOffsets {
namespace {
    enum class ScanKind : uint8_t {
        Rip,
        Disp32,
        Byte
    };

    struct LoadedSection {
        uint64_t RemoteBase = 0;
        std::vector<uint8_t> Bytes;
    };

    struct PatternSpec {
        telemetry_config::SecureOffset* Target = nullptr;
        const char* Pattern = nullptr;
        ScanKind Kind = ScanKind::Disp32;
        uint8_t OperandOffset = 0;
        uint8_t InstructionSize = 0;
        bool Required = false;
    };

    struct PendingOffset64 {
        telemetry_config::SecureOffset* Target = nullptr;
        uint64_t Value = 0;
    };

    struct PendingOffset32 {
        telemetry_config::SecureOffset32* Target = nullptr;
        uint32_t Value = 0;
    };

    RuntimeScanReport g_LastReport{};
    uint64_t g_LastScannedBase = 0;
    std::vector<PendingOffset64> g_Pending64;
    std::vector<PendingOffset32> g_Pending32;
    bool g_PendingHealthKeyRefresh = false;

    template <typename T>
    bool ReadRemote(uint64_t address, T& out) {
        return telemetryMemory::ReadMemory(address, &out, sizeof(T));
    }

    bool ReadRemoteBlock(uint64_t address, void* out, size_t size) {
        return telemetryMemory::ReadMemory(address, out, static_cast<uint64_t>(size));
    }

    int HexValue(char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

    bool ParsePattern(const char* pattern, std::vector<int>& bytes) {
        bytes.clear();
        if (!pattern) return false;

        const char* p = pattern;
        while (*p) {
            if (std::isspace(static_cast<unsigned char>(*p))) {
                ++p;
                continue;
            }

            if (*p == '?') {
                bytes.push_back(-1);
                ++p;
                if (*p == '?') ++p;
                continue;
            }

            const int hi = HexValue(p[0]);
            const int lo = HexValue(p[1]);
            if (hi < 0 || lo < 0) return false;
            bytes.push_back((hi << 4) | lo);
            p += 2;
        }

        return !bytes.empty();
    }

    size_t FindInBuffer(const uint8_t* data, size_t size, const std::vector<int>& pattern) {
        const size_t n = pattern.size();
        if (!data || n == 0 || size < n) return static_cast<size_t>(-1);

        for (size_t i = 0; i <= size - n; ++i) {
            bool match = true;
            for (size_t j = 0; j < n; ++j) {
                if (pattern[j] >= 0 && data[i + j] != static_cast<uint8_t>(pattern[j])) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return static_cast<size_t>(-1);
    }

    uint64_t FindPattern(const std::vector<LoadedSection>& sections, const char* pattern) {
        std::vector<int> parsed;
        if (!ParsePattern(pattern, parsed)) return 0;

        for (const auto& section : sections) {
            const size_t pos = FindInBuffer(section.Bytes.data(), section.Bytes.size(), parsed);
            if (pos != static_cast<size_t>(-1)) {
                return section.RemoteBase + pos;
            }
        }
        return 0;
    }

    uint64_t FindPatternRange(uint64_t start, uint64_t size, const char* pattern) {
        if (!start || !size || size > 0x100000) return 0;

        std::vector<int> parsed;
        if (!ParsePattern(pattern, parsed) || parsed.size() > size) return 0;

        std::vector<uint8_t> buffer(static_cast<size_t>(size), 0);
        if (!ReadRemoteBlock(start, buffer.data(), buffer.size())) return 0;

        const size_t pos = FindInBuffer(buffer.data(), buffer.size(), parsed);
        if (pos == static_cast<size_t>(-1)) return 0;
        return start + pos;
    }

    template <typename T>
    bool ReadValue(const std::vector<LoadedSection>& sections, uint64_t address, T& out) {
        for (const auto& section : sections) {
            const uint64_t begin = section.RemoteBase;
            const uint64_t end = begin + section.Bytes.size();
            if (address >= begin && address + sizeof(T) <= end && address + sizeof(T) >= address) {
                std::memcpy(&out, section.Bytes.data() + (address - begin), sizeof(T));
                return true;
            }
        }
        return ReadRemote(address, out);
    }

    bool ReadSection(uint64_t address, uint64_t size, std::vector<uint8_t>& out) {
        if (!address || size == 0 || size > 0x30000000ULL) return false;

        out.assign(static_cast<size_t>(size), 0);
        constexpr uint64_t kChunk = 0x10000;
        bool anyRead = false;
        for (uint64_t offset = 0; offset < size; offset += kChunk) {
            const uint64_t todo = std::min<uint64_t>(kChunk, size - offset);
            if (ReadRemoteBlock(address + offset, out.data() + offset, static_cast<size_t>(todo))) {
                anyRead = true;
            }
        }
        return anyRead;
    }

    bool LoadExecutableSections(uint64_t base, std::vector<LoadedSection>& sections) {
        sections.clear();

        IMAGE_DOS_HEADER dos{};
        if (!ReadRemote(base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE) return false;
        if (dos.e_lfanew <= 0 || dos.e_lfanew > 0x4000) return false;

        IMAGE_NT_HEADERS64 nt{};
        if (!ReadRemote(base + static_cast<uint32_t>(dos.e_lfanew), nt)) return false;
        if (nt.Signature != IMAGE_NT_SIGNATURE) return false;
        if (nt.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) return false;
        if (nt.FileHeader.NumberOfSections == 0 || nt.FileHeader.NumberOfSections > 96) return false;

        const uint64_t sectionTable = base + static_cast<uint32_t>(dos.e_lfanew) +
            FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + nt.FileHeader.SizeOfOptionalHeader;

        for (uint16_t i = 0; i < nt.FileHeader.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER sh{};
            if (!ReadRemote(sectionTable + (sizeof(IMAGE_SECTION_HEADER) * i), sh)) continue;

            const bool executable = (sh.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0 ||
                (sh.Characteristics & IMAGE_SCN_CNT_CODE) != 0;
            if (!executable) continue;

            const uint64_t virtualSize = sh.Misc.VirtualSize ? sh.Misc.VirtualSize : sh.SizeOfRawData;
            const uint64_t size = std::max<uint64_t>(virtualSize, sh.SizeOfRawData);
            if (size == 0 || size > 0x30000000ULL) continue;

            LoadedSection section{};
            section.RemoteBase = base + sh.VirtualAddress;
            if (ReadSection(section.RemoteBase, size, section.Bytes)) {
                sections.push_back(std::move(section));
            }
        }

        return !sections.empty();
    }

    bool IsUsableValue(ScanKind kind, uint64_t value) {
        if (value == 0) return false;
        if (kind == ScanKind::Byte) return value <= 0xFF;
        if (kind == ScanKind::Disp32) return value <= 0x1000000ULL;
        return value <= 0x300000000ULL;
    }

    bool IsValidPtr(uint64_t value) {
        return value >= 0x100000000ULL && value < 0x00007FFFFFFFFFFFULL;
    }

    void StageOffset(telemetry_config::SecureOffset* target, uint64_t value) {
        if (!target || !value) return;
        for (auto& pending : g_Pending64) {
            if (pending.Target == target) {
                pending.Value = value;
                return;
            }
        }
        g_Pending64.push_back({ target, value });
    }

    void StageOffset(telemetry_config::SecureOffset32* target, uint32_t value) {
        if (!target || !value) return;
        for (auto& pending : g_Pending32) {
            if (pending.Target == target) {
                pending.Value = value;
                return;
            }
        }
        g_Pending32.push_back({ target, value });
    }

    uint64_t CandidateValue(telemetry_config::SecureOffset& target) {
        for (const auto& pending : g_Pending64) {
            if (pending.Target == &target) return pending.Value;
        }
        return static_cast<uint64_t>(target);
    }

    bool ValidateRuntimeCandidates(uint64_t base) {
        namespace o = telemetry_config::offsets;

        const uint64_t xenuine = CandidateValue(o::XenuineDecrypt);
        const uint64_t uworldOffset = CandidateValue(o::UWorld);
        if (!xenuine || !uworldOffset) return false;

        if (!telemetryDecrypt::Initialize(telemetryMemory::ReadMemory, base, xenuine)) {
            return false;
        }

        uint64_t rawUWorld = 0;
        if (!ReadRemote(base + uworldOffset, rawUWorld)) return false;

        const uint64_t uworld = telemetryDecrypt::Xe(rawUWorld);
        if (!IsValidPtr(uworld)) return false;

        const uint64_t gameInstance = telemetryDecrypt::Xe(
            telemetryMemory::Read<uint64_t>(uworld + CandidateValue(o::GameInstance)));
        const uint64_t level = telemetryDecrypt::Xe(
            telemetryMemory::Read<uint64_t>(uworld + CandidateValue(o::CurrentLevel)));
        if (!IsValidPtr(gameInstance) || !IsValidPtr(level)) return false;

        const uint64_t localPlayers = telemetryMemory::Read<uint64_t>(
            gameInstance + CandidateValue(o::LocalPlayer));
        if (!IsValidPtr(localPlayers)) return true;

        const uint64_t localPlayer = telemetryDecrypt::Xe(
            telemetryMemory::Read<uint64_t>(localPlayers));
        if (!IsValidPtr(localPlayer)) return true;

        const uint64_t controller = telemetryDecrypt::Xe(
            telemetryMemory::Read<uint64_t>(localPlayer + CandidateValue(o::PlayerController)));
        if (!IsValidPtr(controller)) return true;

        const uint64_t pawn = telemetryDecrypt::Xe(
            telemetryMemory::Read<uint64_t>(controller + CandidateValue(o::AcknowledgedPawn)));
        if (!IsValidPtr(pawn)) return true;

        const uint64_t mesh = telemetryMemory::Read<uint64_t>(pawn + CandidateValue(o::Mesh));
        if (!IsValidPtr(mesh)) return false;

        const uint64_t boneArray = telemetryMemory::Read<uint64_t>(mesh + CandidateValue(o::BoneArray));
        return IsValidPtr(boneArray);
    }

    void ApplyPendingOffsets() {
        for (const auto& pending : g_Pending64) {
            *pending.Target = pending.Value;
            ++g_LastReport.Applied;
        }
        for (const auto& pending : g_Pending32) {
            *pending.Target = pending.Value;
            ++g_LastReport.Applied;
        }
        if (g_PendingHealthKeyRefresh) {
            telemetry_config::offsets::RefreshHealthKeys();
        }
    }

    bool ResolvePattern(const std::vector<LoadedSection>& sections, uint64_t base, const PatternSpec& spec, uint64_t& value) {
        value = 0;
        const uint64_t addr = FindPattern(sections, spec.Pattern);
        if (!addr) return false;

        switch (spec.Kind) {
        case ScanKind::Rip: {
            int32_t rel = 0;
            if (!ReadValue(sections, addr + spec.OperandOffset, rel)) return false;
            const uint64_t target = addr + spec.InstructionSize + rel;
            if (target <= base) return false;
            value = target - base;
            return true;
        }
        case ScanKind::Disp32: {
            uint32_t disp = 0;
            if (!ReadValue(sections, addr + spec.OperandOffset, disp)) return false;
            value = disp;
            return true;
        }
        case ScanKind::Byte: {
            uint8_t byteValue = 0;
            if (!ReadValue(sections, addr + spec.OperandOffset, byteValue)) return false;
            value = byteValue;
            return true;
        }
        default:
            return false;
        }
    }

    void ApplyPatternSpecs(const std::vector<LoadedSection>& sections, uint64_t base) {
        using namespace telemetry_config;
        namespace o = telemetry_config::offsets;

        const PatternSpec specs[] = {
            { &o::XenuineDecrypt, skCrypt("48 8B 05 ? ? ? ? 49 8B D0 8B CE FF D0 E9"), ScanKind::Rip, 3, 7, true },
            { &o::UWorld, skCrypt("48 8B 05 ? ? ? ? 33 DB 48 39 1D"), ScanKind::Rip, 3, 7, true },
            { &o::GNames, skCrypt("48 8D 0D ? ? ? ? 48 83 3D ? ? ? ? 00 75 ? 48 8B D1 B9 ? ? ? ? 48 8B 05 ? ? ? ? FF D0 EB ? 8B C1 35 ? ? ? ? 05"), ScanKind::Rip, 3, 7, true },
            { &o::GObjects, skCrypt("4C 8B 1D ? ? ? ? 4C 8B B4 24"), ScanKind::Rip, 3, 7, true },
            { &o::PhysxSDK, skCrypt("48 8B 05 ?? ?? ?? ?? C3 CC CC CC CC CC CC CC CC 48 89"), ScanKind::Rip, 3, 7, false },
            { &o::CurrentLevel, skCrypt("48 8B 8F ?? ?? ?? ?? BE ?? ?? ?? ?? BB ?? ?? ?? ?? 41 BF"), ScanKind::Disp32, 3, 0, true },
            { &o::Actors, skCrypt("49 8B 06 48 8B 50 ?? 48 8B 58 ?? 48 85 DB 74 03 FF 43 08"), ScanKind::Byte, 6, 0, true },
            { &o::GameInstance, skCrypt("4C 8B 05 ?? ?? ?? ?? 48 8B 8F ?? ?? ?? ?? 4D 85 C0 75 1A"), ScanKind::Disp32, 10, 0, true },
            { &o::AcknowledgedPawn, skCrypt("48 8B 8F ?? ?? ?? ?? 48 85 C9 75 34 48 8B 9F ?? ?? ?? ?? 48 85 DB"), ScanKind::Disp32, 15, 0, true },
            { &o::LocalPlayer, skCrypt("48 8B 8F ?? ?? ?? ?? 48 85 C9 74 4C 48 8B 89 F0 00 00 00"), ScanKind::Disp32, 15, 0, true },
            { &o::PlayerController, skCrypt("48 8B ?? 38 48 85 C0 74 25 48 8B 53 30 48 85 D2 74"), ScanKind::Byte, 3, 0, true },
            { &o::GameState, skCrypt("49 8B 06 4C 8B 90 ?? ?? ?? ?? 48 8D 05 ?? ?? ?? ?? 4C 3B D0 0F 84"), ScanKind::Disp32, 6, 0, true },
            { &o::PlayerArray, skCrypt("F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 49 8B 8F ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 85 C0"), ScanKind::Disp32, 16, 0, true },
            { &o::Mesh, skCrypt("F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 4C 8B B7 ?? ?? ?? ??"), ScanKind::Disp32, 16, 0, true },
            { &o::Pawn, skCrypt("8B 86 40 04 00 00 89 87 40 04 00 00 8B 86 44 04 00 00 89 87 44 04 00 00 8B 86 48 04 00 00"), ScanKind::Disp32, 26, 0, false },
            { &o::ComponentToWorld, skCrypt("4C 8D 3C C3 49 3B DF 74 5D 48 8B 33 4C 8B 96 ?? ?? ?? ??"), ScanKind::Disp32, 15, 0, true },
            { &o::VehicleMovement, skCrypt("F7 40 08 00 00 00 20 0F 85 ?? ?? ?? ?? 48 8B 9F ?? ?? ?? ?? 48 85 DB"), ScanKind::Disp32, 16, 0, false },
            { &o::LastSubmitTime, skCrypt("C6 44 24 30 01 48 8B 07 4C 8B 88 ?? ?? ?? ?? 4C 63 87"), ScanKind::Disp32, 11, 0, true },
            { &o::PlayerState, skCrypt("48 85 C0 0F 85 ?? ?? ?? ?? 48 8B 96 ?? ?? ?? ?? 41 BF"), ScanKind::Disp32, 12, 0, true },
            { &o::MyHUD, skCrypt("57 41 56 41 57 48 83 EC 30 48 8B 99 ?? ?? ?? ?? 8B F2"), ScanKind::Disp32, 12, 0, false },
            { &o::RootComponent, skCrypt("41 8B 04 37 39 05 ?? ?? ?? ?? 0F 8F ?? ?? ?? ?? 48 8B 87 ?? ?? ?? ??"), ScanKind::Disp32, 19, 0, true },
            { &o::ViewTarget, skCrypt("0F 11 87 50 10 00 00 0F 10 48 30 0F 11 8F 60 10"), ScanKind::Disp32, 3, 0, false },
            { &o::PlayerCameraManager, skCrypt("48 83 BB ?? ?? ?? ?? 00 74 29 48 8B CF E8 ?? ?? ?? ?? 84 C0 74 1D 48 8B 8B"), ScanKind::Disp32, 3, 0, true },
            { &o::CameraCacheRotation, skCrypt("48 8B 86 ?? ?? ?? ?? 48 89 45 D0 48 85 C0 74 03 FF 40 08"), ScanKind::Disp32, 3, 0, true },
            { &o::CameraCacheLocation, skCrypt("48 8B 8E ?? ?? ?? ?? 48 89 4D D8 48 8B 8E ?? ?? ?? ?? 48 89 4D E0"), ScanKind::Disp32, 3, 0, true },
            { &o::CameraCacheFOV, skCrypt("F3 0F 10 81 ?? ?? ?? ?? C3 CC CC 48 83"), ScanKind::Disp32, 4, 0, true },
            { &o::SpectatorPawn, skCrypt("F3 0F 10 8E ?? ?? ?? ?? EB 08 F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06"), ScanKind::Disp32, 4, 0, false },
            { &o::CharacterName, skCrypt("48 8B 85 ?? ?? ?? ?? 8B ?? ?? ?? 48 85 C0 74 1C 48 8B 08"), ScanKind::Disp32, 3, 0, true },
            { &o::CharacterClanInfo, skCrypt("48 8B 9D 30 08 00 00 48 8B 85 ?? ?? 00 00 48 89 45 98 48 89 85"), ScanKind::Disp32, 10, 0, false },
            { &o::DefaultFOV, skCrypt("48 8B 85 ?? ?? 00 00 48 89 45 80 48 89 85 98 00 00 00 48 8B 9D"), ScanKind::Disp32, 3, 0, false },
            { &o::CharacterMovement, skCrypt("C6 87 ?? ?? ?? ?? 00 48 8B 8F ?? ?? ?? ?? 48 83 3D ?? ?? ?? ?? 00"), ScanKind::Disp32, 10, 0, true },
            { &o::EquippedWeapons, skCrypt("48 8B 8D ?? ?? ?? ?? 48 33 CC E8 ?? ?? ?? ?? 4C 8D 9C 24"), ScanKind::Disp32, 3, 0, true },
            { &o::CharacterState, skCrypt("44 88 BF ?? ?? ?? ?? 8B 8F ?? ?? ?? ?? 41 B8 FE FF FF FF"), ScanKind::Disp32, 9, 0, false },
            { &o::CurrentAmmoData, skCrypt("44 89 BF 2C 0E 00 00 44 89 BF 68 0E 00 00 44 88 BF 64 0B 00 00"), ScanKind::Disp32, 10, 0, true },
            { &o::FeatureRepObject, skCrypt("48 83 EC 30 48 8B 99 ?? ?? ?? ?? 0F 29 70 E8 0F 28 F1 48 63 81"), ScanKind::Disp32, 7, 0, false },
            { &o::NumAliveTeams, skCrypt("48 63 FA 48 8B D9 48 8B 89 ?? ?? ?? ?? 41 83 F8 10 77 14"), ScanKind::Disp32, 9, 0, false },
            { &o::HeaFlag, skCrypt("40 53 48 83 EC 20 80 B9 ?? ?? ?? ?? 03 48 8B D9 0F 84"), ScanKind::Disp32, 8, 0, false },
            { &o::InventoryFacade, skCrypt("48 83 3D ?? ?? ?? ?? 00 48 8B 8B ?? ?? ?? ?? 75 16"), ScanKind::Disp32, 11, 0, true },
            { &o::InventoryItems, skCrypt("48 8B 8B 30 06 00 00 BD 01 00 00 00 48 85 C9 0F 85 ?? ?? ?? ?? 48 8B BB"), ScanKind::Disp32, 24, 0, true },
            { &o::ItemPackageItems, skCrypt("48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B B9 ?? ?? ?? ?? 41 BE"), ScanKind::Disp32, 13, 0, false },
            { &o::WeaponProcessor, skCrypt("48 8B F9 48 8B 91 ?? ?? ?? ?? 45 33 FF 48 85 D2 0F 84"), ScanKind::Disp32, 6, 0, true },
            { &o::TrajectoryConfig, skCrypt("48 8B 1E 48 8B 03 48 8B B8 ?? ?? ?? ??"), ScanKind::Disp32, 9, 0, false },
            { &o::BlueZonePosition, skCrypt("F3 0F 10 85 ?? ?? ?? ?? 48 63 C8 48 8D 44 24 50"), ScanKind::Disp32, 4, 0, false },
            { &o::SafetyZonePosition, skCrypt("F3 0F 11 44 24 30 F3 0F 10 85 ?? ?? ?? ?? 49 03 0E 48 89 44 24 28"), ScanKind::Disp32, 10, 0, false },
            { &o::Health1, skCrypt("8A 83 3C 0A 00 00 0A C2 88 83 3C 0A 00 00 48 89 9B 80 0A"), ScanKind::Disp32, 2, 0, false },
            { &o::Health2, skCrypt("48 8B 80 ?? ?? ?? ?? 48 85 C0 0F 84 ?? ?? ?? ?? F2 0F 10 80 ?? ?? ?? ?? F2 0F 11 44 24 20 8B 80"), ScanKind::Disp32, 32, 0, false },
            { &o::Health3, skCrypt("8A 88 92 05 00 00 80 E1 01 48 8B 44 24 30 88 48 28 48 8B 07 8B 88"), ScanKind::Disp32, 22, 0, true },
            { &o::Health4, skCrypt("FF 90 80 01 00 00 84 C0 74 10 48 8B 8F ?? ?? ?? ?? 48 85 C9"), ScanKind::Disp32, 13, 0, true },
            { &o::Health5, skCrypt("44 38 B7 25 0A 00 00 0F 85"), ScanKind::Disp32, 3, 0, true },
            { &o::Health6, skCrypt("48 8B B1 20 0A 00 00 45 33 F6"), ScanKind::Disp32, 3, 0, true },
            { &o::GroggyHealth, skCrypt("4C 89 BF A0 14 00 00 48 8D 8F ?? ?? ?? ?? E8"), ScanKind::Disp32, 10, 0, true },
            { &o::TeamNumber, skCrypt("8B 81 ?? ?? ?? ?? 8D 98 ?? ?? ?? ?? 3D ?? ?? ?? ?? 0F 4C"), ScanKind::Disp32, 2, 0, true },
            { &o::BoneArray, skCrypt("48 8B 81 ?? ?? ?? ?? 48 85 C0 74 04 8B 40 ?? C3 C3 CC CC CC 48 8B C4 48 89 58 08"), ScanKind::Disp32, 3, 0, true },
            { &o::BoneCount, skCrypt("48 83 EC 28 48 8B 81 ?? ?? ?? ?? 49 39"), ScanKind::Disp32, 7, 0, false },
            { &o::Eyes, skCrypt("F3 0F 10 8E ?? ?? ?? ?? 41 83 FD 06 7C 0B 45 8A F3"), ScanKind::Disp32, 4, 0, false },
            { &o::GNamesPtr, skCrypt("49 8B ?? ?? B8 ?? ?? ?? ?? 41 F7 ?? 45 8B ?? 45 8D"), ScanKind::Byte, 3, 0, true },
            { &o::ChunkSize, skCrypt("41 69 ?? ?? ?? ?? ?? 44 2B E8 4D 85 C0"), ScanKind::Disp32, 3, 0, true },
            { &o::Minimap, skCrypt("8B B1 ?? ?? ?? ?? 48 8B 81 ?? ?? ?? ?? 33 FF 48 85 C0"), ScanKind::Disp32, 9, 0, false },
            { &o::CurrentMinimapViewScale, skCrypt("4C 89 9B 9C 04 00 00 89 B3 A4 04 00 00 89 B3 A8 04 00 00"), ScanKind::Disp32, 9, 0, false },
            { &o::ScreenSize, skCrypt("4C 89 9B 9C 04 00 00 89 B3 A4 04 00 00 89 B3 A8 04 00 00"), ScanKind::Disp32, 15, 0, false },
            { &o::ScreenPosX, skCrypt("41 8B 87 B8 04 00 00 41 89 86 B8 04 00 00 41 8B 87 BC 04 00 00"), ScanKind::Disp32, 3, 0, false },
            { &o::ScreenPosY, skCrypt("41 8B 87 B8 04 00 00 41 89 86 B8 04 00 00 41 8B 87 BC 04 00 00"), ScanKind::Disp32, 17, 0, false },
            { &o::SelectMinimapSizeIndex, skCrypt("8B 83 C4 05 00 00 89 87 C4 05 00 00 48 8B 83 C8 05 00 00"), ScanKind::Disp32, 15, 0, false },
            { &o::Mesh3P, skCrypt("E8 ?? ?? ?? ?? 83 F8 01 0F 84 ?? ?? ?? ?? 49 8B 8F ?? ?? ?? ??"), ScanKind::Disp32, 17, 0, false },
            { &o::ComponentLocation, skCrypt("40 80 FD 03 0F 84 ?? ?? ?? ?? F3 0F 10 98 ?? ?? ?? ??"), ScanKind::Disp32, 14, 0, true },
            { &o::StaticMesh, skCrypt("41 0F B7 40 53 48 83 EC 20 4C 8B 81 ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, false },
            { &o::WorldToMap, skCrypt("F3 0F 10 A7 ?? ?? ?? ?? F3 0F 11 A4 24 90 00 00 00 F3 0F 10 AF ?? ?? ?? ??"), ScanKind::Disp32, 21, 0, false },
            { &o::LastRenderTimeOnScreen, skCrypt("F3 0F 10 97 ?? ?? ?? ?? F3 0F 10 8F ?? ?? ?? ?? F3 0F 5C CA"), ScanKind::Disp32, 12, 0, true },
            { &o::LastTeamNum, skCrypt("F7 47 08 00 00 00 20 75 30 8B 83 ?? ?? ?? ?? 41 39"), ScanKind::Disp32, 11, 0, true },
            { &o::SpectatedCount, skCrypt("4C 8D 4C 24 30 45 33 C0 41 8B 97 ?? ?? ?? ?? 48 8D 4C 24 40"), ScanKind::Disp32, 11, 0, true },
            { &o::CurrentWeaponIndex, skCrypt("48 83 EC 20 33 FF 4C 8B FA 80 B9 ?? ?? ?? ?? FF"), ScanKind::Disp32, 11, 0, true },
            { &o::WeaponTrajectoryData, skCrypt("48 8D 8F A8 11 00 00 E8 ?? ?? ?? ??"), ScanKind::Disp32, 3, 0, false },
            { &o::TrajectoryGravityZ, skCrypt("44 89 8C 24 6C 10 00 00 48 8B 84 24 68 10 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::FiringAttachPoint, skCrypt("88 87 55 08 00 00 8B 86 ?? ?? ?? ?? 89 87"), ScanKind::Disp32, 8, 0, false },
            { &o::ScopingAttachPoint, skCrypt("0F 28 D6 41 8B D5 E8 ?? ?? ?? ?? 48 8B 8F ?? ?? ?? ??"), ScanKind::Disp32, 14, 0, false },
            { &o::RecoilValueVector, skCrypt("8B 9D ?? ?? ?? ?? 44 8B 85 ?? ?? ?? ?? 48 8D 8D"), ScanKind::Disp32, 9, 0, false },
            { &o::VerticalRecovery, skCrypt("44 89 A7 D0 10 00 00 44 89 A7 D8 10 00 00 44 89 A7 E0 10"), ScanKind::Disp32, 10, 0, false },
            { &o::AttachedItems, skCrypt("88 87 55 08 00 00 8B 86 ?? ?? ?? ?? 89 87"), ScanKind::Disp32, 8, 0, false },
            { &o::Inventory, skCrypt("F7 40 08 00 00 00 20 75 12 48 8B 83 ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, true },
            { &o::InventoryItemCount, skCrypt("48 8B BB ?? ?? ?? ?? 8B B3 ?? ?? ?? ?? 85 F6 0F 85"), ScanKind::Disp32, 9, 0, false },
            { &o::Equipment, skCrypt("F7 40 08 00 00 00 20 75 12 48 8B 83 ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, false },
            { &o::ItemsArray, skCrypt("48 8B 91 ?? ?? ?? ?? 48 8B D9 E8 ?? ?? ?? ?? 48 8B 93 ?? ?? ?? ??"), ScanKind::Disp32, 19, 0, false },
            { &o::ItemID, skCrypt("F2 0F 10 87 ?? ?? ?? ?? F2 0F 11 03 8B 87 ?? ?? ?? ??"), ScanKind::Disp32, 14, 0, true },
            { &o::ItemTable, skCrypt("4C 8B 8D C0 00 00 00 88 44 24 40 48 8B 85 ?? ?? ?? ??"), ScanKind::Disp32, 14, 0, true },
            { &o::PlayerInput, skCrypt("48 8B 05 ?? ?? ?? ?? 48 8B 88 ?? ?? ?? ?? 48 81 C1"), ScanKind::Disp32, 10, 0, false },
            { &o::InputYawScale, skCrypt("41 0F 28 C3 F3 0F 58 83 ?? ?? ?? ?? F3 0F 11 83 ?? ?? ?? ??"), ScanKind::Disp32, 8, 0, false },
            { &o::bIsScoping_CP, skCrypt("38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ??"), ScanKind::Disp32, 10, 0, true },
            { &o::bIsReloading_CP, skCrypt("38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ?? 75 ?? 38 98 ?? ?? ?? ??"), ScanKind::Disp32, 18, 0, true },
            { &o::VehicleFuel, skCrypt("F3 0F 10 91 E4 02 00 00 F3 0F 10 05 ?? ?? ?? ?? 0F 28 CA 0F 54 0D ?? ?? ?? ?? 0F 2F C1 73 15 F3 0F 10 81 E0 02 00 00"), ScanKind::Disp32, 31, 0, false },
            { &o::VehicleFuelMax, skCrypt("F3 0F 10 91 E4 02 00 00 F3 0F 10 05 ?? ?? ?? ?? 0F 28 CA 0F 54 0D"), ScanKind::Disp32, 4, 0, false },
            { &o::VehicleHealth, skCrypt("F3 0F 10 A3 D8 02 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::VehicleHealthMax, skCrypt("89 87 ?? ?? ?? ?? 45 33 E4 44 88 A7 ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, false },
            { &o::SeatIndex, skCrypt("89 B5 C8 03 00 00 48 8B 83 ?? ?? ?? ?? 8B 48 08"), ScanKind::Disp32, 9, 0, false },
            { &o::ActorsForGC, skCrypt("40 88 B7 CB 07 00 00 40 88 B7 D0 07 00 00"), ScanKind::Disp32, 10, 0, false },
            { &o::SafetyZoneRadius, skCrypt("F3 0F 10 93 ?? ?? ?? ?? 48 8D 54 24 20 0F 28 DA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? F3 0F 10 83 ?? ?? ?? ??"), ScanKind::Disp32, 35, 0, false },
            { &o::BlueZoneRadius, skCrypt("F3 0F 10 93 ?? ?? ?? ?? 48 8D 54 24 20 0F 28 DA ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? F3 0F 10 8B ?? ?? ?? ??"), ScanKind::Disp32, 48, 0, false },
            { &o::ControlRotation, skCrypt("85 C0 0F 84 ?? ?? ?? ?? F3 0F 10 90 0C 04 00 00"), ScanKind::Disp32, 12, 0, false },
            { &o::bShowMouseCursor, skCrypt("F6 83 58 06 00 00 02 75 28 48 8B"), ScanKind::Disp32, 2, 0, false },
            { &o::TimeSeconds, skCrypt("F3 0F 10 81 ?? ?? ?? ?? F3 0F 5C 81 ?? ?? ?? ?? 0F 28 C8 0F 5A C1"), ScanKind::Disp32, 4, 0, false },
            { &o::AccountId, skCrypt("F3 0F 11 87 10 08 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::SquadMemberIndex, skCrypt("F7 43 08 00 00 00 20 75 0B 8B 87 ?? ?? ?? ??"), ScanKind::Disp32, 11, 0, false },
            { &o::PlayerStatistics, skCrypt("48 8B 8F 10 0A 00 00 48 85 C9"), ScanKind::Disp32, 3, 0, false },
            { &o::DamageDealtOnEnemy, skCrypt("4C 89 81 ?? ?? ?? ?? 48 89 B1 ?? ?? ?? ?? 44 89 81 0C 08"), ScanKind::Disp32, 10, 0, false },
            { &o::SurvivalLevel, skCrypt("F3 0F 11 B6 CC 0C 00 00 F3 0F 5C F7 48"), ScanKind::Disp32, 4, 0, false },
            { &o::ping, skCrypt("F3 0F 58 97 ?? ?? ?? ?? F3 0F 11 97 ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, false },
            { &o::Durability, skCrypt("48 8B 06 48 8B CE FF 90 30 01 00 00 F3 0F 10 80 ?? ?? ?? ??"), ScanKind::Disp32, 14, 0, false },
            { &o::ElapsedCookingTime, skCrypt("4C 89 A7 ?? ?? ?? ?? 44 89 A7"), ScanKind::Disp32, 3, 0, false },
            { &o::LeanLeftAlpha_CP, skCrypt("F3 0F 11 87 ?? ?? ?? ?? F3 0F 11 97 ?? ?? ?? ?? 44 88 A7"), ScanKind::Disp32, 12, 0, false },
            { &o::LeanRightAlpha_CP, skCrypt("FF 90 ?? ?? ?? ?? 48 8B 08 48 89 8F ?? ?? ?? ??"), ScanKind::Disp32, 12, 0, false },
            { &o::VehicleRiderComponent, skCrypt("89 8F 20 19 00 00 48 C7 87"), ScanKind::Disp32, 2, 0, false },
            { &o::ReplicatedMovement, skCrypt("48 8B 4D 58 E8 ?? ?? ?? ?? 48 83 C4 20 5D C3 48 8B 8A ?? ?? ?? ??"), ScanKind::Disp32, 17, 0, false },
            { &o::MatchId, skCrypt("BE 00 00 00 20 85 70 08 0F 85 ?? ?? ?? ?? 48 8B 97 ?? ?? ?? ??"), ScanKind::Disp32, 17, 0, false },
            { &o::PlayerName, skCrypt("44 39 BB 28 04 00 00 0F 8E ?? ?? ?? ?? 33 F6 48 8B 15 ?? ?? ?? ?? 48 8B 83 ?? ?? ?? ??"), ScanKind::Disp32, 25, 0, false },
            { &o::LocalPlayers, skCrypt("48 83 3D ?? ?? ?? ?? 00 74 ?? 48 8B 0D ?? ?? ?? ?? 48 8B 01 FF 50"), ScanKind::Rip, 13, 17, false },
            { &o::ObjID, skCrypt("41 8B ?? ?? BB ?? ?? ?? ?? 33 ?? 8B ?? C1 ?? 17"), ScanKind::Byte, 3, 0, true },
            { &o::SurvivalTier, skCrypt("8B 86 40 04 00 00 89 87 40 04 00 00 8B 86 44 04 00 00"), ScanKind::Disp32, 2, 0, false },
            { &o::Durabilitymax, skCrypt("8B 82 14 03 00 00 89 81 14 03 00 00 8B 82 18 03 00 00"), ScanKind::Disp32, 2, 0, false },
            { &o::BallisticCurve, skCrypt("48 8B B8 28 00 00 00 E8 ?? ?? ?? ?? 48 2B E0 33 C9"), ScanKind::Disp32, 3, 0, false },
            { &o::FloatCurves, skCrypt("5D C3 CC 48 8B 8A 38 00 00 00 E9"), ScanKind::Disp32, 6, 0, false },
            { &o::WeaponConfig_WeaponClass, skCrypt("90 49 8B 06 4C 8B 90 78 08 00 00"), ScanKind::Disp32, 12, 0, false },
            { &o::ControlRotation_CP, skCrypt("80 3B 00 0F 85 ?? ?? ?? ?? 48 8B 8F C8 0B 00 00"), ScanKind::Disp32, 12, 0, false },
            { &o::SafetyZoneRadius, skCrypt("F3 0F 10 ?? 14 01 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::BlueZoneRadius, skCrypt("F3 0F 10 ?? 18 01 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::TimeSeconds, skCrypt("F3 0F 10 83 ?? ?? 00 00 F3 0F 58 83 ?? ?? 00 00"), ScanKind::Disp32, 4, 0, false },
            { &o::VerticalRecovery, skCrypt("44 89 A7 D0 10 00 00 4C 89 A7 D8 10 00 00 44 89 A7 E0 10"), ScanKind::Disp32, 10, 0, false }
        };

        for (const auto& spec : specs) {
            if (spec.Required) {
                ++g_LastReport.RequiredTotal;
            }

            uint64_t value = 0;
            if (!ResolvePattern(sections, base, spec, value) || !IsUsableValue(spec.Kind, value)) {
                continue;
            }

            ++g_LastReport.Found;
            if (spec.Required) {
                ++g_LastReport.RequiredFound;
            }

            StageOffset(spec.Target, value);
        }
    }

    void ApplySpecialPatterns(const std::vector<LoadedSection>& sections, uint64_t base) {
        namespace o = telemetry_config::offsets;

        const uint64_t spoof = FindPattern(sections, skCrypt("48 85 C0 74 09 83 38 00 C6 45 77 01 77 04 C6 45 77"));
        if (spoof && spoof > base) {
            StageOffset(&o::SPOOFCALL_GADGET, spoof - base);
            ++g_LastReport.Found;
        }

        const uint64_t lineTrace = FindPattern(sections, skCrypt("E8 ?? ?? ?? ?? 48 8B ?? ?? ?? ?? ?? 48 85 ?? 74 ?? 48 8B ?? ?? ?? ?? 44 8A ?? ?? 00 00 00"));
        if (lineTrace) {
            int32_t rel = 0;
            if (ReadValue(sections, lineTrace + 1, rel)) {
                const uint64_t target = lineTrace + 5 + rel;
                if (target > base) {
                    StageOffset(&o::LineTraceSingle, target - base);
                    ++g_LastReport.Found;
                }
            }
        }

        const uint64_t hook = FindPattern(sections, skCrypt("48 8D 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 01 FF 50"));
        if (hook) {
            int32_t rel = 0;
            if (ReadValue(sections, hook + 3, rel)) {
                const uint64_t target = hook + 7 + rel;
                if (target > base) {
                    StageOffset(&o::HOOK, target - base);
                    ++g_LastReport.Found;
                }
            }
        }

        const uint64_t hook2 = FindPattern(sections, skCrypt("41 3B D8 7D 7C 48 89 54 24 20 4D 85 C9 75 13 B9"));
        if (hook2 && hook2 + 15 > base) {
            StageOffset(&o::HOOK_TWO, (hook2 + 15) - base);
            ++g_LastReport.Found;
        }

        const uint64_t nameDec = FindPattern(sections, skCrypt("41 8B ?? ?? BB ?? ?? ?? ?? 33 ?? 8B ?? C1 ?? 01"));
        if (nameDec) {
            uint32_t key1 = 0;
            uint8_t rval = 0;
            if (ReadValue(sections, nameDec + 5, key1) && ReadValue(sections, nameDec + 15, rval)) {
                StageOffset(&o::DecryptNameIndexXorKey1, key1);
                StageOffset(&o::DecryptNameIndexRval, rval);
                StageOffset(&o::DecryptNameIndexRor, rval);
                g_LastReport.Found += 3;
            }
        }

        const uint64_t name23 = FindPattern(sections, skCrypt("BE ?? ?? ?? ?? 41 23 C6 C1 E1 ?? 0B C1 33 D0 41 BF"));
        if (name23) {
            uint32_t key3 = 0;
            uint8_t dval = 0;
            if (ReadValue(sections, name23 + 1, key3) && ReadValue(sections, name23 + 10, dval) && dval < 32) {
                StageOffset(&o::DecryptNameIndexXorKey3, key3);
                StageOffset(&o::DecryptNameIndexDval, dval);
                StageOffset(&o::DecryptNameIndexSval, 32U - static_cast<uint32_t>(dval));
                g_LastReport.Found += 3;
            }

            const uint64_t key2Addr = FindPatternRange(name23, 128, skCrypt("41 BF ?? ?? ?? ?? 41 33 D7"));
            if (key2Addr) {
                uint32_t key2 = 0;
                if (ReadRemote(key2Addr + 2, key2)) {
                    StageOffset(&o::DecryptNameIndexXorKey2, key2);
                    ++g_LastReport.Found;
                }
            }
        }

        const uint64_t health12 = FindPattern(sections, skCrypt("C7 45 B0 ?? ?? ?? ?? 33 D2 C7 45 B4 ?? ?? ?? ?? C7 45 B8 ?? ?? ?? ??"));
        if (health12) {
            uint32_t key1 = 0;
            uint32_t key2 = 0;
            if (ReadValue(sections, health12 + 12, key1)) {
                StageOffset(&o::HealthKey1, key1);
                ++g_LastReport.Found;
            }
            if (ReadValue(sections, health12 + 19, key2)) {
                StageOffset(&o::HealthKey2, key2);
                ++g_LastReport.Found;
            }
            g_PendingHealthKeyRefresh = true;
        }
    }
}

bool ApplyRuntimeScan(uint64_t baseAddress) {
    if (!baseAddress) {
#ifdef _DEBUG
        std::cout << skCrypt("[OFFSET-MODE] static offsets active: missing module base") << std::endl;
#endif
        return false;
    }
    if (g_LastScannedBase == baseAddress && g_LastReport.Scanned) {
#ifdef _DEBUG
        std::cout << skCrypt("[OFFSET-MODE] ")
                  << (g_LastReport.AppliedRuntimeOffsets ? skCrypt("signature/runtime offsets active") : skCrypt("static offsets active"))
                  << skCrypt(" (cached scan result)") << std::endl;
#endif
        return g_LastReport.AppliedRuntimeOffsets;
    }

    g_LastReport = {};
    g_LastScannedBase = 0;
    g_Pending64.clear();
    g_Pending32.clear();
    g_PendingHealthKeyRefresh = false;

    std::vector<LoadedSection> sections;
    if (!LoadExecutableSections(baseAddress, sections)) {
#ifdef _DEBUG
        std::cout << skCrypt("[OFFSET-MODE] static offsets active: executable sections unreadable") << std::endl;
#endif
        return false;
    }

    ApplyPatternSpecs(sections, baseAddress);
    ApplySpecialPatterns(sections, baseAddress);

    if (!ValidateRuntimeCandidates(baseAddress)) {
        g_LastReport.Scanned = true;
        g_LastReport.AppliedRuntimeOffsets = false;
        g_LastScannedBase = baseAddress;
        g_Pending64.clear();
        g_Pending32.clear();
        g_PendingHealthKeyRefresh = false;
#ifdef _DEBUG
        std::cout << skCrypt("[OFFSET-MODE] static offsets active: signature candidates rejected")
                  << skCrypt(" found=") << g_LastReport.Found
                  << skCrypt(" required=") << g_LastReport.RequiredFound
                  << skCrypt("/") << g_LastReport.RequiredTotal << std::endl;
#endif
        return false;
    }

    ApplyPendingOffsets();

    g_LastReport.Scanned = true;
    g_LastReport.AppliedRuntimeOffsets = g_LastReport.Applied > 0;
    g_LastScannedBase = baseAddress;
    g_Pending64.clear();
    g_Pending32.clear();
    g_PendingHealthKeyRefresh = false;

#ifdef _DEBUG
    std::cout << skCrypt("[OFFSET-MODE] signature/runtime offsets active")
              << skCrypt(" applied=") << g_LastReport.Applied
              << skCrypt(" found=") << g_LastReport.Found
              << skCrypt(" required=") << g_LastReport.RequiredFound
              << skCrypt("/") << g_LastReport.RequiredTotal << std::endl;
#endif

    return g_LastReport.AppliedRuntimeOffsets;
}

void InvalidateRuntimeScanCache(uint64_t baseAddress) {
    if (baseAddress == 0 || g_LastScannedBase == baseAddress) {
        g_LastScannedBase = 0;
        g_LastReport.Scanned = false;
    }
}

const RuntimeScanReport& GetLastReport() {
    return g_LastReport;
}

} // namespace telemetryRuntimeOffsets
