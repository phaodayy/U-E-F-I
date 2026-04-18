#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/GNames.h>
#include <Hack/Decrypt.h>
#include <Hack/LineTrace.h>

namespace LineTraceHook
{



	bool GetLocation(const Player& TargetCharacter, const EBoneIndex& FirstBoneIndex, EBoneIndex* VisibilityBoneIndex)
	{
		if (!TargetCharacter.Skeleton) return false;
		const auto& locationBones = TargetCharacter.Skeleton->LocationBones;
		const FVector cameraLocation = Data::GetCamera().Location;

		// 1. Kiem tra xuong uu tien truoc
		FVector firstBonePos = locationBones[FirstBoneIndex];
		if (firstBonePos.X != 0) {
			if (LineTrace::LineTraceSingle(cameraLocation, firstBonePos)) {
				*VisibilityBoneIndex = FirstBoneIndex;
				return true;
			}
		}

		// 2. Kiem tra cac xuong khac neu xuong uu tien khong nhin thay
		for (EBoneIndex bone : SkeletonLists::Bones)
		{
			if (bone == FirstBoneIndex) continue;
			FVector bonePos = locationBones[bone];
			if (bonePos.X == 0) continue;

			if (LineTrace::LineTraceSingle(cameraLocation, bonePos)) {
				*VisibilityBoneIndex = bone;
				return true;
			}
		}

		return false;
	}
}
