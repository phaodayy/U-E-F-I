#pragma once
#include <cstdint>

// MÔI TRƯỜNG KERNEL DETECTION
#if defined(_KERNEL_MODE) || defined(KM)
#include <ntifs.h>
#include <intrin.h>
#else
#include <windows.h>
#include <intrin.h>
#endif

namespace pubg_config {
    inline const char* version = "2603.1.2.7";
    namespace offsets {
        // --- 1. CORE ANTI-CHEAT (Verified: 2026-04-23) ---
        inline uint64_t XenuineDecrypt = 0x1079C028;
        inline uint64_t UWorld = 0x1225F938;
        inline uint64_t GNames = 0x124EF760;
        inline uint64_t GObjects = 0x12220570; 
        inline uint64_t GNamesPtr = 0x10;
        inline uint64_t ChunkSize = 0x3E4C;
        inline uint64_t ObjID = 0x20;

        // --- 2. UNREAL ENGINE CORE ---
        inline uint64_t CurrentLevel = 0x800;
        inline uint64_t Actors = 0x38;
        inline uint64_t ActorsForGC = 0x7D0;
        inline uint64_t GameInstance = 0x3B0;
        inline uint64_t GameState = 0x278;
        inline uint64_t LocalPlayer = 0xF0;
        inline uint64_t PlayerController = 0x38;
        inline uint64_t AcknowledgedPawn = 0x4A8;
        inline uint64_t PlayerCameraManager = 0x4D0;
        inline uint64_t MyHUD = 0x4C8;
        inline uint64_t PlayerState = 0x418;
        inline uint64_t RootComponent = 0x308;
        inline uint64_t ControlRotation = 0x40C; // Kd
        inline uint64_t Character = 0x438;
        inline uint64_t Pawn = 0x448;

        // --- 3. NAME DECRYPT PARAMS ---
        inline uint64_t DecryptNameIndexRor = 0x1;
        inline uint64_t DecryptNameIndexXorKey1 = 0x7360F24;
        inline uint64_t DecryptNameIndexXorKey2 = 0xB621EC05;
        inline uint64_t DecryptNameIndexXorKey3 = 0x1FF0000;
        inline uint64_t DecryptNameIndexRval = 0x17;
        inline uint64_t DecryptNameIndexSval = 0x7;
        inline uint64_t DecryptNameIndexDval = 0x19;

        // --- 4. CAMERA & HUD ---
        inline uint64_t ViewTarget = 0x1050;
        inline uint64_t CameraCacheLocation = 0xA30;
        inline uint64_t CameraCacheRotation = 0xA10;
        inline uint64_t CameraCacheFOV = 0xA2C;
        inline uint64_t SpectatorPawn = 0x760;
        inline uint64_t DefaultFOV = 0x828;
        inline uint64_t bShowMouseCursor = 0x658;
        inline uint64_t BlockInputWidgetList = 0x5B0;
        inline uint64_t WidgetStateMap = 0x538;
        inline uint64_t Visibility = 0xA9;
        inline uint64_t Minimap = 0x480;
        inline uint64_t CurrentMinimapViewScale = 0x4A4;
        inline uint64_t ScreenPosX = 0x4B8;
        inline uint64_t ScreenPosY = 0x4BC;
        inline uint64_t ScreenSize = 0x4A8;
        inline uint64_t SelectMinimapSizeIndex = 0x5C8;
        inline uint64_t Slot = 0x38;
        inline uint64_t LayoutData = 0x40;
        inline uint64_t Offsets = 0x0;
        inline uint64_t Alignment = 0x20;

        // --- 5. CHARACTER & MESH ---
        inline uint64_t Mesh = 0x4A0;
        inline uint64_t Mesh3P = 0x800;
        inline uint64_t ComponentLocation = 0x330;
        inline uint64_t ComponentToWorld = 0x320;
        inline uint64_t BoneArray = 0xAE8;
        inline uint64_t StaticMesh = 0xAE8;
        inline uint64_t BoneCount = 0xAF0;
        inline uint64_t Eyes = 0x75C;
        inline uint64_t WorldToMap = 0xA04;
        inline uint64_t CharacterState = 0x1020;
        inline uint64_t CharacterName = 0x1D70;
        inline uint64_t CharacterMovement = 0x490;
        inline uint64_t LastUpdateVelocity = 0x3E0;
        inline uint64_t LastSubmitTime = 0x758;
        inline uint64_t LastRenderTimeOnScreen = 0x75C;
        inline uint64_t TeamNumber = 0x7C4;
        inline uint64_t LastTeamNum = 0x2A98;
        inline uint64_t SpectatedCount = 0x113C;
        inline uint64_t bAlwaysCreatePhysicsState = 0x488;

