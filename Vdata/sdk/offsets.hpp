#pragma once
#include <cstdint>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "../sdk/skCrypt.h"

#pragma comment(lib, "wininet.lib")

namespace VDataOffsets {
    // Hidden XOR Key - Generated unique per session
    inline uint64_t g_StorageKey = 0;

    // Encrypted Storage
    inline uint64_t _GWorld = 0;
    inline uint64_t _FNameState = 0;
    inline uint64_t _FNameKey = 0;
    inline uint64_t _FNamePool = 0;
    inline uint64_t _PointerState = 0;
    inline uint64_t _PointerKey = 0;
    inline uint64_t _PersistentLevel = 0;
    inline uint64_t _GameInstance = 0;
    inline uint64_t _GameState = 0;
    inline uint64_t _Levels = 0;
    inline uint64_t _LocalPlayers = 0;
    inline uint64_t _ClientGameInstance = 0;
    inline uint64_t _PlayerController = 0;
    inline uint64_t _ActorArray = 0;
    inline uint64_t _ActorCount = 0;
    inline uint64_t _AcknowledgedPawn = 0;
    inline uint64_t _PlayerState = 0;
    inline uint64_t _PlayerID = 0;
    inline uint64_t _PlayerArray = 0;
    inline uint64_t _SpawnedCharacter = 0;
    inline uint64_t _PawnPrivate = 0;
    inline uint64_t _RootComponent = 0;
    inline uint64_t _RelativeLocation = 0;
    inline uint64_t _RelativeRotation = 0;
    inline uint64_t _RelativeScale3D = 0;
    inline uint64_t _ComponentVelocity = 0;
    inline uint64_t _ComponentToWorld = 0;
    inline uint64_t _Mesh = 0;
    inline uint64_t _MeshComponent = 0;
    inline uint64_t _BoneArray = 0;
    inline uint64_t _BoneArrayCache = 0;
    inline uint64_t _BoneCount = 0;
    inline uint64_t _bForceWireframe = 0;
    inline uint64_t _Mesh1PAresEquip = 0;
    inline uint64_t _Mesh3PAresEquip = 0;
    inline uint64_t _Mesh1PCharacter = 0;
    inline uint64_t _MeshOverlay1P = 0;
    inline uint64_t _Mesh3PMIDs = 0;
    inline uint64_t _Mesh1PMIDs = 0;
    inline uint64_t _Mesh1POverlayMIDs = 0;
    inline uint64_t _ProcMeshBodySetup = 0;
    inline uint64_t _BoundsScale = 0;
    inline uint64_t _LastSubmitTime = 0;
    inline uint64_t _LastRenderTime = 0;
    inline uint64_t _DamageHandler = 0;
    inline uint64_t _BlindManager = 0;
    inline uint64_t _DamageSections = 0;
    inline uint64_t _Health = 0;
    inline uint64_t _CachedLife = 0;
    inline uint64_t _LifeSection = 0;
    inline uint64_t _MaxLifeSection = 0;
    inline uint64_t _bAliveSection = 0;
    inline uint64_t _LocalBlindState = 0;
    inline uint64_t _bIsDormant = 0;
    inline uint64_t _bAlive = 0;
    inline uint64_t _GunRecoveryTime = 0;
    inline uint64_t _StabilityMobile = 0;
    inline uint64_t _WasAlly = 0;
    inline uint64_t _WasInvisible = 0;
    inline uint64_t _WasIntangible = 0;
    inline uint64_t _bIsPlayerCharacter = 0;
    inline uint64_t _TeamComponent = 0;
    inline uint64_t _TeamID = 0;
    inline uint64_t _PlayerCameraManager = 0;
    inline uint64_t _CameraCachePrivate = 0;
    inline uint64_t _ControlRotation = 0;
    inline uint64_t _CameraPosition = 0;
    inline uint64_t _CameraRotation = 0;
    inline uint64_t _CameraFOV = 0;
    inline uint64_t _Inventory = 0;
    inline uint64_t _CurrentEquippable = 0;
    inline uint64_t _EquippableModels = 0;
    inline uint64_t _TotemModels = 0;
    inline uint64_t _CharmMap = 0;
    inline uint64_t _SkinDataAsset = 0;
    inline uint64_t _CharmDataAsset = 0;
    inline uint64_t _CosmeticRandomSeed = 0;
    inline uint64_t _AttachName = 0;
    inline uint64_t _MagazineAmmo = 0;
    inline uint64_t _MaxAmmo = 0;
    inline uint64_t _BombTimeRemaining = 0;
    inline uint64_t _PlantedASite = 0;
    inline uint64_t _PlantedAtSite = 0;
    inline uint64_t _BombDefused = 0;
    inline uint64_t _CurrentDefuser = 0;
    inline uint64_t _DefuseProgress = 0;
    inline uint64_t _GameUserSettings = 0;
    inline uint64_t _AresSettingsManager = 0;
    inline uint64_t _bUseVSync = 0;
    inline uint64_t _ResolutionSizeX = 0;
    inline uint64_t _ResolutionSizeY = 0;
    inline uint64_t _FloatSettingValues = 0;
    inline uint64_t _IntSettingValues = 0;
    inline uint64_t _BoolSettingValues = 0;
    inline uint64_t _StringSettingValues = 0;
    inline uint64_t _Ping = 0;
    inline uint64_t _PlatformPlayer = 0;
    inline uint64_t _SpectatedPlayer = 0;
    inline uint64_t _HideAccountLevel = 0;
    inline uint64_t _AccountLevel = 0;
    inline uint64_t _CompetitiveTier = 0;
    inline uint64_t _PlayerNamePrivate = 0;
    inline uint64_t _UniqueID = 0;
    inline uint64_t _ProcessEvent = 0;
    inline uint64_t _StaticFindObject = 0;
    inline uint64_t _StaticLoadObject = 0;
    inline uint64_t _BoneMatrix = 0;
    inline uint64_t _SetOutlineMode = 0;
    inline uint64_t _FMemoryMalloc = 0;
    inline uint64_t _PlayFinisher = 0;
    inline uint64_t _GetSpreadValues = 0;
    inline uint64_t _GetSpreadAngles = 0;
    inline uint64_t _VecNormalize = 0;
    inline uint64_t _AngleNormalize = 0;
    inline uint64_t _GetFiringLoc = 0;
    inline uint64_t _TriggerVeh = 0;
    inline uint64_t _Version = 0;

