#pragma once

#include <windows.h>
#include <Utils/ue4math/ue4math.h>
#include <Utils/ue4math/quat.h>
#include <Utils/ue4math/vector.h>
#include <Utils/ue4math/matrix.h>
#include <Utils/ue4math/transform.h>
#include <Common/math.h>
#include <Common/VectorHelper.h>
#include <imgui/imgui.h>
#include <iostream>
#include <unordered_map>
#include <shared_mutex>
#include <array>
#include <cmath>
#include <Common/Constant.h>
#include <Common/Entitys.h>
#include <Common/Bone.h>
#include <DMALibrary/Memory/Memory.h>
#include <nlohmann/json.hpp>
#include <Utils/Engine.h>
#include <Utils/ue4math/rotator.h>
#include <deque>
#include <Utils/FNVHash.h>
#include <d3d11.h>
#include <unordered_set>
#include <chrono>
#include <string>

struct ScopeTimer {
    std::string Name;
    std::chrono::high_resolution_clock::time_point Start;

    ScopeTimer(std::string name) : Name(name) {
        Start = std::chrono::high_resolution_clock::now();
    }
    
    ~ScopeTimer() {
        auto End = std::chrono::high_resolution_clock::now();
        auto Duration = std::chrono::duration_cast<std::chrono::milliseconds>(End - Start).count();
        
        // // Cảnh báo nếu cục này tốn hơn 15ms
        // if (Duration > 15) {
        //     printf("[LAG SPIKE] %s ton mat %lld ms!\n", Name.c_str(), Duration);
        // }
    }
};

inline bool g_isMortars = false;
inline float g_MortarsDis = 0.f;
inline float g_MortarsDis2 = 0.f;
inline FVector2D g_MortarsScreen;

inline int currentScale = 700;  // 当前刻度值
inline int targetScale2 = 700;   // 目标刻度值
inline int zuixiaoScale = 121;   // 目标刻度值
inline bool adjusting = false;  // 是否正在调整

// 前向声明场景类型
namespace Physics {
	template<typename T, typename Hash>
	class VisibleScene;
}

enum class PxGeometryType : int32_t
{

	eSPHERE,	   // 0
	ePLANE,		   // 1
	eCAPSULE,	   // 2
	eBOX,		   // 3
	eCONVEXMESH,   // 4
	eTRIANGLEMESH, // 5
	eHEIGHTFIELD,  // 6

	eGEOMETRY_COUNT, //!< internal use only!
	eINVALID = -1	 //!< internal use only!
};

struct FIntVector2D {
	int X, Y;
};

struct FilterDataT
{
	uint32_t word0;
	uint32_t word1;
	uint32_t word2;
	uint32_t word3;
};

struct PrunerPayload
{
	uintptr_t Shape;
	uintptr_t Actor;

	bool operator==(const PrunerPayload& other) const {
		return Shape == other.Shape && Actor == other.Actor;
	}

	bool operator<(const PrunerPayload& other) const {
		if (Shape != other.Shape)
			return Shape < other.Shape;
		return Actor < other.Actor;
	}
};

struct PrunerPayloadHash {
	size_t operator()(const PrunerPayload& p) const {
		return std::hash<uintptr_t>()(p.Shape) ^ (std::hash<uintptr_t>()(p.Actor) << 1);
	}
};


struct Int64Hash {
	size_t operator()(const uint64_t& p) const {
		return std::hash<uint64_t>()(p);
	}
};

struct PxTransformT
{

	Vector4 mRotation{};
	Vector3 mPosition{};

	PxTransformT() = default;

	PxTransformT(const Vector4& rotation, const Vector3& position)
		: mRotation(rotation), mPosition(position)
	{
	}

	// Transform a vector using the transform's rotation and translation
	Vector3 transform(const Vector3& input) const
	{
		return mRotation.rotate(input) + mPosition;
	}

	PxTransformT operator*(const PxTransformT& other) const
	{
		Vector4 newRotation = mRotation * other.mRotation;
		Vector3 newPosition = mRotation.rotate(other.mPosition) + mPosition;
		return PxTransformT(newRotation, newPosition);
	}

	bool isNearlyZero(float value, float epsilon = 1e-6f)
	{
		return fabs(value) < epsilon;
	}

	PxTransformT getInverse() const
	{
		Vector4 invRotation = mRotation.conjugate();
		Vector3 invPosition = mRotation.rotateInv(-mPosition);
		return PxTransformT(invRotation, invPosition);
	}

	bool IsNearlyEqual(const PxTransformT& other, float tolerance = 0.1f) const {
		return mRotation.IsNearlyEqual(other.mRotation, tolerance) &&
			mPosition.IsNearlyEqual(other.mPosition, tolerance);
	}


};



namespace font
{
	extern ImFont* calibri_bold;
	extern ImFont* calibri_bold_hint;
	extern ImFont* myth_bold;
	extern ImFont* myth_bold_b;
	extern ImFont* calibri_regular;
	extern ImFont* icomoon_default;
	extern ImFont* icomoon_menu;
	extern ImFont* pixel_7_small;
	extern ImFont* weapon_val;
	extern ImFont* icomoon;
	extern ImFont* weapon_telemetry ;
}

namespace texture
{
	extern ID3D11ShaderResourceView* background;
	extern ID3D11ShaderResourceView* logo;
	extern ID3D11ShaderResourceView* playermoder;
	extern ID3D11ShaderResourceView* weapon_image;
	extern ID3D11ShaderResourceView* rank;
	extern ID3D11ShaderResourceView* bq;
	extern ID3D11ShaderResourceView* sj;
	extern ID3D11ShaderResourceView* lj;
	extern ID3D11ShaderResourceView* jq;
	extern ID3D11ShaderResourceView* sdq;
	extern ID3D11ShaderResourceView* sq;
	extern ID3D11ShaderResourceView* cfq;
	extern ID3D11ShaderResourceView* pj;
	extern ID3D11ShaderResourceView* yp;
	extern ID3D11ShaderResourceView* fj;
	extern ID3D11ShaderResourceView* zd;
	extern ID3D11ShaderResourceView* tzw;
	extern ID3D11ShaderResourceView* ys;
	extern ID3D11ShaderResourceView* item;

}

struct Wap
{
	ImTextureID precision_calibration_id = 0;
	ImTextureID player_id = 0;
	ImTextureID AK47_id = 0;
	ImTextureID AUG_id = 0;
	ImTextureID AWM_id = 0;
	ImTextureID Berreta686_id = 0;
	ImTextureID BerylM762_id = 0;
	ImTextureID BizonPP19_id = 0;
	ImTextureID Crossbow_id = 0;
	ImTextureID DP12_id = 0;
	ImTextureID DP28_id = 0;
	ImTextureID FNFal_id = 0;
	ImTextureID G36C_id = 0;
	ImTextureID Groza_id = 0;
	ImTextureID HK416_id = 0;
	ImTextureID K2_id = 0;
	ImTextureID Kar98k_id = 0;
	ImTextureID L6_id = 0;
	ImTextureID M16A4_id = 0;
	ImTextureID M24_id = 0;
	ImTextureID M249_id = 0;
	ImTextureID MG3_id = 0;
	ImTextureID Mini14_id = 0;
	ImTextureID Mk12_id = 0;
	ImTextureID Mk14_id = 0;
	ImTextureID Mk47Mutant_id = 0;
	ImTextureID Mosin_id = 0;
	ImTextureID MP5K_id = 0;
	ImTextureID P90_id = 0;
	ImTextureID QBU88_id = 0;
	ImTextureID QBZ95_id = 0;
	ImTextureID Saiga12_id = 0;
	ImTextureID SCAR_L_id = 0;
	ImTextureID SKS_id = 0;
	ImTextureID Thompson_id = 0;
	ImTextureID UMP_id = 0;
	ImTextureID UZI_id = 0;
	ImTextureID Vector_id = 0;
	ImTextureID VSS_id = 0;
	ImTextureID Win1894_id = 0;
	ImTextureID Winchester_id = 0;
	ImTextureID DesertEagle_id = 0;
	ImTextureID FlareGun_id = 0;
	ImTextureID G18_id = 0;
	ImTextureID M9_id = 0;
	ImTextureID M1911_id = 0;
	ImTextureID NagantM1895_id = 0;
	ImTextureID Rhino_id = 0;
	ImTextureID Sawnoff_id = 0;
	ImTextureID vz61Skorpion_id = 0;
};
extern Wap deWap;


