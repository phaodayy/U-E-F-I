#pragma once
#include "math.hpp"
#include "memory.hpp"
#include "offsets.hpp"

namespace telemetryBones {
    inline Vector3 GetBoneWorldPosWithMatrix(int bone_index, uint64_t bone_array, const FTransform& meshToWorld) {
        FTransform boneTransform = telemetryMemory::Read<FTransform>(bone_array + (bone_index * 48));
        FMatrix m1 = UnrealMath::TransformToMatrix(boneTransform);
        FMatrix m2 = UnrealMath::TransformToMatrix(meshToWorld);
        FMatrix combined = UnrealMath::MatrixMultiplication(m1, m2);
        return { combined._41, combined._42, combined._43 };
    }

    inline Vector3 GetBoneWorldPos(int bone_index, uint64_t bone_array, uint64_t mesh) {
        FTransform meshToWorld = telemetryMemory::Read<FTransform>(mesh + telemetryOffsets::ComponentToWorld);
        return GetBoneWorldPosWithMatrix(bone_index, bone_array, meshToWorld);
    }
}
