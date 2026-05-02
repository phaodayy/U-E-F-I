#pragma once

#ifndef _ALLOW_KEYWORD_MACROS
#define _ALLOW_KEYWORD_MACROS
#endif

#include <vector>
#include <mutex>
#include <set>
#include <unordered_map>
#include <string>

#ifndef requires
#define requires _requires_fix
#endif

#include <foundation/Px.h>
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxRigidStatic.h>
#include <geometry/PxTriangleMesh.h>
#include <geometry/PxTriangleMeshGeometry.h>
#include <geometry/PxHeightFieldGeometry.h>
#include <geometry/PxHeightField.h>
#include <geometry/PxHeightFieldDesc.h>
#include <geometry/PxHeightFieldSample.h>
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxRigidActorExt.h>

#undef requires

#include <Common/Data.h>
#include <Common/Entitys.h>
#include <imgui/imgui.h>

// Collision Groups for Query Filtering
namespace PhysXCollisionGroup {
    enum : uint32_t {
        GROUP_NONE      = 0,
        GROUP_STATIC    = (1 << 0),  // Tuong, nha, da
        GROUP_VEHICLE   = (1 << 1),  // Xe co
        GROUP_FOLIAGE   = (1 << 2),  // Co, bui cay (xuyen thau)
        GROUP_TERRAIN   = (1 << 3),  // Dia hinh HeightField
        GROUP_DYNAMIC   = (1 << 4),  // Vat the dong (cua, tuong pha duoc)
        GROUP_ALL       = 0xFFFFFFFF
    };

    inline uint32_t ClassifyFromFilterData(uint32_t queryWord0, uint32_t queryWord3, uint32_t simWord0) {
        if (queryWord0 == 0 && simWord0 == 0) return GROUP_FOLIAGE;
        if (queryWord3 > 0 && queryWord3 < 50) return GROUP_FOLIAGE;
        if (queryWord3 >= 50 && queryWord3 < 200) return GROUP_DYNAMIC;
        return GROUP_STATIC;
    }
}

class PhysXManager {
public:
    PhysXManager() : gFoundation(nullptr), gPhysics(nullptr), gCooking(nullptr), gScene(nullptr), gMaterial(nullptr) {}
    
    bool Init();
    void Cleanup();

    // Raycast kiem tra va cham (Visible Check)
    // collisionMask: chi raycast cac nhom nay (mac dinh: tat ca tru Foliage)
    bool Raycast(const FVector& origin, const FVector& target, 
                 uint32_t collisionMask = PhysXCollisionGroup::GROUP_STATIC | PhysXCollisionGroup::GROUP_VEHICLE | PhysXCollisionGroup::GROUP_TERRAIN | PhysXCollisionGroup::GROUP_DYNAMIC);

    bool RaycastHit(const FVector& origin, const FVector& target, FVector& hitPoint,
                    uint32_t collisionMask = PhysXCollisionGroup::GROUP_STATIC | PhysXCollisionGroup::GROUP_VEHICLE | PhysXCollisionGroup::GROUP_TERRAIN | PhysXCollisionGroup::GROUP_DYNAMIC);

    // Them Mesh vao Scene (voi Cooking Cache)
    void UpdateScene(const std::vector<TriangleMeshData>& meshes, const std::set<PrunerPayload>& removeObjects);

    // Xuat toan bo Scene ra file .obj de debug dia hinh
    void ExportSceneToObj(const std::string& filename);

    struct DebugMesh {
        std::vector<FVector> Vertices;
        std::vector<uint32_t> Indices;
        ImColor Color;
    };
    std::vector<DebugMesh> GetDebugMeshes();

    // Thong ke hieu nang
    struct Stats {
        uint32_t totalActors = 0;
        uint32_t cacheHits = 0;
        uint32_t cacheMisses = 0;
        uint32_t totalRaycasts = 0;
        uint32_t raycastHits = 0;
        float lastCookTimeMs = 0.0f;
    };
    Stats GetStats() const { std::lock_guard<std::mutex> lock(sceneMutex); return stats; }

private:
    physx::PxDefaultAllocator      gAllocator;
    physx::PxDefaultErrorCallback  gErrorCallback;
    physx::PxFoundation*           gFoundation;
    physx::PxPhysics*              gPhysics;
    physx::PxCooking*              gCooking;
    physx::PxScene*                gScene;
    physx::PxMaterial*             gMaterial;
    
    mutable std::mutex              sceneMutex;
    std::unordered_map<uint64_t, physx::PxRigidStatic*> actorMap;

    // Cooking Cache: luu tru PxTriangleMesh da duoc cook de tai su dung
    std::unordered_map<uint64_t, physx::PxTriangleMesh*> cookingCache;

    // Actor -> collision group mapping
    std::unordered_map<uint64_t, uint32_t> actorGroupMap;

    // Performance stats
    Stats stats;
};

extern PhysXManager physxMgr;
