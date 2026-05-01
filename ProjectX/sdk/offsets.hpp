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

namespace ProjectXOffsets {
    // Hidden XOR Key - Generated unique per session
    inline uint64_t g_StorageKey = 0;

    // Encrypted Storage
    inline uint64_t _GWorld = 0;
    inline uint64_t _FNameState = 0;
    inline uint64_t _FNameKey = 0;
    inline uint64_t _FNamePool = 0;
    inline uint64_t _PersistentLevel = 0;
    inline uint64_t _GameInstance = 0;
    inline uint64_t _GameState = 0;
    inline uint64_t _LocalPlayers = 0;
    inline uint64_t _PlayerController = 0;
    inline uint64_t _ActorArray = 0;
    inline uint64_t _ActorCount = 0;
    inline uint64_t _AcknowledgedPawn = 0;
    inline uint64_t _PlayerState = 0;
    inline uint64_t _PlayerArray = 0;
    inline uint64_t _SpawnedCharacter = 0;
    inline uint64_t _PawnPrivate = 0;
    inline uint64_t _RootComponent = 0;
    inline uint64_t _RelativeLocation = 0;
    inline uint64_t _RelativeRotation = 0;
    inline uint64_t _ComponentToWorld = 0;
    inline uint64_t _Mesh = 0;
    inline uint64_t _BoneArray = 0;
    inline uint64_t _BoneArrayCache = 0;
    inline uint64_t _BoneCount = 0;
    inline uint64_t _BoundsScale = 0;
    inline uint64_t _LastSubmitTime = 0;
    inline uint64_t _LastSubmitTimeOnScreen = 0;
    inline uint64_t _DamageHandler = 0;
    inline uint64_t _DamageSections = 0;
    inline uint64_t _Health = 0;
    inline uint64_t _bIsDormant = 0;
    inline uint64_t _bAlive = 0;
    inline uint64_t _WasAlly = 0;
    inline uint64_t _bIsPlayerCharacter = 0;
    inline uint64_t _TeamComponent = 0;
    inline uint64_t _TeamID = 0;
    inline uint64_t _PlayerCameraManager = 0;
    inline uint64_t _CameraCachePrivate = 0;
    inline uint64_t _CameraPosition = 0;
    inline uint64_t _CameraRotation = 0;
    inline uint64_t _CameraFOV = 0;
    inline uint64_t _Inventory = 0;
    inline uint64_t _CurrentEquippable = 0;
    inline uint64_t _MagazineAmmo = 0;
    inline uint64_t _MaxAmmo = 0;
    inline uint64_t _BombTimeRemaining = 0;
    inline uint64_t _PlantedASite = 0;
    inline uint64_t _CurrentDefuser = 0;
    inline uint64_t _DefuseProgress = 0;
    inline uint64_t _Ping = 0;
    inline uint64_t _PlayerNamePrivate = 0;
    inline uint64_t _UniqueID = 0;
    inline uint64_t _Version = 0;

    // Decryption Helper
    inline uint64_t Get(uint64_t val) { return val ^ g_StorageKey; }

