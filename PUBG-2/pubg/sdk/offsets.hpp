#pragma once
#include <.shared/pubg_config.hpp>

/**
 * @namespace PubgOffsets
 * @brief Tầng tương thích (Compatibility Layer)
 * Tất cả các giá trị thực tế hiện đã được chuyển sang .shared/pubg_config.hpp
 * File này chỉ đóng vai trò ánh xạ để không làm hỏng code hiện tại.
 */
namespace PubgOffsets {
    using namespace pubg_config::offsets;

    // --- 1. CORE & ANTI-CHEAT ---
    inline uint64_t XenuineDecrypt = pubg_config::offsets::XenuineDecrypt;
    inline uint64_t UWorld = pubg_config::offsets::UWorld;
    inline uint64_t GNames = pubg_config::offsets::GNames;
    inline uint64_t GObjects = pubg_config::offsets::GObjects;
    inline uint64_t GNamesPtr = pubg_config::offsets::GNamesPtr;
    inline uint64_t ChunkSize = pubg_config::offsets::ChunkSize;
    inline uint64_t ObjID = pubg_config::offsets::ObjID;

    // --- 2. GAME CORE ---
    inline uint64_t GameInstance = pubg_config::offsets::GameInstance;
    inline uint64_t GameState = pubg_config::offsets::GameState;
    inline uint64_t LocalPlayer = pubg_config::offsets::LocalPlayer;
    inline uint64_t PlayerController = pubg_config::offsets::PlayerController;
    inline uint64_t AcknowledgedPawn = pubg_config::offsets::AcknowledgedPawn;
    inline uint64_t PlayerCameraManager = pubg_config::offsets::PlayerCameraManager;
    inline uint64_t MyHUD = pubg_config::offsets::MyHUD;
    inline uint64_t PlayerState = pubg_config::offsets::PlayerState;
    inline uint64_t CurrentLevel = pubg_config::offsets::CurrentLevel;
    inline uint64_t Actors = pubg_config::offsets::Actors;
    inline uint64_t RootComponent = pubg_config::offsets::RootComponent;

    // --- 3. MESH & VISUALS ---
    inline uint64_t Mesh = pubg_config::offsets::Mesh;
    inline uint64_t Mesh3P = pubg_config::offsets::Mesh3P;
    inline uint64_t ComponentLocation = pubg_config::offsets::ComponentLocation;
    inline uint64_t ComponentToWorld = pubg_config::offsets::ComponentToWorld;
    inline uint64_t BoneArray = pubg_config::offsets::BoneArray;
    inline uint64_t LastSubmitTime = pubg_config::offsets::LastSubmitTime;
    inline uint64_t LastRenderTimeOnScreen = pubg_config::offsets::LastRenderTimeOnScreen;
    inline uint64_t Visibility = pubg_config::offsets::Visibility;

    // --- 4. PLAYER INFO ---
    inline uint64_t TeamNumber = pubg_config::offsets::TeamNumber;
    inline uint64_t LastTeamNum = pubg_config::offsets::LastTeamNum;
    inline uint64_t SpectatedCount = pubg_config::offsets::SpectatedCount;
    inline uint64_t GroggyHealth = pubg_config::offsets::GroggyHealth;
    inline uint64_t CharacterName = pubg_config::offsets::CharacterName;
    inline uint64_t PlayerName = pubg_config::offsets::PlayerName;
    inline uint64_t WeaponProcessor = pubg_config::offsets::WeaponProcessor;
    inline uint64_t EquippedWeapons = pubg_config::offsets::EquippedWeapons;
    inline uint64_t CurrentWeaponIndex = pubg_config::offsets::CurrentWeaponIndex;

    // --- 5. MOVEMENT ---
    inline uint64_t CharacterMovement = pubg_config::offsets::CharacterMovement;
    inline uint64_t LastUpdateVelocity = pubg_config::offsets::LastUpdateVelocity;

    // --- 6. RADAR & HUD ---
    inline uint64_t Minimap = pubg_config::offsets::Minimap;
    inline uint64_t CurrentMinimapViewScale1D = pubg_config::offsets::CurrentMinimapViewScale;
    inline uint64_t LastMinimapPos = pubg_config::offsets::ScreenPosX;
    inline uint64_t SelectMinimapSizeIndex = pubg_config::offsets::SelectMinimapSizeIndex;
    inline uint64_t BlockInputWidgetList = pubg_config::offsets::BlockInputWidgetList;
    inline uint64_t WidgetStateMap = pubg_config::offsets::WidgetStateMap;
    inline uint64_t Slot = pubg_config::offsets::Slot;
    inline uint64_t LayoutData = pubg_config::offsets::LayoutData;
    inline uint64_t Alignment = pubg_config::offsets::Alignment;
    inline uint64_t MapGrid_Map = pubg_config::offsets::MapGrid_Map;
    inline uint64_t WorldToMap = pubg_config::offsets::WorldToMap;
    inline uint64_t bShowMouseCursor = pubg_config::offsets::bShowMouseCursor;

    // --- 7. HEALTH LOGIC (Fixing mapping to match 2603 standard) ---
    inline uint64_t Health3 = pubg_config::offsets::Health3;
    inline uint64_t Health4 = pubg_config::offsets::Health4;
    inline uint64_t Health5 = pubg_config::offsets::Health5;
    inline uint64_t Health6 = pubg_config::offsets::Health6;

    // --- 8. ITEM SYSTEM ---
    inline uint64_t InventoryFacade = pubg_config::offsets::InventoryFacade;
    inline uint64_t Inventory = pubg_config::offsets::Inventory;
    inline uint64_t InventoryItems = pubg_config::offsets::InventoryItems;
    inline uint64_t ItemTable = pubg_config::offsets::ItemTable;
    inline uint64_t ItemID = pubg_config::offsets::ItemID;

    // Health Keys Redirection (pointing to latest HealthKey names)
    inline uint32_t HealthKey0 = pubg_config::offsets::HealthKey0;
    inline uint32_t HealthKey1 = pubg_config::offsets::HealthKey1;
    inline uint32_t HealthKey2 = pubg_config::offsets::HealthKey2;
    inline uint32_t HealthKey3 = pubg_config::offsets::HealthKey3;
    inline uint32_t HealthKey4 = pubg_config::offsets::HealthKey4;
    inline uint32_t HealthKey5 = pubg_config::offsets::HealthKey5;
    inline uint32_t HealthKey6 = pubg_config::offsets::HealthKey6;
    inline uint32_t HealthKey7 = pubg_config::offsets::HealthKey7;
    inline uint32_t HealthKey8 = pubg_config::offsets::HealthKey8;
    inline uint32_t HealthKey9 = pubg_config::offsets::HealthKey9;
    inline uint32_t HealthKey10 = pubg_config::offsets::HealthKey10;
    inline uint32_t HealthKey11 = pubg_config::offsets::HealthKey11;
    inline uint32_t HealthKey12 = pubg_config::offsets::HealthKey12;
    inline uint32_t HealthKey13 = pubg_config::offsets::HealthKey13;
    inline uint32_t HealthKey14 = pubg_config::offsets::HealthKey14;
    inline uint32_t HealthKey15 = pubg_config::offsets::HealthKey15;

    // --- 9. MISC ---
    inline uint64_t Gender = pubg_config::offsets::Gender;
    inline uint64_t SPOOFCALL = pubg_config::offsets::SPOOFCALL_GADGET;

    inline uint32_t DecryptCIndex(uint32_t value) {
        return pubg_config::decrypt_cindex(value);
    }
}

namespace ValorantOffsets = PubgOffsets;
