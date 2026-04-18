#pragma once
#include "math.hpp"
#include "memory.hpp"
#include "offsets.hpp"

namespace PubgBones {
    inline Vector3 GetBoneWorldPosWithMatrix(int bone_index, uint64_t bone_array, const FTransform& meshToWorld) {
        FTransform boneTransform = PubgMemory::Read<FTransform>(bone_array + (bone_index * 48));
        FMatrix m1 = UnrealMath::TransformToMatrix(boneTransform);
        FMatrix m2 = UnrealMath::TransformToMatrix(meshToWorld);
        FMatrix combined = UnrealMath::MatrixMultiplication(m1, m2);
        return { combined._41, combined._42, combined._43 };
    }

    inline Vector3 GetBoneWorldPos(int bone_index, uint64_t bone_array, uint64_t mesh) {
        FTransform meshToWorld = PubgMemory::Read<FTransform>(mesh + PubgOffsets::ComponentToWorld);
        return GetBoneWorldPosWithMatrix(bone_index, bone_array, meshToWorld);
    }
}