    // Accessor Functions (Replacing variables with functions to avoid macro issues)
    inline uint64_t GWorld() { return Get(_GWorld); }
    inline uint64_t FNameState() { return Get(_FNameState); }
    inline uint64_t FNameKey() { return Get(_FNameKey); }
    inline uint64_t FNamePool() { return Get(_FNamePool); }
    inline uint64_t Version() { return _Version; }
    inline uint64_t PersistentLevel() { return Get(_PersistentLevel); }
    inline uint64_t GameInstance() { return Get(_GameInstance); }
    inline uint64_t GameState() { return Get(_GameState); }
    inline uint64_t LocalPlayers() { return Get(_LocalPlayers); }
    inline uint64_t PlayerController() { return Get(_PlayerController); }
    inline uint64_t ActorArray() { return Get(_ActorArray); }
    inline uint64_t ActorCount() { return Get(_ActorCount); }
    inline uint64_t AcknowledgedPawn() { return Get(_AcknowledgedPawn); }
    inline uint64_t PlayerState() { return Get(_PlayerState); }
    inline uint64_t PlayerArray() { return Get(_PlayerArray); }
    inline uint64_t SpawnedCharacter() { return Get(_SpawnedCharacter); }
    inline uint64_t PawnPrivate() { return Get(_PawnPrivate); }
    inline uint64_t RootComponent() { return Get(_RootComponent); }
    inline uint64_t RelativeLocation() { return Get(_RelativeLocation); }
    inline uint64_t RelativeRotation() { return Get(_RelativeRotation); }
    inline uint64_t ComponentToWorld() { return Get(_ComponentToWorld); }
    inline uint64_t Mesh() { return Get(_Mesh); }
    inline uint64_t BoneArray() { return Get(_BoneArray); }
    inline uint64_t BoneArrayCache() { return Get(_BoneArrayCache); }
    inline uint64_t BoneCount() { return Get(_BoneCount); }
    inline uint64_t BoundsScale() { return Get(_BoundsScale); }
    inline uint64_t LastSubmitTime() { return Get(_LastSubmitTime); }
    inline uint64_t LastSubmitTimeOnScreen() { return Get(_LastSubmitTimeOnScreen); }
    inline uint64_t DamageHandler() { return Get(_DamageHandler); }
    inline uint64_t DamageSections() { return Get(_DamageSections); }
    inline uint64_t Health() { return Get(_Health); }
    inline uint64_t bIsDormant() { return Get(_bIsDormant); }
    inline uint64_t bAlive() { return Get(_bAlive); }
    inline uint64_t WasAlly() { return Get(_WasAlly); }
    inline uint64_t bIsPlayerCharacter() { return Get(_bIsPlayerCharacter); }
    inline uint64_t TeamComponent() { return Get(_TeamComponent); }
    inline uint64_t TeamID() { return Get(_TeamID); }
    inline uint64_t PlayerCameraManager() { return Get(_PlayerCameraManager); }
    inline uint64_t CameraCachePrivate() { return Get(_CameraCachePrivate); }
    inline uint64_t CameraPosition() { return Get(_CameraPosition); }
    inline uint64_t CameraRotation() { return Get(_CameraRotation); }
    inline uint64_t CameraFOV() { return Get(_CameraFOV); }
    inline uint64_t Inventory() { return Get(_Inventory); }
    inline uint64_t CurrentEquippable() { return Get(_CurrentEquippable); }
    inline uint64_t MagazineAmmo() { return Get(_MagazineAmmo); }
    inline uint64_t MaxAmmo() { return Get(_MaxAmmo); }
    inline uint64_t BombTimeRemaining() { return Get(_BombTimeRemaining); }
    inline uint64_t PlantedASite() { return Get(_PlantedASite); }
    inline uint64_t CurrentDefuser() { return Get(_CurrentDefuser); }
    inline uint64_t DefuseProgress() { return Get(_DefuseProgress); }
    inline uint64_t Ping() { return Get(_Ping); }
    inline uint64_t PlayerNamePrivate() { return Get(_PlayerNamePrivate); }
    inline uint64_t UniqueID() { return Get(_UniqueID); }

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
        std::ifstream file("offsets_ProjectX.txt");
        if (!file.is_open()) return false;
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string data = buffer.str();
        
        if (_FindValueLocal(data, "GWorld") == 0) return false;

