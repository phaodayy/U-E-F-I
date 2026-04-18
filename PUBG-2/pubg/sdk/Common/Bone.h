#pragma once

#include <list>
#include <unordered_map>
#include <Utils/Engine.h>

enum EBoneIndex : int
{
    // Common Bones (Same for both genders in this update)
    Root = 0,
    Pelvis = 1,
    Spine_01 = 2,
    Spine_02 = 3,
    Spine_03 = 4,   // Chest
    Neck_01 = 5,    // Neck
    Head = 10,
    ForeHead = 11, // Estimated nearby head

    // Limb Bones - Defaults to Male
    Upperarm_L = 88,
    Lowerarm_L = 89,
    Hand_L = 90,

    Upperarm_R = 115,
    Lowerarm_R = 116,
    Hand_R = 117,

    Thigh_L = 172,
    Calf_L = 173,
    Foot_L = 174,
    Ball_L = 175,

    Thigh_R = 178,
    Calf_R = 179,
    Foot_R = 180,
    Ball_R = 181,

    // Female Specific (Internal mapping use)
    Female_Upperarm_L = 95,
    Female_Lowerarm_L = 96,
    Female_Hand_L = 97,

    Female_Upperarm_R = 122,
    Female_Lowerarm_R = 123,
    Female_Hand_R = 124,

    Female_Thigh_L = 180,
    Female_Calf_L = 183,
    Female_Foot_L = 181,
    Female_Ball_L = 182,

    Female_Thigh_R = 186,
    Female_Calf_R = 189,
    Female_Foot_R = 187,
    Female_Ball_R = 188
};

inline int GetGenderBoneIndex(EGender gender, EBoneIndex bone) {
    if (gender != EGender::Female) return (int)bone;

    switch (bone) {
    case EBoneIndex::Upperarm_L: return EBoneIndex::Female_Upperarm_L;
    case EBoneIndex::Lowerarm_L: return EBoneIndex::Female_Lowerarm_L;
    case EBoneIndex::Hand_L: return EBoneIndex::Female_Hand_L;
    case EBoneIndex::Upperarm_R: return EBoneIndex::Female_Upperarm_R;
    case EBoneIndex::Lowerarm_R: return EBoneIndex::Female_Lowerarm_R;
    case EBoneIndex::Hand_R: return EBoneIndex::Female_Hand_R;
    case EBoneIndex::Thigh_L: return EBoneIndex::Female_Thigh_L;
    case EBoneIndex::Calf_L: return EBoneIndex::Female_Calf_L;
    case EBoneIndex::Foot_L: return EBoneIndex::Female_Foot_L;
    case EBoneIndex::Ball_L: return EBoneIndex::Female_Ball_L;
    case EBoneIndex::Thigh_R: return EBoneIndex::Female_Thigh_R;
    case EBoneIndex::Calf_R: return EBoneIndex::Female_Calf_R;
    case EBoneIndex::Foot_R: return EBoneIndex::Female_Foot_R;
    case EBoneIndex::Ball_R: return EBoneIndex::Female_Ball_R;
    default: return (int)bone;
    }
}

namespace SkeletonLists {
	static std::list<EBoneIndex> Right_Arm = { EBoneIndex::Neck_01, EBoneIndex::Upperarm_R, EBoneIndex::Lowerarm_R, EBoneIndex::Hand_R };
	static std::list<EBoneIndex> Left_Arm = { EBoneIndex::Neck_01, EBoneIndex::Upperarm_L, EBoneIndex::Lowerarm_L, EBoneIndex::Hand_L };
	static std::list<EBoneIndex> Spine = { EBoneIndex::Neck_01, EBoneIndex::Spine_01, EBoneIndex::Pelvis };
	static std::list<EBoneIndex> Lower_Right = { EBoneIndex::Pelvis, EBoneIndex::Thigh_R, EBoneIndex::Calf_R, EBoneIndex::Foot_R };
	static std::list<EBoneIndex> Lower_Left = { EBoneIndex::Pelvis, EBoneIndex::Thigh_L, EBoneIndex::Calf_L, EBoneIndex::Foot_L };
	static std::list<std::list<EBoneIndex>> Skeleton = { Right_Arm, Left_Arm, Spine, Lower_Right, Lower_Left };

	static std::list<EBoneIndex> Bones = {
		EBoneIndex::Root,
		EBoneIndex::ForeHead,
		EBoneIndex::Head,
		EBoneIndex::Neck_01,
		EBoneIndex::Upperarm_R, EBoneIndex::Lowerarm_R, EBoneIndex::Hand_R,
		EBoneIndex::Upperarm_L, EBoneIndex::Lowerarm_L, EBoneIndex::Hand_L,
		EBoneIndex::Spine_01,EBoneIndex::Spine_03, EBoneIndex::Pelvis,
		EBoneIndex::Thigh_R, EBoneIndex::Calf_R, EBoneIndex::Foot_R,
		EBoneIndex::Thigh_L, EBoneIndex::Calf_L, EBoneIndex::Foot_L,
	};

	static std::list<EBoneIndex> AimBones = {
        EBoneIndex::Spine_03,
		EBoneIndex::Pelvis,
		EBoneIndex::ForeHead,
		EBoneIndex::Head,
		EBoneIndex::Neck_01,
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

};