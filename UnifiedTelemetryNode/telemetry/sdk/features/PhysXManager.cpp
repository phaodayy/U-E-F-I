#include <vector>
#include <mutex>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <chrono>

// Fix C++20 "requires" keyword conflict with PhysX
#define requires _requires_fix

#include "PhysXManager.h"
#include <PxPhysicsAPI.h>
#include <imgui/imgui.h>

#undef requires

using namespace physx;

PhysXManager physxMgr;

namespace {
    constexpr float kPhysxUnitsPerUnrealUnit = 0.01f;
    constexpr float kMinRayDistanceMeters = 0.01f;
    constexpr float kMaxRayDistanceMeters = 20000.0f;
    constexpr size_t kMaxMeshVertices = 250000;
    constexpr size_t kMaxMeshIndices = 750000;

    bool IsFiniteFloat(float value) {
        return std::isfinite(value);
    }

    bool IsFiniteVector(const FVector& value) {
        return IsFiniteFloat(value.X) && IsFiniteFloat(value.Y) && IsFiniteFloat(value.Z);
    }

    bool IsFiniteVector(const Vector3& value) {
        return IsFiniteFloat(value.x) && IsFiniteFloat(value.y) && IsFiniteFloat(value.z);
    }

    bool SceneLooksUsable(PxScene* scene) {
        if (!scene) return false;
        __try {
            const PxSceneFlags flags = scene->getFlags();
            (void)flags;
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    bool SafePhysxRaycast(PxScene* scene,
                          const PxVec3& origin,
                          const PxVec3& direction,
                          PxReal distance,
                          PxRaycastBuffer* hit,
                          bool* outStatus) {
        if (!scene || !hit || !outStatus) return false;
        __try {
            *outStatus = scene->raycast(origin, direction, distance, *hit);
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            *outStatus = false;
            return false;
        }
    }

    bool SafeRemoveAndReleaseActor(PxScene* scene, PxRigidStatic* actor) {
        if (!actor) return true;
        __try {
            if (scene && SceneLooksUsable(scene)) {
                scene->removeActor(*actor);
            }
            actor->release();
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    bool SafeCookTriangleMesh(PxCooking* cooking,
                              const PxTriangleMeshDesc* meshDesc,
                              PxDefaultMemoryOutputStream* writeBuffer) {
        if (!cooking || !meshDesc || !writeBuffer) return false;
        __try {
            return cooking->cookTriangleMesh(*meshDesc, *writeBuffer);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
    }

    bool IsValidMeshData(const TriangleMeshData& meshData) {
        if (meshData.Vertices.empty() || meshData.Indices.empty()) return false;
        if ((meshData.Indices.size() % 3) != 0) return false;
        if (meshData.Vertices.size() > kMaxMeshVertices || meshData.Indices.size() > kMaxMeshIndices) return false;

        for (const Vector3& vertex : meshData.Vertices) {
            if (!IsFiniteVector(vertex)) return false;
        }

        const uint32_t vertexCount = static_cast<uint32_t>(meshData.Vertices.size());
        for (uint32_t index : meshData.Indices) {
            if (index >= vertexCount) return false;
        }

        return true;
    }

    // Hash cho mesh geometry: dua tren so dinh + so index + mau diem dau
    uint64_t ComputeMeshHash(const TriangleMeshData& meshData) {
        uint64_t hash = meshData.Vertices.size() * 0x100000001B3ULL;
        hash ^= meshData.Indices.size() * 0x1337ULL;
        if (!meshData.Vertices.empty()) {
            auto& v = meshData.Vertices[0];
            uint32_t vx = *reinterpret_cast<const uint32_t*>(&v.x);
            uint32_t vy = *reinterpret_cast<const uint32_t*>(&v.y);
            uint32_t vz = *reinterpret_cast<const uint32_t*>(&v.z);
            hash ^= ((uint64_t)vx << 32) | vy;
            hash ^= (uint64_t)vz * 0xDEADBEEFULL;
        }
        if (meshData.Vertices.size() > 1) {
            auto& v = meshData.Vertices[meshData.Vertices.size() / 2];
            uint32_t vx = *reinterpret_cast<const uint32_t*>(&v.x);
            hash ^= (uint64_t)vx * 0xCAFEBABEULL;
        }
        return hash;
    }
}

void PhysXManager::ExportSceneToObj(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "# PhysX Scene Export\n";
    file << "# Actors: " << actorMap.size() << " | Cache entries: " << cookingCache.size() << "\n";
    
    std::lock_guard<std::mutex> lock(sceneMutex);
    if (!SceneLooksUsable(gScene)) return;
    int vertexOffset = 1;

    for (auto const& item : actorMap) {
        PxRigidStatic* actor = item.second;
        if (!actor) continue;

        PxU32 nbShapes = actor->getNbShapes();
        std::vector<PxShape*> shapes(nbShapes);
        actor->getShapes(shapes.data(), nbShapes);

        for (PxShape* shape : shapes) {
            PxGeometryHolder geom = shape->getGeometry();
            if (geom.getType() == physx::PxGeometryType::eTRIANGLEMESH) {
                const PxTriangleMeshGeometry& triGeom = geom.triangleMesh();
                PxTriangleMesh* triMesh = triGeom.triangleMesh;
                if (!triMesh) continue;

                PxTransform pose = actor->getGlobalPose();

                // Ghi cac dinh (Vertices)
                PxU32 nbVerts = triMesh->getNbVertices();
                const PxVec3* verts = triMesh->getVertices();
                for (PxU32 i = 0; i < nbVerts; i++) {
                    PxVec3 v = pose.transform(verts[i]);
                    file << "v " << v.x * 100.0f << " " << v.y * 100.0f << " " << v.z * 100.0f << "\n";
                }

                // Ghi cac mat (Faces)
                PxU32 nbTris = triMesh->getNbTriangles();
                const void* indices = triMesh->getTriangles();
                bool is16Bit = triMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;

                for (PxU32 i = 0; i < nbTris; i++) {
                    uint32_t i0, i1, i2;
                    if (is16Bit) {
                        const uint16_t* idx = (const uint16_t*)indices;
                        i0 = idx[i * 3 + 0]; i1 = idx[i * 3 + 1]; i2 = idx[i * 3 + 2];
                    } else {
                        const uint32_t* idx = (const uint32_t*)indices;
                        i0 = idx[i * 3 + 0]; i1 = idx[i * 3 + 1]; i2 = idx[i * 3 + 2];
                    }
                    file << "f " << i0 + vertexOffset << " " << i1 + vertexOffset << " " << i2 + vertexOffset << "\n";
                }
                vertexOffset += nbVerts;
            }
        }
    }
    file.close();
    Utils::Log(1, "[PhysX] Da xuat Scene ra tep: %s", filename.c_str());
}

bool PhysXManager::Init() {
    if (gFoundation && gPhysics && gCooking && gScene && gMaterial && SceneLooksUsable(gScene)) {
        return true;
    }

    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) return false;

    PxTolerancesScale scale;
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale);
    if (!gPhysics) {
        Cleanup();
        return false;
    }

    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(scale));
    if (!gCooking) {
        Cleanup();
        return false;
    }

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, -981.0f); 
    sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);
    if (!gScene || !gMaterial) {
        Cleanup();
        return false;
    }