        // --- 6. WORLD & RADIUS ---
        inline uint64_t FeatureRepObject = 0xCF0;
        inline uint64_t SafetyZonePosition = 0xB0;
        inline uint64_t SafetyZoneRadius = 0xBC;
        inline uint64_t BlueZoneRadius = 0xCC;
        inline uint64_t BlueZonePosition = 0xC0;
        inline uint64_t NumAliveTeams = 0x480;

        // --- 7. HEALTH SYSTEM ---
        inline uint64_t HeaFlag = 0x3B9;
        inline uint64_t Health1 = 0xA3C;
        inline uint64_t Health2 = 0xA38;
        inline uint64_t Health3 = 0xA24;
        inline uint64_t Health4 = 0xA10;
        inline uint64_t Health5 = 0xA25;
        inline uint64_t Health6 = 0xA20;
        inline uint64_t GroggyHealth = 0x14B0;

        inline uint32_t HealthKey0 = 0xCEC7A593;
        inline uint32_t HealthKey1 = 0x9B63B2A7;
        inline uint32_t HealthKey2 = 0xCAD3C3A5;
        inline uint32_t HealthKey3 = 0xA738484B;
        inline uint32_t HealthKey4 = 0xCC911D0A;
        inline uint32_t HealthKey5 = 0x23DDA185;
        inline uint32_t HealthKey6 = 0x09454BC8;
        inline uint32_t HealthKey7 = 0xA521BA21;
        inline uint32_t HealthKey8 = 0x0BA17A58; 
        inline uint32_t HealthKey9 = 0xB0EFA787;
        inline uint32_t HealthKey10 = 0xE275B2BA;
        inline uint32_t HealthKey11 = 0x878ADBD0;
        inline uint32_t HealthKey12 = 0xBDCC62D5;
        inline uint32_t HealthKey13 = 0xA7934B07;
        inline uint32_t HealthKey14 = 0x4B099E38;
        inline uint32_t HealthKey15 = 0xEEDB2A7D;

        inline uint32_t HealthKeys[] = {
            HealthKey0, HealthKey1, HealthKey2, HealthKey3,
            HealthKey4, HealthKey5, HealthKey6, HealthKey7,
            HealthKey8, HealthKey9, HealthKey10, HealthKey11,
            HealthKey12, HealthKey13, HealthKey14, HealthKey15
        };

        // --- 8. INVENTORY & STATS ---
        inline uint64_t LocalPlayers = 0x124172D0; // kd
        inline uint64_t PlayerArray = 0x410;
        inline uint64_t AccountId = 0x810;
        inline uint64_t PlayerName = 0x420;
        inline uint64_t PlayerStatusType = 0x468;
        inline uint64_t SquadMemberIndex = 0xA1C;
        inline uint64_t PlayerStatistics = 0xA10;
        inline uint64_t DamageDealtOnEnemy = 0x804;
        inline uint64_t ping = 0x3F8;
        inline uint64_t MatchId = 0x510;
        inline uint64_t PartnerLevel = 0x716;
        inline uint64_t SurvivalTier = 0xCC8;
        inline uint64_t SurvivalLevel = 0xCCC;
        inline uint64_t PubgIdData = 0xCC0;
        inline uint64_t CharacterClanInfo = 0x838;

        // --- 9. WEAPONS ---
        inline uint64_t EquippedWeapons = 0x208;
        inline uint64_t WeaponProcessor = 0x968;
        inline uint64_t CurrentWeaponIndex = 0x319;
        inline uint64_t WeaponTrajectoryData = 0x11A8;
        inline uint64_t TrajectoryGravityZ = 0x106C;
        inline uint64_t FiringAttachPoint = 0x8C0;
        inline uint64_t ScopingAttachPoint = 0xB20;
        inline uint64_t TrajectoryConfig = 0x108;
        inline uint64_t BallisticCurve = 0x28;
        inline uint64_t FloatCurves = 0x38;

        inline uint64_t WeaponConfig_WeaponClass = 0x798;
        inline uint64_t CurrentAmmoData = 0xE68;
        inline uint64_t ElapsedCookingTime = 0xB50;
        inline uint64_t AimOffsets = 0x1AB8;
        inline uint64_t RecoilValueVector = 0x1198;
        inline uint64_t VerticalRecovery = 0x10D8;
        inline uint64_t AttachedItems = 0x858;
        inline uint64_t WeaponAttachmentData = 0x128;

