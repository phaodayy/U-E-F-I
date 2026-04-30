#include "context.hpp"
#include "offsets.hpp"
#include "runtime_offsets.hpp"
#include "telemetry_decrypt.hpp"
#include "scanner.hpp"
#include "fname.hpp"
#include "Common/Data.h"
#include "../overlay/core/overlay_menu.hpp"
#include <iostream>
#include <algorithm>
#include <set>
#include <array>
#include <cctype>
#include <utility>
#include <initializer_list>
#include <unordered_map>

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
std::mutex G_PlayersMutex;
std::mutex CachedItemsMutex;
std::vector<DebugActorData> G_DebugActors;
std::mutex G_DebugActorsMutex;
float G_CamFOV = 103.0f;
extern std::string global_account_role;

namespace {
    constexpr float kOverlayVisibilityThreshold = 0.05f;
    const char* g_LastInitializeStatus = "not-started";

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

    struct ShotState {
        uint64_t weapon = 0;
        int lastAmmo = -1;
        uint64_t lastShotTimeMs = 0;
        uint64_t lastSeenTimeMs = 0;
    };

    std::unordered_map<uint64_t, ShotState> g_PlayerShotStates;

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

    __forceinline uint64_t ReadXe(uint64_t addr) {
        uint64_t val = 0;
        if (telemetryMemory::ReadMemory(addr, &val, sizeof(uint64_t)))
            return telemetryDecrypt::Xe(val);
        return 0;
    }

    bool IsVehicleActorName(const std::string& name) {
        return name.find(skCrypt("Vehicle")) != std::string::npos || 
               name.find(skCrypt("BP_")) == 0 ||
               name.find(skCrypt("Dacia")) != std::string::npos ||
               name.find(skCrypt("Uaz")) != std::string::npos ||
               name.find(skCrypt("Buggy")) != std::string::npos ||
               name.find(skCrypt("Motorbike")) != std::string::npos ||
               name.find(skCrypt("Mirado")) != std::string::npos ||
               name.find(skCrypt("Pickup")) != std::string::npos ||
               name.find(skCrypt("Rony")) != std::string::npos;
    }

    std::string ResolveDroppedItemName(uint64_t actor) {
        uint64_t uItem = ReadXe(actor + telemetryOffsets::DroppedItem);
        if (uItem > 0x1000000) {
            uint32_t rawID = Read<uint32_t>(uItem + telemetryOffsets::ObjID);
            return FNameUtils::GetNameFast(telemetryOffsets::DecryptCIndex(rawID));
        }
        return "";
    }

    void AppendDroppedItemGroupItems(uint64_t actor, const Vector3& pos, std::vector<ItemData>& list) {
        uint64_t listPtr = Read<uint64_t>(actor + telemetryOffsets::DroppedItemGroup);
        int cnt = Read<int>(actor + telemetryOffsets::DroppedItemGroup + 0x8);
        if (listPtr > 0x1000000 && cnt > 0 && cnt < 50) {
            for (int j = 0; j < cnt; j++) {
                uint64_t comp = Read<uint64_t>(listPtr + (j * 0x10) + 0x8); 
                if (comp > 0x1000000) {
                    uint64_t uItem = Read<uint64_t>(comp + telemetryOffsets::DroppedItemGroupUItem);
                    if (uItem > 0x1000000) {
                        uint32_t rawID = Read<uint32_t>(uItem + telemetryOffsets::ObjID);
                        std::string rawName = FNameUtils::GetNameFast(telemetryOffsets::DecryptCIndex(rawID));
                        if (!rawName.empty()) {
                            float dist = G_CameraLocation.Distance(pos) / 100.0f;
                            ItemData item{ pos, rawName, "", dist, false, ItemRenderType::Loot };
                            list.push_back(item);
                        }
                    }
                }
            }
        }
    }