    stats = {};
    return true;
}

void PhysXManager::Cleanup() {
    std::lock_guard<std::mutex> lock(sceneMutex);

    for (auto& item : actorMap) {
        SafeRemoveAndReleaseActor(gScene, item.second);
    }
    actorMap.clear();
    actorGroupMap.clear();

    // Release cooking cache (PxTriangleMesh chi can release 1 lan)
    for (auto& item : cookingCache) {
        if (item.second) {
            __try { item.second->release(); } __except(EXCEPTION_EXECUTE_HANDLER) {}
        }
    }
    cookingCache.clear();

    if (gMaterial) { gMaterial->release(); gMaterial = nullptr; }
    if (gScene) { gScene->release(); gScene = nullptr; }
    if (gCooking) { gCooking->release(); gCooking = nullptr; }
    if (gPhysics) { gPhysics->release(); gPhysics = nullptr; }
    if (gFoundation) { gFoundation->release(); gFoundation = nullptr; }

    stats = {};
}

bool PhysXManager::Raycast(const FVector& origin, const FVector& target, uint32_t collisionMask) {
    FVector hitPoint{};
    return !RaycastHit(origin, target, hitPoint, collisionMask);
}

bool PhysXManager::RaycastHit(const FVector& origin, const FVector& target, FVector& hitPoint, uint32_t collisionMask) {
    if (!IsFiniteVector(origin) || !IsFiniteVector(target)) return false;

    PxVec3 pxOrigin(origin.X * kPhysxUnitsPerUnrealUnit,
        origin.Y * kPhysxUnitsPerUnrealUnit,
        origin.Z * kPhysxUnitsPerUnrealUnit);
    PxVec3 pxTarget(target.X * kPhysxUnitsPerUnrealUnit,
        target.Y * kPhysxUnitsPerUnrealUnit,
        target.Z * kPhysxUnitsPerUnrealUnit);

    PxVec3 dir = pxTarget - pxOrigin;
    PxReal dist = dir.magnitude();
    if (!std::isfinite(dist) || dist < kMinRayDistanceMeters || dist > kMaxRayDistanceMeters) return false;
    dir.normalize();

    PxRaycastBuffer hit;
    std::lock_guard<std::mutex> lock(sceneMutex);
    if (!SceneLooksUsable(gScene)) return false;

    stats.totalRaycasts++;

    bool status = false;
    if (!SafePhysxRaycast(gScene, pxOrigin, dir, dist, &hit, &status)) {
        return false;
    }

    if (status && hit.hasBlock && collisionMask != PhysXCollisionGroup::GROUP_ALL) {
        // Kiem tra collision group cua actor bi trung
        PxRigidActor* hitActor = hit.block.actor;
        if (hitActor) {
            uint64_t actorAddr = reinterpret_cast<uint64_t>(hitActor);
            auto it = actorGroupMap.find(actorAddr);
            if (it != actorGroupMap.end()) {
                if ((it->second & collisionMask) == 0) {
                    // Actor nay thuoc nhom bi bo qua (vd: Foliage)
                    return false; // Coi nhu khong bi chan
                }
            }
        }
        stats.raycastHits++;
    }

    if (status && hit.hasBlock) {
        hitPoint = FVector(
            hit.block.position.x / kPhysxUnitsPerUnrealUnit,
            hit.block.position.y / kPhysxUnitsPerUnrealUnit,
            hit.block.position.z / kPhysxUnitsPerUnrealUnit);
        return true;
    }

    return false;
}

