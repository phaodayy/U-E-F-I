#include "Scanner.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>
#include "Decrypt.h"
#include <Common/Offset.h>

// ================================================================
//  SCANNER - Scan tat ca offset dua tren UWorld + GNames da co
//  Logic:
//    1. XenuineDecrypt / UWorld / GNames: dung signature scan
//    2. Cac offset con lai: doc truc tiep tu memory chain
//       (UWorld -> LocalPlayer -> PlayerController -> Pawn -> Mesh...)
//       Hoat dong nhu 1 "verify + detect offset moi" tu dong
// ================================================================

namespace Scanner {

    // ============================================================
    //  Helper: doc pointer hop le (co the encrypted)
    // ============================================================
    static uint64_t ReadXe(uint64_t addr) {
        uint64_t raw = mem.Read<uint64_t>(addr);
        return Decrypt::Xe(raw);
    }

    static bool IsValidPtr(uint64_t ptr) {
        return ptr >= 0x100000000ULL && ptr < 0x00007FFFFFFFFFFFULL;
    }

    // ============================================================
    //  Helper: in ket qua so sanh offset
    // ============================================================
    static void CheckResult(const char* name, uint64_t scanned, uint64_t current = 0) {
        if (scanned == 0) {
            Utils::Log(2, "[SCANNER]   %-28s : KHONG TIM THAY", name);
        } else if (current == 0) {
            Utils::Log(1, "[SCANNER]   %-28s : 0x%llX", name, scanned);
        } else if (scanned == current) {
            Utils::Log(1, "[SCANNER]   %-28s : 0x%llX  (KHOP - OK)", name, scanned);
        } else {
            Utils::Log(0, "[SCANNER]   %-28s : 0x%llX  (KHAC BIET! Hien tai: 0x%llX)", name, scanned, current);
        }
    }

    // ============================================================
    //  Lay RVA tu IP-relative modrm [rip + disp32]
    // ============================================================
    uint64_t GetRelativeAddress(uint64_t instruction_addr, int offset_pos, int instruction_size) {
        int32_t relative_offset = mem.Read<int32_t>(instruction_addr + offset_pos);
        return instruction_addr + instruction_size + relative_offset;
    }

    bool IsValidPointer(uint64_t ptr) {
        return ptr >= 0x100000000ULL && ptr < 0x00007FFFFFFFFFFFULL && (ptr % 8 == 0);
    }

    // ============================================================
    //  Signature scan: XenuineDecrypt
    // ============================================================
    uint64_t FindXenuineDecrypt() {
        const char* pattern = "48 89 0D ? ? ? ? 48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 05";
        uint64_t base = GameData.GameBase;  // Da xac dinh dung process
        uint64_t size = GameData.GameSize > 0 ? GameData.GameSize : 0x10000000;

        uint64_t addr = mem.FindSignature(pattern, base, base + size);
        if (addr) return addr - base;
        return 0;
    }

    // ============================================================
    //  Signature scan: UWorld offset (RVA trong module)
    // ============================================================
    uint64_t FindUWorld() {
        const char* patterns[] = {
            "48 8B 05 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74 06 48 8B 49 70",
            "48 8B 05 ? ? ? ? 48 85 C0 74 08 48 8B 48 70 48 8B 01",
            "48 8B 0D ? ? ? ? 48 85 C9 74 08 48 8B 41 70 48 8B 00"
        };
        uint64_t base = GameData.GameBase;  // Da xac dinh dung process
        uint64_t size = GameData.GameSize > 0 ? GameData.GameSize : 0x10000000;

        for (auto pattern : patterns) {
            uint64_t addr = mem.FindSignature(pattern, base, base + size);
            if (addr) {
                uint64_t ptr = GetRelativeAddress(addr, 3, 7);
                uint64_t off = ptr - base;
                uint64_t enc = mem.Read<uint64_t>(ptr);
                uint64_t dec = Decrypt::Xe(enc);
                if (IsValidPointer(dec)) {
                    uint64_t level = Decrypt::Xe(mem.Read<uint64_t>(dec + Offset::CurrentLevel));
                    if (IsValidPointer(level)) return off;
                }
            }
        }
        return 0;
    }

