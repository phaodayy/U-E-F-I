#pragma once
#include <.shared/telemetry_config.hpp>

/**
 * @namespace telemetryOffsets
 * @brief Tầng tương thích (Compatibility Layer)
 * Tất cả các giá trị thực tế hiện đã được chuyển sang .shared/telemetry_config.hpp
 * File này chỉ đóng vai trò ánh xạ để không làm hỏng code hiện tại.
 */
namespace telemetryOffsets {
    using namespace telemetry_config::offsets;

    // --- 1. CORE & ANTI-integrity_monitor ---
    inline uint64_t XenuineDecrypt = telemetry_config::offsets::XenuineDecrypt;
    inline uint64_t UWorld = telemetry_config::offsets::UWorld;
    inline uint64_t GNames = telemetry_config::offsets::GNames;
    inline uint64_t GObjects = telemetry_config::offsets::GObjects;
    inline uint64_t GNamesPtr = telemetry_config::offsets::GNamesPtr;
    inline uint64_t ChunkSize = telemetry_config::offsets::ChunkSize;
    inline uint64_t ObjID = telemetry_config::offsets::ObjID;

    // --- 2. GAME CORE ---
    inline uint64_t GameInstance = telemetry_config::offsets::GameInstance;
    inline uint64_t GameState = telemetry_config::offsets::GameState;
    inline uint64_t LocalPlayer = telemetry_config::offsets::LocalPlayer;
    inline uint64_t PlayerController = telemetry_config::offsets::PlayerController;
    inline uint64_t AcknowledgedPawn = telemetry_config::offsets::AcknowledgedPawn;
    inline uint64_t PlayerCameraManager = telemetry_config::offsets::PlayerCameraManager;
    inline uint64_t MyHUD = telemetry_config::offsets::MyHUD;
    inline uint64_t PlayerState = telemetry_config::offsets::PlayerState;
    inline uint64_t CurrentLevel = telemetry_config::offsets::CurrentLevel;
    inline uint64_t Actors = telemetry_config::offsets::Actors;
    inline uint64_t RootComponent = telemetry_config::offsets::RootComponent;

    // --- 3. MESH & VISUALS ---
    inline uint64_t Mesh = telemetry_config::offsets::Mesh;
    inline uint64_t Mesh3P = telemetry_config::offsets::Mesh3P;
    inline uint64_t ComponentLocation = telemetry_config::offsets::ComponentLocation;
    inline uint64_t ComponentToWorld = telemetry_config::offsets::ComponentToWorld;
    inline uint64_t BoneArray = telemetry_config::offsets::BoneArray;
    inline uint64_t LastSubmitTime = telemetry_config::offsets::LastSubmitTime;
    inline uint64_t LastRenderTimeOnScreen = telemetry_config::offsets::LastRenderTimeOnScreen;
    inline uint64_t Visibility = telemetry_config::offsets::Visibility;

    // --- 4. PLAYER INFO ---
    inline uint64_t TeamNumber = telemetry_config::offsets::TeamNumber;
    inline uint64_t LastTeamNum = telemetry_config::offsets::LastTeamNum;
    inline uint64_t SpectatedCount = telemetry_config::offsets::SpectatedCount;
    inline uint64_t GroggyHealth = telemetry_config::offsets::GroggyHealth;
    inline uint64_t CharacterName = telemetry_config::offsets::CharacterName;
    inline uint64_t PlayerName = telemetry_config::offsets::PlayerName;
    inline uint64_t PlayerStatistics = telemetry_config::offsets::PlayerStatistics;
    inline uint64_t DamageDealtOnEnemy = telemetry_config::offsets::DamageDealtOnEnemy;
    inline uint64_t SurvivalTier = telemetry_config::offsets::SurvivalTier;
    inline uint64_t SurvivalLevel = telemetry_config::offsets::SurvivalLevel;
    inline uint64_t WeaponProcessor = telemetry_config::offsets::WeaponProcessor;
    inline uint64_t EquippedWeapons = telemetry_config::offsets::EquippedWeapons;
    inline uint64_t CurrentWeaponIndex = telemetry_config::offsets::CurrentWeaponIndex;
    inline uint64_t CurrentAmmoData = telemetry_config::offsets::CurrentAmmoData;

