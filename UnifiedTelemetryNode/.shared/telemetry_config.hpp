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

#include "../../protec/skCrypt.h"

namespace telemetry_config {
    // Key bí mật dùng để mã hóa toàn bộ Offset trong Project
    constexpr uint64_t OFFSET_KEY = 0x7A2B4C1D9E8F3D1CULL;

    // Cấu trúc dữ liệu bảo mật: Tự động XOR khi gán và tự động giải mã khi sử dụng
    struct SecureOffset {
        uint64_t encrypted_val;
        
        // Constructor: Tự động mã hóa khi khai báo
        constexpr SecureOffset(uint64_t val) : encrypted_val(val ^ OFFSET_KEY) {}

        __forceinline SecureOffset& operator=(uint64_t val) {
            encrypted_val = val ^ OFFSET_KEY;
            return *this;
        }

        // Casting operator: Tự động giải mã khi lấy giá trị (Seamless Integration)
        __forceinline operator uint64_t() const {
             // Sử dụng volatile hoặc ror/rol để chống tối ưu hóa của Compiler nếu cần
            return encrypted_val ^ OFFSET_KEY;
        }
    };

    // Cấu trúc cho uint32_t (dành cho HealthKeys và các giá trị nhỏ)
    struct SecureOffset32 {
        uint32_t encrypted_val;
        constexpr SecureOffset32(uint32_t v) : encrypted_val(v ^ (uint32_t)OFFSET_KEY) {}
        __forceinline SecureOffset32& operator=(uint32_t val) {
            encrypted_val = val ^ (uint32_t)OFFSET_KEY;
            return *this;
        }
        __forceinline operator uint32_t() const { return encrypted_val ^ (uint32_t)OFFSET_KEY; }
    };

    inline auto version = skCrypt("2603.1.2.7");
    namespace offsets {
        // --- 1. CORE ANTI-integrity_monitor (XOR Encrypted) ---
        inline SecureOffset XenuineDecrypt = 0x1079C028;
        inline SecureOffset UWorld = 0x1225F938;
        inline SecureOffset GNames = 0x124EF760;
        inline SecureOffset GObjects = 0x12220570; 
        inline SecureOffset GNamesPtr = 0x10;
        inline SecureOffset ChunkSize = 0x3E4C;
        inline SecureOffset ObjID = 0x20;

        // --- 2. UNREAL ENGINE CORE ---
        inline SecureOffset CurrentLevel = 0x800;
        inline SecureOffset Actors = 0x38;
        inline SecureOffset ActorsForGC = 0x7D0;
        inline SecureOffset GameInstance = 0x3B0;
        inline SecureOffset GameState = 0x278;
        inline SecureOffset LocalPlayer = 0xF0;
        inline SecureOffset PlayerController = 0x38;
        inline SecureOffset AcknowledgedPawn = 0x4A8;
        inline SecureOffset PlayerCameraManager = 0x4D0;
        inline SecureOffset MyHUD = 0x4C8;
        inline SecureOffset PlayerState = 0x418;
        inline SecureOffset RootComponent = 0x308;
        inline SecureOffset ControlRotation = 0x40C; // Kd
        inline SecureOffset Character = 0x438;
        inline SecureOffset Pawn = 0x448;

        // --- 3. NAME DECRYPT PARAMS ---
        inline SecureOffset DecryptNameIndexRor = 0x1;
        inline SecureOffset DecryptNameIndexXorKey1 = 0x7360F24;
        inline SecureOffset DecryptNameIndexXorKey2 = 0xB621EC05;
        inline SecureOffset DecryptNameIndexXorKey3 = 0x1FF0000;
        inline SecureOffset DecryptNameIndexRval = 0x17;
        inline SecureOffset DecryptNameIndexSval = 0x7;
        inline SecureOffset DecryptNameIndexDval = 0x19;