    // Decryption Helper
    inline uint64_t Get(uint64_t val) { return val ^ g_StorageKey; }

    // Accessor Functions (Replacing variables with functions to avoid macro issues)
    inline uint64_t GWorld() { return Get(_GWorld); }
    inline uint64_t FNameState() { return Get(_FNameState); }
    inline uint64_t FNameKey() { return Get(_FNameKey); }
    inline uint64_t FNamePool() { return Get(_FNamePool); }
    inline uint64_t PointerState() { return Get(_PointerState); }
    inline uint64_t PointerKey() { return Get(_PointerKey); }
    inline uint64_t Version() { return _Version; }
    inline uint64_t PersistentLevel() { return Get(_PersistentLevel); }
    inline uint64_t GameInstance() { return Get(_GameInstance); }
    inline uint64_t GameState() { return Get(_GameState); }
    inline uint64_t Levels() { return Get(_Levels); }
    inline uint64_t LocalPlayers() { return Get(_LocalPlayers); }
    inline uint64_t ClientGameInstance() { return Get(_ClientGameInstance); }
    inline uint64_t PlayerController() { return Get(_PlayerController); }
    inline uint64_t ActorArray() { return Get(_ActorArray); }
    inline uint64_t ActorCount() { return Get(_ActorCount); }
    inline uint64_t AcknowledgedPawn() { return Get(_AcknowledgedPawn); }
    inline uint64_t PlayerState() { return Get(_PlayerState); }
    inline uint64_t PlayerID() { return Get(_PlayerID); }
    inline uint64_t PlayerArray() { return Get(_PlayerArray); }
    inline uint64_t SpawnedCharacter() { return Get(_SpawnedCharacter); }
    inline uint64_t PawnPrivate() { return Get(_PawnPrivate); }
    inline uint64_t RootComponent() { return Get(_RootComponent); }
    inline uint64_t RelativeLocation() { return Get(_RelativeLocation); }
    inline uint64_t RelativeRotation() { return Get(_RelativeRotation); }
    inline uint64_t RelativeScale3D() { return Get(_RelativeScale3D); }
    inline uint64_t ComponentVelocity() { return Get(_ComponentVelocity); }
    inline uint64_t ComponentToWorld() { return Get(_ComponentToWorld); }
    inline uint64_t Mesh() { return Get(_Mesh); }
    inline uint64_t MeshComponent() { return Get(_MeshComponent); }
    inline uint64_t BoneArray() { return Get(_BoneArray); }
    inline uint64_t BoneArrayCache() { return Get(_BoneArrayCache); }
    inline uint64_t BoneCount() { return Get(_BoneCount); }
    inline uint64_t bForceWireframe() { return Get(_bForceWireframe); }
    inline uint64_t Mesh1PAresEquip() { return Get(_Mesh1PAresEquip); }
    inline uint64_t Mesh3PAresEquip() { return Get(_Mesh3PAresEquip); }
    inline uint64_t Mesh1PCharacter() { return Get(_Mesh1PCharacter); }
    inline uint64_t MeshOverlay1P() { return Get(_MeshOverlay1P); }
    inline uint64_t Mesh3PMIDs() { return Get(_Mesh3PMIDs); }
    inline uint64_t Mesh1PMIDs() { return Get(_Mesh1PMIDs); }
    inline uint64_t Mesh1POverlayMIDs() { return Get(_Mesh1POverlayMIDs); }
    inline uint64_t ProcMeshBodySetup() { return Get(_ProcMeshBodySetup); }
    inline uint64_t BoundsScale() { return Get(_BoundsScale); }
    inline uint64_t LastSubmitTime() { return Get(_LastSubmitTime); }
    inline uint64_t LastRenderTime() { return Get(_LastRenderTime); }
    inline uint64_t DamageHandler() { return Get(_DamageHandler); }
    inline uint64_t BlindManager() { return Get(_BlindManager); }
    inline uint64_t DamageSections() { return Get(_DamageSections); }
    inline uint64_t Health() { return Get(_Health); }
    inline uint64_t CachedLife() { return Get(_CachedLife); }
    inline uint64_t LifeSection() { return Get(_LifeSection); }
    inline uint64_t MaxLifeSection() { return Get(_MaxLifeSection); }
    inline uint64_t bAliveSection() { return Get(_bAliveSection); }
    inline uint64_t LocalBlindState() { return Get(_LocalBlindState); }
    inline uint64_t bIsDormant() { return Get(_bIsDormant); }
    inline uint64_t bAlive() { return Get(_bAlive); }
    inline uint64_t GunRecoveryTime() { return Get(_GunRecoveryTime); }
    inline uint64_t StabilityMobile() { return Get(_StabilityMobile); }
    inline uint64_t WasAlly() { return Get(_WasAlly); }
    inline uint64_t WasInvisible() { return Get(_WasInvisible); }
    inline uint64_t WasIntangible() { return Get(_WasIntangible); }
    inline uint64_t bIsPlayerCharacter() { return Get(_bIsPlayerCharacter); }
    inline uint64_t TeamComponent() { return Get(_TeamComponent); }
    inline uint64_t TeamID() { return Get(_TeamID); }
    inline uint64_t PlayerCameraManager() { return Get(_PlayerCameraManager); }
    inline uint64_t CameraCachePrivate() { return Get(_CameraCachePrivate); }
    inline uint64_t ControlRotation() { return Get(_ControlRotation); }
    inline uint64_t CameraPosition() { return Get(_CameraPosition); }
    inline uint64_t CameraRotation() { return Get(_CameraRotation); }
    inline uint64_t CameraFOV() { return Get(_CameraFOV); }
    inline uint64_t Inventory() { return Get(_Inventory); }
    inline uint64_t CurrentEquippable() { return Get(_CurrentEquippable); }
    inline uint64_t EquippableModels() { return Get(_EquippableModels); }
    inline uint64_t TotemModels() { return Get(_TotemModels); }
    inline uint64_t CharmMap() { return Get(_CharmMap); }
    inline uint64_t SkinDataAsset() { return Get(_SkinDataAsset); }
    inline uint64_t CharmDataAsset() { return Get(_CharmDataAsset); }
    inline uint64_t CosmeticRandomSeed() { return Get(_CosmeticRandomSeed); }
    inline uint64_t AttachName() { return Get(_AttachName); }
    inline uint64_t MagazineAmmo() { return Get(_MagazineAmmo); }
    inline uint64_t MaxAmmo() { return Get(_MaxAmmo); }
    inline uint64_t BombTimeRemaining() { return Get(_BombTimeRemaining); }
    inline uint64_t PlantedASite() { return Get(_PlantedASite); }
    inline uint64_t PlantedAtSite() { return Get(_PlantedAtSite); }
    inline uint64_t BombDefused() { return Get(_BombDefused); }
    inline uint64_t CurrentDefuser() { return Get(_CurrentDefuser); }
    inline uint64_t DefuseProgress() { return Get(_DefuseProgress); }
    inline uint64_t GameUserSettings() { return Get(_GameUserSettings); }
    inline uint64_t AresSettingsManager() { return Get(_AresSettingsManager); }
    inline uint64_t bUseVSync() { return Get(_bUseVSync); }
    inline uint64_t ResolutionSizeX() { return Get(_ResolutionSizeX); }
    inline uint64_t ResolutionSizeY() { return Get(_ResolutionSizeY); }
    inline uint64_t FloatSettingValues() { return Get(_FloatSettingValues); }
    inline uint64_t IntSettingValues() { return Get(_IntSettingValues); }
    inline uint64_t BoolSettingValues() { return Get(_BoolSettingValues); }
    inline uint64_t StringSettingValues() { return Get(_StringSettingValues); }
    inline uint64_t Ping() { return Get(_Ping); }
    inline uint64_t PlatformPlayer() { return Get(_PlatformPlayer); }
    inline uint64_t SpectatedPlayer() { return Get(_SpectatedPlayer); }
    inline uint64_t HideAccountLevel() { return Get(_HideAccountLevel); }
    inline uint64_t AccountLevel() { return Get(_AccountLevel); }
    inline uint64_t CompetitiveTier() { return Get(_CompetitiveTier); }
    inline uint64_t PlayerNamePrivate() { return Get(_PlayerNamePrivate); }
    inline uint64_t UniqueID() { return Get(_UniqueID); }
    inline uint64_t ProcessEvent() { return Get(_ProcessEvent); }
    inline uint64_t StaticFindObject() { return Get(_StaticFindObject); }
    inline uint64_t StaticLoadObject() { return Get(_StaticLoadObject); }
    inline uint64_t BoneMatrix() { return Get(_BoneMatrix); }
    inline uint64_t SetOutlineMode() { return Get(_SetOutlineMode); }
    inline uint64_t FMemoryMalloc() { return Get(_FMemoryMalloc); }
    inline uint64_t PlayFinisher() { return Get(_PlayFinisher); }
    inline uint64_t GetSpreadValues() { return Get(_GetSpreadValues); }
    inline uint64_t GetSpreadAngles() { return Get(_GetSpreadAngles); }
    inline uint64_t VecNormalize() { return Get(_VecNormalize); }
    inline uint64_t AngleNormalize() { return Get(_AngleNormalize); }
    inline uint64_t GetFiringLoc() { return Get(_GetFiringLoc); }
    inline uint64_t TriggerVeh() { return Get(_TriggerVeh); }

