#pragma once
#include <cmath>
#include <mutex>
#include <string>
#include <vector>


struct Vector3 {
  float x, y, z;

  float Distance(const Vector3 &other) const {
    float dx = other.x - x;
    float dy = other.y - y;
    float dz = other.z - z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
  }

  Vector3 operator-(const Vector3 &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }

  Vector3 operator+(const Vector3 &other) const {
    return {x + other.x, y + other.y, z + other.z};
  }

  Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }

  static Vector3 Lerp(const Vector3 &a, const Vector3 &b, float t) {
    return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t};
  }

  float Length() const { return std::sqrt(x * x + y * y + z * z); }
};

struct FQuat {
  float x, y, z, w;
};

struct FRotator {
  float Pitch;
  float Yaw;
  float Roll;
};

struct FString {
    uint64_t Data;
    int Count;
    int Max;
};

struct FTransform {
  FQuat Rotation;
  Vector3 Translation;
  char pad[4];
  Vector3 Scale3D;
  char pad2[4];

  Vector3 ToWorld(const Vector3 &local) const {
    Vector3 scaled = {local.x * Scale3D.x, local.y * Scale3D.y,
                      local.z * Scale3D.z};

    float x2 = Rotation.x * 2.0f, y2 = Rotation.y * 2.0f,
          z2 = Rotation.z * 2.0f;
    float xx2 = Rotation.x * x2, yy2 = Rotation.y * y2, zz2 = Rotation.z * z2;
    float yz2 = Rotation.y * z2, wx2 = Rotation.w * x2;
    float xy2 = Rotation.x * y2, wz2 = Rotation.w * z2;
    float xz2 = Rotation.x * z2, wy2 = Rotation.w * y2;

    Vector3 rotated;
    rotated.x = (1.0f - (yy2 + zz2)) * scaled.x + (xy2 - wz2) * scaled.y +
                (xz2 + wy2) * scaled.z;
    rotated.y = (xy2 + wz2) * scaled.x + (1.0f - (xx2 + zz2)) * scaled.y +
                (yz2 - wx2) * scaled.z;
    rotated.z = (xz2 - wy2) * scaled.x + (yz2 + wx2) * scaled.y +
                (1.0f - (xx2 + yy2)) * scaled.z;

    return {rotated.x + Translation.x, rotated.y + Translation.y,
            rotated.z + Translation.z};
  }
};

struct PlayerData {
  uint64_t ActorPtr = 0;
  uint32_t ObjID = 0;
  std::string Name = "";
  int TeamID = 0;
  bool IsTeammate = false;

  Vector3 Position = {0, 0, 0};
  Vector3 HeadPosition = {0, 0, 0};
  float Health = 100.0f;
  float MaxHealth = 100.0f;
  float Distance = 0.0f;

  uint64_t Mesh = 0;
  uint64_t RootComponent = 0;
  bool IsVisible = true;
};

extern uint64_t G_UWorld;
extern uint64_t G_GameInstance;
extern uint64_t G_PersistentLevel;
extern uint64_t G_LocalPlayer;
extern uint64_t G_PlayerController;
extern uint64_t G_LocalPawn;
extern Vector3 G_CameraLocation;
extern Vector3 G_CameraRotation;
extern std::vector<PlayerData> G_Players;

inline void GetAxes(const Vector3 &rotation, Vector3 &x, Vector3 &y,
                    Vector3 &z) {
  const float PI = 3.14159265358979323846f;
  float pitch = rotation.x * (PI / 180.0f);
  float yaw = rotation.y * (PI / 180.0f);
  float roll = rotation.z * (PI / 180.0f);

  float cp = cosf(pitch), sp = sinf(pitch);
  float cy = cosf(yaw), sy = sinf(yaw);
  float cr = cosf(roll), sr = sinf(roll);

  x = {cp * cy, cp * sy, sp};
  y = {sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp};
  z = {-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp};
}

struct Vector2 {
  float x, y;
};

struct SkeletonLine {
  Vector3 Start;
  Vector3 End;
};

struct CachedPlayerData {
  std::string Name = "";
  Vector3 Position = {0, 0, 0};
  Vector3 HeadPosition = {0, 0, 0};
  Vector3 BoxMin = {0, 0, 0};
  Vector3 BoxMax = {0, 0, 0};
  float Health = 100.0f;
  float MaxHealth = 100.0f;
  float DistanceToMe = 0.0f;
  bool IsVisible = true;
  bool IsTeammate = false;
  int TeamID = 0;
  int BoneCount = 0;
  std::string WeaponName = "";
  bool bIsBot = false;
  bool bIsAlive = true;

  uint64_t MeshAddr = 0;
  uint64_t BoneArrayAddr = 0;

  Vector3 Bone_Head = {0, 0, 0};
  Vector3 Bone_Neck = {0, 0, 0};
  Vector3 Bone_Chest = {0, 0, 0};
  Vector3 Bone_Pelvis = {0, 0, 0};
  Vector3 Bone_LShoulder = {0, 0, 0};
  Vector3 Bone_LElbow = {0, 0, 0};
  Vector3 Bone_LHand = {0, 0, 0};
  Vector3 Bone_RShoulder = {0, 0, 0};
  Vector3 Bone_RElbow = {0, 0, 0};
  Vector3 Bone_RHand = {0, 0, 0};
  Vector3 Bone_LThigh = {0, 0, 0};
  Vector3 Bone_LKnee = {0, 0, 0};
  Vector3 Bone_LFoot = {0, 0, 0};
  Vector3 Bone_RThigh = {0, 0, 0};
  Vector3 Bone_RKnee = {0, 0, 0};
  Vector3 Bone_RFoot = {0, 0, 0};
};

extern std::vector<CachedPlayerData> CachedPlayers;
extern std::mutex g_PlayerMutex;
extern float G_CamFOV;

namespace VDataContext {
bool Initialize(uint32_t process_id, uint64_t base_address);
void UpdateGameData();
bool ReadMemory(uint64_t src, void *dest, uint64_t size);
uint64_t GetBaseAddress();
Vector3 GetBoneWorldPos(int bone_index, uint64_t bone_array, uint64_t mesh);

bool MoveMouse(long x, long y, unsigned short flags = 0);

template <typename T> T Read(uint64_t address) {
  T buffer = {};
  ReadMemory(address, &buffer, sizeof(T));
  return buffer;
}
} 