void PhysXManager::UpdateScene(const std::vector<TriangleMeshData>& meshes, const std::set<PrunerPayload>& removeObjects) {
    if (!gScene || !gCooking || !gPhysics || !gMaterial) return;

    std::lock_guard<std::mutex> lock(sceneMutex);
    if (!SceneLooksUsable(gScene)) return;

    // Xoa cac actor cu
    for (auto const& payload : removeObjects) {
        if (actorMap.count(payload.Actor)) {
            PxRigidStatic* actor = actorMap[payload.Actor];
            uint64_t actorAddr = reinterpret_cast<uint64_t>(actor);
            actorGroupMap.erase(actorAddr);
            SafeRemoveAndReleaseActor(gScene, actor);
            actorMap.erase(payload.Actor);
            stats.totalActors--;
        }
    }

    // Them cac mesh moi (voi Cooking Cache)
    for (auto const& meshData : meshes) {
        if (actorMap.count(meshData.UniqueKey2)) continue;
        if (!IsValidMeshData(meshData)) continue;

        // Phan loai collision group tu FilterData
        uint32_t group = PhysXCollisionGroup::ClassifyFromFilterData(
            meshData.QueryFilterData.word0,
            meshData.QueryFilterData.word3,
            meshData.SimulationFilterData.word0
        );

        // Bo qua Foliage hoan toan (khong can them vao PhysX scene)
        if (group == PhysXCollisionGroup::GROUP_FOLIAGE) continue;

        // Cooking Cache: kiem tra xem mesh nay da duoc cook truoc do chua
        uint64_t meshHash = ComputeMeshHash(meshData);
        PxTriangleMesh* triMesh = nullptr;

        auto cacheIt = cookingCache.find(meshHash);
        if (cacheIt != cookingCache.end()) {
            // Cache HIT: tai su dung mesh da cook
            triMesh = cacheIt->second;
            stats.cacheHits++;
        } else {
            // Cache MISS: cook mesh moi
            auto cookStart = std::chrono::high_resolution_clock::now();

            PxTriangleMeshDesc meshDesc;
            meshDesc.points.count = (PxU32)meshData.Vertices.size();
            meshDesc.points.stride = sizeof(Vector3);
            meshDesc.points.data = meshData.Vertices.data();

            meshDesc.triangles.count = (PxU32)meshData.Indices.size() / 3;
            meshDesc.triangles.stride = 3 * sizeof(uint32_t);
            meshDesc.triangles.data = meshData.Indices.data();

            PxDefaultMemoryOutputStream writeBuffer;
            bool cooked = SafeCookTriangleMesh(gCooking, &meshDesc, &writeBuffer);

            if (cooked) {
                PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
                triMesh = gPhysics->createTriangleMesh(readBuffer);
                if (triMesh) {
                    cookingCache[meshHash] = triMesh;
                }
            }

            auto cookEnd = std::chrono::high_resolution_clock::now();
            stats.lastCookTimeMs = std::chrono::duration<float, std::milli>(cookEnd - cookStart).count();
            stats.cacheMisses++;
        }

        if (triMesh) {
            PxTriangleMeshGeometry geom(triMesh);
            PxRigidStatic* actor = PxCreateStatic(*gPhysics, PxTransform(PxIdentity), geom, *gMaterial);
            if (actor) {
                gScene->addActor(*actor);
                actorMap[meshData.UniqueKey2] = actor;
                actorGroupMap[reinterpret_cast<uint64_t>(actor)] = group;
                stats.totalActors++;
            }
        }
    }
}