    // Internal Decryption for JSON
    inline uint64_t _Parse(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t keyPos = json.find(searchKey);
        if (keyPos == std::string::npos) return 0;
        size_t colonPos = json.find(":", keyPos + searchKey.length());
        if (colonPos == std::string::npos) return 0;
        size_t valStart = json.find("\"", colonPos);
        if (valStart == std::string::npos) return 0;
        valStart++;
        size_t valEnd = json.find("\"", valStart);
        if (valEnd == std::string::npos) return 0;
        std::string valStr = json.substr(valStart, valEnd - valStart);
        try { return std::stoull(valStr, nullptr, 16); } catch (...) { return 0; }
    }

    // [PHASE 1] Initializing decoy data to confuse dumpers
    inline void LdrpInitializeProcessConfig() {
        if (g_StorageKey == 0) {
            FILETIME ft; GetSystemTimeAsFileTime(&ft);
            g_StorageKey = (uint64_t)ft.dwLowDateTime | ((uint64_t)ft.dwHighDateTime << 32);
        }
        
        // Fill with believable garbage offsets (decoy)
        _GWorld = (0x7FF00000000 + (rand() % 0xFFFF)) ^ g_StorageKey;
        _FNameState = (0x7FF00000000 + (rand() % 0xFFFF)) ^ g_StorageKey;
        _ActorArray = 0xAA0 ^ g_StorageKey;
        _Mesh = 0x430 ^ g_StorageKey;
    }