struct TriangleMeshData
{
	std::vector<Vector3> Vertices{};
	std::vector<uint32_t> Indices{};
	uint8_t Flags{};
	FilterDataT QueryFilterData{};
	FilterDataT SimulationFilterData{};
	PrunerPayload UniqueKey1;
	uint64_t UniqueKey2;
	PxGeometryType Type{};
	PxTransformT Transform;
};



struct FRichCurveKeyArray
{
	uintptr_t Data;
	int32_t Count;
};

class USkeletalMeshSocket
{
public:
	uint64_t pSocket;
	struct FName SocketName; // 0x28(0x08)
	struct FName BoneName; // 0x30(0x08)
	struct FVector RelativeLocation; // 0x38(0x0c)
	struct FRotator RelativeRotation; // 0x44(0x0c)
	struct FVector RelativeScale; // 0x50(0x0c)
};

class WeaponData
{
public:
	uint64_t SkeletalMesh;
	uint64_t Skeleton;
	TArray<uint64_t> SkeletalSockets;
	std::vector<USkeletalMeshSocket> SkeletalMeshSockets;
	uint64_t FiringAttachPointSocketBone;

	FRichCurve FloatCurves;
	std::vector<FRichCurveKey> RichCurveKeys;
	char TrajectoryConfig[0x60];
	bool IsInitialized;
	float TrajectoryGravityZ = -9.8f;
	uint64_t Mesh3P;
	FName FiringAttachPoint;
	FName ScopingAttachPoint;
	FRichCurveKeyArray RichCurveKeyArray;
	FTransform ComponentToWorld;
	uint64_t StaticMesh;
	FWeaponTrajectoryConfig TrajectoryConfigs;

	bool ScopeSocket = false;
	uint64_t ScopeAimCameraSocket;
	uint64_t ScopeStaticMeshComponent;

	template<typename T>
	T GetTrajectoryConfig(uint32_t Offset)
	{
		return *(T*)(this->TrajectoryConfig + Offset);
	}
};

struct GNameInfo {
	int ID;
	std::string Name;
	uint64_t pGName;
	uint64_t pName;
	FText FName;
};

struct GTakeHitInfo {
	float ActualDamage = 0.0f;
	struct FName AttackerWeaponName {};
	struct FVector AttackerLocation {};
};

struct ActorEntityInfo {
	uint64_t Entity;
	int Index;
	int ID;
	int DecodeID;
	EntityInfo EntityInfo;
};

inline static const std::vector<int> BoneIndex = {
		EBoneIndex::ForeHead,
		EBoneIndex::Head,
		EBoneIndex::Neck_01,
		EBoneIndex::Spine_03,
		EBoneIndex::Pelvis,
		EBoneIndex::Upperarm_L,
		EBoneIndex::Lowerarm_L,
		EBoneIndex::Upperarm_R,
		EBoneIndex::Lowerarm_R,
		EBoneIndex::Hand_L,
		EBoneIndex::Hand_R,
		EBoneIndex::Thigh_L,
		EBoneIndex::Calf_L,
		EBoneIndex::Thigh_R,
		EBoneIndex::Calf_R,
		EBoneIndex::Foot_L,
		EBoneIndex::Foot_R,
};

struct VehicleWheelInfo {
	uint64_t Vehicle = 0;
	uint64_t pWheel = 0;
	uint64_t Wheel = 0;
	uint64_t Mesh = 0;
	FVector Location{};
	FVector2D ScreenLocation{};
	float Health = 0.0f;
	float DampingRate = 0.0f;
	float ShapeRadius = 0.0f;
	FVector OldLocation{};
	FVector Velocity{};
	WheelState State{};
	float Distance = 0.0f;
	FTransform ComponentToWorld{};
};

struct VehicleInfo {
	EntityType Type;
	uint64_t Entity;
	int ObjID;
	std::string Name;
	std::string ClassName;
	uint64_t RootComponent;
	uint64_t VehicleMovement;
	uint64_t VehicleCommonComponent;
	uint64_t pWheels;
	int WheelsCount;
	int FlatTireCount;
	std::vector<VehicleWheelInfo> Wheels;
	FVector Location;
	FVector2D ScreenLocation;
	float Distance;

	float VehicleFuel;
	float VehicleFuelMax;
	float VehicleHealth;
	float VehicleHealthMax;
};

struct DroppedItemInfo {
	EntityType Type;
	uint64_t Entity;
	int ID;
	uint64_t pDroppedItem;
	uint64_t RootComponent;
	uint64_t ItemTable;
	FVector Location;
	FVector ScreenLocation;
	float Distance;
};

struct ItemInfo {
	EntityType Type;
	WeaponType ItemType;
	uint64_t Entity;
	uint64_t IsEntity;
	uint64_t RootComponent;
	int ID;
	std::string Name;
	std::string DisplayName;
	uint64_t ItemTable;
	FVector Location;
	FVector2D ScreenLocation;
	float Distance;
	bool bHidden = false;
	bool ShowRay = false;
};

struct DroppedItemGroupItemInfo {
	uint64_t pItemGroupComponent;
	uint64_t ItemGroupComponent;
	uint64_t ItemGroupUItem;
	uint64_t ItemTable;
	int ItemID;
};

struct DroppedItemGroupInfo {
	EntityType Type;
	uint64_t Entity;
	int ID;
	uint64_t pDroppedItemGroup;
	int Count;
	std::vector<DroppedItemGroupItemInfo> Items;
};

struct ProjectInfo {
	EntityType Type;
	uint64_t Entity;
	uint64_t pEntity;
	float Time;
	int ID;
	std::string Name;
	std::string EntityName;
	uint64_t RootComponent;
	uint64_t MeshComponent;
	BYTE bVisible = 0;
	FVector Location;
	FVector2D ScreenLocation;
	float Distance;
	float TimeTillExplosion;
	float PostImpactExplosionTimer;
	FVector LastImpactVelocity;
	EProjectileExplodeState ExplodeState;
};

struct PlayerSkeleton {
	std::array<FTransform, 256> Bones{};
	std::array<FVector, 256> LocationBones{};
	std::array<bool, 256> VisibleBones{};
	std::array<FVector2D, 256> ScreenBones{};
};

struct Player
{
	EntityType Type = EntityType::Unknown;
	int ObjID = 0;
	int Sex = 0;
	uint64_t Entity = 0;
	bool IsMe = false;
	bool IsMyTeam = false;
	bool InScreen = true;
	bool InFog = false;
	uint64_t RootComponent = 0;
	uint64_t CharacterMovement = 0;
	uint64_t MeshComponent = 0;
	uint64_t AnimScriptInstance = 0;
	uint64_t StaticMesh = 0;
	BYTE bVisible = 1;
	UCHAR bAlwaysCreatePhysicsState = 0x1;
	uint64_t BoneMatrix = 0;
	uint64_t BoneArray = 0;
	FTransform ComponentToWorld{};
	uint64_t SquadComponent = 0;
	uint64_t AimComponent = 0;
	uint64_t RecoilManager = 0;
	FVector Location{};
	CharacterState State{};
	float Health = 0.0f;
	float Distance = 0.0f;
	float GroggyHealth = 0.0f;
	int KillCount = 0;
	int SurvivalTier = 0;
	int SurvivalLevel = 0;
	int Alignment = 0;
	EGender Gender{};
	BYTE bEncryptedHealth = 0;
	unsigned char EncryptedHealthOffset = 0;
	unsigned char DecryptedHealthOffset = 0;
	EPartnerLevel PartnerLevel{};
	float DamageDealtOnEnemy = 0.0f;
	int SpectatedCount = 0;
	int ListType = 0;
	int SquadMemberIndex = 0;
	std::string ClanName = "";
	GTakeHitInfo TakeHitInfo{};
	ECharacterIconType RadarState{};
	float LastSubmitTime = 0.0f;
	float LastRenderTime = 0.0f;
	float LastRenderTimeOnScreen = 0.0f;
	float LastUpdateTime = 0.0f;
	uint64_t LastBoneUpdateTick = 0;
	uint64_t LastUpdateTick = 0;
	bool IsVisible = false;
	bool IsAimMe = false;
	int TeamID = -100;
	bool IsMortar = false;
	int BoneCount = 0;
	uint64_t VehicleRiderComponent = 0;
	int SeatIndex = -1;
	uint64_t LastVehiclePawn = 0;
	FVector Velocity{};
	FVector ComponentVelocity{};
	FRotator AimOffsets{};
	uint64_t pCharacterName = 0;
	ECharacterState CharacterState{};
	FText CharacterName{};
	uint64_t PlayerState = 0;
	uint64_t WeaponProcessor = 0;
	uint64_t EquippedWeapons = 0;
	BYTE CurrentWeaponIndex = 0;
	uint64_t CurrentWeapon = 0;
	uint64_t EncryptWeaponID = 0;
	int WeaponID = 0;
	float ElapsedCookingTime = 0.0f;
	bool IsScoping = false;
	std::string WeaponName = "";
	EntityInfo WeaponEntityInfo{};
	EAnimPawnState PreEvalPawnState{};
	EWeaponClass WeaponClass{};
	BYTE WeaponClassByte = 0;
	float LeanLeftAlpha_CP = 0.0f;
	float LeanRightAlpha_CP = 0.0f;
	std::string AccountId = "";
	std::string Name = "";

