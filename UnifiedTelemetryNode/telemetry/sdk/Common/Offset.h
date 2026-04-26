#pragma once
#include <.shared/telemetry_config.hpp>

// UNIFIED REDIRECTOR FOR OLD OFFSET.H
namespace Offset {
    using namespace telemetry_config::offsets;

    inline uint64_t& XenuineDecrypt = telemetry_config::offsets::XenuineDecrypt;
    inline uint64_t& UWorld = telemetry_config::offsets::UWorld;
    inline uint64_t& GNames = telemetry_config::offsets::GNames;
    inline uint64_t& GObjects = telemetry_config::offsets::GObjects;
    inline uint64_t& GameInstance = telemetry_config::offsets::GameInstance;
    inline uint64_t& LocalPlayer = telemetry_config::offsets::LocalPlayer;
    inline uint64_t& PlayerController = telemetry_config::offsets::PlayerController;
    inline uint64_t& AcknowledgedPawn = telemetry_config::offsets::AcknowledgedPawn;
    inline uint64_t& Mesh = telemetry_config::offsets::Mesh;
    inline uint64_t& RootComponent = telemetry_config::offsets::RootComponent;
    inline uint64_t& BoneArray = telemetry_config::offsets::BoneArray;
    inline uint64_t& TeamNumber = telemetry_config::offsets::TeamNumber;
    inline uint64_t& SpectatedCount = telemetry_config::offsets::SpectatedCount;
    inline uint64_t& GroggyHealth = telemetry_config::offsets::GroggyHealth;
    inline uint64_t& EquippedWeapons = telemetry_config::offsets::EquippedWeapons;
    inline uint64_t& CharacterName = telemetry_config::offsets::CharacterName;

    inline uint64_t& CameraCacheLocation = telemetry_config::offsets::CameraCacheLocation;
    inline uint64_t& CameraCacheRotation = telemetry_config::offsets::CameraCacheRotation;
    inline uint64_t& CameraCacheFOV = telemetry_config::offsets::CameraCacheFOV;

    inline void Sever_Init() {}
}