std::vector<PhysXManager::DebugMesh> PhysXManager::GetDebugMeshes() {
    std::vector<DebugMesh> debugMeshes;
    if (!gScene) return debugMeshes;

    std::lock_guard<std::mutex> lock(sceneMutex);
    if (!SceneLooksUsable(gScene)) return debugMeshes;

    for (auto const& item : actorMap) {
        PxRigidStatic* actor = item.second;
        if (!actor) continue;

        // Mau sac theo collision group
        uint64_t actorAddr = reinterpret_cast<uint64_t>(actor);
        uint32_t group = PhysXCollisionGroup::GROUP_STATIC;
        auto groupIt = actorGroupMap.find(actorAddr);
        if (groupIt != actorGroupMap.end()) group = groupIt->second;

        ImColor color;
        switch (group) {
            case PhysXCollisionGroup::GROUP_STATIC:  color = ImColor(0, 255, 255, 100); break;
            case PhysXCollisionGroup::GROUP_VEHICLE:  color = ImColor(255, 165, 0, 100); break;
            case PhysXCollisionGroup::GROUP_TERRAIN:  color = ImColor(0, 200, 0, 80); break;
            case PhysXCollisionGroup::GROUP_DYNAMIC:  color = ImColor(255, 255, 0, 100); break;
            case PhysXCollisionGroup::GROUP_FOLIAGE:  color = ImColor(0, 100, 0, 50); break;
            default: color = ImColor(255, 255, 255, 80); break;
        }

        PxU32 nbShapes = actor->getNbShapes();
        std::vector<PxShape*> shapes(nbShapes);
        actor->getShapes(shapes.data(), nbShapes);

        for (PxShape* shape : shapes) {
            PxGeometryHolder geom = shape->getGeometry();
            if (geom.getType() == physx::PxGeometryType::eTRIANGLEMESH) {
                const PxTriangleMeshGeometry& triGeom = geom.triangleMesh();
                PxTriangleMesh* triMesh = triGeom.triangleMesh;
                if (!triMesh) continue;

                DebugMesh dm;
                dm.Color = color;

                PxU32 nbVerts = triMesh->getNbVertices();
                const PxVec3* verts = triMesh->getVertices();
                for (PxU32 i = 0; i < nbVerts; i++) {
                    dm.Vertices.push_back({verts[i].x * 100.0f, verts[i].y * 100.0f, verts[i].z * 100.0f});
                }

                PxU32 nbTris = triMesh->getNbTriangles();
                const void* indices = triMesh->getTriangles();
                bool is16Bit = triMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES;

                for (PxU32 i = 0; i < nbTris * 3; i++) {
                    if (is16Bit)
                        dm.Indices.push_back(((const uint16_t*)indices)[i]);
                    else
                        dm.Indices.push_back(((const uint32_t*)indices)[i]);
                }
                debugMeshes.push_back(dm);
            }
        }
    }
    return debugMeshes;
}