	// [FIX PCIE READ]: Cache Mortar cho luong Render
	uint64_t MortarEntity = 0;
	FRotator MortarRotation = {0.0f, 0.0f, 0.0f};

	// [FIX PCIE READ]: Centralized Aim/Recoil data
	FRotator RecoilADSRotation = {0.0f, 0.0f, 0.0f};
	FRotator ControlRotation = {0.0f, 0.0f, 0.0f};
	FVector RecoilValueVector = {0.0f, 0.0f, 0.0f};

	std::shared_ptr<PlayerSkeleton> Skeleton = std::make_shared<PlayerSkeleton>();
};

struct precision_calibrationConfig
{
	bool FirstAimXOnly = false;
	bool RandomAim = false;
	bool RandomBodyParts = false;  // 是否启用随机身体部位自瞄
	bool AutomaticShooting = false;
	float Smooth = 1.0f;


	bool UseBezierMovement = false;

	int AutomaticShootingFOV = 1;
	int AutomaticShootingTime = 1200;
	int AutomaticShootingKey = VK_RBUTTON;
	bool AimAndShot = false;
	float Threshold = 3.0f;
	int banjiAimDistance = 20;
	int Delay1 = 1200;
	bool enable = true;
	bool VisibleCheck = true;
	bool AutoSwitch = true;
	bool bIsScoping_CP = false;
	bool NoBulletNotAim = false;
	bool IsScopeandAim = false;
	bool ShowFOV = false;
	bool HotkeyMerge = false;
	bool AimWheel = true;
	bool NoRecoil = true;
	bool OriginalRecoil = true;
	bool Prediction = true;
	bool DynamicFov = true;
	int InitialValue = 1500;
	int FOV = 360;
	//int FOV2 = 360;
	int WheelFOV = 60;
	int SwitchingDelay = 0;
	int AimDistance = 800;
	float XSpeed = 5.0f;
	float YSpeed = 5.0f;
	float AimWheelSpeed = 5.0f;
	int RecoilTime = 3;
	int FPS = 165;
	float AimSpeedMaxFactor = 0.6f;
	float AimSpeedMaxFactornear = 0.5f;
	int AimSetDistance = 50;
	bool IgnoreGroggy = true;
	bool Blacklist = true;
	bool LineTraceSingle = false;
	int AimWheelBone = 3;
	bool FOVenable = false;
	int PredictionMode = 0;
	float RandomFactor = 0.3f;
	int RandomInterval = 500;
	int RandomBodyPartCount = 5;  // 随机选择的身体部位数量，默认为5
	int RandomSpeed = 500;        // 随机切换速度(毫秒)，默认为500ms
	bool RandomBodyPartsList[17] = {
			false, // 头顶
			true,  // 头部
			true,  // 脖子
			true,  // 胸部 
			true,  // 裆部
			false, // 左肩
			false, // 左肘
			false, // 右肩
			false, // 右肘
			false, // 左手
			false, // 右手
			false, // 左骨盆
			false, // 左腿骨
			false, // 右骨盆
			false, // 右腿骨
			false, // 左脚
			false  // 右脚
	};
	struct
	{
		int Key = VK_RBUTTON;
		int Mode = 0; // 0: AIM Main, 1: AIM Head
	} First;
	//struct
	//{
	//	//int Key = VK_RBUTTON;
	//	bool Bones[17] = {
	//		false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false
	//	};
	//} Mortar;
	struct
	{
		int Key = VK_LBUTTON;
		bool Bones[17] = {
			false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false
		};
	} Grenade;
	struct
	{
		int Key = VK_SHIFT;
		int Mode = 0; // 0: AIM Main, 1: AIM Head
	} Second;

	struct
	{
		int Key = 17;
		int Mode = 0; // 0: AIM Main, 1: AIM Head
	} Groggy;

	struct
	{
		int Key = 20;
	} Wheel;
};

struct ItemDetail
{
	std::string Name;
	std::string DisplayName;
	WeaponType Type;
	int Group = 0;
	bool ShowRay = false;
};

struct precision_calibrationWeaponConfig
{
	int Key;
	std::unordered_map<std::string, precision_calibrationConfig> Weapon;
};

struct CameraData
{
	FVector Location;
	FRotator Rotation;
	float FOV;
};

struct PlayerRankInfo {
	bool Updated = false;           // 添加 = false
	std::string Tier = "";
	std::string SubTier = "";
	int RankPoint = 0;              // 添加 = 0
	std::string AvgRank = "";
	float WinRatio = 0.0f;
	float KDA = 0.0f;
	std::string KDAToString = "";
	std::string WinRatioToString = "";
	std::string TierToString = "";
};

struct PlayerRankList {
	bool Updated = false;
	std::string AccountId = "";
	std::string PlayerName = "";
	int Tem = 0;                    // 添加 = 0
	float DamageAmount = 0.0f;      // 添加 = 0.0f
	int Survivallevel = 0;          // 添加 = 0
	struct PlayerRankInfo FPP {};
	struct PlayerRankInfo SquadFPP {};
	struct PlayerRankInfo TPP {};
	struct PlayerRankInfo SquadTPP {};
};

struct GamePlayerInfo {
	uint64_t pPlayerInfo = 0;                           // 添加 = 0
	uint64_t pTeam = 0;                                 // 添加 = 0
	uint64_t pCharacter = 0;                            // 添加 = 0
	uint64_t pPlayerName = 0;                           // 添加 = 0
	uint64_t pSavedNetworkAddress = 0;                  // 添加 = 0
	FText FPlayerName{};                                // 添加 {}
	FText FSavedNetworkAddress{};                       // 添加 {}
	std::string PlayerName = "";
	uint64_t pAccountId = 0;                            // 添加 = 0
	FText FAccountId{};                                 // 添加 {}
	FText FNameplateId{};                               // 添加 {}
	std::string AccountId = "";
	int TeamID = 0;
	int SquadMemberIndex = 0;
	bool IsMyTeam = false;
	bool IsSelf = false;
	int PlayerId = 0;
	EPartnerLevel PartnerLevel{};                       // 添加 {}
	int KillCount = 0;                                  // 添加 = 0
	int State = 0;                                      // 添加 = 0
	float DamageDealtOnEnemy = 0.0f;                    // 添加 = 0.0f
	int StatusType = 8; // 8 = 存活 | 11 = 死亡 || 12 = 人机
	uint64_t ptelemetryIdData = 0;                           // 添加 = 0
	FWutelemetryIdData telemetryIdData{};                         // 添加 {}
	FWuCharacterClanInfo CharacterClanInfo{};           // 添加 {}
	FTslPlayerStatisticsForOwner PlayerStatisticsForOwner{}; // 添加 {}
	uint64_t pClanName = 0;                             // 添加 = 0
	FText FClanName{};                                  // 添加 {}
	std::string ClanName = "";
	int ListType = 0; // 0 = 默认 | 1 = 黑名单 | 2 = 白名单
	int Alignment = 0;                                  // 添加 = 0
	int SpectatedCount = 0;                             // 添加 = 0
};

