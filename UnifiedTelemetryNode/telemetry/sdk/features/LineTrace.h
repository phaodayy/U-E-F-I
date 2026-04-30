#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <features/PhysXManager.h>
#include <Common/VisibleScene.h>

namespace LineTrace
{
    static bool LineTraceSingle(FVector TraceStart, FVector TraceEnd, bool oldState = true)
    {
        if (GameData.Scene != Scene::Gaming) {
            return false;
        }

        // Primary: Embree Raycast (fast, CPU-based ray tracing)
        if (GameData.DynamicLoadScene) {
            auto hit = GameData.DynamicLoadScene->Raycast(TraceStart, TraceEnd);
            if (hit.hit.geomID != RTC_INVALID_GEOMETRY_ID && hit.hit.geomID != (RTC_INVALID_GEOMETRY_ID - 1)) {
                return false; // Bi chan boi vat the trong Embree scene
            }
        }

        // Secondary: PhysX Raycast (accurate, physics-based)
        if (GameData.Config.signal_overlay.PhysXLoad) {
            return physxMgr.Raycast(TraceStart, TraceEnd);
        }

        return oldState;
    }

    static TriangleMeshData* getNextHint()
    {
        if (!GameData.DynamicLoadScene) return nullptr;
        auto hit = GameData.DynamicLoadScene->Raycast(
            GameData.LocalPlayerInfo.Location,
            GameData.LocalPlayerInfo.Location + FVector(0, 0, -500)
        );
        if (hit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
            return GameData.DynamicLoadScene->GetGeomeoryData(hit.hit.geomID);
        }
        return nullptr;
    }
}