        // --- 10. INPUT & AIM ---
        inline uint64_t PlayerInput = 0x548;
        inline uint64_t InputAxisProperties = 0x138;
        inline uint64_t InputYawScale = 0x64C;
        inline uint64_t bIsScoping_CP = 0x85D;
        inline uint64_t bIsReloading_CP = 0x735;
        inline uint64_t RecoilADSRotation_CP = 0x824;
        inline uint64_t ControlRotation_CP = 0x64C;
        inline uint64_t LeanLeftAlpha_CP = 0x694;
        inline uint64_t LeanRightAlpha_CP = 0x698;

        // --- 11. ITEMS & LOOT ---
        inline uint64_t InventoryFacade = 0x1728;
        inline uint64_t Inventory = 0x400;
        inline uint64_t InventoryItems = 0x620;
        inline uint64_t InventoryItemCount = 0x628;
        inline uint64_t InventoryItemTagItemCount = 0x40;
        inline uint64_t ItemTable = 0xB0;
        inline uint64_t ItemID = 0x244;
        inline uint64_t Equipment = 0x408;
        inline uint64_t ItemsArray = 0x4F8;
        inline uint64_t DroppedItemGroup = 0x1C0;
        inline uint64_t ItemPackageItems = 0x578;
        inline uint64_t DroppedItemGroupUItem = 0x870;
        inline uint64_t DroppedItem = 0x458;
        inline uint64_t Durability = 0x1E4;
        inline uint64_t Durabilitymax = 0x1E0;

        // --- 12. VEHICLES ---
        inline uint64_t VehicleMovement = 0x468;
        inline uint64_t VehicleRiderComponent = 0x2050;
        inline uint64_t ReplicatedMovement = 0xD0;
        inline uint64_t LastVehiclePawn = 0x270;
        inline uint64_t SeatIndex = 0x230;
        inline uint64_t Wheels = 0x328;
        inline uint64_t WheelLocation = 0x100;
        inline uint64_t DampingRate = 0x54;
        inline uint64_t ShapeRadius = 0x48;
        inline uint64_t VehicleCommonComponent = 0xB30;
        inline uint64_t FloatingVehicleCommonComponent = 0x4D8;
        inline uint64_t VehicleFuel = 0x2E0;
        inline uint64_t VehicleFuelMax = 0x2E4;
        inline uint64_t VehicleHealth = 0x2D8;
        inline uint64_t VehicleHealthMax = 0x2DC;

        // --- 13. ANIMATION & ENGINE ---
        inline uint64_t AnimScriptInstance = 0xE30;
        inline uint64_t PreEvalPawnState = 0x630;
        inline uint64_t AntiCheatCharacterSyncManager = 0xD28;
        inline uint64_t PhysxSDK = 0x11B90808;
        inline uint64_t TimeSeconds = 0x810;
        inline uint64_t TimeTillExplosion = 0x824;
        inline uint64_t ExplodeState = 0x628;
        inline uint64_t TrainingMapGrid = 0x5B0;
        inline uint64_t MortarRotation = 0x520;
        inline uint64_t MortarEntity = 0x108;
        inline uint64_t MapGrid_Map = 0x4A8;
        inline uint64_t Gender = 0xAC8;
        inline uint64_t MouseX = 0x4BF5;
        inline uint64_t MouseY = 0x4BF6;

        // --- 14. MISC ---
        inline uint64_t AttachedStaticComponentMap = 0x1508;
        inline uint64_t StaticSockets = 0xC8;
        inline uint64_t StaticSocketName = 0x30;
        inline uint64_t StaticRelativeScale = 0x50;
        inline uint64_t StaticRelativeLocation = 0x38;
        inline uint64_t StaticRelativeRotation = 0x44;
        inline uint64_t Keys = 0x60;

        // --- 15. SPOOF CALL GADGETS ---
        inline uint64_t SPOOFCALL_GADGET = 0x2423775;
        inline uint64_t LineTraceSingle = 0x83153C;
        inline uint64_t HOOK = 0x11AE68E8;
        inline uint64_t HOOK_TWO = 0xCFEDBAB;
    }

    __forceinline uint32_t decrypt_cindex(uint32_t v) {
        using namespace offsets;
        uint32_t x = v ^ (uint32_t)DecryptNameIndexXorKey1;
        return (((x << (int)DecryptNameIndexDval) | (x >> (int)DecryptNameIndexSval) & (uint32_t)DecryptNameIndexXorKey3) ^ _rotr(x, (int)DecryptNameIndexRval) ^ (uint32_t)DecryptNameIndexXorKey2);
    }
}

// Master Scanner v2.5 Output: All Core Offsets, Name Decryption, and Bone Structure have been verified and synchronized.
