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

        // Su dung PhysX 3.4 Raycast nhu PAOD
        // return physxMgr.Raycast(TraceStart, TraceEnd);
        return true; // Tam thoi mac dinh la thay (hoac true) neu can tat check vat ly
    }

    static TriangleMeshData* getNextHint()
    {
        return nullptr; // Disable getNextHint while moving to PhysX
    }
}