        _GWorld = _FindValueLocal(data, "GWorld") ^ g_StorageKey;
        _FNameState = _FindValueLocal(data, "FNameState") ^ g_StorageKey;
        _FNameKey = _FindValueLocal(data, "FNameKey") ^ g_StorageKey;
        _FNamePool = _FindValueLocal(data, "FNamePool") ^ g_StorageKey;
        _PersistentLevel = _FindValueLocal(data, "PersistentLevel") ^ g_StorageKey;
        _GameInstance = _FindValueLocal(data, "GameInstance") ^ g_StorageKey;
        _GameState = _FindValueLocal(data, "GameState") ^ g_StorageKey;
        _LocalPlayers = _FindValueLocal(data, "LocalPlayers") ^ g_StorageKey;
        _PlayerController = _FindValueLocal(data, "PlayerController") ^ g_StorageKey;
        _ActorArray = _FindValueLocal(data, "ActorArray") ^ g_StorageKey;
        _ActorCount = _FindValueLocal(data, "ActorCount") ^ g_StorageKey;
        _AcknowledgedPawn = _FindValueLocal(data, "AcknowledgedPawn") ^ g_StorageKey;
        _PlayerState = _FindValueLocal(data, "PlayerState") ^ g_StorageKey;
        _PlayerArray = _FindValueLocal(data, "PlayerArray") ^ g_StorageKey;
        _SpawnedCharacter = _FindValueLocal(data, "SpawnedCharacter") ^ g_StorageKey;
        _PawnPrivate = _FindValueLocal(data, "PawnPrivate") ^ g_StorageKey;
        _RootComponent = _FindValueLocal(data, "RootComponent") ^ g_StorageKey;
        _RelativeLocation = _FindValueLocal(data, "RelativeLocation") ^ g_StorageKey;
        _RelativeRotation = _FindValueLocal(data, "RelativeRotation") ^ g_StorageKey;
        _ComponentToWorld = _FindValueLocal(data, "ComponentToWorld") ^ g_StorageKey;
        _Mesh = _FindValueLocal(data, "Mesh") ^ g_StorageKey;
        _BoneArray = _FindValueLocal(data, "BoneArray") ^ g_StorageKey;
        _BoneArrayCache = _FindValueLocal(data, "BoneArrayCache") ^ g_StorageKey;
        _BoneCount = _FindValueLocal(data, "BoneCount") ^ g_StorageKey;
        _BoundsScale = _FindValueLocal(data, "BoundsScale") ^ g_StorageKey;
        _LastSubmitTime = _FindValueLocal(data, "LastSubmitTime") ^ g_StorageKey;
        _LastSubmitTimeOnScreen = _FindValueLocal(data, "LastSubmitTimeOnScreen") ^ g_StorageKey;
        _DamageHandler = _FindValueLocal(data, "DamageHandler") ^ g_StorageKey;
        _DamageSections = _FindValueLocal(data, "DamageSections") ^ g_StorageKey;
        _Health = _FindValueLocal(data, "Health") ^ g_StorageKey;
        _bIsDormant = _FindValueLocal(data, "bIsDormant") ^ g_StorageKey;
        _bAlive = _FindValueLocal(data, "bAlive") ^ g_StorageKey;
        _WasAlly = _FindValueLocal(data, "WasAlly") ^ g_StorageKey;
        _bIsPlayerCharacter = _FindValueLocal(data, "bIsPlayerCharacter") ^ g_StorageKey;
        _TeamComponent = _FindValueLocal(data, "TeamComponent") ^ g_StorageKey;
        _TeamID = _FindValueLocal(data, "TeamID") ^ g_StorageKey;
        _PlayerCameraManager = _FindValueLocal(data, "PlayerCameraManager") ^ g_StorageKey;
        _CameraCachePrivate = _FindValueLocal(data, "CameraCachePrivate") ^ g_StorageKey;
        _CameraPosition = _FindValueLocal(data, "CameraPosition") ^ g_StorageKey;
        _CameraRotation = _FindValueLocal(data, "CameraRotation") ^ g_StorageKey;
        _CameraFOV = _FindValueLocal(data, "CameraFOV") ^ g_StorageKey;
        _Inventory = _FindValueLocal(data, "Inventory") ^ g_StorageKey;
        _CurrentEquippable = _FindValueLocal(data, "CurrentEquippable") ^ g_StorageKey;
        _MagazineAmmo = _FindValueLocal(data, "MagazineAmmo") ^ g_StorageKey;
        _MaxAmmo = _FindValueLocal(data, "MaxAmmo") ^ g_StorageKey;
        _BombTimeRemaining = _FindValueLocal(data, "BombTimeRemaining") ^ g_StorageKey;
        _PlantedASite = _FindValueLocal(data, "PlantedASite") ^ g_StorageKey;
        _CurrentDefuser = _FindValueLocal(data, "CurrentDefuser") ^ g_StorageKey;
        _DefuseProgress = _FindValueLocal(data, "DefuseProgress") ^ g_StorageKey;
        _Ping = _FindValueLocal(data, "Ping") ^ g_StorageKey;
        _PlayerNamePrivate = _FindValueLocal(data, "PlayerNamePrivate") ^ g_StorageKey;
        _UniqueID = _FindValueLocal(data, "UniqueID") ^ g_StorageKey;
        _Version = _FindValueLocal(data, "Version");