struct tMapInfo {
	float TimeStamp = 0.0f;

	struct {
		struct PosInfo {
			float Time = 0;
			FVector Pos;
		};
		std::deque<PosInfo> Info;
	} PosInfo;
};

struct FogPlayerInfo
{
	FVector Location;
};

struct PackageItem {
	uint64_t pItem;
	uint64_t Item;
	uint64_t ItemTable;
	int ItemID;
	std::string Name;
	std::string DisplayName;
	EntityType Type;
	WeaponType ItemType;
};

struct PackageInfo {
	EntityType Type;
	uint64_t Entity;
	int ID;
	std::string Name;
	std::string EntityName;
	uint64_t pDroppedItemGroup;
	uint64_t DroppedItemGroup;

	uint64_t pItemArray;

	int ItemCount;

	FVector Location;
	FVector2D ScreenLocation;
	float Distance;

	std::vector<PackageItem> Items;
};



struct FGameData
{


	char Acct[40] = { 0 };
	std::string HWID = "";
	bool pbForceUpdating = true;
	std::string Url = "";
	std::string Message = "";
	std::string ExperTime = "";
	char* strUrl = nullptr;

	std::string Version = "1.0.9.7";
	nlohmann::json LGKey;
	std::mutex gameDataMutex;

	std::mutex mesh_mutex;
	std::mutex dataMutex;
	std::mutex camera_mutex;

	int FogPlayerCount = 0;

	int currentMenuStyle = 0;

	int ThreadSleep = 2000;
	bool IsLogin = false;
	bool DMAConnected = false;
	bool AntiScreenshotMode = false;
	RECT menuRect = { 0, 0, 0, 0 };
	c_keys Keyboard;
	DWORD PID = 0;
	uint64_t GameBase = 0;
	uint64_t PhysxInstancePtr = 0;
	bool VirtualProtect = false;
	uint64_t HookBase = 0;
	uint64_t GameSize = 0;
	Scene PreviousScene = Scene::FindProcess;
	Scene Scene = Scene::FindProcess;
	std::string MapName;

	uint64_t UWorld;
	uint64_t GNames;
	uint64_t GameState;
	bool BucksackStatus;
	uint64_t GameInstance;
	uint64_t LocalPlayer;
	uint64_t PlayerController;
	bool bShowMouseCursor;
	uint64_t AcknowledgedPawn;
	uint64_t PlayerInput;
	uint64_t LocalPlayerPawn;
	uint64_t PlayerCameraManager;
	uint64_t CameraViewTarget;
	int LocalPlayerTeamID;
	uint64_t CameraRootComponent;
	FRotator ControlRotation;
	FRotator RecoilRotation; // 后坐力旋转数据
	SHORT BulletNumber = 0;
	uint64_t MyHUD;
	uint64_t CurrentLevel;
	uint64_t ActorArray;
	uint64_t ActorArrayEmpty;
	uint64_t Antiintegrity_monitorCharacterSyncManager;
	uint64_t pMatrix;
	BYTE Matrix[64];
	float MatrixVp[4][4];
	FMatrix ViewMatrix;
	int MyTeamID;
	int ActorCount;
	uint64_t Actor;
	Player LocalPlayerInfo;
	float WorldTimeSeconds = 0.f;
	float DeltaSeconds = 0.f;
	float FPS = 0.f;
	bool LineTraceSingle = false;

	// 刷新距离 300m
	float LoadRadius = 300.f;
	// 刷新幅度，只有场景变化大于这个数量才更新
	//uint32_t PhysxRefreshLimit = 0;
	Physics::VisibleScene<PrunerPayload, PrunerPayloadHash>* DynamicLoadScene;
	Physics::VisibleScene<PrunerPayload, PrunerPayloadHash>* DynamicRigidScene;
	Physics::VisibleScene<uint64_t, Int64Hash>* HeightFieldScene;

	TriangleMeshData* NextHintMeshData;
	std::vector<TriangleMeshData*> NearDynamicMeshData{};
	std::vector<TriangleMeshData*> NearStaticMeshData{};

	CameraData Camera;

	std::unordered_map<std::string, uint64_t, FnvHash> Offset;
	std::unordered_map<std::string, EntityInfo, FnvHash> DefaultEntityLists;
	std::unordered_map<std::string, EntityInfo, FnvHash> GNameLists;
	std::unordered_map<int, EntityInfo> GNameListsByID;

	struct
	{
		FVector WorldOriginLocation;
		float ImageMapSize = 204000.f;
		float ImageMapHeight = 8192.f;

		FMapSize MapSize;
		uint64_t MapGrid;
		uint64_t MapWidget;
		uint64_t MiniMapWidget;
		uint64_t Slot;
		bool Visibility;
		FMargin Layout;
		FVector2D Alignment;
		FVector2D Position;
		float ZoomFactor;
		FVector2D WorldCenterLocation;
		float MapSizeFactored;
		FVector SafetyZonePosition;
		float SafetyZoneRadius;
		FVector BlueZonePosition;
		float BlueZoneRadius;
		int SelectMinimapSizeIndex;
		float VehicleSpeed;
		float MiniRadarDistance;
		bool bRadarExtended;
		FVector2D MiniRadarSizeLarge;
		FVector2D MiniRadarFromLarge;
		FVector2D MiniRadarSizeNormal;
		FVector2D MiniRadarFromNormal;
		bool MiniRadarVisibility;
	} Radar;

	struct {
		uint64_t Target;
		EntityType Type = EntityType::Player;
		bool Lock;
		float ScreenDistance = 1000.f;
		int Bone;
		Player TargetPlayerInfo;
		FVector PredictedPos;
	} precision_calibration;

	mutable std::shared_mutex EnemyInfoMapMutex;
	std::unordered_map<uint64_t, tMapInfo> EnemyInfoMap;

	struct
	{
		mutable std::shared_mutex GNameListsMutex;

		// Cloned Map Data for Debug
		mutable std::shared_mutex ClonedMapMutex;
		std::vector<TriangleMeshData> ClonedMapMeshes;

		mutable std::shared_mutex CacheEntitysMutex;
		std::unordered_map<uint64_t, ActorEntityInfo> CacheEntitys;

		mutable std::shared_mutex CachePlayersMutex;
		std::unordered_map<uint64_t, Player> CachePlayers;

		mutable std::shared_mutex PlayersDataMutex;
		std::unordered_map<uint64_t, Player> PlayersData;

		mutable std::shared_mutex PlayersMutex;
		std::unordered_map<uint64_t, Player> Players;

		mutable std::shared_mutex CacheVehiclesMutex;
		std::unordered_map<uint64_t, VehicleInfo> CacheVehicles;

		mutable std::shared_mutex VehiclesMutex;
		std::unordered_map<uint64_t, VehicleInfo> Vehicles;

		mutable std::shared_mutex VehicleWheelsMutex;
		std::unordered_map<uint64_t, VehicleWheelInfo> VehicleWheels;

		mutable std::shared_mutex CacheDroppedItemsMutex;
		std::unordered_map<uint64_t, DroppedItemInfo> CacheDroppedItems;

		mutable std::shared_mutex CacheDroppedItemGroupsMutex;
		std::unordered_map<uint64_t, DroppedItemGroupInfo> CacheDroppedItemGroups;

		mutable std::shared_mutex ItemsMutex;
		std::unordered_map<uint64_t, ItemInfo> Items;

		mutable std::shared_mutex CacheProjectsMutex;
		std::unordered_map<uint64_t, ProjectInfo> CacheProjects;

		mutable std::shared_mutex ProjectsMutex;
		std::unordered_map<uint64_t, ProjectInfo> Projects;

		mutable std::shared_mutex FogPlayersMutex;
		std::unordered_map<uint64_t, FogPlayerInfo> FogPlayers;

		mutable std::shared_mutex CachePackagesMutex;
		std::unordered_map<uint64_t, PackageInfo> CachePackages;

		mutable std::shared_mutex PackagesMutex;
		std::unordered_map<uint64_t, PackageInfo> Packages;
	} Actors;

