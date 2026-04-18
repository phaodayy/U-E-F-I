#include "context.hpp"
#include "offsets.hpp"
#include "pubg_decrypt.hpp"
#include "scanner.hpp"
#include "fname.hpp"
#include "Common/Data.h"
#include "../overlay/overlay_menu.hpp"
#include <iostream>
#include <algorithm>
#include <set>
#include <array>
#include <utility>

using namespace PubgOffsets;
using namespace PubgDecrypt;
using namespace PubgMemory;

// Globals
FGameData GameData; 
uint64_t G_UWorld = 0, G_GameInstance = 0, G_PersistentLevel = 0, G_LocalPlayer = 0, G_PlayerController = 0, G_LocalPawn = 0, G_LocalHUD = 0, G_GameState = 0;
bool G_IsMenuOpen = false;
Vector3 G_CameraLocation = { 0, 0, 0 }, G_CameraRotation = { 0, 0, 0 }, G_LocalPlayerPos = { 0, 0, 0 }, G_LocalPlayerVelocity = { 0, 0, 0 };
uint64_t G_LastScanTime = 0;
RadarData G_Radar;
std::vector<PlayerData> G_Players;
std::vector<ItemData> CachedItems;
float G_CamFOV = 103.0f;

namespace {
    constexpr float kOverlayVisibilityThreshold = 0.05f;

    struct OverlayBoneIndices {
        int head = 6;
        int neck = 5;
        int chest = 4;
        int pelvis = 1;
        int leftShoulder = 88;
        int leftElbow = 89;
        int leftHand = 90;
        int rightShoulder = 115;
        int rightElbow = 116;
        int rightHand = 117;
        int leftThigh = 172;
        int leftKnee = 173;
        int leftFoot = 174;
        int rightThigh = 178;
        int rightKnee = 179;
        int rightFoot = 180;
    };

    PlayerGender NormalizeGender(uint8_t genderValue) {
        return (genderValue == static_cast<uint8_t>(PlayerGender::Female)) ? PlayerGender::Female : PlayerGender::Male;
    }

    OverlayBoneIndices GetOverlayBoneIndices(PlayerGender gender) {
        OverlayBoneIndices idx{};
        if (gender != PlayerGender::Female) {
            return idx;
        }

        idx.leftShoulder = 95;
        idx.leftElbow = 96;
        idx.leftHand = 97;
        idx.rightShoulder = 122;
        idx.rightElbow = 123;
        idx.rightHand = 124;
        idx.leftThigh = 180;
        idx.leftKnee = 183;
        idx.leftFoot = 181;
        idx.rightThigh = 186;
        idx.rightKnee = 189;
        idx.rightFoot = 187;
        return idx;
    }

    bool IsSlateVisible(uint8_t visibilityValue) {
        return visibilityValue <= 2;
    }

    bool IsWorldMapWidgetClass(const std::string& className) {
        return className.find("WorldMapWidget") != std::string::npos ||
            className.find("NewWorldMapWidget") != std::string::npos ||
            className.find("MapGrid") != std::string::npos;
    }

    float GetMapWorldSize(const std::string& mapName) {
        static const std::array<std::pair<const char*, float>, 15> kMapSizes = {{
            {"Tiger_Main", 408000.0f},
            {"Kiki_Main", 408000.0f},
            {"Desert_Main", 408000.0f},
            {"Range_Main", 101175.0f},
            {"Summerland_Main", 101175.0f},
            {"Italy_Main", 101175.0f},
            {"Baltic_Main", 406372.0f},
            {"Neon_Main", 408000.0f},
            {"Heaven_Main", 101175.0f},
            {"Savage_Main", 202387.5f},
            {"DihorOtok_Main", 408000.0f},
            {"Chimera_Main", 153003.0f},
            {"Boardwalk_Main", 51420.0f},
            {"Narrows_Main", 51420.0f},
            {"Pinnacle_Main", 51420.0f}
        }};

        for (const auto& entry : kMapSizes) {
            if (mapName.find(entry.first) != std::string::npos) {
                return entry.second;
            }
        }
        return 406372.0f;
    }

    void ResolveWidgetRect(float left, float top, float width, float height, float alignX, float alignY,
        float screenWidth, float screenHeight, float& outX, float& outY) {
        float x = left;
        float y = top;

        // Canvas slot can encode anchored values as negatives from right/bottom.
        if (left < 0.0f) {
            x = screenWidth + left - width;
        }
        if (top < 0.0f) {
            y = screenHeight + top - height;
        }

        x -= width * alignX;
        y -= height * alignY;

        outX = x;
        outY = y;
    }

    int NormalizeTeamId(int team) {
        if (team >= 100000) {
            team -= 100000;
        }
        if (team < 0 || team > 1000) {
            return 0;
        }
        return team;
    }
}

