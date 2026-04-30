#pragma once
#include <.shared/telemetry_config.hpp>

/**
 * @namespace telemetryOffsets
 * Compatibility layer for the shared encrypted offset table.
 * Keep these as references so runtime signature scans update every caller.
 */
namespace telemetryOffsets {
    using namespace telemetry_config::offsets;

    // --- 1. CORE & ANTI-integrity_monitor ---
    inline auto& XenuineDecrypt = telemetry_config::offsets::XenuineDecrypt;
    inline auto& UWorld = telemetry_config::offsets::UWorld;
    inline auto& GNames = telemetry_config::offsets::GNames;
    inline auto& GObjects = telemetry_config::offsets::GObjects;
    inline auto& GNamesPtr = telemetry_config::offsets::GNamesPtr;
    inline auto& ChunkSize = telemetry_config::offsets::ChunkSize;
    inline auto& ObjID = telemetry_config::offsets::ObjID;

    // --- 2. GAME CORE ---
    inline auto& GameInstance = telemetry_config::offsets::GameInstance;
    inline auto& GameState = telemetry_config::offsets::GameState;
    inline auto& LocalPlayer = telemetry_config::offsets::LocalPlayer;
    inline auto& PlayerController = telemetry_config::offsets::PlayerController;
    inline auto& AcknowledgedPawn = telemetry_config::offsets::AcknowledgedPawn;
    inline auto& PlayerCameraManager = telemetry_config::offsets::PlayerCameraManager;
    inline auto& MyHUD = telemetry_config::offsets::MyHUD;
    inline auto& PlayerState = telemetry_config::offsets::PlayerState;
    inline auto& CurrentLevel = telemetry_config::offsets::CurrentLevel;
    inline auto& Actors = telemetry_config::offsets::Actors;
    inline auto& RootComponent = telemetry_config::offsets::RootComponent;

    // --- 3. MESH & VISUALS ---
    inline auto& Mesh = telemetry_config::offsets::Mesh;
    inline auto& Mesh3P = telemetry_config::offsets::Mesh3P;
    inline auto& ComponentLocation = telemetry_config::offsets::ComponentLocation;
    inline auto& ComponentToWorld = telemetry_config::offsets::ComponentToWorld;
    inline auto& BoneArray = telemetry_config::offsets::BoneArray;
    inline auto& LastSubmitTime = telemetry_config::offsets::LastSubmitTime;
    inline auto& LastRenderTimeOnScreen = telemetry_config::offsets::LastRenderTimeOnScreen;
    inline auto& Visibility = telemetry_config::offsets::Visibility;

    // --- 4. PLAYER INFO ---
    inline auto& TeamNumber = telemetry_config::offsets::TeamNumber;
    inline auto& LastTeamNum = telemetry_config::offsets::LastTeamNum;
    inline auto& SpectatedCount = telemetry_config::offsets::SpectatedCount;
    inline auto& GroggyHealth = telemetry_config::offsets::GroggyHealth;
    inline auto& CharacterName = telemetry_config::offsets::CharacterName;
    inline auto& PlayerName = telemetry_config::offsets::PlayerName;
    inline auto& PlayerStatistics = telemetry_config::offsets::PlayerStatistics;
    inline auto& DamageDealtOnEnemy = telemetry_config::offsets::DamageDealtOnEnemy;
    inline auto& SurvivalTier = telemetry_config::offsets::SurvivalTier;
    inline auto& SurvivalLevel = telemetry_config::offsets::SurvivalLevel;
    inline auto& WeaponProcessor = telemetry_config::offsets::WeaponProcessor;
    inline auto& EquippedWeapons = telemetry_config::offsets::EquippedWeapons;
    inline auto& CurrentWeaponIndex = telemetry_config::offsets::CurrentWeaponIndex;
    inline auto& CurrentAmmoData = telemetry_config::offsets::CurrentAmmoData;

    // --- 5. MOVEMENT ---
    inline auto& CharacterMovement = telemetry_config::offsets::CharacterMovement;
    inline auto& LastUpdateVelocity = telemetry_config::offsets::LastUpdateVelocity;
    inline auto& AimOffsets = telemetry_config::offsets::AimOffsets;
    inline auto& AnimScriptInstance = telemetry_config::offsets::AnimScriptInstance;
    inline auto& bIsScoping_CP = telemetry_config::offsets::bIsScoping_CP;
    inline auto& bIsReloading_CP = telemetry_config::offsets::bIsReloading_CP;
    inline auto& RecoilADSRotation_CP = telemetry_config::offsets::RecoilADSRotation_CP;
    inline auto& RecoilValueVector = telemetry_config::offsets::RecoilValueVector;
    inline auto& ControlRotation_CP = telemetry_config::offsets::ControlRotation_CP;