        return true;
    }

    inline bool LdrpLoadLibraryConfig(int pId) {
        if (LdrpLoadLibraryConfigLocal()) {
            std::cout << "[+] Loaded offsets from local file offsets_ProjectX.txt" << std::endl;
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
        _PersistentLevel = _Parse(data, skCrypt("PersistentLevel")) ^ g_StorageKey;
        _GameInstance = _Parse(data, skCrypt("GameInstance")) ^ g_StorageKey;
        _GameState = _Parse(data, skCrypt("GameState")) ^ g_StorageKey;
        _LocalPlayers = _Parse(data, skCrypt("LocalPlayers")) ^ g_StorageKey;
        _PlayerController = _Parse(data, skCrypt("PlayerController")) ^ g_StorageKey;
        _ActorArray = _Parse(data, skCrypt("ActorArray")) ^ g_StorageKey;
        _ActorCount = _Parse(data, skCrypt("ActorCount")) ^ g_StorageKey;
        _AcknowledgedPawn = _Parse(data, skCrypt("AcknowledgedPawn")) ^ g_StorageKey;
        _PlayerState = _Parse(data, skCrypt("PlayerState")) ^ g_StorageKey;
        _PlayerArray = _Parse(data, skCrypt("PlayerArray")) ^ g_StorageKey;
        _SpawnedCharacter = _Parse(data, skCrypt("SpawnedCharacter")) ^ g_StorageKey;
        _PawnPrivate = _Parse(data, skCrypt("PawnPrivate")) ^ g_StorageKey;
        _RootComponent = _Parse(data, skCrypt("RootComponent")) ^ g_StorageKey;
        _RelativeLocation = _Parse(data, skCrypt("RelativeLocation")) ^ g_StorageKey;
        _RelativeRotation = _Parse(data, skCrypt("RelativeRotation")) ^ g_StorageKey;
        _ComponentToWorld = _Parse(data, skCrypt("ComponentToWorld")) ^ g_StorageKey;
        _Mesh = _Parse(data, skCrypt("Mesh")) ^ g_StorageKey;
        _BoneArray = _Parse(data, skCrypt("BoneArray")) ^ g_StorageKey;
        _BoneArrayCache = _Parse(data, skCrypt("BoneArrayCache")) ^ g_StorageKey;
        _BoneCount = _Parse(data, skCrypt("BoneCount")) ^ g_StorageKey;
        _BoundsScale = _Parse(data, skCrypt("BoundsScale")) ^ g_StorageKey;
        _LastSubmitTime = _Parse(data, skCrypt("LastSubmitTime")) ^ g_StorageKey;
        _LastSubmitTimeOnScreen = _Parse(data, skCrypt("LastSubmitTimeOnScreen")) ^ g_StorageKey;
        _DamageHandler = _Parse(data, skCrypt("DamageHandler")) ^ g_StorageKey;
        _DamageSections = _Parse(data, skCrypt("DamageSections")) ^ g_StorageKey;
        _Health = _Parse(data, skCrypt("Health")) ^ g_StorageKey;
        _bIsDormant = _Parse(data, skCrypt("bIsDormant")) ^ g_StorageKey;
        _bAlive = _Parse(data, skCrypt("bAlive")) ^ g_StorageKey;
        _WasAlly = _Parse(data, skCrypt("WasAlly")) ^ g_StorageKey;
        _bIsPlayerCharacter = _Parse(data, skCrypt("bIsPlayerCharacter")) ^ g_StorageKey;
        _TeamComponent = _Parse(data, skCrypt("TeamComponent")) ^ g_StorageKey;
        _TeamID = _Parse(data, skCrypt("TeamID")) ^ g_StorageKey;
        _PlayerCameraManager = _Parse(data, skCrypt("PlayerCameraManager")) ^ g_StorageKey;
        _CameraCachePrivate = _Parse(data, skCrypt("CameraCachePrivate")) ^ g_StorageKey;
        _CameraPosition = _Parse(data, skCrypt("CameraPosition")) ^ g_StorageKey;
        _CameraRotation = _Parse(data, skCrypt("CameraRotation")) ^ g_StorageKey;
        _CameraFOV = _Parse(data, skCrypt("CameraFOV")) ^ g_StorageKey;
        _Inventory = _Parse(data, skCrypt("Inventory")) ^ g_StorageKey;
        _CurrentEquippable = _Parse(data, skCrypt("CurrentEquippable")) ^ g_StorageKey;
        _MagazineAmmo = _Parse(data, skCrypt("MagazineAmmo")) ^ g_StorageKey;
        _MaxAmmo = _Parse(data, skCrypt("MaxAmmo")) ^ g_StorageKey;
        _BombTimeRemaining = _Parse(data, skCrypt("BombTimeRemaining")) ^ g_StorageKey;
        _PlantedASite = _Parse(data, skCrypt("PlantedASite")) ^ g_StorageKey;
        _CurrentDefuser = _Parse(data, skCrypt("CurrentDefuser")) ^ g_StorageKey;
        _DefuseProgress = _Parse(data, skCrypt("DefuseProgress")) ^ g_StorageKey;
        _Ping = _Parse(data, skCrypt("Ping")) ^ g_StorageKey;
        _PlayerNamePrivate = _Parse(data, skCrypt("PlayerNamePrivate")) ^ g_StorageKey;
        _UniqueID = _Parse(data, skCrypt("UniqueID")) ^ g_StorageKey;

        return (_Parse(data, skCrypt("GWorld")) != 0);
    }
}