        // --- 4. CAMERA & HUD ---
        inline SecureOffset ViewTarget = 0x1050;
        inline SecureOffset CameraCacheLocation = 0xA30;
        inline SecureOffset CameraCacheRotation = 0xA10;
        inline SecureOffset CameraCacheFOV = 0xA2C;
        inline SecureOffset SpectatorPawn = 0x760;
        inline SecureOffset DefaultFOV = 0x828;
        inline SecureOffset bShowMouseCursor = 0x658;
        inline SecureOffset BlockInputWidgetList = 0x5B0;
        inline SecureOffset WidgetStateMap = 0x538;
        inline SecureOffset Visibility = 0xA9;
        inline SecureOffset Minimap = 0x480;
        inline SecureOffset CurrentMinimapViewScale = 0x4A4;
        inline SecureOffset ScreenPosX = 0x4B8;
        inline SecureOffset ScreenPosY = 0x4BC;
        inline SecureOffset ScreenSize = 0x4A8;
        inline SecureOffset SelectMinimapSizeIndex = 0x5C8;
        inline SecureOffset Slot = 0x38;
        inline SecureOffset LayoutData = 0x40;
        inline SecureOffset Offsets = 0x0;
        inline SecureOffset Alignment = 0x20;

        // --- 5. CHARACTER & MESH ---
        inline SecureOffset Mesh = 0x4A0;
        inline SecureOffset Mesh3P = 0x800;
        inline SecureOffset ComponentLocation = 0x330;
        inline SecureOffset ComponentToWorld = 0x320;
        inline SecureOffset BoneArray = 0xAE8;
        inline SecureOffset StaticMesh = 0xAE8;
        inline SecureOffset BoneCount = 0xAF0;
        inline SecureOffset Eyes = 0x75C;
        inline SecureOffset WorldToMap = 0xA04;
        inline SecureOffset CharacterState = 0x1020;
        inline SecureOffset CharacterName = 0x1D70;
        inline SecureOffset CharacterMovement = 0x490;
        inline SecureOffset LastUpdateVelocity = 0x3E0;
        inline SecureOffset LastSubmitTime = 0x758;
        inline SecureOffset LastRenderTimeOnScreen = 0x75C;
        inline SecureOffset TeamNumber = 0x7C4;
        inline SecureOffset LastTeamNum = 0x2A98;
        inline SecureOffset SpectatedCount = 0x113C;
        inline SecureOffset bAlwaysCreatePhysicsState = 0x488;

        // --- 6. WORLD & RADIUS ---
        inline SecureOffset FeatureRepObject = 0xCF0;
        inline SecureOffset SafetyZonePosition = 0xB0;
        inline SecureOffset SafetyZoneRadius = 0xBC;
        inline SecureOffset BlueZoneRadius = 0xCC;
        inline SecureOffset BlueZonePosition = 0xC0;
        inline SecureOffset NumAliveTeams = 0x480;

        // --- 7. HEALTH SYSTEM ---
        inline SecureOffset HeaFlag = 0x3B9;
        inline SecureOffset Health1 = 0xA3C;
        inline SecureOffset Health2 = 0xA38;
        inline SecureOffset Health3 = 0xA24;
        inline SecureOffset Health4 = 0xA10;
        inline SecureOffset Health5 = 0xA25;
        inline SecureOffset Health6 = 0xA20;
        inline SecureOffset GroggyHealth = 0x14B0;

        inline SecureOffset32 HealthKey0 = 0xCEC7A593;
        inline SecureOffset32 HealthKey1 = 0x9B63B2A7;
        inline SecureOffset32 HealthKey2 = 0xCAD3C3A5;
        inline SecureOffset32 HealthKey3 = 0xA738484B;
        inline SecureOffset32 HealthKey4 = 0xCC911D0A;
        inline SecureOffset32 HealthKey5 = 0x23DDA185;
        inline SecureOffset32 HealthKey6 = 0x09454BC8;
        inline SecureOffset32 HealthKey7 = 0xA521BA21;
        inline SecureOffset32 HealthKey8 = 0x0BA17A58; 
        inline SecureOffset32 HealthKey9 = 0xB0EFA787;
        inline SecureOffset32 HealthKey10 = 0xE275B2BA;
        inline SecureOffset32 HealthKey11 = 0x878ADBD0;
        inline SecureOffset32 HealthKey12 = 0xBDCC62D5;
        inline SecureOffset32 HealthKey13 = 0xA7934B07;
        inline SecureOffset32 HealthKey14 = 0x4B099E38;
        inline SecureOffset32 HealthKey15 = 0xEEDB2A7D;