namespace PubgContext {

    bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        return PubgMemory::ReadMemory(src, dest, size);
    }

    uint64_t GetBaseAddress() { return PubgMemory::g_BaseAddress; }

    bool WorldToScreen(Vector3 world_pos, Vector2& screen_pos) {
        const float PI_val = 3.14159265f;
        float radPitch = G_CameraRotation.x * PI_val / 180.0f;
        float radYaw = G_CameraRotation.y * PI_val / 180.0f;
        float radRoll = G_CameraRotation.z * PI_val / 180.0f;

        float sp = sinf(radPitch), cp = cosf(radPitch);
        float sy = sinf(radYaw), cy = cosf(radYaw);
        float sr = sinf(radRoll), cr = cosf(radRoll);

        float axis_x[3] = { cp * cy, cp * sy, sp };
        float axis_y[3] = { sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp };
        float axis_z[3] = { -(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp };

        Vector3 vDelta = world_pos - G_CameraLocation;

        float transformed_x = vDelta.x * axis_y[0] + vDelta.y * axis_y[1] + vDelta.z * axis_y[2];
        float transformed_y = vDelta.x * axis_z[0] + vDelta.y * axis_z[1] + vDelta.z * axis_z[2];
        float transformed_z = vDelta.x * axis_x[0] + vDelta.y * axis_x[1] + vDelta.z * axis_x[2];

        if (transformed_z < 0.05f) return false;

        float ScreenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
        float ScreenHeight = (float)GetSystemMetrics(SM_CYSCREEN);

        float FovAngle = (G_CamFOV <= 0.0f) ? 90.0f : G_CamFOV;
        float fov_rad = FovAngle * PI_val / 360.0f;
        float fov_tan = tanf(fov_rad);

        float ScreenCX = ScreenWidth / 2.0f;
        float ScreenCY = ScreenHeight / 2.0f;

        screen_pos.x = ScreenCX + (transformed_x / fov_tan) * ScreenCX / transformed_z;
        screen_pos.y = ScreenCY - (transformed_y / fov_tan) * ScreenCX / transformed_z;

        return (screen_pos.x > -100 && screen_pos.x < ScreenWidth + 100 && screen_pos.y > -100 && screen_pos.y < ScreenHeight + 100);
    }

    inline uint64_t ReadXe(uint64_t address) {
        uint64_t val = 0;
        if (!PubgMemory::ReadMemory(address, &val, sizeof(uint64_t))) return 0;
        return PubgDecrypt::Xe(val);
    }

    bool Initialize(uint32_t process_id, uint64_t base_address) {
        PubgMemory::g_ProcessId = process_id;
        PubgMemory::g_BaseAddress = base_address;
        PubgMemory::RefreshProcessContext();
        if (PubgMemory::g_BaseAddress == 0) {
            PubgMemory::g_BaseAddress = base_address;
        }

        if (!PubgDecrypt::Initialize(PubgMemory::ReadMemory, PubgMemory::g_BaseAddress, PubgOffsets::XenuineDecrypt)) return false;

        uint64_t rawUWorld = 0;
        if (PubgMemory::ReadMemory(PubgMemory::g_BaseAddress + PubgOffsets::UWorld, &rawUWorld, sizeof(uint64_t))) {
            G_UWorld = PubgDecrypt::Xe(rawUWorld);
            if (G_UWorld > 0x1000000) {
                G_PersistentLevel = ReadXe(G_UWorld + PubgOffsets::CurrentLevel);
                if (G_PersistentLevel > 0x1000000) return true;
            }
        }
        return false;
    }

    void UpdateGameData() {
        if (!PubgMemory::g_BaseAddress) return;
        
        uint64_t rawUWorld = 0;
        if (!PubgMemory::ReadMemory(PubgMemory::g_BaseAddress + PubgOffsets::UWorld, &rawUWorld, sizeof(uint64_t))) return;

        G_UWorld = PubgDecrypt::Xe(rawUWorld);
        if (!G_UWorld) return;

        G_GameInstance = ReadXe(G_UWorld + PubgOffsets::GameInstance);
        G_PersistentLevel = ReadXe(G_UWorld + PubgOffsets::CurrentLevel);
        G_GameState = ReadXe(G_UWorld + PubgOffsets::GameState);

        uint64_t localPlayerPtr = ReadXe(Read<uint64_t>(G_GameInstance + PubgOffsets::LocalPlayer));
        bool inGame = false;
        std::set<int> attachedItems;

        if (localPlayerPtr) {
            G_LocalPlayer = localPlayerPtr;
            G_PlayerController = ReadXe(G_LocalPlayer + PubgOffsets::PlayerController);
            if (G_PlayerController) {
                G_LocalPawn = ReadXe(G_PlayerController + PubgOffsets::AcknowledgedPawn);
                G_LocalHUD = Read<uint64_t>(G_PlayerController + PubgOffsets::MyHUD);
                
                // --- UPDATE MENU STATE ---
                // bShowMouseCursor is a bitfield in PlayerController. We check if the bit is set.
                // In UE4, this bit is usually part of a bitmask at an offset.
                G_IsMenuOpen = (Read<uint8_t>(G_PlayerController + PubgOffsets::bShowMouseCursor) >> 4) & 1; 


                if (G_LocalPawn > 0x1000000) {
                    inGame = true;
                    // Position/Velocity/Camera are now updated in real-time by UpdateCamera()

                    // Inventory filtering
                    uint64_t invF = ReadXe(G_LocalPawn + PubgOffsets::InventoryFacade);
                    if (invF) {
                        uint64_t inv = Read<uint64_t>(invF + PubgOffsets::Inventory);
                        if (inv) {
                            uint64_t arr = Read<uint64_t>(inv + PubgOffsets::InventoryItems);
                            int cnt = Read<int>(inv + PubgOffsets::InventoryItems + 0x8);
                            if (cnt > 0 && cnt < 100) {
                                for (int i = 0; i < cnt; i++) {
                                    uint64_t item = Read<uint64_t>(arr + i * 8);
                                    if (item) {
                                        uint64_t table = Read<uint64_t>(item + PubgOffsets::ItemTable);
                                        if (table) attachedItems.insert(Read<int>(table + PubgOffsets::ItemID));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Dynamic radar/widget scan (minimap + world map)
        static ULONGLONG lastRadarScan = 0;
        static ULONGLONG lastMapInfoUpdate = 0;
        const ULONGLONG now = GetTickCount64();
        if (now - lastRadarScan > 300) {
            lastRadarScan = now;
            const float screenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
            const float screenHeight = (float)GetSystemMetrics(SM_CYSCREEN);

            G_Radar.IsMiniMapVisible = false;
            G_Radar.IsWorldMapVisible = false;
            G_Radar.WorldMapWidth = 0.0f;
            G_Radar.WorldMapHeight = 0.0f;

            float worldRawLeft = 0.0f;
            float worldRawTop = 0.0f;
            float worldAlignX = 0.5f;
            float worldAlignY = 0.5f;

            if (G_LocalHUD) {
                uint64_t blockList = Read<uint64_t>(G_LocalHUD + PubgOffsets::BlockInputWidgetList);
                int count = Read<int>(G_LocalHUD + PubgOffsets::BlockInputWidgetList + 0x8);
                if (count > 0 && count < 500) {
                    for (int i = 0; i < count; i++) {
                        const uint64_t widget = Read<uint64_t>(blockList + (i * 8));
                        if (!widget) continue;

                        const uint32_t objID = PubgOffsets::DecryptCIndex(Read<uint32_t>(widget + PubgOffsets::ObjID));
                        const std::string className = FNameUtils::GetNameFast(objID);
                        if (className.empty()) continue;

                        if (className.find("MinimapOriginalType") != std::string::npos) {
                            const uint8_t visibility = Read<uint8_t>(widget + PubgOffsets::Visibility);
                            G_Radar.IsMiniMapVisible = IsSlateVisible(visibility);
                            G_Radar.SelectMinimapSizeIndex = Read<int>(widget + PubgOffsets::SelectMinimapSizeIndex);
                            G_Radar.MiniMapSizeIndex = G_Radar.SelectMinimapSizeIndex;
                            G_Radar.CurrentMinimapViewScale = Read<float>(widget + PubgOffsets::CurrentMinimapViewScale1D);

                            const uint64_t slot = Read<uint64_t>(widget + PubgOffsets::Slot);
                            if (slot) {
                                const float left = Read<float>(slot + PubgOffsets::LayoutData + 0x0);
                                const float top = Read<float>(slot + PubgOffsets::LayoutData + 0x4);
                                const float width = Read<float>(slot + PubgOffsets::LayoutData + 0x8);
                                const float height = Read<float>(slot + PubgOffsets::LayoutData + 0xC);
                                const float alignX = Read<float>(slot + PubgOffsets::LayoutData + PubgOffsets::Alignment + 0x0);
                                const float alignY = Read<float>(slot + PubgOffsets::LayoutData + PubgOffsets::Alignment + 0x4);

                                float resolvedX = 0.0f;
                                float resolvedY = 0.0f;
                                ResolveWidgetRect(left, top, width, height, alignX, alignY, screenWidth, screenHeight, resolvedX, resolvedY);

                                if (width > 40.0f && height > 40.0f) {
                                    G_Radar.ScreenPosX = resolvedX;
                                    G_Radar.ScreenPosY = resolvedY;
                                    G_Radar.ScreenSize = width;
                                    G_Radar.ScreenSizeY = height;
                                    G_Radar.AlignmentX = alignX;
                                    G_Radar.AlignmentY = alignY;
                                }
                            }
                            continue;
                        }

                        if (IsWorldMapWidgetClass(className)) {
                            const uint8_t visibility = Read<uint8_t>(widget + PubgOffsets::Visibility);
                            const bool worldMapVisible = IsSlateVisible(visibility);
                            if (!worldMapVisible) continue;

                            uint64_t mapGrid = Read<uint64_t>(widget + PubgOffsets::MapGrid_Map);
                            if (mapGrid < 0x1000000) {
                                mapGrid = widget;
                            }

                            const uint64_t slot = Read<uint64_t>(mapGrid + PubgOffsets::Slot);
                            if (!slot) continue;

                            const float left = Read<float>(slot + PubgOffsets::LayoutData + 0x0);
                            const float top = Read<float>(slot + PubgOffsets::LayoutData + 0x4);
                            const float width = Read<float>(slot + PubgOffsets::LayoutData + 0x8);
                            const float height = Read<float>(slot + PubgOffsets::LayoutData + 0xC);
                            const float alignX = Read<float>(slot + PubgOffsets::LayoutData + PubgOffsets::Alignment + 0x0);
                            const float alignY = Read<float>(slot + PubgOffsets::LayoutData + PubgOffsets::Alignment + 0x4);

                            if (width > 100.0f && height > 100.0f) {
                                float resolvedX = 0.0f;
                                float resolvedY = 0.0f;
                                ResolveWidgetRect(left, top, width, height, alignX, alignY, screenWidth, screenHeight, resolvedX, resolvedY);

                                G_Radar.IsWorldMapVisible = true;
                                G_Radar.WorldMapX = resolvedX;
                                G_Radar.WorldMapY = resolvedY;
                                G_Radar.WorldMapWidth = width;
                                G_Radar.WorldMapHeight = height;

                                worldRawLeft = left;
                                worldRawTop = top;
                                worldAlignX = alignX;
                                worldAlignY = alignY;

                                const float currentPosX = (width * (alignX - 0.5f)) - left;
                                const float currentPosY = (height * (alignY - 0.5f)) - top;
                                const float zoomFactor = width / 1080.0f;
                                if (zoomFactor > 0.001f) {
                                    G_Radar.WorldMapZoomFactor = zoomFactor;
                                    G_Radar.WorldMapPosition = {
                                        currentPosX / 1080.0f / zoomFactor * 2.0f,
                                        currentPosY / 1080.0f / zoomFactor * 2.0f
                                    };
                                }
                            }
                        }
                    }
                }
            }

            if (now - lastMapInfoUpdate > 3000 || G_Radar.MapWorldSize <= 0.0f) {
                lastMapInfoUpdate = now;
                const uint32_t mapObjId = PubgOffsets::DecryptCIndex(Read<uint32_t>(G_UWorld + PubgOffsets::ObjID));
                const std::string mapName = FNameUtils::GetNameFast(mapObjId);
                G_Radar.MapWorldSize = GetMapWorldSize(mapName);

                const int worldOriginX = Read<int>(G_UWorld + PubgOffsets::WorldToMap);
                const int worldOriginY = Read<int>(G_UWorld + PubgOffsets::WorldToMap + 0x4);
                G_Radar.WorldOriginLocation = { (float)worldOriginX, (float)worldOriginY, 0.0f };
            }

            if (G_Radar.IsWorldMapVisible && G_Radar.WorldMapWidth > 10.0f && G_Radar.MapWorldSize > 0.0f) {
                const float mapScale = G_Radar.WorldMapZoomFactor;
                if (mapScale > 0.001f) {
                    const float posX = G_Radar.WorldMapPosition.x;
                    const float posY = G_Radar.WorldMapPosition.y;

                    G_Radar.MapSizeFactored = G_Radar.MapWorldSize / mapScale;
                    G_Radar.WorldCenterLocation = {
                        G_Radar.MapWorldSize * (1.0f + posX),
                        G_Radar.MapWorldSize * (1.0f + posY),
                        0.0f
                    };
                }
            }

            // Keep sane defaults so world-map overlay still has a valid transform
            // when map widget metadata is missing.
            if (G_Radar.MapWorldSize > 0.0f) {
                if (G_Radar.MapSizeFactored <= 1.0f) {
                    G_Radar.MapSizeFactored = G_Radar.MapWorldSize;
                }
                if (G_Radar.WorldCenterLocation.IsZero()) {
                    G_Radar.WorldCenterLocation = { G_Radar.MapWorldSize, G_Radar.MapWorldSize, 0.0f };
                }
            }
        }

        // Dynamic fallback when HUD widget is not ready
        if (G_Radar.ScreenSize < 40.0f || G_Radar.ScreenSizeY < 40.0f) {
            const float screenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
            const float screenHeight = (float)GetSystemMetrics(SM_CYSCREEN);
            const float scaleSize = screenHeight * 0.2315f;
            const float fallbackSize = (scaleSize > 180.0f) ? scaleSize : 180.0f;
            G_Radar.ScreenSize = fallbackSize;
            G_Radar.ScreenSizeY = fallbackSize;
            G_Radar.ScreenPosX = screenWidth - fallbackSize - 20.0f;
            G_Radar.ScreenPosY = screenHeight - fallbackSize - 20.0f;
        }

        g_Menu.current_scene = inGame ? Scene::Gaming : Scene::Lobby;
        if (!inGame) { G_Players.clear(); CachedItems.clear(); return; }

        std::vector<PlayerData> tempPlayers;
        std::vector<ItemData> tempItems;
        std::vector<uint64_t> seenPlayerStates;
        float localPlayerEyes = 0.0f;

        ULONGLONG scanStartTime = GetTickCount64();

        if (G_LocalPawn > 0x1000000) {
            uint64_t localMesh = Read<uint64_t>(G_LocalPawn + PubgOffsets::Mesh);
            if (localMesh > 0x1000000) {
                localPlayerEyes = Read<float>(localMesh + PubgOffsets::LastRenderTimeOnScreen);
            }
        }

        uint64_t actorListPtr = ReadXe(G_PersistentLevel + PubgOffsets::Actors);
        if (!actorListPtr) return;
        uint64_t actorArray = Read<uint64_t>(actorListPtr);
        int actorCount = Read<int>(actorListPtr + 0x8);
        if (actorCount <= 0 || actorCount > 10000) return;

        int localTeam = 0;
        if (G_LocalPawn > 0x1000000) {
            // PAOD-style priority: read LastTeamNum directly from character first.
            localTeam = NormalizeTeamId(Read<int>(G_LocalPawn + PubgOffsets::LastTeamNum));
            if (localTeam == 0) {
                uint64_t localPlayerState = Read<uint64_t>(G_LocalPawn + PubgOffsets::PlayerState);
                if (localPlayerState < 0x1000000) {
                    localPlayerState = ReadXe(G_LocalPawn + PubgOffsets::PlayerState);
                }
                if (localPlayerState > 0x1000000) {
                    localTeam = NormalizeTeamId(Read<int>(localPlayerState + PubgOffsets::TeamNumber));
                    if (localTeam == 0) {
                        localTeam = NormalizeTeamId(Read<int>(localPlayerState + 0x444));
                    }
                }
            }
        }

        for (int i = 0; i < actorCount; i++) {
            uintptr_t actor = Read<uintptr_t>(actorArray + (i * 0x8));
            if (!actor || actor == G_LocalPawn) continue;

            // Character detection (Mesh + ClassName)
            uint64_t mesh = Read<uintptr_t>(actor + PubgOffsets::Mesh);
            if (!mesh) continue;

            uint32_t objID = PubgOffsets::DecryptCIndex(Read<uint32_t>(actor + PubgOffsets::ObjID));
            std::string cname = FNameUtils::GetNameFast(objID);
            
            bool isPlayer = (cname.find("PlayerMale") != std::string::npos || 
                             cname.find("PlayerFemale") != std::string::npos || 
                             cname.find("AIPawn") != std::string::npos ||
                             cname.find("NPC_") != std::string::npos ||
                             cname.find("ZombieNpc") != std::string::npos);

            if (isPlayer) {
                uint64_t playerState = Read<uintptr_t>(actor + PubgOffsets::PlayerState);
                if (playerState < 0x1000000) playerState = ReadXe(actor + PubgOffsets::PlayerState); // Fallback try decrypt if valid

                // Duplicate filter: multiple actors can point to the same PlayerState
                if (playerState > 0x1000000) {
                    bool alreadySeen = false;
                    for (auto s : seenPlayerStates) if (s == playerState) { alreadySeen = true; break; }
                    if (alreadySeen) continue;
                    seenPlayerStates.push_back(playerState);
                }

                uint64_t root = ReadXe(actor + PubgOffsets::RootComponent);
                if (!root) continue;
                Vector3 pos = Read<Vector3>(root + PubgOffsets::ComponentLocation);
                if (pos.IsZero()) continue;
                float dist = G_CameraLocation.Distance(pos) / 100.0f;
                if (dist > 1000.0f) continue;

                PlayerData p{};
                p.ActorPtr = actor; p.MeshAddr = mesh; p.Position = pos; p.Distance = dist;
                // Read CharacterName directly from Pawn as per gamebaseRael.txt
                uint64_t pNameData = Read<uint64_t>(actor + PubgOffsets::CharacterName); 
                if (pNameData > 0x1000000) {
                    wchar_t buf[32] = {0};
                    if (PubgContext::ReadMemory(pNameData, buf, 31 * sizeof(wchar_t))) {
                        std::string narrowName;
                        for (int k = 0; k < 31 && buf[k]; ++k) { // Break on null-terminator
                            if (buf[k] >= 32 && buf[k] <= 126) narrowName += (char)buf[k]; // safe ASCII limit
                            else narrowName += '?'; // fallback for weird unicode or chinese chars
                        }
                        if (!narrowName.empty() && narrowName != "Player") p.Name = narrowName;
                    }
                }

                uint64_t movement = Read<uint64_t>(actor + PubgOffsets::CharacterMovement);
                if (movement) {
                    p.Velocity = Read<Vector3>(movement + PubgOffsets::LastUpdateVelocity);
                }
                
                // PAOD-style priority: LastTeamNum on actor, then PlayerState TeamNumber fallback.
                p.TeamID = NormalizeTeamId(Read<int>(actor + PubgOffsets::LastTeamNum));
                if (p.TeamID == 0 && playerState > 0x1000000) {
                    p.TeamID = NormalizeTeamId(Read<int>(playerState + PubgOffsets::TeamNumber));
                    if (p.TeamID == 0) {
                        p.TeamID = NormalizeTeamId(Read<int>(playerState + 0x444));
                    }
                }

                p.Health = PubgHealth::DecryptHealth(actor);
                p.GroggyHealth = Read<float>(actor + PubgOffsets::GroggyHealth);
                p.IsGroggy = (p.GroggyHealth > 0.0f && p.Health <= 0.0f);
                p.SpectatedCount = Read<int>(playerState + PubgOffsets::SpectatedCount);
                p.Gender = NormalizeGender(Read<uint8_t>(actor + PubgOffsets::Gender));
                
                float enemyEyes = Read<float>(mesh + PubgOffsets::LastRenderTimeOnScreen);
                if (localPlayerEyes > 0.0f) {
                    p.IsVisible = (enemyEyes + kOverlayVisibilityThreshold >= localPlayerEyes);
                } else {
                    float lastSubmit = Read<float>(mesh + PubgOffsets::LastSubmitTime);
                    p.IsVisible = (enemyEyes + kOverlayVisibilityThreshold >= lastSubmit);
                }

                // Weapon info
                uint64_t weaponProc = Read<uint64_t>(actor + PubgOffsets::WeaponProcessor);
                if (weaponProc) {
                    uint64_t equippedAddr = Read<uint64_t>(weaponProc + PubgOffsets::EquippedWeapons);
                    uint8_t currentIdx = Read<uint8_t>(weaponProc + PubgOffsets::CurrentWeaponIndex);
                    if (currentIdx < 3) {
                        uint64_t weapon = Read<uint64_t>(equippedAddr + (currentIdx * 8));
                        if (weapon) {
                            int32_t wID = PubgOffsets::DecryptCIndex(Read<int32_t>(weapon + PubgOffsets::ObjID));
                            std::string rawWeap = FNameUtils::GetNameFast(wID);
                            
                            // Basic Clean up for UI (Replace JSON dependencies)
                            if (rawWeap.find("Weap") == 0) rawWeap.erase(0, 4);
                            if (rawWeap.find("Item_Weapon_") == 0) rawWeap.erase(0, 12);
                            if (rawWeap.length() >= 2 && rawWeap.substr(rawWeap.length() - 2) == "_C") rawWeap.erase(rawWeap.length() - 2);
                            
                            if (rawWeap == "HK416" || rawWeap == "DuncansHK416") rawWeap = "M416";
                            else if (rawWeap == "AK47" || rawWeap == "LunchmeatsAK47") rawWeap = "AKM";
                            else if (rawWeap == "MadsFNFal" || rawWeap == "FNFal") rawWeap = "SLR";
                            else if (rawWeap == "MadsQBU88") rawWeap = "QBU";
                            else if (rawWeap == "OriginS12") rawWeap = "O12";
                            else if (rawWeap == "JuliesKar98k") rawWeap = "Kar98k";
                            else if (rawWeap == "JuliesM24") rawWeap = "M24";
                            else if (rawWeap == "BizonPP19") rawWeap = "Bizon";
                            
                            p.WeaponName = rawWeap;
                        }
                    }
                }

                p.IsTeammate = (localTeam != 0 && p.TeamID == localTeam);

                uint64_t boneArray = Read<uint64_t>(mesh + PubgOffsets::BoneArray);
                if (boneArray) {
                    p.BoneArrayAddr = boneArray;
                    FTransform meshToWorld = Read<FTransform>(mesh + PubgOffsets::ComponentToWorld);
                    const OverlayBoneIndices idx = GetOverlayBoneIndices(p.Gender);
                    p.Bone_Head = PubgBones::GetBoneWorldPosWithMatrix(idx.head, boneArray, meshToWorld);
                    p.HeadPosition = p.Bone_Head;
                    p.Bone_Pelvis = PubgBones::GetBoneWorldPosWithMatrix(idx.pelvis, boneArray, meshToWorld);
                    p.Bone_LFoot = PubgBones::GetBoneWorldPosWithMatrix(idx.leftFoot, boneArray, meshToWorld);
                    p.Bone_RFoot = PubgBones::GetBoneWorldPosWithMatrix(idx.rightFoot, boneArray, meshToWorld);
                    
                    // Box Bottom is the lowest point between feet
                    p.FeetPosition = (p.Bone_LFoot.z < p.Bone_RFoot.z) ? p.Bone_LFoot : p.Bone_RFoot;

                    if (dist < 300.0f && g_Menu.esp_skeleton) {
                        p.Bone_Neck = PubgBones::GetBoneWorldPosWithMatrix(idx.neck, boneArray, meshToWorld);
                        p.Bone_Chest = PubgBones::GetBoneWorldPosWithMatrix(idx.chest, boneArray, meshToWorld);
                        p.Bone_LShoulder = PubgBones::GetBoneWorldPosWithMatrix(idx.leftShoulder, boneArray, meshToWorld);
                        p.Bone_LElbow = PubgBones::GetBoneWorldPosWithMatrix(idx.leftElbow, boneArray, meshToWorld);
                        p.Bone_LHand = PubgBones::GetBoneWorldPosWithMatrix(idx.leftHand, boneArray, meshToWorld);
                        p.Bone_RShoulder = PubgBones::GetBoneWorldPosWithMatrix(idx.rightShoulder, boneArray, meshToWorld);
                        p.Bone_RElbow = PubgBones::GetBoneWorldPosWithMatrix(idx.rightElbow, boneArray, meshToWorld);
                        p.Bone_RHand = PubgBones::GetBoneWorldPosWithMatrix(idx.rightHand, boneArray, meshToWorld);
                        p.Bone_LThigh = PubgBones::GetBoneWorldPosWithMatrix(idx.leftThigh, boneArray, meshToWorld);
                        p.Bone_LKnee = PubgBones::GetBoneWorldPosWithMatrix(idx.leftKnee, boneArray, meshToWorld);
                        p.Bone_RThigh = PubgBones::GetBoneWorldPosWithMatrix(idx.rightThigh, boneArray, meshToWorld);
                        p.Bone_RKnee = PubgBones::GetBoneWorldPosWithMatrix(idx.rightKnee, boneArray, meshToWorld);
                    }
                }
                else {
                    p.HeadPosition = pos + Vector3{ 0,0,170 };
                    p.FeetPosition = p.Position; // Fallback
                    p.FeetPosition.z -= 90.0f; // Guess feet if no bones accessible
                }

                tempPlayers.push_back(p);
            } 
            else {
                // Throttle Item/Vehicle/Drop scan to save CPU
                static ULONGLONG lastItemScan = 0;
                if (GetTickCount64() - lastItemScan > 1000) {
                    if (tempItems.size() < 400) {
                        // objID & cname already fetched above!
                        
                        bool isLoot = (cname.find("Dropped") != std::string::npos);
                        bool isVeh  = (cname.find("Vehicle") != std::string::npos || cname.find("Uaz") != std::string::npos || cname.find("Dacia") != std::string::npos);
                        bool isAir  = (cname.find("AirDrop") != std::string::npos || cname.find("CarePackage") != std::string::npos);
                        bool isBox  = (cname.find("DeadBox") != std::string::npos || cname.find("ItemPackage") != std::string::npos);

                        if (isLoot || isVeh || isAir || isBox) {
                            uint64_t root = ReadXe(actor + PubgOffsets::RootComponent);
                            if (root) {
                                Vector3 pos = Read<Vector3>(root + PubgOffsets::ComponentLocation);
                                float dist = G_CameraLocation.Distance(pos) / 100.0f;
                                if (dist < (isLoot ? 100.0f : 1000.0f)) {
                                    ItemData item;
                                    item.Position = pos; item.Distance = dist;
                                    item.Name = isVeh ? "Vehicle" : (isAir ? "Air Drop" : (isBox ? "Dead Box" : "Loot"));
                                    item.IsImportant = (isAir || isVeh);
                                    tempItems.push_back(item);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Final swap
        G_Players = tempPlayers;
        G_LastScanTime = scanStartTime;
        if (!tempItems.empty()) {
            CachedItems = tempItems;
            static ULONGLONG lastUpdate = 0;
            lastUpdate = GetTickCount64();
        }
    }
    void UpdateCamera() {
        if (G_PlayerController && G_LocalPawn) {
            uint64_t camMgr = Read<uint64_t>(G_PlayerController + PubgOffsets::PlayerCameraManager);
            if (camMgr) {
                // Bulk read Camera Info (Valorant-Style) to reduce Syscalls
                struct CamInfo {
                    Vector3 Rotation; // 0xA10
                    char _pad[16];    // 0xA1C - 0xA2C (Chính xác 16 byte padding)
                    float FOV;        // 0xA2C
                    Vector3 Location; // 0xA30
                } ci;

                if (ReadMemory(camMgr + PubgOffsets::CameraCacheRotation, &ci, sizeof(CamInfo))) {
                    G_CameraLocation = ci.Location;
                    G_CameraRotation = ci.Rotation;
                    G_CamFOV = ci.FOV;
                }
            }

            // Sync Local Player Position in Realtime
            uint64_t rootComp = ReadXe(G_LocalPawn + PubgOffsets::RootComponent);
            if (rootComp) G_LocalPlayerPos = Read<Vector3>(rootComp + PubgOffsets::ComponentLocation);
            
            uint64_t movement = Read<uint64_t>(G_LocalPawn + PubgOffsets::CharacterMovement);
            if (movement) {
                G_LocalPlayerVelocity = Read<Vector3>(movement + PubgOffsets::LastUpdateVelocity);
                G_Radar.VehicleSpeed = G_LocalPlayerVelocity.Length() * 0.036f;
            }
        }
    }

    void SyncLivePlayers(std::vector<PlayerData>& players) {
        for (auto& p : players) {
            if (p.Distance > 300.0f) continue;
            if (p.MeshAddr) {
                uint64_t boneArray = Read<uint64_t>(p.MeshAddr + PubgOffsets::BoneArray);
                if (boneArray) {
                    FTransform meshToWorld = Read<FTransform>(p.MeshAddr + PubgOffsets::ComponentToWorld);
                    p.Gender = NormalizeGender(Read<uint8_t>(p.ActorPtr + PubgOffsets::Gender));
                    const OverlayBoneIndices idx = GetOverlayBoneIndices(p.Gender);
                    p.Bone_Head = PubgBones::GetBoneWorldPosWithMatrix(idx.head, boneArray, meshToWorld);
                    p.HeadPosition = p.Bone_Head;
                    p.Bone_Pelvis = PubgBones::GetBoneWorldPosWithMatrix(idx.pelvis, boneArray, meshToWorld);
                    p.Bone_LFoot = PubgBones::GetBoneWorldPosWithMatrix(idx.leftFoot, boneArray, meshToWorld);
                    p.Bone_RFoot = PubgBones::GetBoneWorldPosWithMatrix(idx.rightFoot, boneArray, meshToWorld);
                    p.FeetPosition = (p.Bone_LFoot.z < p.Bone_RFoot.z) ? p.Bone_LFoot : p.Bone_RFoot;

                    p.Bone_Neck = PubgBones::GetBoneWorldPosWithMatrix(idx.neck, boneArray, meshToWorld);
                    p.Bone_Chest = PubgBones::GetBoneWorldPosWithMatrix(idx.chest, boneArray, meshToWorld);
                    p.Bone_LShoulder = PubgBones::GetBoneWorldPosWithMatrix(idx.leftShoulder, boneArray, meshToWorld);
                    p.Bone_LElbow = PubgBones::GetBoneWorldPosWithMatrix(idx.leftElbow, boneArray, meshToWorld);
                    p.Bone_LHand = PubgBones::GetBoneWorldPosWithMatrix(idx.leftHand, boneArray, meshToWorld);
                    p.Bone_RShoulder = PubgBones::GetBoneWorldPosWithMatrix(idx.rightShoulder, boneArray, meshToWorld);
                    p.Bone_RElbow = PubgBones::GetBoneWorldPosWithMatrix(idx.rightElbow, boneArray, meshToWorld);
                    p.Bone_RHand = PubgBones::GetBoneWorldPosWithMatrix(idx.rightHand, boneArray, meshToWorld);
                    p.Bone_LThigh = PubgBones::GetBoneWorldPosWithMatrix(idx.leftThigh, boneArray, meshToWorld);
                    p.Bone_LKnee = PubgBones::GetBoneWorldPosWithMatrix(idx.leftKnee, boneArray, meshToWorld);
                    p.Bone_RThigh = PubgBones::GetBoneWorldPosWithMatrix(idx.rightThigh, boneArray, meshToWorld);
                    p.Bone_RKnee = PubgBones::GetBoneWorldPosWithMatrix(idx.rightKnee, boneArray, meshToWorld);
                }
            }
        }
    }
}
