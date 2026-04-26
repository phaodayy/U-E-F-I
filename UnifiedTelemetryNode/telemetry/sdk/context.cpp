#include "context.hpp"
#include "offsets.hpp"
#include "telemetry_decrypt.hpp"
#include "scanner.hpp"
#include "fname.hpp"
#include "Common/Data.h"
#include "../overlay/overlay_menu.hpp"
#include <iostream>
#include <algorithm>
#include <set>
#include <array>
#include <utility>

using namespace telemetryOffsets;
using namespace telemetryDecrypt;
using namespace telemetryMemory;

// Globals
FGameData GameData; 
uint64_t G_UWorld = 0, G_GameInstance = 0, G_PersistentLevel = 0, G_LocalPlayer = 0, G_PlayerController = 0, G_LocalPawn = 0, G_LocalHUD = 0, G_GameState = 0;
bool G_IsMenuOpen = false;
Vector3 G_CameraLocation = { 0, 0, 0 }, G_CameraRotation = { 0, 0, 0 }, G_LocalPlayerPos = { 0, 0, 0 }, G_LocalPlayerVelocity = { 0, 0, 0 };
uint64_t G_LastScanTime = 0;
RadarData G_Radar;
std::vector<PlayerData> G_Players;
int G_LocalSpectatedCount = 0;
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
        // 0: Visible, 1: Collapsed, 2: Hidden, 3: HitTestInvisible, 4: SelfHitTestInvisible
        return visibilityValue == 0 || visibilityValue == 3 || visibilityValue == 4; 
    }
    
    bool IsWorldMapWidgetClass(const std::string& className) {
        return className.find(skCrypt("WorldMap")) != std::string::npos ||
            className.find(skCrypt("MapGrid")) != std::string::npos ||
            className.find(skCrypt("ChildCenter")) != std::string::npos;
    }

    float GetMapWorldSize(const std::string& mapName) {
        static const std::array<std::pair<const char*, float>, 15> kMapSizes = {{
            {skCrypt("Tiger_Main"), 408000.0f},
            {skCrypt("Kiki_Main"), 408000.0f},
            {skCrypt("Desert_Main"), 408000.0f},
            {skCrypt("Range_Main"), 101175.0f},
            {skCrypt("Summerland_Main"), 101175.0f},
            {skCrypt("Italy_Main"), 101175.0f},
            {skCrypt("Baltic_Main"), 406372.0f},
            {skCrypt("Neon_Main"), 408000.0f},
            {skCrypt("Heaven_Main"), 101175.0f},
            {skCrypt("Savage_Main"), 202387.5f},
            {skCrypt("DihorOtok_Main"), 408000.0f},
            {skCrypt("Chimera_Main"), 153003.0f},
            {skCrypt("Boardwalk_Main"), 51420.0f},
            {skCrypt("Narrows_Main"), 51420.0f},
            {skCrypt("Pinnacle_Main"), 51420.0f}
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

namespace telemetryContext {

    bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        return telemetryMemory::ReadMemory(src, dest, size);
    }

    uint64_t GetBaseAddress() { return telemetryMemory::g_BaseAddress; }

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
        if (!telemetryMemory::ReadMemory(address, &val, sizeof(uint64_t))) return 0;
        return telemetryDecrypt::Xe(val);
    }

    bool Initialize(uint32_t process_id, uint64_t base_address) {
        telemetryMemory::g_ProcessId = process_id;
        telemetryMemory::g_BaseAddress = base_address;
        telemetryMemory::RefreshProcessContext();
        if (telemetryMemory::g_BaseAddress == 0) {
            telemetryMemory::g_BaseAddress = base_address;
        }

        if (!telemetryDecrypt::Initialize(telemetryMemory::ReadMemory, telemetryMemory::g_BaseAddress, telemetryOffsets::XenuineDecrypt)) return false;

        uint64_t rawUWorld = 0;
        if (telemetryMemory::ReadMemory(telemetryMemory::g_BaseAddress + telemetryOffsets::UWorld, &rawUWorld, sizeof(uint64_t))) {
            G_UWorld = telemetryDecrypt::Xe(rawUWorld);
            if (G_UWorld > 0x1000000) return true;
        }
        return false;
    }

    void UpdateGameData() {
        if (!telemetryMemory::g_BaseAddress) return;
        
        static ULONGLONG lastGlobalRefresh = 0;
        const ULONGLONG now = GetTickCount64();

        // CACHE: Primary pointers only need refresh every 2 seconds
        if (G_UWorld != 0 && (now - lastGlobalRefresh < 2000)) {
            // Skip reading UWorld, GameInstance, etc.
        } else {
            uint64_t rawUWorld = 0;
            if (telemetryMemory::ReadMemory(telemetryMemory::g_BaseAddress + telemetryOffsets::UWorld, &rawUWorld, sizeof(uint64_t))) {
                G_UWorld = telemetryDecrypt::Xe(rawUWorld);
                if (G_UWorld) {
                    G_GameInstance = ReadXe(G_UWorld + telemetryOffsets::GameInstance);
                    G_PersistentLevel = ReadXe(G_UWorld + telemetryOffsets::CurrentLevel);
                    G_GameState = ReadXe(G_UWorld + telemetryOffsets::GameState);
                    lastGlobalRefresh = now;
                }
            }
        }
        
        if (!G_UWorld) return;

        uint64_t localPlayerPtr = ReadXe(Read<uint64_t>(G_GameInstance + telemetryOffsets::LocalPlayer));
        bool inGame = false;
        std::set<int> attachedItems;

        if (localPlayerPtr) {
            G_LocalPlayer = localPlayerPtr;
            G_PlayerController = ReadXe(G_LocalPlayer + telemetryOffsets::PlayerController);
            if (G_PlayerController) {
                G_LocalPawn = ReadXe(G_PlayerController + telemetryOffsets::AcknowledgedPawn);
                G_LocalHUD = Read<uint64_t>(G_PlayerController + telemetryOffsets::MyHUD);
                
                // --- UPDATE MENU STATE ---
                // bShowMouseCursor is a bitfield in PlayerController. We check if the bit is set.
                // In UE4, this bit is usually part of a bitmask at an offset.
                G_IsMenuOpen = (Read<uint8_t>(G_PlayerController + telemetryOffsets::bShowMouseCursor) >> 4) & 1; 


                if (G_LocalPawn > 0x1000000) {
                    inGame = true;
                    
                    // --- GET LOCAL SPECTATED COUNT ---
                    uint64_t localPlayerState = Read<uint64_t>(G_LocalPawn + telemetryOffsets::PlayerState);
                    if (localPlayerState < 0x1000000) localPlayerState = ReadXe(G_LocalPawn + telemetryOffsets::PlayerState);
                    if (localPlayerState > 0x1000000) {
                        G_LocalSpectatedCount = Read<int>(localPlayerState + telemetryOffsets::SpectatedCount);
                    } else {
                        G_LocalSpectatedCount = 0;
                    }
                    
                    // Position/Velocity/Camera are now updated in real-time by UpdateCamera()

                    // Inventory filtering
                    uint64_t invF = ReadXe(G_LocalPawn + telemetryOffsets::InventoryFacade);
                    if (invF) {
                        uint64_t inv = Read<uint64_t>(invF + telemetryOffsets::Inventory);
                        if (inv) {
                            uint64_t arr = Read<uint64_t>(inv + telemetryOffsets::InventoryItems);
                            int cnt = Read<int>(inv + telemetryOffsets::InventoryItems + 0x8);
                            if (cnt > 0 && cnt < 100) {
                                for (int i = 0; i < cnt; i++) {
                                    uint64_t item = Read<uint64_t>(arr + i * 8);
                                    if (item) {
                                        uint64_t table = Read<uint64_t>(item + telemetryOffsets::ItemTable);
                                        if (table) attachedItems.insert(Read<int>(table + telemetryOffsets::ItemID));
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
                uint64_t widgetListAddress = G_LocalHUD + telemetryOffsets::BlockInputWidgetList;
                uint32_t widgetCount = Read<uint32_t>(widgetListAddress + 0x8);
                uint64_t widgetData = Read<uint64_t>(widgetListAddress);
                
                if (widgetData && widgetCount > 0 && widgetCount < 200) {
                    for (uint32_t i = 0; i < widgetCount; i++) {
                        uint64_t widgetPtr = Read<uint64_t>(widgetData + (i * 8));
                        if (widgetPtr) {
                            uint32_t widgetIdRaw = Read<uint32_t>(widgetPtr + telemetryOffsets::ObjID);
                            uint32_t widgetIdDec = telemetryOffsets::DecryptCIndex(widgetIdRaw);
                            std::string className = FNameUtils::GetNameFast(widgetIdDec);
                            
                            uint8_t vis = Read<uint8_t>(widgetPtr + telemetryOffsets::Visibility);
                            
                            if (IsWorldMapWidgetClass(className)) {
                                if (IsSlateVisible(vis)) {
                                    // Logic for map data will follow after the loop or in a sub-check
                                }
                            }
                        }
                    }
                }

                uint64_t blockList = Read<uint64_t>(G_LocalHUD + telemetryOffsets::BlockInputWidgetList);
                int count = Read<int>(G_LocalHUD + telemetryOffsets::BlockInputWidgetList + 0x8);
                if (count > 0 && count < 500) {
                    for (int i = 0; i < count; i++) {
                        const uint64_t widget = Read<uint64_t>(blockList + (i * 8));
                        if (!widget) continue;

                        const uint32_t objID = telemetryOffsets::DecryptCIndex(Read<uint32_t>(widget + telemetryOffsets::ObjID));
                        const std::string className = FNameUtils::GetNameFast(objID);
                        if (className.empty()) continue;

                        if (className.find(skCrypt("MinimapOriginalType")) != std::string::npos) {
                            const uint8_t visibility = Read<uint8_t>(widget + telemetryOffsets::Visibility);
                            G_Radar.IsMiniMapVisible = IsSlateVisible(visibility);
                            G_Radar.SelectMinimapSizeIndex = Read<int>(widget + telemetryOffsets::SelectMinimapSizeIndex);
                            G_Radar.MiniMapSizeIndex = G_Radar.SelectMinimapSizeIndex;
                            G_Radar.CurrentMinimapViewScale = Read<float>(widget + telemetryOffsets::CurrentMinimapViewScale1D);

                            const uint64_t slot = Read<uint64_t>(widget + telemetryOffsets::Slot);
                            if (slot) {
                                const float left = Read<float>(slot + telemetryOffsets::LayoutData + 0x0);
                                const float top = Read<float>(slot + telemetryOffsets::LayoutData + 0x4);
                                const float width = Read<float>(slot + telemetryOffsets::LayoutData + 0x8);
                                const float height = Read<float>(slot + telemetryOffsets::LayoutData + 0xC);
                                const float alignX = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x0);
                                const float alignY = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x4);

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
                            const uint8_t visibility = Read<uint8_t>(widget + telemetryOffsets::Visibility);
                            const bool worldMapVisible = IsSlateVisible(visibility);
                            if (!worldMapVisible) continue;

                            uint64_t mapGrid = Read<uint64_t>(widget + telemetryOffsets::MapGrid_Map);
                            if (mapGrid < 0x1000000) {
                                mapGrid = widget;
                            }

                            const uint64_t slot = Read<uint64_t>(mapGrid + telemetryOffsets::Slot);
                            if (!slot) continue;

                            const float left = Read<float>(slot + telemetryOffsets::LayoutData + 0x0);
                            const float top = Read<float>(slot + telemetryOffsets::LayoutData + 0x4);
                            const float width = Read<float>(slot + telemetryOffsets::LayoutData + 0x8);
                            const float height = Read<float>(slot + telemetryOffsets::LayoutData + 0xC);
                            const float alignX = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x0);
                            const float alignY = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x4);

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

                                const float zoomState = Read<float>(widget + 0xA28);
                                G_Radar.WorldMapZoomFactor = (zoomState > 0.001f) ? zoomState : 1.0f;
                                
                                const float currentPosX = (width * (alignX - 0.5f)) - left;
                                const float currentPosY = (height * (alignY - 0.5f)) - top;
                                
                                G_Radar.WorldMapPosition = {
                                    currentPosX / 1080.0f,
                                    currentPosY / 1080.0f
                                };
                            }
                        }
                    }
                }
            }

            if (now - lastMapInfoUpdate > 3000 || G_Radar.MapWorldSize <= 0.0f) {
                lastMapInfoUpdate = now;
                const uint32_t mapObjId = telemetryOffsets::DecryptCIndex(Read<uint32_t>(G_UWorld + telemetryOffsets::ObjID));
                const std::string mapName = FNameUtils::GetNameFast(mapObjId);
                G_Radar.MapWorldSize = GetMapWorldSize(mapName);

                float worldOriginX = Read<float>(G_UWorld + telemetryOffsets::WorldToMap);
                float worldOriginY = Read<float>(G_UWorld + telemetryOffsets::WorldToMap + 0x4);
                
                // Fallback scan for origin if primary offset returns zero
                if (worldOriginX == 0.0f && worldOriginY == 0.0f) {
                    worldOriginX = Read<float>(G_UWorld + 0x940);
                    worldOriginY = Read<float>(G_UWorld + 0x944);
                    if (worldOriginX == 0.0f && worldOriginY == 0.0f) {
                        worldOriginX = Read<float>(G_UWorld + 0x930);
                        worldOriginY = Read<float>(G_UWorld + 0x934);
                    }
                }

                // SECURITY CLEANUP: If it's still garbage (-7.9e20), set to zero
                if (worldOriginX < -1000000.0f || worldOriginX > 1000000.0f) worldOriginX = 0.0f;
                if (worldOriginY < -1000000.0f || worldOriginY > 1000000.0f) worldOriginY = 0.0f;

                G_Radar.WorldOriginLocation = { worldOriginX, worldOriginY, 0.0f };
            }

            // Update G_Radar.IsWorldMapVisible and other properties based on widget scan
            if (!G_Radar.IsWorldMapVisible) {
                // Minimap defaults (handled by minimap widgets)
            } else {
                // FORCE PIXEL SCALE: The MapSizeFactored must be in screen pixels (e.g. 1080)
                // If the read width is garbage or world-units, override with screenHeight
                G_Radar.MapSizeFactored = (float)screenHeight; 
                
                // If we have a valid width from widget, use it but cap it at screenWidth
                if (G_Radar.WorldMapWidth > 50.0f && G_Radar.WorldMapWidth <= (float)screenWidth) {
                    G_Radar.MapSizeFactored = G_Radar.WorldMapWidth;
                }
            }

            // Training Map Special: Range_Main usually centers at 0,0 with 2km size
            if (G_Radar.MapWorldSize == 101175.0f) {
                G_Radar.WorldCenterLocation = { 0.0f, 0.0f, 0.0f };
            }

            // Removed legacy scaling logic that was causing coordinate drift
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
        if (!inGame) { G_Players.clear(); CachedItems.clear(); G_LocalSpectatedCount = 0; return; }

        std::vector<PlayerData> tempPlayers;
        std::vector<ItemData> tempItems;
        std::vector<uint64_t> seenPlayerStates;
        float localPlayerEyes = 0.0f;

        ULONGLONG scanStartTime = GetTickCount64();

        if (G_LocalPawn > 0x1000000) {
            uint64_t localMesh = Read<uint64_t>(G_LocalPawn + telemetryOffsets::Mesh);
            if (localMesh > 0x1000000) {
                localPlayerEyes = Read<float>(localMesh + telemetryOffsets::LastRenderTimeOnScreen);
            }
        }

        uint64_t actorListPtr = ReadXe(G_PersistentLevel + telemetryOffsets::Actors);
        if (!actorListPtr) return;
        uint64_t actorArray = Read<uint64_t>(actorListPtr);
        int actorCount = Read<int>(actorListPtr + 0x8);
        if (actorCount <= 0 || actorCount > 10000) return;

        int localTeam = 0;
        if (G_LocalPawn > 0x1000000) {
            // PAOD-style priority: read LastTeamNum directly from character first.
            localTeam = NormalizeTeamId(Read<int>(G_LocalPawn + telemetryOffsets::LastTeamNum));
            if (localTeam == 0) {
                uint64_t localPlayerState = Read<uint64_t>(G_LocalPawn + telemetryOffsets::PlayerState);
                if (localPlayerState < 0x1000000) {
                    localPlayerState = ReadXe(G_LocalPawn + telemetryOffsets::PlayerState);
                }
                if (localPlayerState > 0x1000000) {
                    localTeam = NormalizeTeamId(Read<int>(localPlayerState + telemetryOffsets::TeamNumber));
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
            uint64_t mesh = Read<uintptr_t>(actor + telemetryOffsets::Mesh);
            if (!mesh) continue;

            uint32_t objID = telemetryOffsets::DecryptCIndex(Read<uint32_t>(actor + telemetryOffsets::ObjID));
            std::string cname = FNameUtils::GetNameFast(objID);
            
            bool isPlayer = (cname.find(skCrypt("PlayerMale")) != std::string::npos || 
                             cname.find(skCrypt("PlayerFemale")) != std::string::npos || 
                             cname.find(skCrypt("AIPawn")) != std::string::npos ||
                             cname.find(skCrypt("NPC_")) != std::string::npos ||
                             cname.find(skCrypt("ZombieNpc")) != std::string::npos);

            if (isPlayer) {
                uint64_t playerState = Read<uintptr_t>(actor + telemetryOffsets::PlayerState);
                if (playerState < 0x1000000) playerState = ReadXe(actor + telemetryOffsets::PlayerState); // Fallback try decrypt if valid

                // Duplicate filter: multiple actors can point to the same PlayerState
                if (playerState > 0x1000000) {
                    bool alreadySeen = false;
                    for (auto s : seenPlayerStates) if (s == playerState) { alreadySeen = true; break; }
                    if (alreadySeen) continue;
                    seenPlayerStates.push_back(playerState);
                }

                uint64_t root = ReadXe(actor + telemetryOffsets::RootComponent);
                if (!root) continue;
                Vector3 pos = Read<Vector3>(root + telemetryOffsets::ComponentLocation);
                if (pos.IsZero()) continue;
                float dist = G_CameraLocation.Distance(pos) / 100.0f;
                if (dist > 1000.0f) continue;

                PlayerData p{};
                p.ActorPtr = actor; p.MeshAddr = mesh; p.Position = pos; p.Distance = dist;
                // Read CharacterName directly from Pawn as per gamebaseRael.txt
                uint64_t pNameData = Read<uint64_t>(actor + telemetryOffsets::CharacterName); 
                if (pNameData > 0x1000000) {
                    wchar_t buf[32] = {0};
                    if (telemetryContext::ReadMemory(pNameData, buf, 31 * sizeof(wchar_t))) {
                        std::string narrowName;
                        for (int k = 0; k < 31 && buf[k]; ++k) { // Break on null-terminator
                            if (buf[k] >= 32 && buf[k] <= 126) narrowName += (char)buf[k]; // safe ASCII limit
                            else narrowName += '?'; // fallback for weird unicode or chinese chars
                        }
                        if (!narrowName.empty() && narrowName != skCrypt("Player")) p.Name = narrowName;
                    }
                }

                uint64_t movement = Read<uint64_t>(actor + telemetryOffsets::CharacterMovement);
                if (movement) {
                    p.Velocity = Read<Vector3>(movement + telemetryOffsets::LastUpdateVelocity);
                }
                
                // PAOD-style priority: LastTeamNum on actor, then PlayerState TeamNumber fallback.
                p.TeamID = NormalizeTeamId(Read<int>(actor + telemetryOffsets::LastTeamNum));
                if (p.TeamID == 0 && playerState > 0x1000000) {
                    p.TeamID = NormalizeTeamId(Read<int>(playerState + telemetryOffsets::TeamNumber));
                    if (p.TeamID == 0) {
                        p.TeamID = NormalizeTeamId(Read<int>(playerState + 0x444));
                    }
                }

                p.Health = telemetryHealth::DecryptHealth(actor);
                p.GroggyHealth = Read<float>(actor + telemetryOffsets::GroggyHealth);
                p.IsGroggy = (p.GroggyHealth > 0.0f && p.Health <= 0.0f);
                p.SpectatedCount = Read<int>(playerState + telemetryOffsets::SpectatedCount);
                p.Gender = NormalizeGender(Read<uint8_t>(actor + telemetryOffsets::Gender));
                
                float enemyEyes = Read<float>(mesh + telemetryOffsets::LastRenderTimeOnScreen);
                if (localPlayerEyes > 0.0f) {
                    p.IsVisible = (enemyEyes + kOverlayVisibilityThreshold >= localPlayerEyes);
                } else {
                    float lastSubmit = Read<float>(mesh + telemetryOffsets::LastSubmitTime);
                    p.IsVisible = (enemyEyes + kOverlayVisibilityThreshold >= lastSubmit);
                }

                // Weapon info
                uint64_t weaponProc = Read<uint64_t>(actor + telemetryOffsets::WeaponProcessor);
                if (weaponProc) {
                    uint64_t equippedAddr = Read<uint64_t>(weaponProc + telemetryOffsets::EquippedWeapons);
                    uint8_t currentIdx = Read<uint8_t>(weaponProc + telemetryOffsets::CurrentWeaponIndex);
                    if (currentIdx < 3) {
                        uint64_t weapon = Read<uint64_t>(equippedAddr + (currentIdx * 8));
                        if (weapon) {
                            int32_t wID = telemetryOffsets::DecryptCIndex(Read<int32_t>(weapon + telemetryOffsets::ObjID));
                            std::string rawWeap = FNameUtils::GetNameFast(wID);
                            
                            // Basic Clean up for UI (Replace JSON dependencies)
                            if (rawWeap.find(skCrypt("Weap")) == 0) rawWeap.erase(0, 4);
                            if (rawWeap.find(skCrypt("Item_Weapon_")) == 0) rawWeap.erase(0, 12);
                            if (rawWeap.length() >= 2 && rawWeap.substr(rawWeap.length() - 2) == skCrypt("_C")) rawWeap.erase(rawWeap.length() - 2);
                            
                            if (rawWeap == skCrypt("HK416") || rawWeap == skCrypt("DuncansHK416")) rawWeap = skCrypt("M416");
                            else if (rawWeap == skCrypt("AK47") || rawWeap == skCrypt("LunchmeatsAK47")) rawWeap = skCrypt("AKM");
                            else if (rawWeap == skCrypt("MadsFNFal") || rawWeap == skCrypt("FNFal")) rawWeap = skCrypt("SLR");
                            else if (rawWeap == skCrypt("MadsQBU88")) rawWeap = skCrypt("QBU");
                            else if (rawWeap == skCrypt("OriginS12")) rawWeap = skCrypt("O12");
                            else if (rawWeap == skCrypt("JuliesKar98k")) rawWeap = skCrypt("Kar98k");
                            else if (rawWeap == skCrypt("JuliesM24")) rawWeap = skCrypt("M24");
                            else if (rawWeap == skCrypt("BizonPP19")) rawWeap = skCrypt("Bizon");
                            
                            p.WeaponName = rawWeap;
                        }
                    }
                }

                p.IsTeammate = (localTeam != 0 && p.TeamID == localTeam);

                uint64_t boneArray = Read<uint64_t>(mesh + telemetryOffsets::BoneArray);
                if (boneArray) {
                    p.BoneArrayAddr = boneArray;
                    FTransform meshToWorld = Read<FTransform>(mesh + telemetryOffsets::ComponentToWorld);
                    const OverlayBoneIndices idx = GetOverlayBoneIndices(p.Gender);
                    p.Bone_Head = telemetryBones::GetBoneWorldPosWithMatrix(idx.head, boneArray, meshToWorld);
                    p.HeadPosition = p.Bone_Head;
                    p.Bone_Pelvis = telemetryBones::GetBoneWorldPosWithMatrix(idx.pelvis, boneArray, meshToWorld);
                    p.Bone_LFoot = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftFoot, boneArray, meshToWorld);
                    p.Bone_RFoot = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightFoot, boneArray, meshToWorld);
                    
                    // Box Bottom is the lowest point between feet
                    p.FeetPosition = (p.Bone_LFoot.z < p.Bone_RFoot.z) ? p.Bone_LFoot : p.Bone_RFoot;

                    if (dist < 300.0f && g_Menu.esp_skeleton) {
                        p.Bone_Neck = telemetryBones::GetBoneWorldPosWithMatrix(idx.neck, boneArray, meshToWorld);
                        p.Bone_Chest = telemetryBones::GetBoneWorldPosWithMatrix(idx.chest, boneArray, meshToWorld);
                        p.Bone_LShoulder = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftShoulder, boneArray, meshToWorld);
                        p.Bone_LElbow = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftElbow, boneArray, meshToWorld);
                        p.Bone_LHand = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftHand, boneArray, meshToWorld);
                        p.Bone_RShoulder = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightShoulder, boneArray, meshToWorld);
                        p.Bone_RElbow = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightElbow, boneArray, meshToWorld);
                        p.Bone_RHand = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightHand, boneArray, meshToWorld);
                        p.Bone_LThigh = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftThigh, boneArray, meshToWorld);
                        p.Bone_LKnee = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftKnee, boneArray, meshToWorld);
                        p.Bone_RThigh = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightThigh, boneArray, meshToWorld);
                        p.Bone_RKnee = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightKnee, boneArray, meshToWorld);
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
                        
                        bool isLoot = (cname.find(skCrypt("Dropped")) != std::string::npos);
                        bool isVeh  = (cname.find(skCrypt("Vehicle")) != std::string::npos || cname.find(skCrypt("Uaz")) != std::string::npos || cname.find(skCrypt("Dacia")) != std::string::npos);
                        bool isAir  = (cname.find(skCrypt("AirDrop")) != std::string::npos || cname.find(skCrypt("CarePackage")) != std::string::npos);
                        bool isBox  = (cname.find(skCrypt("DeadBox")) != std::string::npos || cname.find(skCrypt("ItemPackage")) != std::string::npos);
                        bool isProj = (cname.find(skCrypt("Proj")) != std::string::npos || cname.find(skCrypt("Grenade")) != std::string::npos || cname.find(skCrypt("Molotov")) != std::string::npos);

                        if (isLoot || isVeh || isAir || isBox || isProj) {
                            uint64_t root = ReadXe(actor + telemetryOffsets::RootComponent);
                            if (root) {
                                Vector3 pos = Read<Vector3>(root + telemetryOffsets::ComponentLocation);
                                float dist = G_CameraLocation.Distance(pos) / 100.0f;
                                if (dist < (isLoot ? 100.0f : 1000.0f)) {
                                    ItemData item;
                                    item.Position = pos; item.Distance = dist;
                                    item.Name = isVeh ? skCrypt("Vehicle") : (isAir ? skCrypt("Air Drop") : (isBox ? skCrypt("Dead Box") : (isProj ? skCrypt("PROJECTILE") : skCrypt("Loot"))));
                                    item.IsImportant = (isAir || isVeh || isProj);
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
            uint64_t camMgr = Read<uint64_t>(G_PlayerController + telemetryOffsets::PlayerCameraManager);
            if (camMgr) {
                // Bulk read Camera Info (Valorant-Style) to reduce Syscalls
                struct CamInfo {
                    Vector3 Rotation; // 0xA10
                    char _pad[16];    // 0xA1C - 0xA2C (Chính xác 16 byte padding)
                    float FOV;        // 0xA2C
                    Vector3 Location; // 0xA30
                } ci;

                if (ReadMemory(camMgr + telemetryOffsets::CameraCacheRotation, &ci, sizeof(CamInfo))) {
                    G_CameraLocation = ci.Location;
                    G_CameraRotation = ci.Rotation;
                    G_CamFOV = ci.FOV;
                }
            }

            // Sync Local Player Position in Realtime
            uint64_t rootComp = ReadXe(G_LocalPawn + telemetryOffsets::RootComponent);
            if (rootComp) G_LocalPlayerPos = Read<Vector3>(rootComp + telemetryOffsets::ComponentLocation);
            
            uint64_t movement = Read<uint64_t>(G_LocalPawn + telemetryOffsets::CharacterMovement);
            if (movement) {
                G_LocalPlayerVelocity = Read<Vector3>(movement + telemetryOffsets::LastUpdateVelocity);
                G_Radar.VehicleSpeed = G_LocalPlayerVelocity.Length() * 0.036f;
            }
        }
    }

    void SyncLivePlayers(std::vector<PlayerData>& players) {
        for (auto& p : players) {
            if (p.Distance > 300.0f) continue;
            if (p.MeshAddr) {
                uint64_t boneArray = Read<uint64_t>(p.MeshAddr + telemetryOffsets::BoneArray);
                if (boneArray) {
                    FTransform meshToWorld = Read<FTransform>(p.MeshAddr + telemetryOffsets::ComponentToWorld);
                    p.Gender = NormalizeGender(Read<uint8_t>(p.ActorPtr + telemetryOffsets::Gender));
                    const OverlayBoneIndices idx = GetOverlayBoneIndices(p.Gender);
                    p.Bone_Head = telemetryBones::GetBoneWorldPosWithMatrix(idx.head, boneArray, meshToWorld);
                    p.HeadPosition = p.Bone_Head;
                    p.Bone_Pelvis = telemetryBones::GetBoneWorldPosWithMatrix(idx.pelvis, boneArray, meshToWorld);
                    p.Bone_LFoot = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftFoot, boneArray, meshToWorld);
                    p.Bone_RFoot = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightFoot, boneArray, meshToWorld);
                    p.FeetPosition = (p.Bone_LFoot.z < p.Bone_RFoot.z) ? p.Bone_LFoot : p.Bone_RFoot;

                    p.Bone_Neck = telemetryBones::GetBoneWorldPosWithMatrix(idx.neck, boneArray, meshToWorld);
                    p.Bone_Chest = telemetryBones::GetBoneWorldPosWithMatrix(idx.chest, boneArray, meshToWorld);
                    p.Bone_LShoulder = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftShoulder, boneArray, meshToWorld);
                    p.Bone_LElbow = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftElbow, boneArray, meshToWorld);
                    p.Bone_LHand = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftHand, boneArray, meshToWorld);
                    p.Bone_RShoulder = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightShoulder, boneArray, meshToWorld);
                    p.Bone_RElbow = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightElbow, boneArray, meshToWorld);
                    p.Bone_RHand = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightHand, boneArray, meshToWorld);
                    p.Bone_LThigh = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftThigh, boneArray, meshToWorld);
                    p.Bone_LKnee = telemetryBones::GetBoneWorldPosWithMatrix(idx.leftKnee, boneArray, meshToWorld);
                    p.Bone_RThigh = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightThigh, boneArray, meshToWorld);
                    p.Bone_RKnee = telemetryBones::GetBoneWorldPosWithMatrix(idx.rightKnee, boneArray, meshToWorld);
                }
            }
        }
    }
}