	struct ConfigStruct
	{
		struct OverlayConfig
		{
			POINT pt;
			bool bEnable = true;
			int ModeKey = VK_INSERT;
			bool FusionMode = false;
			int Quit_key = VK_END;
			bool UseThread = true;
			int MonitorCurrentIdx = 0;
			int ScreenWidth = 1728;// 1728;
			int ScreenHeight = 1080;// 1028;
			int ScreenX = 0;
			int ScreenY = 0;
			bool ShowFPS = true;
			bool VSync = false;
			HWND hWnd = NULL;
			FVector2D SettingSize = {};
			FVector2D SettingPos = {};
			FVector2D AuthSize = {};
			FVector2D AuthPos = {};
			bool Click = false;
		} Overlay;

		struct
		{
			bool Debug = false;
			bool Enable = true;
			int EnableKey = VK_F2;
			bool Recoilenanlek = false;

			bool YDownEnable = false;//独立压枪开关
			bool RedDownEnable = false;//红点自适应压枪
			bool RedDownAimEnable = false;//
			bool aimboot = false;

			bool GrenadePredict = true;
			bool PanzerFaust = true;
			bool MortarPredict = true;
			float MortarFOV = 10.0f;
			float MortarSmooth = 1.0f;
			int MortarDelay = 50;
			int MortarKey = VK_RBUTTON;
			int Grenade = 0;
			int Mortar = 0;
			int Mortar2 = 0;
			bool EnableEnhancedRecoil = false;
			int Mode = 0;
			int ConfigIndex = 0;
			int Controller = 4; // 4 = Hypervisor PS/2 backend
			int COM = 0;
			int Delay = 0;
			char IP[256] = "192.168.2.188";
			char Port[256] = "";
			char UUID[256] = "";
			bool Connected = false;
			bool ShowFOV = false;
			int LockMode = 0;
			float FOVColor[4] = { 255.0f, 255.0f, 255.0f, 1.0f };
			bool ShowWheelFOV = false;
			float WheelFOVColor[4] = { 184.f / 255.f, 134.f / 255.f, 11.f / 255.f, 1.0f };
			bool ShowPoint = true;
			int PointSize = 3;
			float PointColor[4] = { 255 / 255.f, 0.f / 255.f, 0.f / 255.f, 1.0f };
			bool AutoConnect = false;
			int yRecoil[6] = { 11,13,15,20,23,25 };
			int xRecoil[6] = { 11,13,15,20,23,25 };
			int interval[6] = { 1,1,1,1,1,1 };
			float yScale[6] = { 0.7f, 0.9f, 1.1f, 1.3f, 1.5f, 1.7f };
			std::unordered_map<int, precision_calibrationWeaponConfig> Configs = {
				{0, {VK_F2, {
					{"AR", precision_calibrationConfig()},
					{"DMR", precision_calibrationConfig()},
					{"SR", precision_calibrationConfig()},
					{"HG", precision_calibrationConfig()},
					{"LMG", precision_calibrationConfig()},
					{"SG", precision_calibrationConfig()},
					{"SMG", precision_calibrationConfig()},
					{"Grenade", precision_calibrationConfig()},
					{"Mortar", precision_calibrationConfig()},
					{"PanzerFaust100M1", precision_calibrationConfig()},
					{"Other", precision_calibrationConfig()},
					{"None", precision_calibrationConfig()}
				}}},
				{1, {VK_F3, {
					{"AR", precision_calibrationConfig()},
					{"DMR", precision_calibrationConfig()},
					{"SR", precision_calibrationConfig()},
					{"HG", precision_calibrationConfig()},
					{"LMG", precision_calibrationConfig()},
					{"SG", precision_calibrationConfig()},
					{"SMG", precision_calibrationConfig()},
					{"Grenade", precision_calibrationConfig()},
					{"Mortar", precision_calibrationConfig()},
					{"PanzerFaust100M1", precision_calibrationConfig()},
					{"Other", precision_calibrationConfig()},
					{"None", precision_calibrationConfig()}
				}}},
			};
		} precision_calibration;

		struct
		{
			int ClearKey = VK_F9;
		} Function;

		struct
		{
			bool LowModel = false;
			bool MediumModel = false;
			bool HighModel = true;
			bool LoadStaticModel = false; // 是否需要加载静态模型
			
			bool Enable = true;
			bool Stroke = true;
			float StrokeSize = 1.0f; // Do day vien chu (stroke) - tang len de chu dam hon
			int DistanceStyle = 1;

			bool VisibleCheck = true;

			bool PhysXLoad = false;
			bool PhysXDebug = false;
			bool MortarDebug = false;
			bool ESPDebug = false;
			bool PhysXDyh = false;

			bool AimExpandInfo = true;
			bool DynamicHealthColor = true;
			bool TargetedRay = true;
			bool PlayerLine = false;
			bool VisibleCheckRay = false;
			bool VisibleCheckRayDebug = false;
			bool VisibleCheckRayOnlyDebug = false;
			bool DrawTerrainMesh = false;
			bool LockedHiddenBones = false;
			int HeightFieldMutex = 3000;
			int PhysxLoadRadius = 600;
			int PhysxStaticRefreshInterval = 3000;
			int PhysxDynamicRefreshInterval = 3000;
			int PhysxRefreshLimit = 2500;

			bool Skeleton = true;
			bool suodingbianse = false;
			bool Nickname = true;
			bool HeadDrawing = false;
			bool ClanName = true;
			bool TeamID = true;
			bool Dis = true;
			bool Weapon = true;
			bool xianshixuetiao = true;
			bool Health = false;
			bool health_bar = true;
			bool dansehealth_bar = false;
			bool Partner = true;
			bool ShowInfos = true;
			int DistanceMax = 600;
			float PredictionTime = 0.025f;
			int SkeletonWidth = 2;
			bool AdjustableDistance = false; // 默认距离为100
			int FontSize = 14;
			int WeaponDistanceMax = InfoDistanceMax;
			int InfoDistanceMax = 300;
			int RayWidth = 2;
			int HealthBarPos = 0;
			int HealthBarStyle = 0;
			float HealthBarWidth = 40.0f;
			float HealthBarHeight = 6.0f;
			float HealthBarAlpha = 1.0f;
			int FocusModeKey = VK_OEM_3;
			bool FocusMode = false;
			int duiyouKey = VK_F1;
			int Playerskey = VK_F10;
			bool duiyou = false;
			bool Level = true;
			bool Kills = true;
			bool Damage = false;
			bool Spectate = true;
			bool Rank = true;
			bool Lastman = true;
			bool TPP = false;
			bool FPP = false;
			bool TPPKD = true;
			bool FPPKD = false;
			bool Offline = true;
			bool Downed = true;
			bool DisplayFrame = false;
			bool Alignment = true;
			bool GameInfo = true;
			bool showico = true;
			char ServerIP[256] = "";
			char SpectateAddr[256] = "";
			bool DangerWarning = true;
			bool DataSwitch = true;
			int DataSwitchkey = VK_F8;
			char RankSize[256] = "32";
			int Mousekey = VK_F11;
			bool Mouse = false;
			std::string RankModeUrl = "";
			bool Tier = false;
			bool RankPoint = false;
			bool KDA = false;
			struct
			{


			}WebRadar;



			struct
			{
				struct
				{
					float Skeleton[4] = { 255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} aim;

				struct
				{
					float Skeleton[4] = { 0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} Info;

				struct
				{
					float Skeleton[4] = { 0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} xuetiaoyanse;

				struct
				{
					float Line[4] = { 225.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} Ray;

				struct
				{
					float Info[4] = { 225.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 225.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f, 255.0f / 255.0f };
				} Default;

				struct
				{
					float Info[4] = { 255.0f / 255.0f, 0.0f / 255.0f, 204.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 255.0f / 255.0f, 0.0f / 255.0f, 204.0f / 255.0f, 255.0f / 255.0f };
				} Partner;

				struct
				{
					float Info[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} Groggy;

				struct
				{
					float Info[4] = { 128.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 128.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
				} Dangerous;

				struct
				{
					float Info[4] = { 247.0f / 255.0f, 101.0f / 255.0f, 101.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 247.0f / 255.0f, 101.0f / 255.0f, 101.0f / 255.0f, 255.0f / 255.0f };
				} Blacklist;

				struct
				{
					float Info[4] = { 79.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 79.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
				} Whitelist;

