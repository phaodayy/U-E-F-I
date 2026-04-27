#pragma once
#include <cmath>
#include <string>
#include <vector>
#include <cstdint>
#include "math.hpp"
#include "memory.hpp"
#include "bones.hpp"
#include "health.hpp"

enum class PlayerGender : uint8_t {
    Male = 0,
    Female = 1
};

struct PlayerData {
    std::string Name = "";
    Vector3 Position = { 0, 0, 0 };
    Vector3 Velocity = { 0, 0, 0 };
    Vector3 HeadPosition = { 0, 0, 0 };
    Vector3 FeetPosition = { 0, 0, 0 };
    float Health = 0;
    float GroggyHealth = 0;
    float Distance = 0;
    int TeamID = 0;
    int Kills = 0;
    int SurvivalLevel = 0;
    int SpectatedCount = 0;
    std::string WeaponName = "";
    bool IsGroggy = false;
    bool IsVisible = false;
    bool IsTeammate = false;
    PlayerGender Gender = PlayerGender::Male;
    uint64_t ActorPtr = 0;
    uintptr_t MeshAddr = 0;
    uintptr_t BoneArrayAddr = 0;

    Vector3 Bone_Head, Bone_Neck, Bone_Chest, Bone_Pelvis;
    Vector3 Bone_LShoulder, Bone_LElbow, Bone_LHand;
    Vector3 Bone_RShoulder, Bone_RElbow, Bone_RHand;
    Vector3 Bone_LThigh, Bone_LKnee, Bone_LFoot;
    Vector3 Bone_RThigh, Bone_RKnee, Bone_RFoot;
};

extern uint64_t G_UWorld, G_GameInstance, G_PersistentLevel, G_LocalPlayer, G_PlayerController, G_LocalPawn, G_LocalHUD;
extern bool G_IsMenuOpen;
extern Vector3 G_CameraLocation, G_CameraRotation, G_LocalPlayerPos, G_LocalPlayerVelocity;
extern uint64_t G_LastScanTime;
struct RadarData {
    bool IsMiniMapVisible = false;
    bool IsWorldMapVisible = false;
    int MiniMapSizeIndex = 0;
    float ScreenPosX = 0;
    float ScreenPosY = 0;
    float ScreenSize = 0;
    float ScreenSizeY = 0;
    float AlignmentX = 0.5f;
    float AlignmentY = 0.5f;
    float CurrentMinimapViewScale = 1.0f;
    int SelectMinimapSizeIndex = 0;
    float CurrentScale = 1.0f;
    Vector2 WorldMapPosition = { 0, 0 };
    float WorldMapZoomFactor = 0.0f;
    float VehicleSpeed = 0.0f;
    Vector3 WorldOriginLocation = { 0, 0, 0 };
    Vector3 WorldCenterLocation = { 0, 0, 0 };
    float MapSizeFactored = 1.0f;
    float MapWorldSize = 0.0f;
    float WorldMapX = 0.0f;
    float WorldMapY = 0.0f;
    float WorldMapWidth = 0.0f;
    float WorldMapHeight = 0.0f;
    float ZoomScale = 1.0f;
};

extern RadarData G_Radar;
struct ItemData {
    Vector3 Position;
    std::string Name;
    float Distance;
    bool IsImportant;
};

#include <mutex>

extern std::vector<PlayerData> G_Players;
extern std::mutex G_PlayersMutex; // Mutex for G_Players
extern int G_LocalSpectatedCount;
extern std::vector<ItemData> CachedItems;
extern std::mutex CachedItemsMutex; // Mutex for CachedItems
struct DebugActorData {
    uint64_t Address;
    std::string ClassName;
    Vector3 Position;
    float Distance;
};

extern std::vector<DebugActorData> G_DebugActors;
extern std::mutex G_DebugActorsMutex;
extern float G_CamFOV;

namespace telemetryContext {
    bool Initialize(uint32_t process_id, uint64_t base_address);
    void UpdateGameData();
    void UpdateCamera();
    bool ReadMemory(uint64_t src, void* dest, uint64_t size);
    template <typename T>
    inline T Read(uint64_t address) {
        T buffer = {};
        ReadMemory(address, &buffer, sizeof(T));
        return buffer;
    }
    uint64_t GetBaseAddress();
    bool WorldToScreen(Vector3 world_pos, Vector2& screen_pos);
    void SyncLivePlayers(std::vector<PlayerData>& players);
}
