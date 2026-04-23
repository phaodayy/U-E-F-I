#include "context.hpp"
#include "../../.shared/shared.hpp"
#include "../gui/menu.h"
#include "../overlay/overlay_menu.hpp"
#include "fname.hpp"
#include "offsets.hpp"
#include "memory.hpp"
#include <Windows.h>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <winternl.h>

uint64_t G_UWorld = 0;
uint64_t G_GameInstance = 0;
uint64_t G_PersistentLevel = 0;
uint64_t G_LocalPlayer = 0;
uint64_t G_PlayerController = 0;
uint64_t G_LocalPawn = 0;
Vector3 G_CameraLocation = {0, 0, 0};
Vector3 G_CameraRotation = {0, 0, 0};
std::vector<PlayerData> G_Players;
std::vector<CachedPlayerData> CachedPlayers;
std::mutex g_PlayerMutex;
float G_CamFOV = 103.0f;

static uint32_t g_ProcessId = 0;
static uint64_t g_BaseAddress = 0;

static bool InternalReadMemory(uint64_t src, void *dest, uint64_t size) {
  return ValorantMemory::ReadMemory(src, dest, size);
}

struct DVector3 {
  double x, y, z;
};
struct DQuat {
  double x, y, z, w;
};
struct DFTransform {
  DQuat Rotation;
  DVector3 Translation;
  double _pad;
  DVector3 Scale;
  double _pad2;
};

struct Mat4 {
  float m[4][4];
};

static Mat4 TransformToMatrix(const DFTransform &t) {
  Mat4 r = {};
  r.m[3][0] = (float)t.Translation.x;
  r.m[3][1] = (float)t.Translation.y;
  r.m[3][2] = (float)t.Translation.z;

  double x2 = t.Rotation.x * 2.0, y2 = t.Rotation.y * 2.0,
         z2 = t.Rotation.z * 2.0;
  double xx2 = t.Rotation.x * x2, yy2 = t.Rotation.y * y2,
         zz2 = t.Rotation.z * z2;
  double yz2 = t.Rotation.y * z2, wx2 = t.Rotation.w * x2;
  double xy2 = t.Rotation.x * y2, wz2 = t.Rotation.w * z2;
  double xz2 = t.Rotation.x * z2, wy2 = t.Rotation.w * y2;

  r.m[0][0] = (float)((1.0 - (yy2 + zz2)) * t.Scale.x);
  r.m[1][1] = (float)((1.0 - (xx2 + zz2)) * t.Scale.y);
  r.m[2][2] = (float)((1.0 - (xx2 + yy2)) * t.Scale.z);
  r.m[2][1] = (float)((yz2 - wx2) * t.Scale.z);
  r.m[1][2] = (float)((yz2 + wx2) * t.Scale.y);
  r.m[1][0] = (float)((xy2 - wz2) * t.Scale.y);
  r.m[0][1] = (float)((xy2 + wz2) * t.Scale.x);
  r.m[2][0] = (float)((xz2 + wy2) * t.Scale.z);
  r.m[0][2] = (float)((xz2 - wy2) * t.Scale.x);
  r.m[0][3] = 0;
  r.m[1][3] = 0;
  r.m[2][3] = 0;
  r.m[3][3] = 1;
  return r;
}

static Mat4 MatMul(const Mat4 &a, const Mat4 &b) {
  Mat4 r = {};
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < 4; j++)
      for (int k = 0; k < 4; k++)
        r.m[i][j] += a.m[i][k] * b.m[k][j];
  return r;
}