				struct
				{
					float Info[4] = { 0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f };
				} AI;

				struct
				{
					float Info[4] = { 184.f / 255.0f, 255.0f / 255.0f, 249.0f / 255.0f, 255.0f / 255.0f };
					float Skeleton[4] = { 184.f / 255.0f, 255.0f / 255.0f, 249.0f / 255.0f, 255.0f / 255.0f };
				} Visible;
			} Color;
		} signal_overlay;

		struct
		{
			bool Enable = true;
			int EnableKey = VK_F1;
			//int GroupKey = VK_F1;
			int DistanceMax = 50;
			int FontSize = 13;
			bool Combination = false;
			bool ShowIcon = false;
			bool ShowIconAndText = true;
			bool ShowRay = false;
			int GroupKey = VK_F1;
			bool ShowDistance = true;
			bool AccessoriesFilter = true;
			bool HandheldWeaponFilter = true;
			bool FilterHelmets = true;
			bool ItemLimit = true;
			int ThresholdX = 80;
			int RayWidth = 2;
			int ThresholdY = 15;
			int GroupAKey = VK_NUMPAD0;
			int GroupBKey = VK_NUMPAD1;
			int GroupCKey = VK_NUMPAD2;
			int GroupDKey = VK_NUMPAD3;
			int ShowGroup = 0;
			std::unordered_set<int> ShowGroups;
			float RayColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f }; // 白色
			float GroupAColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f }; // 白色
			float GroupBColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f }; // 白色
			float GroupCColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f }; // 白色
			float GroupDColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f }; // 白色

			std::unordered_map<std::string, ItemDetail, FnvHash> Lists = {
			};
		} Item;

		struct
		{
			int Bandage = 0;
			int FirstAidKit = 4;
			int MedicalKit = 0;
			int Painkiller = 5;
			int EnergyDrink = 5;
			int Adrenaline = 0;
			int C4 = 0;
			int Other = 255;
			int Grenade = 4;
			int Flashbang = 0;
			int Molotov = 0;
			int BlueZoneGrenade = 0;
			int StickyBomb = 0;
			int SmokeGrenade = 4;
		} ItemFiltering;

		struct
		{
			bool Enable = true;
			struct
			{
				bool ShowPlayer = true;
				bool ShowVehicle = true;
				bool ShowAirDrop = true;
				bool ShowDeadBox = true;
				bool ShowMapColor = true;
				bool MapRoom = true;
				int FontSize = 12;
				float Map_size = 4;
				float MapColor[4] = { 64 / 255.f, 77 / 255.f, 236 / 255.f, 190 / 255.f };
			} Main;

			struct
			{
				bool ShowPlayer = true;
				bool ShowVehicle = true;
				bool ShowAirDrop = true;
				bool ShowDeadBox = true;
				bool ShowMapColor = true;
				bool MapRoom = true;
				int FontSize = 10;
				float Map_size = 4;
				float MapColor[4] = { 64 / 255.f, 77 / 255.f, 236 / 255.f, 190 / 255.f };
			} Mini;
		} Radar;

		struct
		{
			bool Enable = false;
			int RedDot = 50;
			int x2 = 50;
			int x3 = 50;
			int x4 = 50;
			int x6 = 50;
			int x8 = 50;
			int Delay = 0;
		} Recoil;

		struct
		{
			bool GrenadeEnable = true;               // 是否启用手雷自瞄
			bool MortarShooting = true;

			float LeadFactor = 1.2f;          // 提前量系数

			bool Enable = true;
			int DistanceMax = 100;
			int FontSize = 14;
			bool TextShowChareTime = true;
			bool BarShowChareTime = false;
			bool ShowChareTime = true;
			bool GrenadeTrajectory = true;
			int ChareFontSize = 14;
			int TrajectorySize = 6;
			bool explosionrange = true;
			bool GrenadePrediction = true;
			float Color[4] = { 255.0f / 255.0f, 0.f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
			float ChareColor[4] = { 255.0f / 255.0f, 0.f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
			float TrajectoryColor[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f }; // 淡黄色，透明度50%
			float explosionrangeColor[4] = { 255.0f / 255.0f, 0.f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
			int CurrentLanguage = 0;
			bool ShowMenuCoordinates = false;
		} Project;

		struct
		{
			bool Enable = false;
			bool Durability = true;
			bool Health = true;
			bool ShowIcon = false;
			bool ShowIconAndText = true;
			int EnableKey = VK_F4;
			int DistanceMax = 600;
			int FontSize = 14;
			float Color[4] = { 0.f / 255.0f, 255.0f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
			float Healthbarcolor[4] = { 0.f / 255.0f, 255.0f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
			float Fuelbarcolor[4] = { 255.f / 255.0f, 0.f / 255.0f, 0.f / 255.0f, 255.0f / 255.0f };
		} Vehicle;

		struct
		{
			bool Enable = true;
			bool ShowItems = true;
			int EnableKey = VK_F5;
			int DistanceMax = 1000;
			int FontSize = 14;
			float Color[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
		} AirDrop;

		struct
		{
			bool Enable = false;
			bool ShowItems = true;
			int EnableKey = VK_F6;
			int DistanceMax = 100;
			int FontSize = 14;
			float Color[4] = { 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f };
		} DeadBox;

		struct
		{
			bool Enable = false;
			bool ShowDistance = true;
			int DistanceMax = 200;
			int FontSize = 14;
		} Early;

		struct
		{
			bool Show = true;
			int ShowKey = VK_HOME;
			int rongheqi = VK_INSERT;
		} Menu;

		struct
		{
			bool Setting = true;
			bool Players = false;
			bool Main = true;
			bool IsLogin = false;
			struct
			{
				char LoginUsername[128];
				char LoginPassword[128];
				char RegUsername[128];
				char RegPassword[128];
				char UnBindLoginUsername[128];
				char UnBindLoginPassword[128];
				char Key[128];
			} LoginForm;
		} Window;

		struct
		{
			int RankMode = 2;
			int MarkKey = VK_MBUTTON;
		} PlayerList;
		struct
		{
			int SelectMinimapSizeIndex;
			float VehicleSpeed;
			float MiniRadarDistance;
			bool bRadarExtended;
			FVector2D MiniRadarSizeLarge;
			FVector2D MiniRadarFromLarge;
			FVector2D MiniRadarSizeNormal;
			FVector2D MiniRadarFromNormal;
			bool MiniRadarVisibility;
		} RadarConfig;
		struct
		{
			bool AutoShotEnabled = true;
			int AutoShotKey = VK_CAPITAL;
		} Macro;
		struct
		{
			std::string cardKey;
			char globalBuf[1024] = "\0";
			std::string ExpireTime = "";
		} eyou;
		struct {
			char IP[256] = "";
			char  Port[256] = "";
			char  SubTitle[256] = "";
			char  PIN[256] = "";
			std::string RadarUrl;
			bool isWebRadarConnect = false;
			bool isWebRadarEnable = false;
			int fps = 30;
		} WebRadar;
		struct {
			bool EnablePerformanceMonitor = false;
		} Debug;
	} Config;


	mutable std::shared_mutex PlayerRankListsMutex;
	mutable std::shared_mutex PlayerBlackListsMutex;
	mutable std::shared_mutex PlayerWhiteListsMutex;
	mutable std::shared_mutex PlayerListsMutex;

	std::unordered_map<std::string, GamePlayerInfo> PlayerLists;
	std::unordered_map<std::string, PlayerRankList> PlayerRankLists;
	std::unordered_map<std::string, int> PlayerBlackLists;
	std::unordered_map<std::string, int> PlayerWhiteLists;

	std::unordered_map<std::string, PlayerRankList> PlayerSegmentLists;
	mutable std::shared_mutex PlayerSegmentListsMutex;

	int PlayerCount = 0;
	int NumAliveTeams = 0;

	struct {
		float DataThreadMs = 0.0f;     // Thời gian đọc dữ liệu (Players, Bones, v.v.)
		float RenderThreadMs = 0.0f;   // Thời gian vẽ Overlay
		float AimThreadMs = 0.0f;      // Thời gian xử lý precision_calibration
		float ActorThreadMs = 0.0f;    // Thời gian quét Actors
		float ItemThreadMs = 0.0f;     // Thời gian quét Items
		float RadarThreadMs = 0.0f;    // Thời gian luồng Radar
		float DmaLatencyMs = 0.0f;     // Độ trễ phản hồi từ Card DMA
		int DmaReadCount = 0;          // Số lượng lệnh đọc DMA mỗi frame
	} Performance;

	// [FIX PCIE READ]: Centralized Local Player Data for precision_calibration
	struct {
		FRotator ControlRotation = {0.0f, 0.0f, 0.0f};
		FRotator RecoilADSRotation = {0.0f, 0.0f, 0.0f};
		FVector RecoilValueVector = {0.0f, 0.0f, 0.0f};
		uint64_t LastUpdateTick = 0;
	} LocalAimData;
	mutable std::mutex LocalAimDataMutex;
};

extern FGameData GameData;



namespace Data
{
	// 查询接口函数
	inline std::unordered_map<std::string, PlayerRankList> GetPlayerSegmentLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerSegmentListsMutex);
		return GameData.PlayerSegmentLists;
	}

	inline void SetPlayerSegmentLists(std::unordered_map<std::string, PlayerRankList> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerSegmentListsMutex);
		GameData.PlayerSegmentLists = value;
	}

	inline PlayerRankList GetPlayerSegmentListsItem(std::string key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerSegmentListsMutex);
		return GameData.PlayerSegmentLists[key];
	}