    // [PHASE 2] Loading real protected library config (Cloud Fetch)
    inline uint64_t _ParseLocal(const std::string& line) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) return 0;
        size_t valStart = line.find_first_not_of(" \t", eqPos + 1);
        if (valStart == std::string::npos) return 0;
        
        size_t plusPos = line.find('+', valStart);
        if (plusPos != std::string::npos) {
            std::string part1 = line.substr(valStart, plusPos - valStart);
            std::string part2 = line.substr(plusPos + 1);
            try { return std::stoull(part1, nullptr, 0) + std::stoull(part2, nullptr, 0); } catch (...) { return 0; }
        }
        
        try { return std::stoull(line.substr(valStart), nullptr, 0); } catch (...) { return 0; }
    }

    inline uint64_t _FindValueLocal(const std::string& fileData, const std::string& key) {
        std::string searchKey = " " + key + " =";
        size_t keyPos = fileData.find(searchKey);
        if (keyPos == std::string::npos) return 0;
        
        size_t lineEnd = fileData.find(';', keyPos);
        if (lineEnd == std::string::npos) lineEnd = fileData.find('\n', keyPos);
        if (lineEnd == std::string::npos) return 0;
        
        return _ParseLocal(fileData.substr(keyPos, lineEnd - keyPos));
    }

    inline bool LdrpLoadLibraryConfigLocal() {
        std::ifstream file("vdata.txt");
        if (!file.is_open()) return false;
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();
        
        if (_FindValueLocal(data, "GWorld") == 0) return false;

        _GWorld = _FindValueLocal(data, "GWorld") ^ g_StorageKey;
        _FNameState = _FindValueLocal(data, "FNameState") ^ g_StorageKey;
        _FNameKey = _FindValueLocal(data, "FNameKey") ^ g_StorageKey;
        _FNamePool = _FindValueLocal(data, "FNamePool") ^ g_StorageKey;
        _PointerState = _FindValueLocal(data, "PointerState") ^ g_StorageKey;
        _PointerKey = _FindValueLocal(data, "PointerKey") ^ g_StorageKey;
        _PersistentLevel = _FindValueLocal(data, "PersistentLevel") ^ g_StorageKey;
        _GameInstance = _FindValueLocal(data, "GameInstance") ^ g_StorageKey;
        _GameState = _FindValueLocal(data, "GameState") ^ g_StorageKey;
        _Levels = _FindValueLocal(data, "Levels") ^ g_StorageKey;
        _LocalPlayers = _FindValueLocal(data, "LocalPlayers") ^ g_StorageKey;
        _ClientGameInstance = _FindValueLocal(data, "ClientGameInstance") ^ g_StorageKey;
        _PlayerController = _FindValueLocal(data, "PlayerController") ^ g_StorageKey;
        _ActorArray = _FindValueLocal(data, "ActorArray") ^ g_StorageKey;
        _ActorCount = _FindValueLocal(data, "ActorCount") ^ g_StorageKey;
        _AcknowledgedPawn = _FindValueLocal(data, "AcknowledgedPawn") ^ g_StorageKey;
        _PlayerState = _FindValueLocal(data, "PlayerState") ^ g_StorageKey;
        _PlayerID = _FindValueLocal(data, "PlayerID") ^ g_StorageKey;
        _PlayerArray = _FindValueLocal(data, "PlayerArray") ^ g_StorageKey;
        _SpawnedCharacter = _FindValueLocal(data, "SpawnedCharacter") ^ g_StorageKey;
        _PawnPrivate = _FindValueLocal(data, "PawnPrivate") ^ g_StorageKey;
        _RootComponent = _FindValueLocal(data, "RootComponent") ^ g_StorageKey;
        _RelativeLocation = _FindValueLocal(data, "RelativeLocation") ^ g_StorageKey;
        _RelativeRotation = _FindValueLocal(data, "RelativeRotation") ^ g_StorageKey;
        _RelativeScale3D = _FindValueLocal(data, "RelativeScale3D") ^ g_StorageKey;
        _ComponentVelocity = _FindValueLocal(data, "ComponentVelocity") ^ g_StorageKey;
        _ComponentToWorld = _FindValueLocal(data, "ComponentToWorld") ^ g_StorageKey;
        _Mesh = _FindValueLocal(data, "Mesh") ^ g_StorageKey;
        _MeshComponent = _FindValueLocal(data, "MeshComponent") ^ g_StorageKey;
        _BoneArray = _FindValueLocal(data, "BoneArray") ^ g_StorageKey;
        _BoneArrayCache = _FindValueLocal(data, "BoneArrayCache") ^ g_StorageKey;
        _BoneCount = _FindValueLocal(data, "BoneCount") ^ g_StorageKey;
        _bForceWireframe = _FindValueLocal(data, "bForceWireframe") ^ g_StorageKey;
        _Mesh1PAresEquip = _FindValueLocal(data, "Mesh1PAresEquip") ^ g_StorageKey;
        _Mesh3PAresEquip = _FindValueLocal(data, "Mesh3PAresEquip") ^ g_StorageKey;
        _Mesh1PCharacter = _FindValueLocal(data, "Mesh1PCharacter") ^ g_StorageKey;
        _MeshOverlay1P = _FindValueLocal(data, "MeshOverlay1P") ^ g_StorageKey;
        _Mesh3PMIDs = _FindValueLocal(data, "Mesh3PMIDs") ^ g_StorageKey;
        _Mesh1PMIDs = _FindValueLocal(data, "Mesh1PMIDs") ^ g_StorageKey;
        _Mesh1POverlayMIDs = _FindValueLocal(data, "Mesh1POverlayMIDs") ^ g_StorageKey;
        _ProcMeshBodySetup = _FindValueLocal(data, "ProcMeshBodySetup") ^ g_StorageKey;
        _BoundsScale = _FindValueLocal(data, "BoundsScale") ^ g_StorageKey;
        _LastSubmitTime = _FindValueLocal(data, "LastSubmitTime") ^ g_StorageKey;
        _LastRenderTime = _FindValueLocal(data, "LastRenderTime") ^ g_StorageKey;
        _DamageHandler = _FindValueLocal(data, "DamageHandler") ^ g_StorageKey;
        _BlindManager = _FindValueLocal(data, "BlindManager") ^ g_StorageKey;
        _DamageSections = _FindValueLocal(data, "DamageSections") ^ g_StorageKey;
        _Health = _FindValueLocal(data, "Health") ^ g_StorageKey;
        _CachedLife = _FindValueLocal(data, "CachedLife") ^ g_StorageKey;
        _LifeSection = _FindValueLocal(data, "LifeSection") ^ g_StorageKey;
        _MaxLifeSection = _FindValueLocal(data, "MaxLifeSection") ^ g_StorageKey;
        _bAliveSection = _FindValueLocal(data, "bAliveSection") ^ g_StorageKey;
        _LocalBlindState = _FindValueLocal(data, "LocalBlindState") ^ g_StorageKey;
        _bIsDormant = _FindValueLocal(data, "bIsDormant") ^ g_StorageKey;
        _bAlive = _FindValueLocal(data, "bAlive") ^ g_StorageKey;
        _GunRecoveryTime = _FindValueLocal(data, "GunRecoveryTime") ^ g_StorageKey;
        _StabilityMobile = _FindValueLocal(data, "StabilityMobile") ^ g_StorageKey;
        _WasAlly = _FindValueLocal(data, "WasAlly") ^ g_StorageKey;
        _WasInvisible = _FindValueLocal(data, "WasInvisible") ^ g_StorageKey;
        _WasIntangible = _FindValueLocal(data, "WasIntangible") ^ g_StorageKey;
        _TeamComponent = _FindValueLocal(data, "TeamComponent") ^ g_StorageKey;
        _TeamID = _FindValueLocal(data, "TeamID") ^ g_StorageKey;
        _PlayerCameraManager = _FindValueLocal(data, "PlayerCameraManager") ^ g_StorageKey;
        _CameraCachePrivate = _FindValueLocal(data, "CameraCachePrivate") ^ g_StorageKey;
        _ControlRotation = _FindValueLocal(data, "ControlRotation") ^ g_StorageKey;
        _Inventory = _FindValueLocal(data, "Inventory") ^ g_StorageKey;
        _CurrentEquippable = _FindValueLocal(data, "CurrentEquippable") ^ g_StorageKey;
        _EquippableModels = _FindValueLocal(data, "EquippableModels") ^ g_StorageKey;
        _TotemModels = _FindValueLocal(data, "TotemModels") ^ g_StorageKey;
        _CharmMap = _FindValueLocal(data, "CharmMap") ^ g_StorageKey;
        _SkinDataAsset = _FindValueLocal(data, "SkinDataAsset") ^ g_StorageKey;
        _CharmDataAsset = _FindValueLocal(data, "CharmDataAsset") ^ g_StorageKey;
        _CosmeticRandomSeed = _FindValueLocal(data, "CosmeticRandomSeed") ^ g_StorageKey;
        _AttachName = _FindValueLocal(data, "AttachName") ^ g_StorageKey;
        _MagazineAmmo = _FindValueLocal(data, "MagazineAmmo") ^ g_StorageKey;
        _MaxAmmo = _FindValueLocal(data, "MaxAmmo") ^ g_StorageKey;
        _BombTimeRemaining = _FindValueLocal(data, "BombTimeRemaining") ^ g_StorageKey;
        _PlantedASite = _FindValueLocal(data, "PlantedASite") ^ g_StorageKey;
        _PlantedAtSite = _FindValueLocal(data, "PlantedAtSite") ^ g_StorageKey;
        _BombDefused = _FindValueLocal(data, "BombDefused") ^ g_StorageKey;
        _CurrentDefuser = _FindValueLocal(data, "CurrentDefuser") ^ g_StorageKey;
        _DefuseProgress = _FindValueLocal(data, "DefuseProgress") ^ g_StorageKey;
        _GameUserSettings = _FindValueLocal(data, "GameUserSettings") ^ g_StorageKey;
        _AresSettingsManager = _FindValueLocal(data, "AresSettingsManager") ^ g_StorageKey;
        _bUseVSync = _FindValueLocal(data, "bUseVSync") ^ g_StorageKey;
        _ResolutionSizeX = _FindValueLocal(data, "ResolutionSizeX") ^ g_StorageKey;
        _ResolutionSizeY = _FindValueLocal(data, "ResolutionSizeY") ^ g_StorageKey;
        _FloatSettingValues = _FindValueLocal(data, "FloatSettingValues") ^ g_StorageKey;
        _IntSettingValues = _FindValueLocal(data, "IntSettingValues") ^ g_StorageKey;
        _BoolSettingValues = _FindValueLocal(data, "BoolSettingValues") ^ g_StorageKey;
        _StringSettingValues = _FindValueLocal(data, "StringSettingValues") ^ g_StorageKey;
        _Ping = _FindValueLocal(data, "Ping") ^ g_StorageKey;
        _PlatformPlayer = _FindValueLocal(data, "PlatformPlayer") ^ g_StorageKey;
        _SpectatedPlayer = _FindValueLocal(data, "SpectatedPlayer") ^ g_StorageKey;
        _HideAccountLevel = _FindValueLocal(data, "HideAccountLevel") ^ g_StorageKey;
        _AccountLevel = _FindValueLocal(data, "AccountLevel") ^ g_StorageKey;
        _CompetitiveTier = _FindValueLocal(data, "CompetitiveTier") ^ g_StorageKey;
        _PlayerNamePrivate = _FindValueLocal(data, "PlayerNamePrivate") ^ g_StorageKey;
        _ProcessEvent = _FindValueLocal(data, "ProcessEvent") ^ g_StorageKey;
        _StaticFindObject = _FindValueLocal(data, "StaticFindObject") ^ g_StorageKey;
        _StaticLoadObject = _FindValueLocal(data, "StaticLoadObject") ^ g_StorageKey;
        _BoneMatrix = _FindValueLocal(data, "BoneMatrix") ^ g_StorageKey;
        _SetOutlineMode = _FindValueLocal(data, "SetOutlineMode") ^ g_StorageKey;
        _FMemoryMalloc = _FindValueLocal(data, "FMemoryMalloc") ^ g_StorageKey;
        _PlayFinisher = _FindValueLocal(data, "PlayFinisher") ^ g_StorageKey;
        _GetSpreadValues = _FindValueLocal(data, "GetSpreadValues") ^ g_StorageKey;
        _GetSpreadAngles = _FindValueLocal(data, "GetSpreadAngles") ^ g_StorageKey;
        _VecNormalize = _FindValueLocal(data, "VecNormalize") ^ g_StorageKey;
        _AngleNormalize = _FindValueLocal(data, "AngleNormalize") ^ g_StorageKey;
        _GetFiringLoc = _FindValueLocal(data, "GetFiringLoc") ^ g_StorageKey;
        _TriggerVeh = _FindValueLocal(data, "TriggerVeh") ^ g_StorageKey;
        _UniqueID = _FindValueLocal(data, "UniqueID") ^ g_StorageKey;
        _Version = _FindValueLocal(data, "Version");

        return true;
    }

    inline bool LdrpLoadLibraryConfig(int pId) {
        if (LdrpLoadLibraryConfigLocal()) {
            std::cout << "[+] Loaded offsets from local file vdata.txt" << std::endl;
            return true;
        }

        HINTERNET hInt = InternetOpenA(skCrypt("LdrpEngine"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInt) return false;

        std::string res = skCrypt("https://licensing-backend.donghiem114.workers.dev/public/offsets/");
        res += std::to_string(pId);

        HINTERNET hU = InternetOpenUrlA(hInt, res.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hU) { InternetCloseHandle(hInt); return false; }

        std::string data = "";
        char buf[2048]; DWORD bR = 0;
        while (InternetReadFile(hU, buf, sizeof(buf) - 1, &bR) && bR > 0) {
            buf[bR] = 0; data += buf;
        }
        InternetCloseHandle(hU); InternetCloseHandle(hInt);

        if (data.length() < 10) return false;

        // Store with XOR Encryption
        _GWorld = _Parse(data, skCrypt("GWorld")) ^ g_StorageKey;
        _FNameState = _Parse(data, skCrypt("FNameState")) ^ g_StorageKey;
        _FNameKey = _Parse(data, skCrypt("FNameKey")) ^ g_StorageKey;
        _FNamePool = _Parse(data, skCrypt("FNamePool")) ^ g_StorageKey;
        _PointerState = _Parse(data, skCrypt("PointerState")) ^ g_StorageKey;
        _PointerKey = _Parse(data, skCrypt("PointerKey")) ^ g_StorageKey;
        _PersistentLevel = _Parse(data, skCrypt("PersistentLevel")) ^ g_StorageKey;
        _GameInstance = _Parse(data, skCrypt("GameInstance")) ^ g_StorageKey;
        _GameState = _Parse(data, skCrypt("GameState")) ^ g_StorageKey;
        _Levels = _Parse(data, skCrypt("Levels")) ^ g_StorageKey;
        _LocalPlayers = _Parse(data, skCrypt("LocalPlayers")) ^ g_StorageKey;
        _ClientGameInstance = _Parse(data, skCrypt("ClientGameInstance")) ^ g_StorageKey;
        _PlayerController = _Parse(data, skCrypt("PlayerController")) ^ g_StorageKey;
        _ActorArray = _Parse(data, skCrypt("ActorArray")) ^ g_StorageKey;
        _ActorCount = _Parse(data, skCrypt("ActorCount")) ^ g_StorageKey;
        _AcknowledgedPawn = _Parse(data, skCrypt("AcknowledgedPawn")) ^ g_StorageKey;
        _PlayerState = _Parse(data, skCrypt("PlayerState")) ^ g_StorageKey;
        _PlayerID = _Parse(data, skCrypt("PlayerID")) ^ g_StorageKey;
        _PlayerArray = _Parse(data, skCrypt("PlayerArray")) ^ g_StorageKey;
        _SpawnedCharacter = _Parse(data, skCrypt("SpawnedCharacter")) ^ g_StorageKey;
        _PawnPrivate = _Parse(data, skCrypt("PawnPrivate")) ^ g_StorageKey;
        _RootComponent = _Parse(data, skCrypt("RootComponent")) ^ g_StorageKey;
        _RelativeLocation = _Parse(data, skCrypt("RelativeLocation")) ^ g_StorageKey;
        _RelativeRotation = _Parse(data, skCrypt("RelativeRotation")) ^ g_StorageKey;
        _RelativeScale3D = _Parse(data, skCrypt("RelativeScale3D")) ^ g_StorageKey;
        _ComponentVelocity = _Parse(data, skCrypt("ComponentVelocity")) ^ g_StorageKey;
        _ComponentToWorld = _Parse(data, skCrypt("ComponentToWorld")) ^ g_StorageKey;
        _Mesh = _Parse(data, skCrypt("Mesh")) ^ g_StorageKey;
        _MeshComponent = _Parse(data, skCrypt("MeshComponent")) ^ g_StorageKey;
        _BoneArray = _Parse(data, skCrypt("BoneArray")) ^ g_StorageKey;
        _BoneArrayCache = _Parse(data, skCrypt("BoneArrayCache")) ^ g_StorageKey;
        _BoneCount = _Parse(data, skCrypt("BoneCount")) ^ g_StorageKey;
        _bForceWireframe = _Parse(data, skCrypt("bForceWireframe")) ^ g_StorageKey;
        _Mesh1PAresEquip = _Parse(data, skCrypt("Mesh1PAresEquip")) ^ g_StorageKey;
        _Mesh3PAresEquip = _Parse(data, skCrypt("Mesh3PAresEquip")) ^ g_StorageKey;
        _Mesh1PCharacter = _Parse(data, skCrypt("Mesh1PCharacter")) ^ g_StorageKey;
        _MeshOverlay1P = _Parse(data, skCrypt("MeshOverlay1P")) ^ g_StorageKey;
        _Mesh3PMIDs = _Parse(data, skCrypt("Mesh3PMIDs")) ^ g_StorageKey;
        _Mesh1PMIDs = _Parse(data, skCrypt("Mesh1PMIDs")) ^ g_StorageKey;
        _Mesh1POverlayMIDs = _Parse(data, skCrypt("Mesh1POverlayMIDs")) ^ g_StorageKey;
        _ProcMeshBodySetup = _Parse(data, skCrypt("ProcMeshBodySetup")) ^ g_StorageKey;
        _BoundsScale = _Parse(data, skCrypt("BoundsScale")) ^ g_StorageKey;
        _LastSubmitTime = _Parse(data, skCrypt("LastSubmitTime")) ^ g_StorageKey;
        _LastRenderTime = _Parse(data, skCrypt("LastRenderTime")) ^ g_StorageKey;
        _DamageHandler = _Parse(data, skCrypt("DamageHandler")) ^ g_StorageKey;
        _BlindManager = _Parse(data, skCrypt("BlindManager")) ^ g_StorageKey;
        _DamageSections = _Parse(data, skCrypt("DamageSections")) ^ g_StorageKey;
        _Health = _Parse(data, skCrypt("Health")) ^ g_StorageKey;
        _CachedLife = _Parse(data, skCrypt("CachedLife")) ^ g_StorageKey;
        _LifeSection = _Parse(data, skCrypt("LifeSection")) ^ g_StorageKey;
        _MaxLifeSection = _Parse(data, skCrypt("MaxLifeSection")) ^ g_StorageKey;
        _bAliveSection = _Parse(data, skCrypt("bAliveSection")) ^ g_StorageKey;
        _LocalBlindState = _Parse(data, skCrypt("LocalBlindState")) ^ g_StorageKey;
        _bIsDormant = _Parse(data, skCrypt("bIsDormant")) ^ g_StorageKey;
        _bAlive = _Parse(data, skCrypt("bAlive")) ^ g_StorageKey;
        _GunRecoveryTime = _Parse(data, skCrypt("GunRecoveryTime")) ^ g_StorageKey;
        _StabilityMobile = _Parse(data, skCrypt("StabilityMobile")) ^ g_StorageKey;
        _WasAlly = _Parse(data, skCrypt("WasAlly")) ^ g_StorageKey;
        _WasInvisible = _Parse(data, skCrypt("WasInvisible")) ^ g_StorageKey;
        _WasIntangible = _Parse(data, skCrypt("WasIntangible")) ^ g_StorageKey;
        _TeamComponent = _Parse(data, skCrypt("TeamComponent")) ^ g_StorageKey;
        _TeamID = _Parse(data, skCrypt("TeamID")) ^ g_StorageKey;
        _PlayerCameraManager = _Parse(data, skCrypt("PlayerCameraManager")) ^ g_StorageKey;
        _CameraCachePrivate = _Parse(data, skCrypt("CameraCachePrivate")) ^ g_StorageKey;
        _ControlRotation = _Parse(data, skCrypt("ControlRotation")) ^ g_StorageKey;
        _Inventory = _Parse(data, skCrypt("Inventory")) ^ g_StorageKey;
        _CurrentEquippable = _Parse(data, skCrypt("CurrentEquippable")) ^ g_StorageKey;
        _EquippableModels = _Parse(data, skCrypt("EquippableModels")) ^ g_StorageKey;
        _TotemModels = _Parse(data, skCrypt("TotemModels")) ^ g_StorageKey;
        _CharmMap = _Parse(data, skCrypt("CharmMap")) ^ g_StorageKey;
        _SkinDataAsset = _Parse(data, skCrypt("SkinDataAsset")) ^ g_StorageKey;
        _CharmDataAsset = _Parse(data, skCrypt("CharmDataAsset")) ^ g_StorageKey;
        _CosmeticRandomSeed = _Parse(data, skCrypt("CosmeticRandomSeed")) ^ g_StorageKey;
        _AttachName = _Parse(data, skCrypt("AttachName")) ^ g_StorageKey;
        _MagazineAmmo = _Parse(data, skCrypt("MagazineAmmo")) ^ g_StorageKey;
        _MaxAmmo = _Parse(data, skCrypt("MaxAmmo")) ^ g_StorageKey;
        _BombTimeRemaining = _Parse(data, skCrypt("BombTimeRemaining")) ^ g_StorageKey;
        _PlantedASite = _Parse(data, skCrypt("PlantedASite")) ^ g_StorageKey;
        _PlantedAtSite = _Parse(data, skCrypt("PlantedAtSite")) ^ g_StorageKey;
        _BombDefused = _Parse(data, skCrypt("BombDefused")) ^ g_StorageKey;
        _CurrentDefuser = _Parse(data, skCrypt("CurrentDefuser")) ^ g_StorageKey;
        _DefuseProgress = _Parse(data, skCrypt("DefuseProgress")) ^ g_StorageKey;
        _GameUserSettings = _Parse(data, skCrypt("GameUserSettings")) ^ g_StorageKey;
        _AresSettingsManager = _Parse(data, skCrypt("AresSettingsManager")) ^ g_StorageKey;
        _bUseVSync = _Parse(data, skCrypt("bUseVSync")) ^ g_StorageKey;
        _ResolutionSizeX = _Parse(data, skCrypt("ResolutionSizeX")) ^ g_StorageKey;
        _ResolutionSizeY = _Parse(data, skCrypt("ResolutionSizeY")) ^ g_StorageKey;
        _FloatSettingValues = _Parse(data, skCrypt("FloatSettingValues")) ^ g_StorageKey;
        _IntSettingValues = _Parse(data, skCrypt("IntSettingValues")) ^ g_StorageKey;
        _BoolSettingValues = _Parse(data, skCrypt("BoolSettingValues")) ^ g_StorageKey;
        _StringSettingValues = _Parse(data, skCrypt("StringSettingValues")) ^ g_StorageKey;
        _Ping = _Parse(data, skCrypt("Ping")) ^ g_StorageKey;
        _PlatformPlayer = _Parse(data, skCrypt("PlatformPlayer")) ^ g_StorageKey;
        _SpectatedPlayer = _Parse(data, skCrypt("SpectatedPlayer")) ^ g_StorageKey;
        _HideAccountLevel = _Parse(data, skCrypt("HideAccountLevel")) ^ g_StorageKey;
        _AccountLevel = _Parse(data, skCrypt("AccountLevel")) ^ g_StorageKey;
        _CompetitiveTier = _Parse(data, skCrypt("CompetitiveTier")) ^ g_StorageKey;
        _PlayerNamePrivate = _Parse(data, skCrypt("PlayerNamePrivate")) ^ g_StorageKey;
        _ProcessEvent = _Parse(data, skCrypt("ProcessEvent")) ^ g_StorageKey;
        _StaticFindObject = _Parse(data, skCrypt("StaticFindObject")) ^ g_StorageKey;
        _StaticLoadObject = _Parse(data, skCrypt("StaticLoadObject")) ^ g_StorageKey;
        _BoneMatrix = _Parse(data, skCrypt("BoneMatrix")) ^ g_StorageKey;
        _SetOutlineMode = _Parse(data, skCrypt("SetOutlineMode")) ^ g_StorageKey;
        _FMemoryMalloc = _Parse(data, skCrypt("FMemoryMalloc")) ^ g_StorageKey;
        _PlayFinisher = _Parse(data, skCrypt("PlayFinisher")) ^ g_StorageKey;
        _GetSpreadValues = _Parse(data, skCrypt("GetSpreadValues")) ^ g_StorageKey;
        _GetSpreadAngles = _Parse(data, skCrypt("GetSpreadAngles")) ^ g_StorageKey;
        _VecNormalize = _Parse(data, skCrypt("VecNormalize")) ^ g_StorageKey;
        _AngleNormalize = _Parse(data, skCrypt("AngleNormalize")) ^ g_StorageKey;
        _GetFiringLoc = _Parse(data, skCrypt("GetFiringLoc")) ^ g_StorageKey;
        _TriggerVeh = _Parse(data, skCrypt("TriggerVeh")) ^ g_StorageKey;
        _UniqueID = _Parse(data, skCrypt("UniqueID")) ^ g_StorageKey;

        return (_Parse(data, skCrypt("GWorld")) != 0);
    }
}