        inline SecureOffset32 HealthKeys[] = {
            HealthKey0, HealthKey1, HealthKey2, HealthKey3,
            HealthKey4, HealthKey5, HealthKey6, HealthKey7,
            HealthKey8, HealthKey9, HealthKey10, HealthKey11,
            HealthKey12, HealthKey13, HealthKey14, HealthKey15
        };

        inline void RefreshHealthKeys() {
            HealthKeys[0] = HealthKey0; HealthKeys[1] = HealthKey1;
            HealthKeys[2] = HealthKey2; HealthKeys[3] = HealthKey3;
            HealthKeys[4] = HealthKey4; HealthKeys[5] = HealthKey5;
            HealthKeys[6] = HealthKey6; HealthKeys[7] = HealthKey7;
            HealthKeys[8] = HealthKey8; HealthKeys[9] = HealthKey9;
            HealthKeys[10] = HealthKey10; HealthKeys[11] = HealthKey11;
            HealthKeys[12] = HealthKey12; HealthKeys[13] = HealthKey13;
            HealthKeys[14] = HealthKey14; HealthKeys[15] = HealthKey15;
        }

        // --- 8. INVENTORY & STATS ---
        inline SecureOffset LocalPlayers = 0x124172D0; // kd
        inline SecureOffset PlayerArray = 0x410;
        inline SecureOffset AccountId = 0x810;
        inline SecureOffset PlayerName = 0x420;
        inline SecureOffset PlayerStatusType = 0x468;
        inline SecureOffset SquadMemberIndex = 0xA1C;
        inline SecureOffset PlayerStatistics = 0xA10;
        inline SecureOffset DamageDealtOnEnemy = 0x804;
        inline SecureOffset ping = 0x3F8;
        inline SecureOffset MatchId = 0x510;
        inline SecureOffset PartnerLevel = 0x716;
        inline SecureOffset SurvivalTier = 0xCC8;
        inline SecureOffset SurvivalLevel = 0xCCC;
        inline SecureOffset telemetryIdData = 0xCC0;
        inline SecureOffset CharacterClanInfo = 0x838;

        // --- 9. WEAPONS ---
        inline SecureOffset EquippedWeapons = 0x208;
        inline SecureOffset WeaponProcessor = 0x968;
        inline SecureOffset CurrentWeaponIndex = 0x319;
        inline SecureOffset WeaponTrajectoryData = 0x11A8;
        inline SecureOffset TrajectoryGravityZ = 0x106C;
        inline SecureOffset FiringAttachPoint = 0x8C0;
        inline SecureOffset ScopingAttachPoint = 0xB20;
        inline SecureOffset TrajectoryConfig = 0x108;
        inline SecureOffset BallisticCurve = 0x28;
        inline SecureOffset FloatCurves = 0x38;

        inline SecureOffset WeaponConfig_WeaponClass = 0x798;
        inline SecureOffset CurrentAmmoData = 0xE68;
        inline SecureOffset ElapsedCookingTime = 0xB50;
        inline SecureOffset AimOffsets = 0x1AB8;
        inline SecureOffset RecoilValueVector = 0x1198;
        inline SecureOffset VerticalRecovery = 0x10D8;
        inline SecureOffset AttachedItems = 0x858;
        inline SecureOffset WeaponAttachmentData = 0x128;

        // --- 10. INPUT & AIM ---
        inline SecureOffset PlayerInput = 0x548;
        inline SecureOffset InputAxisProperties = 0x138;
        inline SecureOffset InputYawScale = 0x64C;
        inline SecureOffset bIsScoping_CP = 0x85D;
        inline SecureOffset bIsReloading_CP = 0x735;
        inline SecureOffset RecoilADSRotation_CP = 0x824;
        inline SecureOffset ControlRotation_CP = 0x64C;
        inline SecureOffset LeanLeftAlpha_CP = 0x694;
        inline SecureOffset LeanRightAlpha_CP = 0x698;

