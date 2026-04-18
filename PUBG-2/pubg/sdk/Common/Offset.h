#pragma once
#include <.shared/pubg_config.hpp>

// UNIFIED REDIRECTOR FOR OLD OFFSET.H
namespace Offset {
    using namespace pubg_config::offsets;

    inline uint64_t& XenuineDecrypt = pubg_config::offsets::XenuineDecrypt;
    inline uint64_t& UWorld = pubg_config::offsets::UWorld;
    inline uint64_t& GNames = pubg_config::offsets::GNames;
    inline uint64_t& GObjects = pubg_config::offsets::GObjects;
    inline uint64_t& GameInstance = pubg_config::offsets::GameInstance;
    inline uint64_t& LocalPlayer = pubg_config::offsets::LocalPlayer;
    inline uint64_t& PlayerController = pubg_config::offsets::PlayerController;
    inline uint64_t& AcknowledgedPawn = pubg_config::offsets::AcknowledgedPawn;
    inline uint64_t& Mesh = pubg_config::offsets::Mesh;
    inline uint64_t& RootComponent = pubg_config::offsets::RootComponent;
    inline uint64_t& BoneArray = pubg_config::offsets::BoneArray;
    inline uint64_t& TeamNumber = pubg_config::offsets::TeamNumber;
    inline uint64_t& SpectatedCount = pubg_config::offsets::SpectatedCount;
    inline uint64_t& GroggyHealth = pubg_config::offsets::GroggyHealth;
    inline uint64_t& EquippedWeapons = pubg_config::offsets::EquippedWeapons;
    inline uint64_t& CharacterName = pubg_config::offsets::CharacterName;

    inline uint64_t& CameraCacheLocation = pubg_config::offsets::CameraCacheLocation;
    inline uint64_t& CameraCacheRotation = pubg_config::offsets::CameraCacheRotation;
    inline uint64_t& CameraCacheFOV = pubg_config::offsets::CameraCacheFOV;

    inline void Sever_Init() {}
}