    // ============================================================
    //  Signature scan: GNames offset
    // ============================================================
    uint64_t FindGNames() {
        const char* patterns[] = {
            "48 8B 05 ? ? ? ? 48 8B 0C C8",
            "48 8B 15 ? ? ? ? 48 85 D2 74 07",
            "48 8B 0D ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 8B 01 FF 90"
        };
        uint64_t base = GameData.GameBase;  // Da xac dinh dung process
        uint64_t size = GameData.GameSize > 0 ? GameData.GameSize : 0x10000000;

        for (auto pattern : patterns) {
            uint64_t addr = mem.FindSignature(pattern, base, base + size);
            if (addr) {
                uint64_t ptr = GetRelativeAddress(addr, 3, 7);
                uint64_t off = ptr - base;
                uint64_t enc = mem.Read<uint64_t>(ptr);
                uint64_t dec1 = Decrypt::Xe(enc);
                if (IsValidPointer(dec1)) {
                    uint64_t enc2 = mem.Read<uint64_t>(dec1 + Offset::GNamesPtr);
                    uint64_t dec2 = Decrypt::Xe(enc2);
                    if (IsValidPointer(dec2)) return off;
                }
            }
        }
        return 0;
    }

    // ============================================================
    //  SCAN CHUOI POINTER TU UWORLD
    //  Dung UWorld + Decrypt da co san -> tim offset moi
    // ============================================================

    struct ChainResult {
        uint64_t UWorld         = 0;
        uint64_t GameInstance   = 0;
        uint64_t LocalPlayer    = 0;
        uint64_t PlayerController = 0;
        uint64_t AcknowledgedPawn = 0;
        uint64_t PlayerCameraManager = 0;
        uint64_t GameState      = 0;
        uint64_t CurrentLevel   = 0;
        uint64_t ActorArray     = 0;

        // Pawn
        uint64_t Mesh           = 0;
        uint64_t RootComponent  = 0;
        uint64_t CharacterMovement = 0;
        uint64_t PlayerState    = 0;
        uint64_t WeaponProcessor = 0;

        // Camera
        uint64_t CameraCacheLocation = 0;
        uint64_t CameraCacheRotation = 0;
        uint64_t CameraCacheFOV  = 0;

        // PhysX
        uint64_t PhysicsScene   = 0;
        uint64_t mPhysXScene    = 0;
    };

    // ---------------------------------------------------------------
    //  Scan GameInstance offset (UWorld + offset -> GameInstance)
    //  Brute-force range 0x100 - 0x400 step 8
    // ---------------------------------------------------------------
    static uint64_t ScanGameInstance(uint64_t uworld) {
        for (uint64_t off = 0x100; off <= 0x400; off += 8) {
            uint64_t v = ReadXe(uworld + off);
            if (!IsValidPtr(v)) continue;
            // Kiem tra LocalPlayers array tai GameInstance + 0x70
            uint64_t lp = mem.Read<uint64_t>(v + Offset::LocalPlayer);
            if (!IsValidPtr(lp)) continue;
            uint64_t lp0 = Decrypt::Xe(mem.Read<uint64_t>(lp));
            if (!IsValidPtr(lp0)) continue;
            // PlayerController phai hop le
            uint64_t pc = Decrypt::Xe(mem.Read<uint64_t>(lp0 + Offset::PlayerController));
            if (IsValidPtr(pc)) return off;
        }
        return 0;
    }

    // ---------------------------------------------------------------
    //  Scan CurrentLevel offset (UWorld + offset -> Level)
    // ---------------------------------------------------------------
    static uint64_t ScanCurrentLevel(uint64_t uworld) {
        for (uint64_t off = 0x800; off <= 0x1200; off += 8) {
            uint64_t v = ReadXe(uworld + off);
            if (!IsValidPtr(v)) continue;
            // Actors array tai Level + 0x118
            uint64_t actRef = Decrypt::Xe(mem.Read<uint64_t>(v + Offset::Actors));
            if (IsValidPtr(actRef)) return off;
        }
        return 0;
    }