        // --- 11. ITEMS & LOOT ---
        inline SecureOffset InventoryFacade = 0x1728;
        inline SecureOffset Inventory = 0x400;
        inline SecureOffset InventoryItems = 0x620;
        inline SecureOffset InventoryItemCount = 0x628;
        inline SecureOffset InventoryItemTagItemCount = 0x40;
        inline SecureOffset ItemTable = 0xB0;
        inline SecureOffset ItemID = 0x244;
        inline SecureOffset Equipment = 0x408;
        inline SecureOffset ItemsArray = 0x4F8;
        inline SecureOffset DroppedItemGroup = 0x1C0;
        inline SecureOffset ItemPackageItems = 0x578;
        inline SecureOffset DroppedItemGroupUItem = 0x870;
        inline SecureOffset DroppedItem = 0x458;
        inline SecureOffset Durability = 0x1E4;
        inline SecureOffset Durabilitymax = 0x1E0;

        // --- 12. VEHICLES ---
        inline SecureOffset VehicleMovement = 0x468;
        inline SecureOffset VehicleRiderComponent = 0x2050;
        inline SecureOffset ReplicatedMovement = 0xD0;
        inline SecureOffset LastVehiclePawn = 0x270;
        inline SecureOffset SeatIndex = 0x230;
        inline SecureOffset Wheels = 0x328;
        inline SecureOffset WheelLocation = 0x100;
        inline SecureOffset DampingRate = 0x54;
        inline SecureOffset ShapeRadius = 0x48;
        inline SecureOffset VehicleCommonComponent = 0xB30;
        inline SecureOffset FloatingVehicleCommonComponent = 0x4D8;
        inline SecureOffset VehicleFuel = 0x2E0;
        inline SecureOffset VehicleFuelMax = 0x2E4;
        inline SecureOffset VehicleHealth = 0x2D8;
        inline SecureOffset VehicleHealthMax = 0x2DC;

        // --- 13. ANIMATION & ENGINE ---
        inline SecureOffset AnimScriptInstance = 0xE30;
        inline SecureOffset PreEvalPawnState = 0x630;
        inline SecureOffset Antiintegrity_monitorCharacterSyncManager = 0xD28;
        inline SecureOffset PhysxSDK = 0x11B90808;
        inline SecureOffset TimeSeconds = 0x810;
        inline SecureOffset TimeTillExplosion = 0x824;
        inline SecureOffset ExplodeState = 0x628;
        inline SecureOffset TrainingMapGrid = 0x5B0;
        inline SecureOffset MortarRotation = 0x520;
        inline SecureOffset MortarEntity = 0x108;
        inline SecureOffset MapGrid_Map = 0x4A8;
        inline SecureOffset Gender = 0xAC8;
        inline SecureOffset MouseX = 0x4BF5;
        inline SecureOffset MouseY = 0x4BF6;

        // --- 14. MISC ---
        inline SecureOffset AttachedStaticComponentMap = 0x1508;
        inline SecureOffset StaticSockets = 0xC8;
        inline SecureOffset StaticSocketName = 0x30;
        inline SecureOffset StaticRelativeScale = 0x50;
        inline SecureOffset StaticRelativeLocation = 0x38;
        inline SecureOffset StaticRelativeRotation = 0x44;
        inline SecureOffset Keys = 0x60;

        // --- 15. SPOOF CALL GADGETS ---
        inline SecureOffset SPOOFCALL_GADGET = 0x2423775;
        inline SecureOffset LineTraceSingle = 0x83153C;
        inline SecureOffset HOOK = 0x11AE68E8;
        inline SecureOffset HOOK_TWO = 0xCFEDBAB;
    }

    __forceinline SecureOffset32 decrypt_cindex(uint32_t v) {
        using namespace offsets;
        uint32_t x = v ^ (uint32_t)DecryptNameIndexXorKey1;
        return (((x << (int)DecryptNameIndexDval) | (x >> (int)DecryptNameIndexSval) & (uint32_t)DecryptNameIndexXorKey3) ^ _rotr(x, (int)DecryptNameIndexRval) ^ (uint32_t)DecryptNameIndexXorKey2);
    }
}

// Master Scanner v2.5 Output: All Core Offsets, Name Decryption, and Bone Structure have been verified and synchronized.
