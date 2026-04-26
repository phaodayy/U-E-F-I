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
#include <cooking/PxCooking.h>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxRigidActorExt.h>

#undef requires

#include <Common/Data.h>
#include <Common/Entitys.h>
#include <imgui/imgui.h>

class PhysXManager {
public:
    PhysXManager() : gFoundation(nullptr), gPhysics(nullptr), gCooking(nullptr), gScene(nullptr), gMaterial(nullptr) {}
    
    bool Init();
    void Cleanup();

    // Raycast kiem tra va cham (Visible Check)
    bool Raycast(const FVector& origin, const FVector& target);

    // Them Mesh vao Scene (Nhu PAOD)
    void UpdateScene(const std::vector<TriangleMeshData>& meshes, const std::set<PrunerPayload>& removeObjects);

    // Xuat toan bo Scene ra file .obj de debug dia hinh
    void ExportSceneToObj(const std::string& filename);

    struct DebugMesh {
        std::vector<FVector> Vertices;
        std::vector<uint32_t> Indices;
        ImColor Color;
    };
    std::vector<DebugMesh> GetDebugMeshes();

private:
    physx::PxDefaultAllocator      gAllocator;
    physx::PxDefaultErrorCallback  gErrorCallback;
    physx::PxFoundation*           gFoundation;
    physx::PxPhysics*              gPhysics;
    physx::PxCooking*              gCooking;
    physx::PxScene*                gScene;
    physx::PxMaterial*             gMaterial;
    
    std::mutex              sceneMutex;
    std::unordered_map<uint64_t, physx::PxRigidStatic*> actorMap; // Luu tru cac mesh trong scene
};

extern PhysXManager physxMgr;