    // ---------------------------------------------------------------
    //  Scan GameState offset (UWorld + offset -> GameState)
    // ---------------------------------------------------------------
    static uint64_t ScanGameState(uint64_t uworld) {
        for (uint64_t off = 0x50; off <= 0x200; off += 8) {
            uint64_t v = ReadXe(uworld + off);
            if (!IsValidPtr(v)) continue;
            // PlayerArray tai GameState + 0x418
            uint64_t pa = mem.Read<uint64_t>(v + Offset::PlayerArray);
            if (!IsValidPtr(pa)) continue;
            int cnt = mem.Read<int>(v + Offset::PlayerArray + 8);
            if (cnt > 0 && cnt <= 200) return off;
        }
        return 0;
    }

    // ---------------------------------------------------------------
    //  Scan cam ket qua Camera: CameraCacheLocation/Rotation/FOV
    //  Tim trong PlayerCameraManager
    // ---------------------------------------------------------------
    struct Vector3_Scanner { float x, y, z; };
    static void ScanCameraOffsets(uint64_t camMgr, uint64_t& locOff, uint64_t& rotOff, uint64_t& fovOff) {
        const int CHUNK = 0x1300;
        std::vector<uint8_t> buf(CHUNK, 0);
        mem.Read(camMgr, buf.data(), CHUNK);

        auto readF1 = [&](uint64_t off) -> float {
            float v = 0;
            if (off + 4 <= (uint64_t)CHUNK) memcpy(&v, buf.data() + off, 4);
            return v;
        };
        auto readF3 = [&](uint64_t off) -> Vector3_Scanner {
            Vector3_Scanner v{ 0, 0, 0 };
            if (off + 12 <= (uint64_t)CHUNK) memcpy(&v, buf.data() + off, 12);
            return v;
        };

        CameraData cam = GameData.Camera;
        float scanX = cam.Location.X, scanY = cam.Location.Y, scanZ = cam.Location.Z;
        float scanPitch = cam.Rotation.Pitch, scanYaw = cam.Rotation.Yaw, scanFov = cam.FOV;

        locOff = rotOff = fovOff = 0;

        for (int64_t off = 0x1200 - 4; off >= 0xF00; off -= 4) {
            float fov = readF1((uint64_t)off);
            if (scanFov > 40.0f && scanFov < 130.0f && fabsf(fov - scanFov) < 0.5f) {
                fovOff = (uint64_t)off;
                break;
            }
        }

        if (fovOff != 0) {
            for (int64_t delta = 0x50; delta >= 0x10; delta -= 4) {
                uint64_t testOff = fovOff - (uint64_t)delta;
                if (testOff < 0x100) continue;
                auto v = readF3(testOff);
                if (scanX != 0 && fabsf(v.x - scanX) < 200.0f
                               && fabsf(v.y - scanY) < 200.0f
                               && fabsf(v.z - scanZ) < 200.0f) {
                    locOff = testOff; break;
                }
            }
            for (int64_t delta = 0x40; delta >= 0x04; delta -= 4) {
                uint64_t testOff = fovOff - (uint64_t)delta;
                if (testOff < 0x100) continue;
                auto v = readF3(testOff);
                if (scanPitch != 0 && fabsf(v.x - scanPitch) < 5.0f
                                   && fabsf(v.y - scanYaw) < 5.0f) {
                    rotOff = testOff; break;
                }
            }
        }

        if (fovOff == 0 || locOff == 0 || rotOff == 0) {
            Utils::Log(0, "[SCANNER]   Camera: Fallback sang quet toan bo (fov=%d loc=%d rot=%d)",
                fovOff!=0, locOff!=0, rotOff!=0);
            if (locOff == 0) {
                for (uint64_t off = 0x400; off + 12 <= (uint64_t)CHUNK; off += 4) {
                    auto v = readF3(off);
                    if (scanX != 0 && fabsf(v.x - scanX) < 100.0f &&
                        fabsf(v.y - scanY) < 100.0f && fabsf(v.z - scanZ) < 100.0f) {
                        locOff = off; break;
                    }
                }
            }
            if (rotOff == 0) {
                for (uint64_t off = 0x400; off + 8 <= (uint64_t)CHUNK; off += 4) {
                    auto v = readF3(off);
                    if (scanPitch != 0 && fabsf(v.x - scanPitch) < 2.0f &&
                        fabsf(v.y - scanYaw) < 2.0f) {
                        rotOff = off; break;
                    }
                }
            }
            if (fovOff == 0) {
                for (uint64_t off = 0xE00; off + 4 <= (uint64_t)CHUNK; off += 4) {
                    float fov = readF1(off);
                    if (scanFov > 40.0f && scanFov < 130.0f && fabsf(fov - scanFov) < 0.5f) {
                        fovOff = off; break;
                    }
                }
            }
        }
    }

