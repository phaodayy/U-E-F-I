#pragma once
#include <.shared/telemetry_config.hpp>

// UNIFIED REDIRECTOR FOR OLD OFFSET.H
namespace Offset {
    using namespace telemetry_config::offsets;

    inline auto& XenuineDecrypt = telemetry_config::offsets::XenuineDecrypt;
    inline auto& UWorld = telemetry_config::offsets::UWorld;
    inline auto& GNames = telemetry_config::offsets::GNames;
    inline auto& GObjects = telemetry_config::offsets::GObjects;
    inline auto& GameInstance = telemetry_config::offsets::GameInstance;
    inline auto& LocalPlayer = telemetry_config::offsets::LocalPlayer;
    inline auto& PlayerController = telemetry_config::offsets::PlayerController;
    inline auto& AcknowledgedPawn = telemetry_config::offsets::AcknowledgedPawn;
    inline auto& Mesh = telemetry_config::offsets::Mesh;
    inline auto& RootComponent = telemetry_config::offsets::RootComponent;
    inline auto& BoneArray = telemetry_config::offsets::BoneArray;
    inline auto& TeamNumber = telemetry_config::offsets::TeamNumber;
    inline auto& SpectatedCount = telemetry_config::offsets::SpectatedCount;
    inline auto& GroggyHealth = telemetry_config::offsets::GroggyHealth;
    inline auto& EquippedWeapons = telemetry_config::offsets::EquippedWeapons;
    inline auto& CharacterName = telemetry_config::offsets::CharacterName;
    inline auto& Physx = telemetry_config::offsets::PhysxSDK;

    inline auto& CameraCacheLocation = telemetry_config::offsets::CameraCacheLocation;
    inline auto& CameraCacheRotation = telemetry_config::offsets::CameraCacheRotation;
    inline auto& CameraCacheFOV = telemetry_config::offsets::CameraCacheFOV;

    inline void Sever_Init() {}
}