    // --- 6. RADAR & HUD ---
    inline auto& Minimap = telemetry_config::offsets::Minimap;
    inline auto& CurrentMinimapViewScale1D = telemetry_config::offsets::CurrentMinimapViewScale;
    inline auto& LastMinimapPos = telemetry_config::offsets::ScreenPosX;
    inline auto& SelectMinimapSizeIndex = telemetry_config::offsets::SelectMinimapSizeIndex;
    inline auto& BlockInputWidgetList = telemetry_config::offsets::BlockInputWidgetList;
    inline auto& WidgetStateMap = telemetry_config::offsets::WidgetStateMap;
    inline auto& Slot = telemetry_config::offsets::Slot;
    inline auto& LayoutData = telemetry_config::offsets::LayoutData;
    inline auto& Alignment = telemetry_config::offsets::Alignment;
    inline auto& MapGrid_Map = telemetry_config::offsets::MapGrid_Map;
    inline auto& WorldToMap = telemetry_config::offsets::WorldToMap;
    inline auto& bShowMouseCursor = telemetry_config::offsets::bShowMouseCursor;

    // --- 7. HEALTH LOGIC (Fixing mapping to match 2603 standard) ---
    inline auto& Health3 = telemetry_config::offsets::Health3;
    inline auto& Health4 = telemetry_config::offsets::Health4;
    inline auto& Health5 = telemetry_config::offsets::Health5;
    inline auto& Health6 = telemetry_config::offsets::Health6;

    // --- 8. ITEM SYSTEM ---
    inline auto& InventoryFacade = telemetry_config::offsets::InventoryFacade;
    inline auto& Inventory = telemetry_config::offsets::Inventory;
    inline auto& InventoryItems = telemetry_config::offsets::InventoryItems;
    inline auto& Equipment = telemetry_config::offsets::Equipment;
    inline auto& ItemsArray = telemetry_config::offsets::ItemsArray;
    inline auto& ItemTable = telemetry_config::offsets::ItemTable;
    inline auto& ItemID = telemetry_config::offsets::ItemID;
    inline auto& Durability = telemetry_config::offsets::Durability;
    inline auto& Durabilitymax = telemetry_config::offsets::Durabilitymax;

    // Health Keys Redirection (pointing to latest HealthKey names)
    inline auto& HealthKey0 = telemetry_config::offsets::HealthKey0;
    inline auto& HealthKey1 = telemetry_config::offsets::HealthKey1;
    inline auto& HealthKey2 = telemetry_config::offsets::HealthKey2;
    inline auto& HealthKey3 = telemetry_config::offsets::HealthKey3;
    inline auto& HealthKey4 = telemetry_config::offsets::HealthKey4;
    inline auto& HealthKey5 = telemetry_config::offsets::HealthKey5;
    inline auto& HealthKey6 = telemetry_config::offsets::HealthKey6;
    inline auto& HealthKey7 = telemetry_config::offsets::HealthKey7;
    inline auto& HealthKey8 = telemetry_config::offsets::HealthKey8;
    inline auto& HealthKey9 = telemetry_config::offsets::HealthKey9;
    inline auto& HealthKey10 = telemetry_config::offsets::HealthKey10;
    inline auto& HealthKey11 = telemetry_config::offsets::HealthKey11;
    inline auto& HealthKey12 = telemetry_config::offsets::HealthKey12;
    inline auto& HealthKey13 = telemetry_config::offsets::HealthKey13;
    inline auto& HealthKey14 = telemetry_config::offsets::HealthKey14;
    inline auto& HealthKey15 = telemetry_config::offsets::HealthKey15;

    // --- 9. MISC ---
    inline auto& Gender = telemetry_config::offsets::Gender;
    inline auto& SPOOFCALL = telemetry_config::offsets::SPOOFCALL_GADGET;

    inline uint32_t DecryptCIndex(uint32_t value) {
        return telemetry_config::decrypt_cindex(value);
    }
}

namespace ValorantOffsets = telemetryOffsets;