    static void ScanPhysX(uint64_t uworld, uint64_t& psOff, uint64_t& mpsOff) {
        psOff = mpsOff = 0;
        struct Cand { uint64_t psOff; uint64_t mpsOff; uint32_t ts; int score; };
        std::vector<Cand> cands;

        for (uint64_t off = 0x700; off <= 0xD00; off += 8) {
            uint64_t ps = mem.Read<uint64_t>(uworld + off);
            if (!IsValidPtr(ps)) continue;

            for (uint64_t iOff = 0x40; iOff <= 0x300; iOff += 8) {
                uint64_t mps = mem.Read<uint64_t>(ps + iOff);
                if (!IsValidPtr(mps)) continue;

                const uint64_t TS_OFFS[] = { 0x3B90, 0x3B8C, 0x3BC0, 0x3B94 };
                uint32_t bestTs = 0;
                for (auto tsOff : TS_OFFS) {
                    uint32_t ts = mem.Read<uint32_t>(mps + tsOff);
                    if (ts > 100 && ts < 999999999 && ts != 0x3F800000) {
                        bestTs = ts; break;
                    }
                }
                if (bestTs == 0) continue;

                uint64_t p0 = mem.Read<uint64_t>(mps + 0x3B78);
                uint64_t p1 = mem.Read<uint64_t>(mps + 0x3B78 + 0x30);
                if (!IsValidPtr(p0)) continue;

                Cand c;
                c.psOff = off;
                c.mpsOff = iOff;
                c.ts = bestTs;
                c.score = 1 + (IsValidPtr(p1) ? 2 : 0) + (int)(bestTs / 100000);
                cands.push_back(c);
            }
        }

        if (!cands.empty()) {
            std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) { return a.score > b.score; });
            psOff  = cands[0].psOff;
            mpsOff = cands[0].mpsOff;
        }
    }

    void ScanAll() {
        Utils::Log(1, "[SCANNER] ====================================================");
        Utils::Log(1, "[SCANNER]  BAT DAU SCAN - Dung UWorld + Decrypt co san");
        Utils::Log(1, "[SCANNER] ====================================================");

        if (GameData.PID == 0 || !IsValidPtr(GameData.GameBase)) {
            Utils::Log(2, "[SCANNER] LOI: Game chua chay hoac GameBase khong hop le!");
            return;
        }
        if (!IsValidPtr(GameData.UWorld)) {
            Utils::Log(2, "[SCANNER] LOI: UWorld chua san sang (0x%llX)!", GameData.UWorld);
            return;
        }

        uint64_t UW = GameData.UWorld;
        Utils::Log(1, "[SCANNER] UWorld = 0x%llX | Map = %s", UW, GameData.MapName.c_str());

        // BLOCK 1: Signature scan
        CheckResult("XenuineDecrypt", FindXenuineDecrypt(), Offset::XenuineDecrypt);
        CheckResult("UWorld",         FindUWorld(),         Offset::UWorld);
        CheckResult("GNames",         FindGNames(),         Offset::GNames);

        // BLOCK 2: Scan tu UWorld chain
        uint64_t scGI_off = ScanGameInstance(UW);
        CheckResult("GameInstance", scGI_off, Offset::GameInstance);
        uint64_t GameInstance = (scGI_off != 0) ? ReadXe(UW + scGI_off) : GameData.GameInstance;

        uint64_t scCL_off = ScanCurrentLevel(UW);
        CheckResult("CurrentLevel", scCL_off, Offset::CurrentLevel);

        uint64_t scGS_off = ScanGameState(UW);
        CheckResult("GameState", scGS_off, Offset::GameState);

        // BLOCK 3: LocalPlayer -> Pawn chain
        uint64_t LocalPlayers = mem.Read<uint64_t>(GameInstance + Offset::LocalPlayer);
        uint64_t LocalPlayer  = IsValidPtr(LocalPlayers) ? Decrypt::Xe(mem.Read<uint64_t>(LocalPlayers)) : 0;
        
        if (IsValidPtr(LocalPlayer)) {
            uint64_t scPC = 0;
            for (uint64_t off = 0x28; off <= 0x60; off += 8) {
                uint64_t v = Decrypt::Xe(mem.Read<uint64_t>(LocalPlayer + off));
                if (IsValidPtr(v) && IsValidPtr(Decrypt::Xe(mem.Read<uint64_t>(v + Offset::AcknowledgedPawn)))) {
                    scPC = off; break;
                }
            }
            CheckResult("PlayerController", scPC, Offset::PlayerController);

            uint64_t PC = scPC ? Decrypt::Xe(mem.Read<uint64_t>(LocalPlayer + scPC)) : 0;
            if (IsValidPtr(PC)) {
                uint64_t scAP = 0;
                for (uint64_t off = 0x490; off <= 0x520; off += 8) {
                    uint64_t v = Decrypt::Xe(mem.Read<uint64_t>(PC + off));
                    if (IsValidPtr(v) && IsValidPtr(Decrypt::Xe(mem.Read<uint64_t>(v + Offset::Mesh)))) {
                        scAP = off; break;
                    }
                }
                CheckResult("AcknowledgedPawn", scAP, Offset::AcknowledgedPawn);

                uint64_t Pawn = scAP ? Decrypt::Xe(mem.Read<uint64_t>(PC + scAP)) : 0;
                if (IsValidPtr(Pawn)) {
                    uint64_t scMesh = 0;
                    for (uint64_t off = 0x450; off <= 0x600; off += 8) {
                        uint64_t v = mem.Read<uint64_t>(Pawn + off);
                        if (IsValidPtr(v) && IsValidPtr(mem.Read<uint64_t>(v + Offset::AnimScriptInstance))) {
                            scMesh = off; break;
                        }
                    }
                    CheckResult("Mesh", scMesh, Offset::Mesh);

                    uint64_t scRC = 0;
                    for (uint64_t off = 0x330; off <= 0x420; off += 8) {
                        uint64_t v = Decrypt::Xe(mem.Read<uint64_t>(Pawn + off));
                        if (IsValidPtr(v) && mem.Read<float>(v + Offset::ComponentLocation) != 0) {
                            scRC = off; break;
                        }
                    }
                    CheckResult("RootComponent", scRC, Offset::RootComponent);
                }
            }
        }

        // BLOCK 4: PhysX - Skip scanning as we use static offset
        /*
        uint64_t scPS_off = 0, scMPS_off = 0;
        ScanPhysX(UW, scPS_off, scMPS_off);
        CheckResult("PhysicsScene",  scPS_off,  Offset::PhysicsScene);
        CheckResult("mPhysXScene",   scMPS_off, Offset::mPhysXScene);
        */

        Utils::Log(1, "[SCANNER] ====================================================");
        Utils::Log(1, "[SCANNER]  HOAN TAT SCAN OFFSET");
        Utils::Log(1, "[SCANNER] ====================================================");
    }

} // namespace Scanner
