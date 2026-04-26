#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <diagnostic_node/GNames.h>
#include <Utils/FNVHash.h>
#include <Common/Offset.h>
#include <unordered_set>

class Actors
{
public:
    static void Update()
    {
        Throttler Throttlered;

        uint64_t lastActorArrayPtr  = 0;
        uint32_t lastActorCount     = 0;
        std::unordered_map<uint64_t, ActorEntityInfo> CacheEntitys;
        std::vector<uint64_t> actors;
        
        // Cumulative categorization maps
        std::unordered_map<uint64_t, Player>             CumulativePlayers;
        std::unordered_map<uint64_t, VehicleInfo>        CumulativeVehicles;
        std::unordered_map<uint64_t, DroppedItemInfo>    CumulativeDroppedItems;
        std::unordered_map<uint64_t, DroppedItemGroupInfo> CumulativeDroppedItemGroups;
        std::unordered_map<uint64_t, ProjectInfo>        CumulativeProjects;
        std::unordered_map<uint64_t, PackageInfo>        CumulativePackages;

        size_t currentScanIndex = 0;
        uint64_t lastFullRefreshTick = 0;

        while (true)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            uint64_t currentTick = GetTickCount64();

            if (GameData.Scene != Scene::Gaming)
            {
                CacheEntitys.clear();
                CumulativePlayers.clear();
                CumulativeVehicles.clear();
                CumulativeDroppedItems.clear();
                CumulativeDroppedItemGroups.clear();
                CumulativeProjects.clear();
                CumulativePackages.clear();
                lastActorArrayPtr = 0;
                lastActorCount    = 0;
                actors.clear();
                currentScanIndex = 0;
                GameData.Performance.ActorThreadMs = 0.0f;
                Sleep(GameData.ThreadSleep);
                continue;
            }

            // 1. Refresh Actor List periodically (Every 2s) or if empty
            if (currentTick - lastFullRefreshTick > 2000 || actors.empty()) {
                auto hScatter = mem.CreateScatterHandle();
                uint64_t actorDataPtr = 0;
                int32_t  actorCount   = 0;
                mem.AddScatterRead(hScatter, GameData.ActorArray,       &actorDataPtr);
                mem.AddScatterRead(hScatter, GameData.ActorArray + 0x8,  &actorCount);
                mem.ExecuteReadScatter(hScatter);
                mem.CloseScatterHandle(hScatter);

                if (actorCount > 0 && actorCount <= 3000 && Utils::ValidPtr(actorDataPtr)) {
                    if (actorCount > 3000) actorCount = 3000;
                    
                    if (actorCount != (int)lastActorCount || actorDataPtr != lastActorArrayPtr) {
                        actors.assign(actorCount, 0); 
                        mem.Read(actorDataPtr, actors.data(), sizeof(uint64_t) * actorCount);
                        lastActorCount = actorCount;
                        lastActorArrayPtr = actorDataPtr;
                        // Reset cumulative on array change to avoid stale data
                        CumulativePlayers.clear();
                        CumulativeVehicles.clear();
                        CumulativeDroppedItems.clear();
                        CumulativeDroppedItemGroups.clear();
                        CumulativeProjects.clear();
                        CumulativePackages.clear();
                    }
                }
                lastFullRefreshTick = currentTick;
                currentScanIndex = 0;
            }

            if (actors.empty()) {
                Sleep(100);
                continue;
            }

            // 2. Incremental Batch Scan (Process 200 actors per iteration)
            const size_t batchSize = 200;
            size_t endIdx = (std::min)(currentScanIndex + batchSize, actors.size());
            
            std::vector<ActorEntityInfo> batchEntitys;
            for (size_t i = currentScanIndex; i < endIdx; ++i) {
                uint64_t Actor = actors[i];
                if (Actor == 0) continue;
                
                auto it = CacheEntitys.find(Actor);
                if (it != CacheEntitys.end()) {
                    batchEntitys.push_back(it->second);
                } else {
                    ActorEntityInfo Entity; Entity.Entity = Actor; Entity.ID = 0; Entity.DecodeID = 0;
                    batchEntitys.push_back(Entity);
                }
            }

            // Scatter Read IDs for new ones in batch
            auto hBatch = mem.CreateScatterHandle();
            bool hasNewIDs = false;
            for (auto& Entity : batchEntitys) {
                if (Entity.ID == 0) {
                    mem.AddScatterRead(hBatch, Entity.Entity + Offset::ObjID, (int*)&Entity.ID);
                    hasNewIDs = true;
                }
            }
            if (hasNewIDs) mem.ExecuteReadScatter(hBatch);
            mem.CloseScatterHandle(hBatch);

            // Process Batch
            std::vector<int> NeedGetNameIDs;
            {
                std::shared_lock<std::shared_mutex> gnameLock(Data::GetGNameListsMutex());
                const auto& gnameLists = Data::GetGNameListsByIDRef();

                for (ActorEntityInfo& Entity : batchEntitys) {
                    if (Entity.ID != 0 && Entity.DecodeID == 0) Entity.DecodeID = Decrypt::CIndex(Entity.ID);
                    if (Entity.DecodeID == 0) continue;

                    if (Entity.EntityInfo.ID == 0) {
                        auto it = gnameLists.find(Entity.DecodeID);
                        if (it != gnameLists.end()) {
                            Entity.EntityInfo = it->second;
                        } else {
                            NeedGetNameIDs.push_back(Entity.DecodeID);
                            continue;
                        }
                    }
                    
                    CacheEntitys[Entity.Entity] = Entity;
                    const auto& info = Entity.EntityInfo;
                    
                    if (info.Type == EntityType::Player || info.Type == EntityType::AI) {
                        Player p; p.Type = info.Type; p.Entity = Entity.Entity; p.ObjID = Entity.DecodeID;
                        CumulativePlayers[p.Entity] = p;
                    } else if (info.Type == EntityType::Vehicle) {
                        VehicleInfo v; v.Type = info.Type; v.Entity = Entity.Entity; v.ObjID = Entity.DecodeID; v.Name = info.DisplayName; v.ClassName = info.Name;
                        CumulativeVehicles[v.Entity] = v;
                    } else if (info.Type == EntityType::Project) {
                        ProjectInfo prj; prj.Type = info.Type; prj.Entity = Entity.Entity; prj.ID = Entity.DecodeID; prj.Name = info.DisplayName; prj.EntityName = info.Name;
                        CumulativeProjects[prj.Entity] = prj;
                    } else if (info.Type == EntityType::DroppedItem) {
                        DroppedItemInfo item; item.Type = info.Type; item.Entity = Entity.Entity; item.ID = Entity.DecodeID;
                        CumulativeDroppedItems[item.Entity] = item;
                    } else if (info.Type == EntityType::DroppedItemGroup) {
                        DroppedItemGroupInfo group; group.Type = info.Type; group.Entity = Entity.Entity; group.ID = Entity.DecodeID;
                        CumulativeDroppedItemGroups[group.Entity] = group;
                    } else if (info.Type == EntityType::DeadBox || info.Type == EntityType::AirDrop) {
                        PackageInfo pkg; pkg.Type = info.Type; pkg.Entity = Entity.Entity; pkg.ID = Entity.DecodeID; pkg.Name = info.DisplayName;
                        CumulativePackages[pkg.Entity] = pkg;
                    }
                }
            }

            // Update scan index
            currentScanIndex = endIdx;
            if (currentScanIndex >= actors.size()) currentScanIndex = 0;

            // 3. Periodic Garbage Collection (Every 10s)
            static uint64_t lastCleanTickGC = 0;
            if (currentTick - lastCleanTickGC > 10000) {
                lastCleanTickGC = currentTick;
                std::unordered_set<uint64_t> currentActorsSet(actors.begin(), actors.end());
                
                auto cleanMap = [&](auto& m) {
                    for (auto it = m.begin(); it != m.end(); ) {
                        if (currentActorsSet.find(it->first) == currentActorsSet.end()) it = m.erase(it);
                        else ++it;
                    }
                };
                cleanMap(CacheEntitys);
                cleanMap(CumulativePlayers);
                cleanMap(CumulativeVehicles);
                cleanMap(CumulativeDroppedItems);
                cleanMap(CumulativeDroppedItemGroups);
                cleanMap(CumulativeProjects);
                cleanMap(CumulativePackages);
            }

            // Sync with global state
            Data::SetCachePlayers(CumulativePlayers);
            Data::SetCacheVehicles(CumulativeVehicles);
            Data::SetCacheDroppedItems(CumulativeDroppedItems);
            Data::SetCacheDroppedItemGroups(CumulativeDroppedItemGroups);
            Data::SetCacheProjects(CumulativeProjects);
            Data::SetCachePackages(CumulativePackages);

            if (!NeedGetNameIDs.empty()) {
                std::sort(NeedGetNameIDs.begin(), NeedGetNameIDs.end());
                NeedGetNameIDs.erase(std::unique(NeedGetNameIDs.begin(), NeedGetNameIDs.end()), NeedGetNameIDs.end());
                if (NeedGetNameIDs.size() > 50) NeedGetNameIDs.resize(50);
                std::vector<int> localIDs = NeedGetNameIDs;
                Throttlered.executeTask(skCrypt("ReadGNames"), std::chrono::milliseconds(1000), [localIDs] {
                    std::thread([localIDs]() { GNames::ReadGNames(const_cast<std::vector<int>&>(localIDs)); }).detach();
                });
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            GameData.Performance.ActorThreadMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
            
            // Fast loop for incremental updates (10ms sleep)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};