    float GetMapWorldSize(const std::string& mapName) {
        static const std::array<std::pair<const char*, float>, 15> kMapSizes = {{
            {skCrypt("Tiger_Main"), 408000.0f},
            {skCrypt("Kiki_Main"), 408000.0f},
            {skCrypt("Desert_Main"), 408000.0f},
            {skCrypt("Range_Main"), 101175.0f},
            {skCrypt("Summerland_Main"), 101175.0f},
            {skCrypt("Italy_Main"), 101175.0f},
            {skCrypt("Baltic_Main"), 408000.0f},
            {skCrypt("Neon_Main"), 408000.0f},
            {skCrypt("Heaven_Main"), 51420.0f},
            {skCrypt("Savage_Main"), 202387.5f},
            {skCrypt("DihorOtok_Main"), 408000.0f},
            {skCrypt("Chimera_Main"), 152950.0f},
            {skCrypt("Boardwalk_Main"), 51420.0f},
            {skCrypt("Narrows_Main"), 51420.0f},
            {skCrypt("Pinnacle_Main"), 51420.0f}
        }};

        for (const auto& entry : kMapSizes) {
            if (mapName.find(entry.first) != std::string::npos) {
                return entry.second;
            }
        }
        return 408000.0f;
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

    float NormalizeAngle(float angle) {
        while (angle < -180.0f) angle += 360.0f;
        while (angle > 180.0f) angle -= 360.0f;
        return angle;
    }

    float AbsAngleDelta(float a, float b) {
        return std::fabs(NormalizeAngle(a - b));
    }

    void ReadAimAngles(uint64_t actor, PlayerData& player) {
        player.HasAimYaw = false;
        player.IsAimingAtLocal = false;
        if (actor <= 0x1000000) return;

        const Vector3 aimOffsets = Read<Vector3>(actor + telemetryOffsets::AimOffsets);
        if (!std::isfinite(aimOffsets.y) || std::fabs(aimOffsets.y) > 720.0f) return;

        player.AimPitch = std::isfinite(aimOffsets.x) ? std::clamp(aimOffsets.x, -180.0f, 180.0f) : 0.0f;
        player.AimYaw = NormalizeAngle(aimOffsets.y);
        player.HasAimYaw = true;
    }

    void UpdateAimThreat(PlayerData& player) {
        player.IsAimingAtLocal = false;
        if (!player.HasAimYaw || player.IsTeammate || player.Health <= 0.0f || player.Distance > 600.0f ||
            player.Position.IsZero() || G_LocalPlayerPos.IsZero()) {
            return;
        }

        const float dx = G_LocalPlayerPos.x - player.Position.x;
        const float dy = G_LocalPlayerPos.y - player.Position.y;
        const float horizontal = std::sqrt(dx * dx + dy * dy);
        if (horizontal < 1.0f) return;

        const float yawToLocal = std::atan2(dy, dx) * 57.2957795f;
        const float dz = (G_LocalPlayerPos.z + 80.0f) - (player.Position.z + 80.0f);
        const float pitchToLocal = std::atan2(dz, horizontal) * 57.2957795f;
        const float yawDelta = AbsAngleDelta(player.AimYaw, yawToLocal);
        const float pitchDelta = std::fabs(player.AimPitch - pitchToLocal);

        player.IsAimingAtLocal = yawDelta <= 9.0f && pitchDelta <= 25.0f;
    }

    void ReadPlayerStats(uint64_t playerState, PlayerData& player) {
        if (playerState <= 0x1000000) return;

        const int kills = Read<int>(playerState + telemetryOffsets::PlayerStatistics);
        if (kills >= 0 && kills < 100) {
            player.Kills = kills;
        }

        const float damage = Read<float>(playerState + telemetryOffsets::DamageDealtOnEnemy);
        if (std::isfinite(damage) && damage >= 0.0f && damage < 30000.0f) {
            player.DamageDealt = damage;
        }

        const int tier = Read<int>(playerState + telemetryOffsets::SurvivalTier);
        const int level = Read<int>(playerState + telemetryOffsets::SurvivalLevel);
        if (tier > 0 && tier < 20 && level > 0 && level <= 500) {
            player.SurvivalLevel = (tier - 1) * 500 + level;
        } else if (level > 0 && level < 10000) {
            player.SurvivalLevel = level;
        }
    }

    void ReadAnimState(uint64_t mesh, PlayerData& player) {
        player.IsScoping = false;
        player.IsReloading = false;
        if (mesh <= 0x1000000) return;

        const uint64_t anim = Read<uint64_t>(mesh + telemetryOffsets::AnimScriptInstance);
        if (anim <= 0x1000000) return;

        player.IsScoping = Read<uint8_t>(anim + telemetryOffsets::bIsScoping_CP) != 0;
        player.IsReloading = Read<uint8_t>(anim + telemetryOffsets::bIsReloading_CP) != 0;
    }

    void ReadWeaponAmmo(uint64_t weapon, PlayerData& player) {
        player.HasAmmo = false;
        player.Ammo = 0;
        player.AmmoMax = 0;
        player.IsFiring = false;
        if (weapon <= 0x1000000) return;

        const uint64_t now = GetTickCount64();
        const int ammo = Read<int>(weapon + telemetryOffsets::CurrentAmmoData);
        const int ammoMax = Read<int>(weapon + telemetryOffsets::CurrentAmmoData - sizeof(int));
        if (ammo >= 0 && ammo <= 500) {
            player.Ammo = ammo;
            player.HasAmmo = true;
        }
        if (ammoMax > 0 && ammoMax <= 500) {
            player.AmmoMax = ammoMax;
        }

        if (!player.HasAmmo || player.ActorPtr <= 0x1000000) return;

        ShotState& state = g_PlayerShotStates[player.ActorPtr];
        if (state.weapon != weapon) {
            state.weapon = weapon;
            state.lastAmmo = player.Ammo;
            state.lastShotTimeMs = 0;
            state.lastSeenTimeMs = now;
            player.LastShotTimeMs = 0;
            return;
        }

        if (state.lastAmmo >= 0 && player.Ammo < state.lastAmmo && !player.IsReloading) {
            const int spent = state.lastAmmo - player.Ammo;
            if (spent > 0 && spent <= 50) {
                state.lastShotTimeMs = now;
            }
        }

        state.lastAmmo = player.Ammo;
        state.lastSeenTimeMs = now;
        player.LastShotTimeMs = state.lastShotTimeMs;
        player.IsFiring = state.lastShotTimeMs != 0 && (now - state.lastShotTimeMs) <= 220;

        if (g_PlayerShotStates.size() > 256) {
            for (auto it = g_PlayerShotStates.begin(); it != g_PlayerShotStates.end(); ) {
                if (now - it->second.lastSeenTimeMs > 15000) {
                    it = g_PlayerShotStates.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
}

namespace telemetryContext {

    bool ReadMemory(uint64_t src, void* dest, uint64_t size) {
        return telemetryMemory::ReadMemory(src, dest, size);
    }

    uint64_t GetBaseAddress() { return telemetryMemory::g_BaseAddress; }

    const char* GetLastInitializeStatus() {
        return g_LastInitializeStatus;
    }

    bool TryInitializeWorldFromCurrentOffsets(uint64_t baseAddress) {
        if (!baseAddress) {
            g_LastInitializeStatus = "base-address-empty";
            return false;
        }

        if (!telemetryDecrypt::Initialize(telemetryMemory::ReadMemory, baseAddress, telemetryOffsets::XenuineDecrypt)) {
            g_LastInitializeStatus = "decrypt-init-failed";
            return false;
        }

        uint64_t rawUWorld = 0;
        if (!telemetryMemory::ReadMemory(baseAddress + telemetryOffsets::UWorld, &rawUWorld, sizeof(uint64_t))) {
            g_LastInitializeStatus = "uworld-read-failed";
            return false;
        }

        G_UWorld = telemetryDecrypt::Xe(rawUWorld);
        if (G_UWorld > 0x1000000) {
            g_LastInitializeStatus = "ready";
            return true;
        }

        g_LastInitializeStatus = rawUWorld ? "uworld-decrypt-invalid" : "uworld-empty";
        return false;
    }

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

    static bool IsValidGamePtr(uint64_t value) {
        return value > 0x1000000;
    }

    static bool HasAnyToken(const std::string& value, const std::initializer_list<const char*>& tokens) {
        for (const char* token : tokens) {
            if (value.find(token) != std::string::npos) return true;
        }
        return false;
    }

    static bool IsVehicleActorName(const std::string& className) {
        return HasAnyToken(className, {
            "Vehicle", "Uaz", "Dacia", "Buggy", "Motorbike", "Dirtbike",
            "Boat", "PG117", "AquaRail", "AirBoat", "BRDM", "Scooter",
            "Snowbike", "Snowmobile", "Niva", "Tuk", "MiniBus", "Van",
            "LootTruck", "Food_Truck", "Train", "Mirado", "PickupTruck",
            "Rony", "Blanc", "Motorglider", "Bicycle", "Mountainbike",
            "Rubber_boat", "Rubberboat", "PonyCoupe", "PicoBus",
            "ATV", "Porter", "Pillar_Car", "CoupeRB", "McLaren",
            "DBX", "Vantage", "Urus", "Countach", "Classic",
            "Panigale", "BearV2", "StrongBox"
        });
    }

    static std::string ResolveItemNameFromTable(uint64_t itemTable) {
        if (!IsValidGamePtr(itemTable)) return "";

        const int32_t itemId = Read<int32_t>(itemTable + telemetryOffsets::ItemID);
        if (itemId <= 0) return "";

        return FNameUtils::GetNameFast(itemId);
    }

    static std::string ResolveItemNameFromUItem(uint64_t uItem) {
        if (!IsValidGamePtr(uItem)) return "";

        const uint64_t itemTable = Read<uint64_t>(uItem + telemetryOffsets::ItemTable);
        std::string name = ResolveItemNameFromTable(itemTable);
        if (!name.empty()) return name;

        const uint32_t rawObjId = Read<uint32_t>(uItem + telemetryOffsets::ObjID);
        if (rawObjId == 0) return "";

        const uint32_t objId = telemetryOffsets::DecryptCIndex(rawObjId);
        return FNameUtils::GetNameFast(objId);
    }

    static int ExtractGearLevel(const std::string& itemName) {
        if (itemName.find(skCrypt("Lv3")) != std::string::npos ||
            itemName.find(skCrypt("_G_")) != std::string::npos ||
            itemName.find(skCrypt("_C_01")) != std::string::npos) return 3;
        if (itemName.find(skCrypt("Lv2")) != std::string::npos ||
            itemName.find(skCrypt("_F_")) != std::string::npos ||
            itemName.find(skCrypt("_D_")) != std::string::npos) return 2;
        if (itemName.find(skCrypt("Lv1")) != std::string::npos ||
            itemName.find(skCrypt("_E_")) != std::string::npos) return 1;
        return 0;
    }

    static float ReadDurabilityPercent(uint64_t item) {
        const float cur = Read<float>(item + telemetryOffsets::Durability);
        const float max = Read<float>(item + telemetryOffsets::Durabilitymax);
        if (!std::isfinite(cur) || !std::isfinite(max) || max <= 1.0f) return 0.0f;
        return std::clamp((cur / max) * 100.0f, 0.0f, 100.0f);
    }

    static void ReadPlayerEquipment(uint64_t actor, PlayerData& player) {
        player.HelmetLevel = 0;
        player.VestLevel = 0;
        player.BackpackLevel = 0;
        player.HelmetDurability = 0.0f;
        player.VestDurability = 0.0f;
        player.BackpackDurability = 0.0f;
        if (!IsValidGamePtr(actor)) return;

        const uint64_t inventoryFacade = ReadXe(actor + telemetryOffsets::InventoryFacade);
        if (!IsValidGamePtr(inventoryFacade)) return;

        uint64_t equipment = Read<uint64_t>(inventoryFacade + telemetryOffsets::Equipment);
        if (!IsValidGamePtr(equipment)) {
            equipment = ReadXe(inventoryFacade + telemetryOffsets::Equipment);
        }
        if (!IsValidGamePtr(equipment)) return;

        const uint64_t items = Read<uint64_t>(equipment + telemetryOffsets::ItemsArray);
        const int32_t count = Read<int32_t>(equipment + telemetryOffsets::ItemsArray + 0x8);
        if (!IsValidGamePtr(items) || count <= 0 || count > 32) return;

        for (int32_t i = 0; i < count; ++i) {
            const uint64_t item = Read<uint64_t>(items + i * sizeof(uint64_t));
            if (!IsValidGamePtr(item)) continue;

            const std::string itemName = ResolveItemNameFromUItem(item);
            if (itemName.empty()) continue;

            const int level = ExtractGearLevel(itemName);
            const float durability = ReadDurabilityPercent(item);
            if (itemName.find(skCrypt("Head")) != std::string::npos ||
                itemName.find(skCrypt("Helmet")) != std::string::npos) {
                player.HelmetLevel = (std::max)(player.HelmetLevel, level);
                player.HelmetDurability = (std::max)(player.HelmetDurability, durability);
            } else if (itemName.find(skCrypt("Armor")) != std::string::npos ||
                       itemName.find(skCrypt("Vest")) != std::string::npos) {
                player.VestLevel = (std::max)(player.VestLevel, level);
                player.VestDurability = (std::max)(player.VestDurability, durability);
            } else if (itemName.find(skCrypt("Back")) != std::string::npos ||
                       itemName.find(skCrypt("Backpack")) != std::string::npos) {
                player.BackpackLevel = (std::max)(player.BackpackLevel, level);
                player.BackpackDurability = (std::max)(player.BackpackDurability, durability);
            }
        }
    }

    static std::string ResolveDroppedItemName(uint64_t actor) {
        const uint64_t droppedItem = ReadXe(actor + telemetryOffsets::DroppedItem);
        return ResolveItemNameFromUItem(droppedItem);
    }

    static void AppendDroppedItemGroupItems(uint64_t actor, const Vector3& fallbackPos, std::vector<ItemData>& outItems) {
        const uint64_t groupArray = Read<uint64_t>(actor + telemetryOffsets::DroppedItemGroup);
        const int32_t groupCount = Read<int32_t>(actor + telemetryOffsets::DroppedItemGroup + 0x8);
        if (!IsValidGamePtr(groupArray) || groupCount <= 0 || groupCount > 100) return;

        for (int32_t idx = 0; idx < groupCount && outItems.size() < 400; ++idx) {
            const uint64_t itemGroupComponent = Read<uint64_t>(groupArray + (idx * 0x10));
            if (!IsValidGamePtr(itemGroupComponent)) continue;

            const uint64_t uItem = Read<uint64_t>(itemGroupComponent + telemetryOffsets::DroppedItemGroupUItem);
            const std::string itemName = ResolveItemNameFromUItem(uItem);
            if (itemName.empty()) continue;

            Vector3 itemPos = Read<Vector3>(itemGroupComponent + telemetryOffsets::ComponentLocation);
            if (itemPos.IsZero()) itemPos = fallbackPos;
            if (itemPos.IsZero()) continue;

            const float dist = G_CameraLocation.Distance(itemPos) / 100.0f;
            if (dist <= 0.0f || dist > 100.0f) continue;

            ItemData item{};
            item.Position = itemPos;
            item.Distance = dist;
            item.Name = itemName;
            item.IsImportant = itemName.find(skCrypt("Item_Weapon_")) == 0 ||
                itemName.find(skCrypt("Flare")) != std::string::npos;
            outItems.push_back(item);
        }
    }

    bool Initialize(uint32_t process_id, uint64_t base_address) {
        static uint32_t lastScanPid = 0;
        static uint64_t lastScanBase = 0;
        static bool scannedRuntimeForProcess = false;
        static ULONGLONG lastRuntimeScanTick = 0;
        static uint32_t failedInitCount = 0;
        constexpr ULONGLONG kRuntimeScanRetryMs = 15000;

        telemetryMemory::g_ProcessId = process_id;
        telemetryMemory::g_BaseAddress = base_address;
        if (!telemetryMemory::RefreshProcessContext()) {
            g_LastInitializeStatus = "process-context-refresh-failed";
            return false;
        }
        if (telemetryMemory::g_BaseAddress == 0) {
            telemetryMemory::g_BaseAddress = base_address;
        }
        if (telemetryMemory::g_BaseAddress == 0) {
            g_LastInitializeStatus = "base-address-empty";
            return false;
        }

        if (lastScanPid != process_id || lastScanBase != telemetryMemory::g_BaseAddress) {
            lastScanPid = process_id;
            lastScanBase = telemetryMemory::g_BaseAddress;
            scannedRuntimeForProcess = false;
            lastRuntimeScanTick = 0;
            failedInitCount = 0;
            telemetryRuntimeOffsets::InvalidateRuntimeScanCache();
        }

        if (TryInitializeWorldFromCurrentOffsets(telemetryMemory::g_BaseAddress)) {
#ifdef _DEBUG
            std::cout << skCrypt("[INIT] offset source=compiled-static status=ready") << std::endl;
#endif
            failedInitCount = 0;
            return true;
        }

        ++failedInitCount;
        const ULONGLONG now = GetTickCount64();
        const bool retryableInitFailure =
            std::string(g_LastInitializeStatus) == skCrypt("decrypt-init-failed") ||
            std::string(g_LastInitializeStatus) == skCrypt("uworld-read-failed") ||
            std::string(g_LastInitializeStatus) == skCrypt("uworld-decrypt-invalid") ||
            std::string(g_LastInitializeStatus) == skCrypt("uworld-empty");
        if (scannedRuntimeForProcess &&
            retryableInitFailure &&
            (lastRuntimeScanTick == 0 || now - lastRuntimeScanTick >= kRuntimeScanRetryMs)) {
            telemetryRuntimeOffsets::InvalidateRuntimeScanCache(telemetryMemory::g_BaseAddress);
            scannedRuntimeForProcess = false;
#ifdef _DEBUG
            std::cout << skCrypt("[INIT] runtime scan cache reset after ")
                      << failedInitCount
                      << skCrypt(" failed init attempts; status=")
                      << g_LastInitializeStatus
                      << std::endl;
#endif
        }

        if (!scannedRuntimeForProcess) {
            const char* preScanStatus = g_LastInitializeStatus;
            const bool usingRuntimeSignatures = telemetryRuntimeOffsets::ApplyRuntimeScan(telemetryMemory::g_BaseAddress);
            scannedRuntimeForProcess = true;
            lastRuntimeScanTick = GetTickCount64();
#ifdef _DEBUG
            const auto& runtimeReport = telemetryRuntimeOffsets::GetLastReport();
            std::cout << skCrypt("[INIT] offset source=")
                      << (usingRuntimeSignatures ? skCrypt("signature-scan") : skCrypt("compiled-static"))
                      << skCrypt(" pre_scan_status=") << preScanStatus
                      << skCrypt(" scanned=") << (runtimeReport.Scanned ? skCrypt("yes") : skCrypt("no"))
                      << skCrypt(" applied=") << runtimeReport.Applied
                      << skCrypt(" required=") << runtimeReport.RequiredFound
                      << skCrypt("/") << runtimeReport.RequiredTotal << std::endl;
#endif
            if (usingRuntimeSignatures && TryInitializeWorldFromCurrentOffsets(telemetryMemory::g_BaseAddress)) {
                failedInitCount = 0;
                return true;
            }
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
        static ULONGLONG nextWorldMapScanIntervalMs = 240;
        const ULONGLONG radarScanIntervalMs = G_Radar.IsWorldMapVisible ? nextWorldMapScanIntervalMs : 100;
        if (now - lastRadarScan > radarScanIntervalMs) {
            lastRadarScan = now;
            if (G_Radar.IsWorldMapVisible) {
                nextWorldMapScanIntervalMs = 200 + (GetTickCount64() % 101);
            }
            const float screenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
            const float screenHeight = (float)GetSystemMetrics(SM_CYSCREEN);

            G_Radar.IsMiniMapVisible = false;
            G_Radar.IsWorldMapVisible = false;
            G_Radar.WorldMapX = 0.0f;
            G_Radar.WorldMapY = 0.0f;
            G_Radar.WorldMapWidth = 0.0f;
            G_Radar.WorldMapHeight = 0.0f;

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
                            const float rawMiniMapViewScale = Read<float>(widget + telemetryOffsets::CurrentMinimapViewScale1D);
                            G_Radar.CurrentMinimapViewScale =
                                (std::isfinite(rawMiniMapViewScale) && rawMiniMapViewScale > 0.01f && rawMiniMapViewScale < 20.0f)
                                    ? rawMiniMapViewScale
                                    : 1.0f;

                            const uint64_t slot = Read<uint64_t>(widget + telemetryOffsets::Slot);
                            float left = 0.0f;
                            float top = 0.0f;
                            float width = 0.0f;
                            float height = 0.0f;
                            float alignX = 0.5f;
                            float alignY = 0.5f;
                            if (slot) {
                                left = Read<float>(slot + telemetryOffsets::LayoutData + 0x0);
                                top = Read<float>(slot + telemetryOffsets::LayoutData + 0x4);
                                width = Read<float>(slot + telemetryOffsets::LayoutData + 0x8);
                                height = Read<float>(slot + telemetryOffsets::LayoutData + 0xC);
                                alignX = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x0);
                                alignY = Read<float>(slot + telemetryOffsets::LayoutData + telemetryOffsets::Alignment + 0x4);

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

/*
#ifdef _DEBUG
                            static ULONGLONG lastMiniMapWidgetLog = 0;
                            static float lastMiniMapWidgetScale = 0.0f;
                            static int lastMiniMapSizeIndex = -1;
                            const bool keyMiniMapPressed = telemetryMemory::IsKeyDown('N');
                            const bool scaleChanged = std::fabs(G_Radar.CurrentMinimapViewScale - lastMiniMapWidgetScale) > 0.03f ||
                                G_Radar.SelectMinimapSizeIndex != lastMiniMapSizeIndex;
                            if ((keyMiniMapPressed || scaleChanged) && now - lastMiniMapWidgetLog > 450) {
                                lastMiniMapWidgetLog = now;
                                lastMiniMapWidgetScale = G_Radar.CurrentMinimapViewScale;
                                lastMiniMapSizeIndex = G_Radar.SelectMinimapSizeIndex;
                                std::cout << "[DEBUG][MINIMAP_WIDGET] key_n=" << (keyMiniMapPressed ? 1 : 0)
                                    << " visible=" << (G_Radar.IsMiniMapVisible ? 1 : 0)
                                    << " size_index=" << G_Radar.SelectMinimapSizeIndex
                                    << " raw_scale=" << rawMiniMapViewScale
                                    << " used_scale=" << G_Radar.CurrentMinimapViewScale
                                    << " slot=0x" << std::hex << slot << std::dec
                                    << " layout=(" << left << "," << top << "," << width << "," << height << ")"
                                    << " align=(" << alignX << "," << alignY << ")"
                                    << " rect=(" << G_Radar.ScreenPosX << "," << G_Radar.ScreenPosY
                                    << "," << G_Radar.ScreenSize << "," << G_Radar.ScreenSizeY << ")"
                                    << std::endl;
                            }
#endif
*/
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

                                const float layoutZoom = width / 1080.0f;
                                const float zoomState = Read<float>(widget + 0xA28);
                                G_Radar.WorldMapZoomFactor = (layoutZoom > 0.001f)
                                    ? layoutZoom
                                    : ((zoomState > 0.001f) ? zoomState : 1.0f);
                                
                                const float currentPosX = (width * (alignX - 0.5f)) - left;
                                const float currentPosY = (height * (alignY - 0.5f)) - top;
                                const float safeZoom = (G_Radar.WorldMapZoomFactor > 0.001f)
                                    ? G_Radar.WorldMapZoomFactor
                                    : 1.0f;
                                
                                G_Radar.WorldMapPosition = {
                                    currentPosX / 1080.0f / safeZoom * 2.0f,
                                    currentPosY / 1080.0f / safeZoom * 2.0f
                                };
                            }
                        }
                    }
                }
            }

            if (now - lastMapInfoUpdate > 3000 || G_Radar.MapWorldSize <= 0.0f) {
                lastMapInfoUpdate = now;
                const std::string mapName = FNameUtils::GetNameFast(
                    telemetryOffsets::DecryptCIndex(Read<uint32_t>(G_UWorld + telemetryOffsets::ObjID)));
                G_Radar.MapWorldSize = GetMapWorldSize(mapName);

                const int32_t worldOriginRawX = Read<int32_t>(G_UWorld + telemetryOffsets::WorldToMap);
                const int32_t worldOriginRawY = Read<int32_t>(G_UWorld + telemetryOffsets::WorldToMap + 0x4);
                float worldOriginX = static_cast<float>(worldOriginRawX);
                float worldOriginY = static_cast<float>(worldOriginRawY);

                if (worldOriginRawX < -1000000 || worldOriginRawX > 1000000 ||
                    worldOriginRawY < -1000000 || worldOriginRawY > 1000000) {
                    worldOriginX = 0.0f;
                    worldOriginY = 0.0f;
                }

                G_Radar.WorldOriginLocation = { worldOriginX, worldOriginY, 0.0f };
            }

            if (G_Radar.WorldMapZoomFactor <= 0.001f) {
                G_Radar.WorldMapZoomFactor = 1.0f;
            }

            if (G_Radar.MapWorldSize > 1000.0f) {
                G_Radar.MapSizeFactored = G_Radar.MapWorldSize / G_Radar.WorldMapZoomFactor;
                G_Radar.WorldCenterLocation = {
                    G_Radar.MapWorldSize * (1.0f + G_Radar.WorldMapPosition.x),
                    G_Radar.MapWorldSize * (1.0f + G_Radar.WorldMapPosition.y),
                    0.0f
                };
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
        if (!inGame) { 
            { std::lock_guard<std::mutex> lock(G_PlayersMutex); G_Players.clear(); }
            { std::lock_guard<std::mutex> lock(CachedItemsMutex); CachedItems.clear(); }
            G_LocalSpectatedCount = 0; 
            return; 
        }

        std::vector<PlayerData> tempPlayers;
        std::vector<ItemData> tempItems;
        std::vector<uint64_t> seenPlayerStates;
        float localPlayerEyes = 0.0f;

        ULONGLONG scanStartTime = GetTickCount64();
        bool isAdmin = (global_account_role == skCrypt("admin"));
        std::vector<DebugActorData> tempDebug;

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
            if (!actor) continue;

            uint32_t objID = telemetryOffsets::DecryptCIndex(Read<uint32_t>(actor + telemetryOffsets::ObjID));
            std::string cname = FNameUtils::GetNameFast(objID);

            if (isAdmin && tempDebug.size() < 1000) {
                uint64_t root = ReadXe(actor + telemetryOffsets::RootComponent);
                Vector3 p = {0,0,0};
                if (root) p = Read<Vector3>(root + telemetryOffsets::ComponentLocation);
                float dist = G_CameraLocation.Distance(p) / 100.0f;
                if (dist <= 50.0f) {
                    tempDebug.push_back({ actor, cname, p, dist });
                }
            }

            if (actor == G_LocalPawn) continue;
            
            const bool isBot = (cname.find(skCrypt("AIPawn")) != std::string::npos ||
                                cname.find(skCrypt("NPC_")) != std::string::npos ||
                                cname.find(skCrypt("ZombieNpc")) != std::string::npos ||
                                cname.find(skCrypt("UltAIPawn")) != std::string::npos ||
                                cname.find(skCrypt("ZDF2_NPC")) != std::string::npos);

            bool isPlayer = (cname.find(skCrypt("PlayerMale")) != std::string::npos ||
                             cname.find(skCrypt("PlayerFemale")) != std::string::npos ||
                             isBot);

            if (isPlayer) {
                uint64_t mesh = Read<uintptr_t>(actor + telemetryOffsets::Mesh);
                if (!mesh) continue;

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
                p.ActorPtr = actor; p.MeshAddr = mesh; p.Position = pos; p.Distance = dist; p.IsBot = isBot;
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

                ReadAnimState(mesh, p);
                ReadAimAngles(actor, p);
                
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
                ReadPlayerStats(playerState, p);
                ReadPlayerEquipment(actor, p);
                
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
                            p.WeaponPtr = weapon;
                            ReadWeaponAmmo(weapon, p);
                            int32_t wID = telemetryOffsets::DecryptCIndex(Read<int32_t>(weapon + telemetryOffsets::ObjID));
                            std::string rawWeap = FNameUtils::GetNameFast(wID);
                            
                            // Basic Clean up for UI (Replace JSON dependencies)
                            if (rawWeap.find(skCrypt("Weap")) == 0) rawWeap.erase(0, 4);
                            if (rawWeap.find(skCrypt("Item_Weapon_")) == 0) rawWeap.erase(0, 12);
                            if (rawWeap.length() >= 2 && rawWeap.substr(rawWeap.length() - 2) == skCrypt("_C")) rawWeap.erase(rawWeap.length() - 2);
                            
                            if (rawWeap == skCrypt("HK416") || rawWeap == skCrypt("DuncansHK416") || rawWeap == skCrypt("Duncans_M416")) rawWeap = skCrypt("M416");
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
                UpdateAimThreat(p);

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
                static ULONGLONG lastItemScanTime = 0;
                if (now - lastItemScanTime > 500) {
                    bool isLoot = (cname.find(skCrypt("Dropped")) != std::string::npos);
                    bool isVeh  = IsVehicleActorName(cname);
                    bool isAir  = (cname.find(skCrypt("AirDrop")) != std::string::npos || cname.find(skCrypt("CarePackage")) != std::string::npos);
                    bool isBox  = (cname.find(skCrypt("DeadBox")) != std::string::npos || cname.find(skCrypt("ItemPackage")) != std::string::npos);
                    bool isProj = (cname.find(skCrypt("Proj")) != std::string::npos || cname.find(skCrypt("Grenade")) != std::string::npos || cname.find(skCrypt("Molotov")) != std::string::npos);

                    if (isLoot || isVeh || isAir || isBox || isProj) {
                        uint64_t root = ReadXe(actor + telemetryOffsets::RootComponent);
                        if (root) {
                            Vector3 pos = Read<Vector3>(root + telemetryOffsets::ComponentLocation);
                            float dist = G_CameraLocation.Distance(pos) / 100.0f;
                            if (dist < (isLoot ? 150.0f : 1000.0f)) {
                                if (isLoot && cname.find(skCrypt("DroppedItemGroup")) != std::string::npos) {
                                    AppendDroppedItemGroupItems(actor, pos, tempItems);
                                }
                                else {
                                    ItemData item;
                                    item.Position = pos; item.Distance = dist;
                                    item.ClassName = cname;
                                    
                                    std::string cleanName = cname;
                                    if (isLoot && cleanName.find(skCrypt("DroppedItem")) != std::string::npos) {
                                        std::string itemNameReal = ResolveDroppedItemName(actor);
                                        if (!itemNameReal.empty()) cleanName = itemNameReal;
                                    }

                                    const bool isCatalogItem = cleanName.find(skCrypt("Item_")) == 0;
                                    if (!isCatalogItem) {
                                        if (cleanName.find(skCrypt("ProjItem_")) == 0) cleanName.erase(0, 9);

                                        if (cleanName.length() >= 2 && cleanName.substr(cleanName.length() - 2) == skCrypt("_C")) 
                                            cleanName.erase(cleanName.length() - 2);

                                        const size_t lastUnderscore = cleanName.find_last_of('_');
                                        if (lastUnderscore != std::string::npos && lastUnderscore > 0) {
                                            bool isNumericSuffix = true;
                                            for (size_t k = lastUnderscore + 1; k < cleanName.size(); ++k) {
                                                if (!std::isdigit(static_cast<unsigned char>(cleanName[k]))) {
                                                    isNumericSuffix = false;
                                                    break;
                                                }
                                            }
                                            if (isNumericSuffix) cleanName.resize(lastUnderscore);
                                        }
                                    }

                                    // Correct Name Logic
                                    if (isVeh) item.Name = skCrypt("Vehicle");
                                    else if (isAir) item.Name = skCrypt("Air Drop");
                                    else if (isBox) item.Name = skCrypt("Dead Box");
                                    else item.Name = cleanName;

                                    item.RenderType = isVeh ? ItemRenderType::Vehicle :
                                        (isAir ? ItemRenderType::AirDrop : (isBox ? ItemRenderType::DeadBox : 
                                        (isProj ? ItemRenderType::Projectile : ItemRenderType::Loot)));
                                    item.IsImportant = (isAir || isVeh || isProj || cleanName.find(skCrypt("Flare")) != std::string::npos);
                                    
                                    tempItems.push_back(item);
                                }
                            }
                        }
                    }
                    if (i == actorCount - 1) lastItemScanTime = now;
                }
            }
        }

        // Final swap with Mutex protection to prevent BSOD during Render
        {
            std::lock_guard<std::mutex> lock(G_PlayersMutex);
            G_Players = std::move(tempPlayers);
        }
        
        G_LastScanTime = scanStartTime;

        if (isAdmin) {
            std::lock_guard<std::mutex> lock(G_DebugActorsMutex);
            G_DebugActors = std::move(tempDebug);
        }

        static ULONGLONG lastItemScan = 0;
        if (!tempItems.empty() || (GetTickCount64() - lastItemScan > 5000)) {
            std::lock_guard<std::mutex> lock(CachedItemsMutex);
            CachedItems = std::move(tempItems);
            lastItemScan = GetTickCount64();
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
            if (p.ActorPtr) {
                ReadAimAngles(p.ActorPtr, p);
                UpdateAimThreat(p);
            }

            if (p.MeshAddr) {
                ReadAnimState(p.MeshAddr, p);
            }

            if (p.WeaponPtr) {
                ReadWeaponAmmo(p.WeaponPtr, p);
            }

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
