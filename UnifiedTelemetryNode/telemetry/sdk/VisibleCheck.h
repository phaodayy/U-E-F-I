#pragma once
#include <Common/VisibleScene.h>
#include <Common/Data.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <mutex>
#include <memory>
#include <thread>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <diagnostic_node/PhysXManager.h>
#include <Common/Offset.h>

namespace VisibleCheck {

    // --- NEW: Engine-based Visible Check (Lightweight & High Performance) ---
    // Logic from bfmonkey: (enemyEyes + 0.05f >= localPlayerEyes)
    // Actor + 0x488 = Mesh (Raw) -> Mesh + 0x75C = Eyes timestamp (float)
    inline bool IsVisibleByRenderTime(uint64_t entityPtr) {
        if (!Utils::ValidPtr(entityPtr)) return false;

        // 1. Get Enemy Mesh from Actor (Actor + 0x488) - Mesh is a raw pointer
        uint64_t meshPtr = mem.Read<uint64_t>(entityPtr + 0x488);
        if (!Utils::ValidPtr(meshPtr)) return false;

        // 2. Read LastRenderTimeOnScreen (Mesh + 0x75C) as float
        float enemyEyes = mem.Read<float>(meshPtr + 0x75C);

        // 3. Get LocalPlayer Mesh for comparison (Pawn + 0x488)
        uint64_t localPawn = GameData.AcknowledgedPawn;
        if (!Utils::ValidPtr(localPawn)) return true; // Fail-safe: Thấy nếu lỗi local pawn

        uint64_t localMeshPtr = mem.Read<uint64_t>(localPawn + 0x488);
        if (!Utils::ValidPtr(localMeshPtr)) return true;

        float localPlayerEyes = mem.Read<float>(localMeshPtr + 0x75C);

        // 4. Comparison Logic: enemyEyes + 0.05f >= localPlayerEyes
        return (enemyEyes + 0.05f >= localPlayerEyes);
    }

