#include <vector>
#include <mutex>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <iostream>

// Fix C++20 "requires" keyword conflict with PhysX
#define requires _requires_fix

#include "PhysXManager.h"
#include <PxPhysicsAPI.h>
#include <imgui/imgui.h>

#undef requires

using namespace physx;

PhysXManager physxMgr;

void PhysXManager::ExportSceneToObj(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    file << "# PhysX Scene Export\n";
    
    std::lock_guard<std::mutex> lock(sceneMutex);
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
    Utils::Log(1, "[PhysX] Da xuat Scene ra tệp: %s", filename.c_str());
}

bool PhysXManager::Init() {
    static bool isInit = false;
    if (isInit) return true;

    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);
    if (!gFoundation) return false;

    PxTolerancesScale scale;
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale);
    if (!gPhysics) return false;

    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(scale));

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, 0.0f, -981.0f); 
    sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;

    gScene = gPhysics->createScene(sceneDesc);
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.1f);

    isInit = true;
    return true;
}

bool PhysXManager::Raycast(const FVector& origin, const FVector& target) {
    if (!gScene) return true;

    PxVec3 pxOrigin(origin.X / 100.0f, origin.Y / 100.0f, origin.Z / 100.0f);
    PxVec3 pxTarget(target.X / 100.0f, target.Y / 100.0f, target.Z / 100.0f);

    PxVec3 dir = pxTarget - pxOrigin;
    PxReal dist = dir.magnitude();
    if (dist < 0.01f) return true;
    dir.normalize();

    PxRaycastBuffer hit;
    std::lock_guard<std::mutex> lock(sceneMutex);
    bool status = gScene->raycast(pxOrigin, dir, dist, hit);

    return !status; 
}

void PhysXManager::UpdateScene(const std::vector<TriangleMeshData>& meshes, const std::set<PrunerPayload>& removeObjects) {
    if (!gScene || !gCooking) return;

    std::lock_guard<std::mutex> lock(sceneMutex);

    for (auto const& payload : removeObjects) {
        if (actorMap.count(payload.Actor)) {
            gScene->removeActor(*actorMap[payload.Actor]);
            actorMap[payload.Actor]->release();
            actorMap.erase(payload.Actor);
        }
    }

    for (auto const& meshData : meshes) {
        if (actorMap.count(meshData.UniqueKey2)) continue;

        PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = (PxU32)meshData.Vertices.size();
        meshDesc.points.stride = sizeof(Vector3);
        meshDesc.points.data = meshData.Vertices.data();

        meshDesc.triangles.count = (PxU32)meshData.Indices.size() / 3;
        meshDesc.triangles.stride = 3 * sizeof(uint32_t);
        meshDesc.triangles.data = meshData.Indices.data();

        PxDefaultMemoryOutputStream writeBuffer;
        if (gCooking->cookTriangleMesh(meshDesc, writeBuffer)) {
            PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
            PxTriangleMesh* triMesh = gPhysics->createTriangleMesh(readBuffer);
            
            if (triMesh) {
                PxTriangleMeshGeometry geom(triMesh);
                PxRigidStatic* actor = PxCreateStatic(*gPhysics, PxTransform(PxIdentity), geom, *gMaterial);
                if (actor) {
                    gScene->addActor(*actor);
                    actorMap[meshData.UniqueKey2] = actor;
                }
            }
        }
    }
}

std::vector<PhysXManager::DebugMesh> PhysXManager::GetDebugMeshes() {
    std::vector<DebugMesh> debugMeshes;
    if (!gScene) return debugMeshes;

    std::lock_guard<std::mutex> lock(sceneMutex);

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

                DebugMesh dm;
                dm.Color = ImColor(0, 255, 255, 100); 

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