    // --- 5. MOVEMENT ---
    inline uint64_t CharacterMovement = telemetry_config::offsets::CharacterMovement;
    inline uint64_t LastUpdateVelocity = telemetry_config::offsets::LastUpdateVelocity;
    inline uint64_t AimOffsets = telemetry_config::offsets::AimOffsets;
    inline uint64_t AnimScriptInstance = telemetry_config::offsets::AnimScriptInstance;
    inline uint64_t bIsScoping_CP = telemetry_config::offsets::bIsScoping_CP;
    inline uint64_t bIsReloading_CP = telemetry_config::offsets::bIsReloading_CP;

    // --- 6. RADAR & HUD ---
    inline uint64_t Minimap = telemetry_config::offsets::Minimap;
    inline uint64_t CurrentMinimapViewScale1D = telemetry_config::offsets::CurrentMinimapViewScale;
    inline uint64_t LastMinimapPos = telemetry_config::offsets::ScreenPosX;
    inline uint64_t SelectMinimapSizeIndex = telemetry_config::offsets::SelectMinimapSizeIndex;
    inline uint64_t BlockInputWidgetList = telemetry_config::offsets::BlockInputWidgetList;
    inline uint64_t WidgetStateMap = telemetry_config::offsets::WidgetStateMap;
    inline uint64_t Slot = telemetry_config::offsets::Slot;
    inline uint64_t LayoutData = telemetry_config::offsets::LayoutData;
    inline uint64_t Alignment = telemetry_config::offsets::Alignment;
    inline uint64_t MapGrid_Map = telemetry_config::offsets::MapGrid_Map;
    inline uint64_t WorldToMap = telemetry_config::offsets::WorldToMap;
    inline uint64_t bShowMouseCursor = telemetry_config::offsets::bShowMouseCursor;

    // --- 7. HEALTH LOGIC (Fixing mapping to match 2603 standard) ---
    inline uint64_t Health3 = telemetry_config::offsets::Health3;
    inline uint64_t Health4 = telemetry_config::offsets::Health4;
    inline uint64_t Health5 = telemetry_config::offsets::Health5;
    inline uint64_t Health6 = telemetry_config::offsets::Health6;

    // --- 8. ITEM SYSTEM ---
    inline uint64_t InventoryFacade = telemetry_config::offsets::InventoryFacade;
    inline uint64_t Inventory = telemetry_config::offsets::Inventory;
    inline uint64_t InventoryItems = telemetry_config::offsets::InventoryItems;
    inline uint64_t ItemTable = telemetry_config::offsets::ItemTable;
    inline uint64_t ItemID = telemetry_config::offsets::ItemID;

    // Health Keys Redirection (pointing to latest HealthKey names)
    inline uint32_t HealthKey0 = telemetry_config::offsets::HealthKey0;
    inline uint32_t HealthKey1 = telemetry_config::offsets::HealthKey1;
    inline uint32_t HealthKey2 = telemetry_config::offsets::HealthKey2;
    inline uint32_t HealthKey3 = telemetry_config::offsets::HealthKey3;
    inline uint32_t HealthKey4 = telemetry_config::offsets::HealthKey4;
    inline uint32_t HealthKey5 = telemetry_config::offsets::HealthKey5;
    inline uint32_t HealthKey6 = telemetry_config::offsets::HealthKey6;
    inline uint32_t HealthKey7 = telemetry_config::offsets::HealthKey7;
    inline uint32_t HealthKey8 = telemetry_config::offsets::HealthKey8;
    inline uint32_t HealthKey9 = telemetry_config::offsets::HealthKey9;
    inline uint32_t HealthKey10 = telemetry_config::offsets::HealthKey10;
    inline uint32_t HealthKey11 = telemetry_config::offsets::HealthKey11;
    inline uint32_t HealthKey12 = telemetry_config::offsets::HealthKey12;
    inline uint32_t HealthKey13 = telemetry_config::offsets::HealthKey13;
    inline uint32_t HealthKey14 = telemetry_config::offsets::HealthKey14;
    inline uint32_t HealthKey15 = telemetry_config::offsets::HealthKey15;

    // --- 9. MISC ---
    inline uint64_t Gender = telemetry_config::offsets::Gender;
    inline uint64_t SPOOFCALL = telemetry_config::offsets::SPOOFCALL_GADGET;

    inline uint32_t DecryptCIndex(uint32_t value) {
        return telemetry_config::decrypt_cindex(value);
    }
}

namespace ValorantOffsets = telemetryOffsets;