    inline void UpdateSceneByRange() {
        Throttler throttler;
        std::unordered_map<PrunerPayload, PxTransformT, PrunerPayloadHash> cache;
        std::set<PrunerPayload> currentSceneObjects;
        std::unordered_map<PrunerPayload, uint64_t, PrunerPayloadHash> alwaysCheckShape;
        uint32_t lastUpdateTimestamp = 0;
        std::set<PrunerPayload> willRemoveObjects;
        FVector currentPosition;

        while (GameData.Scene == Scene::Gaming) {
            auto px_instance_ptr = mem.Read<uint64_t>(GameData.GameBase + Offset::Physx);
            auto px_scene_arr_ptr = mem.Read<uint64_t>(px_instance_ptr + 0x8);
            auto px_scene_ptr = mem.Read<uint64_t>(px_scene_arr_ptr);

            if (!Utils::ValidPtr(px_scene_ptr)) {
                static uint64_t last_log_time = 0;
                if (GetTickCount64() - last_log_time > 5000) {
                    Utils::Log(2, "[PHYSX] Scene not found at 0x%llX", GameData.GameBase + Offset::Physx);
                    last_log_time = GetTickCount64();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            throttler.executeTaskWithSleep(
                "UpdateSceneByRangeSleep",
                std::chrono::milliseconds(GameData.Config.signal_overlay.PhysxStaticRefreshInterval),
                [&]() {
                    auto start = std::chrono::high_resolution_clock::now();
                    
                    CameraData cam = Data::GetCamera();
                    if (cam.Location.IsNearlyZero() && cam.FOV == 0.f) return;
                    currentPosition = cam.Location + GameData.Radar.WorldOriginLocation;

                    auto meshes = GamePhysX::LoadShapeByRange(
                        lastUpdateTimestamp, cache, currentSceneObjects, 
                        willRemoveObjects, alwaysCheckShape, 
                        { currentPosition.X, currentPosition.Y, currentPosition.Z },
                        GameData.Config.signal_overlay.PhysxLoadRadius * 100.f,
                        GameData.Config.signal_overlay.PhysxRefreshLimit
                    );

                    if (!meshes.empty() || !willRemoveObjects.empty()) {
                        static uint64_t last_mesh_log = 0;
                        if (GetTickCount64() - last_mesh_log > 3000) {
                            Utils::Log(1, "[PHYSX] Cloned %d new meshes (Total: %d)", meshes.size(), GameData.Actors.ClonedMapMeshes.size());
                            last_mesh_log = GetTickCount64();
                        }
                        if (GameData.DynamicLoadScene) {
                            GameData.DynamicLoadScene->UpdateMesh(meshes, willRemoveObjects);
                            physxMgr.UpdateScene(meshes, willRemoveObjects);
                        }

                        // [MAP CLONER DEBUG]: Update ClonedMapMeshes
                        {
                            std::unique_lock<std::shared_mutex> lock(GameData.Actors.ClonedMapMutex);
                            // Remove objects
                            for (const auto& payload : willRemoveObjects) {
                                GameData.Actors.ClonedMapMeshes.erase(
                                    std::remove_if(GameData.Actors.ClonedMapMeshes.begin(), GameData.Actors.ClonedMapMeshes.end(),
                                        [&](const TriangleMeshData& m) { return m.UniqueKey1 == payload; }),
                                    GameData.Actors.ClonedMapMeshes.end());
                            }
                            // Add new objects
                            for (const auto& mesh : meshes) {
                                GameData.Actors.ClonedMapMeshes.push_back(mesh);
                            }
                        }
                        
                        willRemoveObjects.clear();
                    }
                });
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    inline void UpdateDynamicRigid() {
        Throttler throttler;
        std::unordered_map<PrunerPayload, PxTransformT, PrunerPayloadHash> cache;
        std::set<PrunerPayload> currentSceneObjects;
        std::unordered_map<PrunerPayload, uint64_t, PrunerPayloadHash> alwaysCheckShape;
        std::set<PrunerPayload> willRemoveObjects;
        FVector currentPosition;

        while (GameData.Scene == Scene::Gaming) {
            throttler.executeTaskWithSleep(
                "UpdateDynamicRigidSleep",
                std::chrono::milliseconds(5000), 
                [&]() {
                    CameraData cam = Data::GetCamera();
                    if (cam.Location.IsNearlyZero() && cam.FOV == 0.f) return;
                    currentPosition = cam.Location + GameData.Radar.WorldOriginLocation;
                    
                    auto meshes = GamePhysX::LoadDynamicRigidShape(
                        currentSceneObjects, cache, alwaysCheckShape, willRemoveObjects, 
                        { currentPosition.X, currentPosition.Y, currentPosition.Z }, 
                        200.f * 100.f 
                    );

                    if (!meshes.empty() || !willRemoveObjects.empty()) {
                        if (GameData.DynamicRigidScene) {
                            GameData.DynamicRigidScene->UpdateMesh(meshes, willRemoveObjects);
                        }

                        // [MAP CLONER DEBUG]: Update ClonedMapMeshes
                        {
                            std::unique_lock<std::shared_mutex> lock(GameData.Actors.ClonedMapMutex);
                            // Remove objects
                            for (const auto& payload : willRemoveObjects) {
                                GameData.Actors.ClonedMapMeshes.erase(
                                    std::remove_if(GameData.Actors.ClonedMapMeshes.begin(), GameData.Actors.ClonedMapMeshes.end(),
                                        [&](const TriangleMeshData& m) { return m.UniqueKey1 == payload; }),
                                    GameData.Actors.ClonedMapMeshes.end());
                            }
                            // Add new objects
                            for (const auto& mesh : meshes) {
                                GameData.Actors.ClonedMapMeshes.push_back(mesh);
                            }
                        }

                        willRemoveObjects.clear();
                    }
                });

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
        }
    }

    inline void UpdateDynamicHeightField() {
        Throttler throttler;
        std::set<PrunerPayload> uniqueKeySet;
        std::set<PrunerPayload> heightFieldSet;
        std::set<uint64_t> heightFieldSamplePtrSet;
        std::set<uint64_t> willRemoveHeightFieldKey;
        uint32_t lastUpdateTimestamp = 0;

        while (GameData.Scene == Scene::Gaming) {
            throttler.executeTaskWithSleep(
                "UpdateDynamicHeightFieldSleep",
                std::chrono::milliseconds(5000), 
                [&]() {
                    auto meshes = GamePhysX::RefreshDynamicLoadHeightField(
                        lastUpdateTimestamp, uniqueKeySet, heightFieldSet, 
                        heightFieldSamplePtrSet, willRemoveHeightFieldKey
                    );

                    if (!meshes.empty() || !willRemoveHeightFieldKey.empty()) {
                        if (GameData.HeightFieldScene) {
                            GameData.HeightFieldScene->UpdateMesh(meshes, willRemoveHeightFieldKey);
                        }

                        // [MAP CLONER DEBUG]: Update ClonedMapMeshes (HeightField)
                        {
                            std::unique_lock<std::shared_mutex> lock(GameData.Actors.ClonedMapMutex);
                            // HeightField removal logic is based on UniqueKey2 (sample ptr)
                            for (const auto& key : willRemoveHeightFieldKey) {
                                GameData.Actors.ClonedMapMeshes.erase(
                                    std::remove_if(GameData.Actors.ClonedMapMeshes.begin(), GameData.Actors.ClonedMapMeshes.end(),
                                        [&](const TriangleMeshData& m) { return m.UniqueKey2 == key; }),
                                    GameData.Actors.ClonedMapMeshes.end());
                            }
                            // Add new meshes
                            for (const auto& mesh : meshes) {
                                GameData.Actors.ClonedMapMeshes.push_back(mesh);
                            }
                        }

                        willRemoveHeightFieldKey.clear();
                    }
                });

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
        }
    }
}