namespace ValorantContext {
bool ReadMemory(uint64_t src, void *dest, uint64_t size) {
  return InternalReadMemory(src, dest, size);
}

uint64_t GetBaseAddress() { return g_BaseAddress; }

Vector3 GetBoneWorldPos(int boneIndex, uint64_t boneArray, uint64_t meshPtr) {
  DFTransform compToWorld =
      Read<DFTransform>(meshPtr + ValorantOffsets::ComponentToWorld());
  DFTransform bone = Read<DFTransform>(boneArray + (boneIndex * 0x60));
  Mat4 result = MatMul(TransformToMatrix(bone), TransformToMatrix(compToWorld));
  return {result.m[3][0], result.m[3][1], result.m[3][2]};
}

bool MoveMouse(long x, long y, unsigned short flags) {
  return ValorantMemory::MoveMouse(x, y, flags);
}

bool Initialize(uint32_t process_id, uint64_t base_address) {
  g_ProcessId = process_id;
  g_BaseAddress = base_address;

  ValorantMemory::g_ProcessId = process_id;
  if (!ValorantMemory::RefreshProcessContext()) return false;

  uint64_t uworldPtr1 =
      Read<uint64_t>(g_BaseAddress + ValorantOffsets::GWorld());
  G_UWorld = Read<uint64_t>(uworldPtr1);

  if (G_UWorld && G_UWorld > 0x10000) {
    G_GameInstance = Read<uint64_t>(G_UWorld + ValorantOffsets::GameInstance());
    G_PersistentLevel =
        Read<uint64_t>(G_UWorld + ValorantOffsets::PersistentLevel());
    if (G_PersistentLevel && G_PersistentLevel > 0x10000) {
      return true;
    }
  }
  return false;
}


void UpdateGameData() {
  static int logCounter = 0;
  bool shouldLog = (logCounter++ % 100 == 0);

  uint64_t uworldPtr1 = Read<uint64_t>(g_BaseAddress + ValorantOffsets::GWorld());
  uint64_t currentUWorld = Read<uint64_t>(uworldPtr1);

  /*
  if (shouldLog) {
    std::cout << "[DEBUG] GWorld Ptr: " << std::hex << uworldPtr1 << " -> " << currentUWorld << std::dec << std::endl;
  }
  */

  if (!currentUWorld || currentUWorld < 0x10000) {
    // if (shouldLog) std::cout << "[DEBUG] Invalid GWorld" << std::endl;
    G_UWorld = 0;
    G_PersistentLevel = 0;
    G_LocalPlayer = 0;
    G_PlayerController = 0;
    G_LocalPawn = 0;
    return;
  }

  if (currentUWorld != G_UWorld) {
    G_UWorld = currentUWorld;
    G_GameInstance = Read<uint64_t>(G_UWorld + ValorantOffsets::GameInstance());
    G_PersistentLevel =
        Read<uint64_t>(G_UWorld + ValorantOffsets::PersistentLevel());
    /*
    if (shouldLog) {
        std::cout << "[DEBUG] World Changed! GameInstance: " << std::hex << G_GameInstance 
                  << " PersistentLevel: " << G_PersistentLevel << std::dec << std::endl;
    }
    */
    G_LocalPlayer = 0;
    G_PlayerController = 0;
    G_LocalPawn = 0;
  }

  if (!G_PersistentLevel) {
    // if (shouldLog) std::cout << "[DEBUG] No PersistentLevel" << std::endl;
    return;
  }

  std::vector<CachedPlayerData> tempPlayers;

  if (!G_LocalPlayer || !G_PlayerController || !G_LocalPawn) {
    uint64_t localPlayersArray =
        Read<uint64_t>(G_GameInstance + ValorantOffsets::LocalPlayers());
    if (localPlayersArray) {
      G_LocalPlayer = Read<uint64_t>(localPlayersArray);
      if (G_LocalPlayer) {
        G_PlayerController =
            Read<uint64_t>(G_LocalPlayer + ValorantOffsets::PlayerController());
        if (G_PlayerController) {
          G_LocalPawn = Read<uint64_t>(G_PlayerController +
                                       ValorantOffsets::AcknowledgedPawn());
        }
      }
    }
    /*
    if (shouldLog) {
        std::cout << "[DEBUG] LocalPlayer: " << std::hex << G_LocalPlayer 
                  << " PC: " << G_PlayerController << " Pawn: " << G_LocalPawn << std::dec << std::endl;
    }
    */
  } else {
    uint64_t checkPawn =
        Read<uint64_t>(G_PlayerController + ValorantOffsets::AcknowledgedPawn());
    if (checkPawn != G_LocalPawn) {
      G_LocalPawn = checkPawn;
      // if (shouldLog) std::cout << "[DEBUG] LocalPawn updated to: " << std::hex << G_LocalPawn << std::dec << std::endl;
    }
  }

  if (!G_LocalPawn) {
    // if (shouldLog) std::cout << "[DEBUG] No LocalPawn" << std::endl;
    return;
  }

  if (G_PlayerController) {
    uint64_t camMgr = Read<uint64_t>(G_PlayerController +
                                     ValorantOffsets::PlayerCameraManager());
    if (camMgr) {
      struct DV3 {
        double x, y, z;
      };
      struct FMinViewInfo {
        DV3 Location;
        DV3 Rotation;
        float FOV;
      };
      FMinViewInfo vi = Read<FMinViewInfo>(
          camMgr + ValorantOffsets::CameraCachePrivate() + 0x10);
      G_CameraLocation = {(float)vi.Location.x, (float)vi.Location.y,
                          (float)vi.Location.z};
      G_CameraRotation = {(float)vi.Rotation.x, (float)vi.Rotation.y,
                          (float)vi.Rotation.z};
      if (vi.FOV > 0.0f && vi.FOV < 170.0f)
        G_CamFOV = vi.FOV;
    } else {
        // if (shouldLog) std::cout << "[DEBUG] No CameraManager" << std::endl;
    }
  }

  uint64_t localMesh = Read<uint64_t>(G_LocalPawn + ValorantOffsets::Mesh());
  Vector3 localPos = {0, 0, 0};
  if (localMesh) {
    uint64_t localBoneArr =
        Read<uint64_t>(localMesh + ValorantOffsets::BoneArray());
    if (!localBoneArr)
      localBoneArr =
          Read<uint64_t>(localMesh + ValorantOffsets::BoneArrayCache());
    if (localBoneArr)
      localPos = GetBoneWorldPos(0, localBoneArr, localMesh);
    // else if (shouldLog) std::cout << "[DEBUG] Local BoneArray null" << std::endl;
  } /* else if (shouldLog) {
      std::cout << "[DEBUG] Local Mesh null" << std::endl;
  } */

  uint64_t actorArray =
      Read<uint64_t>(G_PersistentLevel + ValorantOffsets::ActorArray());
  int actorCount = Read<int>(G_PersistentLevel + ValorantOffsets::ActorCount());

  /*
  if (shouldLog) {
      std::cout << "[DEBUG] ActorArray: " << std::hex << actorArray << std::dec << " Count: " << actorCount << std::endl;
  }
  */

  if (!actorArray || actorCount <= 0 || actorCount > 10000) {
    // if (shouldLog) std::cout << "[DEBUG] Invalid ActorArray or count" << std::endl;
    return;
  }

  int validActors = 0;
  int actorSkips_null = 0;
  int actorSkips_mesh = 0;
  int actorSkips_damage = 0;
  int actorSkips_team = 0;
  int actorSkips_health = 0;
  int actorSkips_bones = 0;
  int actorSkips_dist = 0;
  int actorSkips_fname = 0;

  for (int i = 0; i < actorCount; i++) {
    uint64_t actor = Read<uint64_t>(actorArray + (i * 8));
    if (!actor) {
        actorSkips_null++;
        continue;
    }
    if (actor == G_LocalPawn)
      continue;

    uint64_t mesh = Read<uint64_t>(actor + ValorantOffsets::Mesh());
    if (!mesh) {
      actorSkips_mesh++;
      continue;
    }

    uint64_t damageHandler =
        Read<uint64_t>(actor + ValorantOffsets::DamageHandler());
    if (!damageHandler) {
      actorSkips_damage++;
      continue;
    }

    if (GUI::esp_team_check) {
      bool wasAlly = Read<bool>(actor + ValorantOffsets::WasAlly());
      if (wasAlly) {
        actorSkips_team++;
        continue;
      }
    }

    float health = Read<float>(damageHandler + ValorantOffsets::Health());
    if (health <= 0.0f || health > 200.0f) {
      actorSkips_health++;
      continue;
    }

    uint64_t boneArray = Read<uint64_t>(mesh + ValorantOffsets::BoneArray());
    if (!boneArray)
      boneArray = Read<uint64_t>(mesh + ValorantOffsets::BoneArrayCache());
    if (!boneArray) {
      actorSkips_bones++;
      continue;
    }

    if (mesh == localMesh)
      continue;

    Vector3 rootPos = GetBoneWorldPos(0, boneArray, mesh);
    Vector3 headPos = GetBoneWorldPos(8, boneArray, mesh);
    float dist = localPos.Distance(rootPos);

    if (dist < 10.0f || dist > 100000.0f) {
      actorSkips_dist++;
      continue;
    }

    int boneCount = Read<int>(mesh + ValorantOffsets::BoneCount());

    CachedPlayerData player;
    player.Position = rootPos;
    player.HeadPosition = headPos;
    player.DistanceToMe = dist / 100.0f;

    static const std::unordered_map<std::string, std::string> agentMap = {
        {"Rift_PC_C", "Astra"},        {"Breach_PC_C", "Breach"},
        {"Sarge_PC_C", "Brimstone"},   {"Deadeye_PC_C", "Chamber"},
        {"Gumshoe_PC_C", "Cypher"},    {"Wushu_PC_C", "Jett"},
        {"Grenadier_PC_C", "Kay/o"},   {"Killjoy_PC_C", "Killjoy"},
        {"Sprinter_PC_C", "Neon"},     {"Wraith_PC_C", "Omen"},
        {"Phoenix_PC_C", "Phoenix"},   {"Clay_PC_C", "Raze"},
        {"Vampire_PC_C", "Reyna"},     {"Thorne_PC_C", "Sage"},
        {"Guide_PC_C", "Skye"},        {"Hunter_PC_C", "Sova"},
        {"Pandemic_PC_C", "Viper"},    {"Stealth_PC_C", "Yoru"},
        {"BountyHunter_PC_C", "Fade"}, {"Mage_PC_C", "Harbor"},
        {"TrainingBot_PC_C", "Bot"}};

    {
      uint64_t clazz = Read<uint64_t>(actor + 0x10);
      std::string agentName;
      bool isBot = false;
      bool fnameResolved = false;
      bool shouldSkip = false;
      if (clazz) {
        int32_t classFName = Read<int32_t>(clazz + 0x18);
        if (classFName > 0 && classFName < 0x2000000) { // Increased limit for FName
          std::string raw = FNameUtils::GetNameFast(classFName);

          if (!raw.empty()) {
            fnameResolved = true;
            size_t slash = raw.rfind('/');
            if (slash != std::string::npos)
              raw = raw.substr(slash + 1);
            if (raw.find("Default__") == 0)
              raw = raw.substr(9);

            auto it = agentMap.find(raw);
            if (it != agentMap.end()) {
              agentName = it->second;
            } else if (raw.find("Bot") != std::string::npos ||
                       raw.find("Target") != std::string::npos ||
                       raw.find("Dummy") != std::string::npos ||
                       raw.find("Training") != std::string::npos) {
              isBot = true;
            } else if (raw.find("_PC_C") != std::string::npos) {
            } else {
              shouldSkip = true;
            }
          }
        }
      }
      if (shouldSkip) {
        actorSkips_fname++;
        continue;
      }
      if (!fnameResolved) {
        if (boneCount != 101 && boneCount != 103 && boneCount != 104) {
          actorSkips_fname++;
          continue;
        }
      }
      if (isBot)
        player.Name = "Bot";
      else if (!agentName.empty() && agentName.size() >= 2 &&
               agentName.size() <= 20)
        player.Name = agentName;
      else
        player.Name = "Player";
    }

    validActors++;

    player.Health = health;
    player.MaxHealth = 100.0f;
    player.IsVisible = true;
    player.IsTeammate = false;
    player.TeamID = 0;
    player.BoneCount = boneCount;
    player.MeshAddr = mesh;
    player.BoneArrayAddr = boneArray;

    player.bIsAlive = Read<bool>(damageHandler + ValorantOffsets::bAlive());
    player.bIsBot = (player.Name == "Bot");

    float lastSubmit = Read<float>(mesh + ValorantOffsets::LastSubmitTime());
    float lastRender =
        Read<float>(mesh + ValorantOffsets::LastSubmitTimeOnScreen());
    float visionTick = 0.06f; 
    if (isnan(lastSubmit) || isnan(lastRender) || isinf(lastSubmit) ||
        isinf(lastRender)) {
      player.IsVisible = true;
    } else {
      player.IsVisible = (lastRender + visionTick >= lastSubmit);
    }

    static const std::unordered_map<std::string, std::string> weaponMap = {
        {"Ability_Melee_Base_C", "Knife"},
        {"BasePistol_C", "Classic"},
        {"SawedOffShotgun_C", "Shorty"},
        {"AutomaticPistol_C", "Frenzy"},
        {"LugerPistol_C", "Ghost"},
        {"RevolverPistol_C", "Sheriff"},
        {"Vector_C", "Stinger"},
        {"SubMachineGun_MP5_C", "Spectre"},
        {"AutomaticShotgun_C", "Judge"},
        {"PumpShotgun_C", "Bucky"},
        {"AssaultRifle_Burst_C", "Bulldog"},
        {"DMR_C", "Guardian"},
        {"AssaultRifle_ACR_C", "Phantom"},
        {"AssaultRifle_AK_C", "Vandal"},
        {"LeverSniperRifle_C", "Marshal"},
        {"BoltSniper_C", "Operator"},
        {"LightMachineGun_C", "Ares"},
        {"HeavyMachineGun_C", "Odin"}};

    uint64_t inventory = Read<uint64_t>(actor + ValorantOffsets::Inventory());
    if (inventory) {
      uint64_t equippable =
          Read<uint64_t>(inventory + ValorantOffsets::CurrentEquippable());
      if (equippable) {
        uint64_t wepClass = Read<uint64_t>(equippable + 0x10);
        if (wepClass) {
          int32_t classFName = Read<int32_t>(wepClass + 0x18);
          if (classFName > 0 && classFName < 0x200000) {
            std::string raw = FNameUtils::GetNameFast(classFName);
            if (!raw.empty()) {
              size_t slash = raw.rfind('/');
              if (slash != std::string::npos)
                raw = raw.substr(slash + 1);
              if (raw.find("Default__") == 0)
                raw = raw.substr(9);

              auto it = weaponMap.find(raw);
              if (it != weaponMap.end()) {
                player.WeaponName = it->second;
              } else {
                size_t posC = raw.rfind("_C");
                if (posC != std::string::npos && posC > 0) {
                  size_t prev = raw.rfind('_', posC - 1);
                  if (prev != std::string::npos && posC - prev > 1)
                    player.WeaponName = raw.substr(prev + 1, posC - prev - 1);
                }
              }
            }
          }
        }
        if (player.WeaponName.empty())
          player.WeaponName = "Weapon";
      }
    }

    player.BoxMin = {rootPos.x - 40, rootPos.y - 40, rootPos.z};
    player.BoxMax = {rootPos.x + 40, rootPos.y + 40, headPos.z + 20};

    player.Bone_Head = headPos;
    if (boneCount == 103) {
      player.Bone_Neck = GetBoneWorldPos(9, boneArray, mesh);
      player.Bone_LShoulder = GetBoneWorldPos(33, boneArray, mesh);
      player.Bone_LElbow = GetBoneWorldPos(30, boneArray, mesh);
      player.Bone_LHand = GetBoneWorldPos(32, boneArray, mesh);
      player.Bone_RShoulder = GetBoneWorldPos(58, boneArray, mesh);
      player.Bone_RElbow = GetBoneWorldPos(55, boneArray, mesh);
      player.Bone_RHand = GetBoneWorldPos(57, boneArray, mesh);
      player.Bone_LThigh = GetBoneWorldPos(63, boneArray, mesh);
      player.Bone_LKnee = GetBoneWorldPos(65, boneArray, mesh);
      player.Bone_LFoot = GetBoneWorldPos(69, boneArray, mesh);
      player.Bone_RThigh = GetBoneWorldPos(77, boneArray, mesh);
      player.Bone_RKnee = GetBoneWorldPos(79, boneArray, mesh);
      player.Bone_RFoot = GetBoneWorldPos(83, boneArray, mesh);
    } else if (boneCount == 104) {
      player.Bone_Neck = GetBoneWorldPos(21, boneArray, mesh);
      player.Bone_LShoulder = GetBoneWorldPos(23, boneArray, mesh);
      player.Bone_LElbow = GetBoneWorldPos(24, boneArray, mesh);
      player.Bone_LHand = GetBoneWorldPos(25, boneArray, mesh);
      player.Bone_RShoulder = GetBoneWorldPos(49, boneArray, mesh);
      player.Bone_RElbow = GetBoneWorldPos(50, boneArray, mesh);
      player.Bone_RHand = GetBoneWorldPos(51, boneArray, mesh);
      player.Bone_LThigh = GetBoneWorldPos(77, boneArray, mesh);
      player.Bone_LKnee = GetBoneWorldPos(78, boneArray, mesh);
      player.Bone_LFoot = GetBoneWorldPos(80, boneArray, mesh);
      player.Bone_RThigh = GetBoneWorldPos(84, boneArray, mesh);
      player.Bone_RKnee = GetBoneWorldPos(85, boneArray, mesh);
      player.Bone_RFoot = GetBoneWorldPos(87, boneArray, mesh);
    } else if (boneCount == 101) {
      player.Bone_Neck = GetBoneWorldPos(21, boneArray, mesh);
      player.Bone_LShoulder = GetBoneWorldPos(23, boneArray, mesh);
      player.Bone_LElbow = GetBoneWorldPos(24, boneArray, mesh);
      player.Bone_LHand = GetBoneWorldPos(25, boneArray, mesh);
      player.Bone_RShoulder = GetBoneWorldPos(49, boneArray, mesh);
      player.Bone_RElbow = GetBoneWorldPos(50, boneArray, mesh);
      player.Bone_RHand = GetBoneWorldPos(51, boneArray, mesh);
      player.Bone_LThigh = GetBoneWorldPos(75, boneArray, mesh);
      player.Bone_LKnee = GetBoneWorldPos(76, boneArray, mesh);
      player.Bone_LFoot = GetBoneWorldPos(78, boneArray, mesh);
      player.Bone_RThigh = GetBoneWorldPos(82, boneArray, mesh);
      player.Bone_RKnee = GetBoneWorldPos(83, boneArray, mesh);
      player.Bone_RFoot = GetBoneWorldPos(85, boneArray, mesh);
    } else {
      player.Bone_Neck = headPos;
    }
    player.Bone_Chest = GetBoneWorldPos(6, boneArray, mesh);
    player.Bone_Pelvis = GetBoneWorldPos(3, boneArray, mesh);

    tempPlayers.push_back(player);
  }

  /*
  if (shouldLog) {
      std::cout << "[DEBUG] Valid Players: " << validActors 
                << " (Skips: null:" << actorSkips_null 
                << " mesh:" << actorSkips_mesh 
                << " dmg:" << actorSkips_damage 
                << " team:" << actorSkips_team 
                << " hp:" << actorSkips_health 
                << " bones:" << actorSkips_bones 
                << " dist:" << actorSkips_dist 
                << " fname:" << actorSkips_fname 
                << ")" << std::endl;
  }
  */

  CachedPlayers = std::move(tempPlayers);
}
} 