	inline void SetPlayerSegmentListsItem(std::string key, PlayerRankList value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerSegmentListsMutex);
		GameData.PlayerSegmentLists[key] = value;
	}

	//查询接口函数

	inline void SetCachePackages(std::unordered_map<uint64_t, PackageInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CachePackagesMutex);
		GameData.Actors.CachePackages.swap(value);
	}

	inline std::unordered_map<uint64_t, PackageInfo> GetCachePackages()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CachePackagesMutex);
		return GameData.Actors.CachePackages;
	}

	inline void SetPackages(std::unordered_map<uint64_t, PackageInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.PackagesMutex);
		GameData.Actors.Packages.swap(value);
	}

	inline void CopyPackages(std::unordered_map<uint64_t, PackageInfo>& outPackages)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PackagesMutex);
		outPackages = GameData.Actors.Packages;
	}

	inline std::unordered_map<uint64_t, PackageInfo> GetPackages()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PackagesMutex);
		return GameData.Actors.Packages;
	}


	inline std::unordered_map<uint64_t, tMapInfo> GetEnemyInfoMap()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.EnemyInfoMapMutex);
		return GameData.EnemyInfoMap;
	}

	inline void SetEnemyInfoMap(std::unordered_map<uint64_t, tMapInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.EnemyInfoMapMutex);
		GameData.EnemyInfoMap.swap(value);
	}

	static inline std::unordered_map<uint64_t, GamePlayerInfo> GlobalPlayerCache;
	inline std::unordered_map<uint64_t, GamePlayerInfo> GetGlobalPlayerCache() { return GlobalPlayerCache; }
	inline void SetGlobalPlayerCacheItem(uint64_t key, GamePlayerInfo value) { GlobalPlayerCache[key] = value; }

	inline std::unordered_map<std::string, GamePlayerInfo> GetPlayerLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerListsMutex);
		return GameData.PlayerLists;
	}

	inline void SetPlayerLists(std::unordered_map<std::string, GamePlayerInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerListsMutex);
		GameData.PlayerLists.swap(value);
	}

	inline std::unordered_map<std::string, PlayerRankList> GetPlayerRankLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerRankListsMutex);
		return GameData.PlayerRankLists;
	}

	inline void SetPlayerRankLists(std::unordered_map<std::string, PlayerRankList> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerRankListsMutex);
		GameData.PlayerRankLists.swap(value);
	}

	inline PlayerRankList GetPlayerRankListsItem(std::string key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerRankListsMutex);
		return GameData.PlayerRankLists[key];
	}

	inline void SetPlayerRankListsItem(std::string key, PlayerRankList value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerRankListsMutex);
		GameData.PlayerRankLists[key] = value;
	}

	inline int GetPlayerWhiteListsItem(std::string key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerWhiteListsMutex);
		if (GameData.PlayerWhiteLists.count(key) > 0)
		{
			return GameData.PlayerWhiteLists[key];
		}

		return 0;
	}

	inline void DeletePlayerWhiteListsItem(std::string key)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerWhiteListsMutex);
		GameData.PlayerWhiteLists.erase(key);
	}

	inline void SetPlayerWhiteListsItem(std::string key, int value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerWhiteListsMutex);
		GameData.PlayerWhiteLists[key] = value;
	}

	inline void SetPlayerWhiteLists(std::unordered_map<std::string, int> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerWhiteListsMutex);
		GameData.PlayerWhiteLists.swap(value);
	}

	inline std::unordered_map<std::string, int> GetPlayerWhiteLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerWhiteListsMutex);
		return GameData.PlayerWhiteLists;
	}

	inline void CopyPlayersData(std::unordered_map<uint64_t, Player>& out)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersDataMutex);
		out = GameData.Actors.PlayersData;
	}

	inline void CopyCachePlayers(std::unordered_map<uint64_t, Player>& out)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CachePlayersMutex);
		out = GameData.Actors.CachePlayers;
	}

	inline int GetPlayerBlackListsItem(std::string key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerBlackListsMutex);
		auto it = GameData.PlayerBlackLists.find(key);
		if (it != GameData.PlayerBlackLists.end())
		{
			return it->second;
		}
		return 0;
	}

	inline void DeletePlayerBlackListsItem(std::string key)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerBlackListsMutex);
		GameData.PlayerBlackLists.erase(key);
	}

	inline void SetPlayerBlackListsItem(std::string key, int value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerBlackListsMutex);
		GameData.PlayerBlackLists[key] = value;
	}

	inline void SetPlayerBlackLists(std::unordered_map<std::string, int> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerBlackListsMutex);
		GameData.PlayerBlackLists.swap(value);
	}

	inline void SetPlayerListsItem(std::string name, GamePlayerInfo value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.PlayerListsMutex);
		GameData.PlayerLists[name] = value;
	}

	inline std::unordered_map<std::string, int> GetPlayerBlackLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.PlayerBlackListsMutex);
		return GameData.PlayerBlackLists;
	}

	inline void SetCachePlayers(std::unordered_map<uint64_t, Player> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CachePlayersMutex);
		GameData.Actors.CachePlayers.swap(value); // Rất nhanh, O(1)
	}

	inline std::unordered_map<uint64_t, Player> GetCachePlayers()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CachePlayersMutex);
		return GameData.Actors.CachePlayers;
	}

	inline void SetCacheEntitys(std::unordered_map<uint64_t, ActorEntityInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CacheEntitysMutex);
		GameData.Actors.CacheEntitys.swap(value);
	}

	inline std::unordered_map<uint64_t, ActorEntityInfo> GetCacheEntitys()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CacheEntitysMutex);
		return GameData.Actors.CacheEntitys;
	}

	inline void SetPlayersData(std::unordered_map<uint64_t, Player> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.PlayersDataMutex);
		GameData.Actors.PlayersData.swap(value);
	}

	inline std::unordered_map<uint64_t, Player> GetPlayersData()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersDataMutex);
		return GameData.Actors.PlayersData;
	}

	inline void SetPlayers(std::unordered_map<uint64_t, Player> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.PlayersMutex);
		GameData.Actors.Players.swap(value);
	}

	inline void CopyPlayers(std::unordered_map<uint64_t, Player>& outPlayers)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersMutex);
		outPlayers = GameData.Actors.Players;
	}

	// TOI UU: Lay tham chieu truc tiep de Render khong bi doi
	inline const std::unordered_map<uint64_t, Player>& GetPlayersRef()
	{
		return GameData.Actors.Players;
	}

	inline std::unordered_map<uint64_t, Player> GetPlayers()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersMutex);
		return GameData.Actors.Players;
	}

	inline Player GetPlayersItem(uint64_t Key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersMutex);
		return GameData.Actors.Players[Key];
	}

	// Hàm lấy con trỏ trực tiếp (Toi uu cho precision_calibration)
	inline Player* GetPlayersItemPtr(uint64_t Key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.PlayersMutex);
		if (GameData.Actors.Players.count(Key) > 0)
			return &GameData.Actors.Players[Key];
		return nullptr;
	}

	inline void SetVehicles(std::unordered_map<uint64_t, VehicleInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.VehiclesMutex);
		GameData.Actors.Vehicles.swap(value);
	}

	inline void CopyVehicles(std::unordered_map<uint64_t, VehicleInfo>& outVehicles)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.VehiclesMutex);
		outVehicles = GameData.Actors.Vehicles;
	}

	inline std::unordered_map<uint64_t, VehicleInfo> GetVehicles()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.VehiclesMutex);
		return GameData.Actors.Vehicles;
	}

	inline void SetCacheVehicles(std::unordered_map<uint64_t, VehicleInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CacheVehiclesMutex);
		GameData.Actors.CacheVehicles.swap(value);
	}

	inline std::unordered_map<uint64_t, VehicleInfo> GetCacheVehicles()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CacheVehiclesMutex);
		return GameData.Actors.CacheVehicles;
	}

	inline void SetVehiclWheels(std::unordered_map<uint64_t, VehicleWheelInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.VehicleWheelsMutex);
		GameData.Actors.VehicleWheels.swap(value);
	}

	inline void CopyVehicleWheels(std::unordered_map<uint64_t, VehicleWheelInfo>& outWheels)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.VehicleWheelsMutex);
		outWheels = GameData.Actors.VehicleWheels;
	}

	inline std::unordered_map<uint64_t, VehicleWheelInfo> GetVehicleWheels()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.VehicleWheelsMutex);
		return GameData.Actors.VehicleWheels;
	}

	inline VehicleWheelInfo GetVehicleWheelsItem(uint64_t Key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.VehicleWheelsMutex);
		return GameData.Actors.VehicleWheels[Key];
	}

	inline EntityInfo GetGNameListsByIDItem(int key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		auto it = GameData.GNameListsByID.find(key);
		if (it != GameData.GNameListsByID.end())
		{
			return it->second;
		}

		return EntityInfo{ "Unknown", EntityType::Unknown, 0, WeaponType::Other };
	}

	inline std::shared_mutex& GetGNameListsMutex()
	{
		return GameData.Actors.GNameListsMutex;
	}

	inline const std::unordered_map<int, EntityInfo>& GetGNameListsByIDRef()
	{
		return GameData.GNameListsByID;
	}

	inline EntityInfo GetGNameListsItem(std::string key)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		auto it = GameData.GNameLists.find(key);
		if (it != GameData.GNameLists.end())
		{
			return it->second;
		}

		return EntityInfo{ "Unknown", EntityType::Unknown, 0, WeaponType::Other };
	}

	inline std::unordered_map<std::string, EntityInfo, FnvHash> GetGNameLists()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		return GameData.GNameLists;
	}

	inline std::pair<std::unordered_map<std::string, EntityInfo, FnvHash>, std::unordered_map<int, EntityInfo>> GetGNameListsAndGNameListsByID()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		return { GameData.GNameLists, GameData.GNameListsByID };
	}

	inline void SetGNameLists(std::unordered_map<std::string, EntityInfo, FnvHash> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		GameData.GNameLists.swap(value);
	}

	inline void ClearGNameListsAndGNameListsByID()
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		//GameData.GNameLists = EntityLists;
		GameData.GNameListsByID.clear();
	}

	inline void SetGNameListsAndGNameListsByID(const std::unordered_map<std::string, EntityInfo, FnvHash>& value, const std::unordered_map<int, EntityInfo>& value1)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		for (auto item : value)
		{
			GameData.GNameLists[item.first] = item.second;
		}
		for (auto item : value1)
		{
			GameData.GNameListsByID[item.first] = item.second;
		}
	}

	inline void SetGNameListsAndGNameListsByIDItem(std::string key, EntityInfo value, int key1, EntityInfo value1)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		GameData.GNameLists[key] = value;
		GameData.GNameListsByID[key1] = value1;
	}

	inline std::unordered_map<int, EntityInfo> GetGNameListsByID()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		return GameData.GNameListsByID;
	}

	inline void SetGNameListsByID(std::unordered_map<int, EntityInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.GNameListsMutex);
		GameData.GNameListsByID.swap(value);
	}

	inline void SetCacheDroppedItems(std::unordered_map<uint64_t, DroppedItemInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CacheDroppedItemsMutex);
		GameData.Actors.CacheDroppedItems.swap(value); // Swap to reduce lock time
	}

	inline std::unordered_map<uint64_t, DroppedItemInfo> GetCacheDroppedItems()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CacheDroppedItemsMutex);
		return GameData.Actors.CacheDroppedItems;
	}

	inline void SetCacheDroppedItemGroups(std::unordered_map<uint64_t, DroppedItemGroupInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CacheDroppedItemGroupsMutex);
		GameData.Actors.CacheDroppedItemGroups.swap(value);
	}

	inline std::unordered_map<uint64_t, DroppedItemGroupInfo> GetCacheDroppedItemGroups()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CacheDroppedItemGroupsMutex);
		return GameData.Actors.CacheDroppedItemGroups;
	}

	inline void SetItems(std::unordered_map<uint64_t, ItemInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.ItemsMutex);
		GameData.Actors.Items.swap(value);
	}

	inline void CopyItems(std::unordered_map<uint64_t, ItemInfo>& outItems)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.ItemsMutex);
		outItems = GameData.Actors.Items;
	}

	inline std::unordered_map<uint64_t, ItemInfo> GetItems()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.ItemsMutex);
		return GameData.Actors.Items;
	}

	inline void SetCacheProjects(std::unordered_map<uint64_t, ProjectInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.CacheProjectsMutex);
		GameData.Actors.CacheProjects.swap(value);
	}

	inline std::unordered_map<uint64_t, ProjectInfo> GetCacheProjects()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.CacheProjectsMutex);
		return GameData.Actors.CacheProjects;
	}

	inline void CopyProjects(std::unordered_map<uint64_t, ProjectInfo>& outProjects)
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.ProjectsMutex);
		outProjects = GameData.Actors.Projects;
	}

	inline void SetProjects(std::unordered_map<uint64_t, ProjectInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.ProjectsMutex);
		GameData.Actors.Projects.swap(value);
	}


	inline std::unordered_map<uint64_t, ProjectInfo> GetProjects()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.ProjectsMutex);
		return GameData.Actors.Projects;
	}

	inline void SetFogPlayers(std::unordered_map<uint64_t, FogPlayerInfo> value)
	{
		std::unique_lock<std::shared_mutex> lock(GameData.Actors.FogPlayersMutex);
		GameData.Actors.FogPlayers.swap(value);
	}

	inline std::unordered_map<uint64_t, FogPlayerInfo> GetFogPlayers()
	{
		std::shared_lock<std::shared_mutex> lock(GameData.Actors.FogPlayersMutex);
		return GameData.Actors.FogPlayers;
	}

	inline void SetLocalAimData(FRotator controlRot, FRotator recoilAdsRot, FVector recoilValue)
	{
		std::lock_guard<std::mutex> lock(GameData.LocalAimDataMutex);
		GameData.LocalAimData.ControlRotation = controlRot;
		GameData.LocalAimData.RecoilADSRotation = recoilAdsRot;
		GameData.LocalAimData.RecoilValueVector = recoilValue;
		GameData.LocalAimData.LastUpdateTick = GetTickCount64();
	}

	struct AimData {
		FRotator ControlRotation;
		FRotator RecoilADSRotation;
		FVector RecoilValueVector;
		uint64_t LastUpdateTick;
	};

	inline AimData GetLocalAimData()
	{
		std::lock_guard<std::mutex> lock(GameData.LocalAimDataMutex);
		return { GameData.LocalAimData.ControlRotation, GameData.LocalAimData.RecoilADSRotation, GameData.LocalAimData.RecoilValueVector, GameData.LocalAimData.LastUpdateTick };
	}

	inline CameraData GetCamera()
	{
		std::lock_guard<std::mutex> lock(GameData.camera_mutex);
		return GameData.Camera;
	}

	inline void SetCamera(const CameraData& value)
	{
		std::lock_guard<std::mutex> lock(GameData.camera_mutex);
		GameData.Camera = value;
	}
}
