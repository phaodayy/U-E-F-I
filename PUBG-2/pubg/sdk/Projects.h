#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/GNames.h>
#include <Common/Offset.h>

class Projects
{
public:
    static void Update()
    {
        auto hScatter = mem.CreateScatterHandle();
        std::unordered_map<uint64_t, float> Times;
        Throttler Throttlered;
        while (true)
        {
            if (GameData.Scene != Scene::Gaming)
            {
                Times.clear();
                Data::SetCacheProjects({});
                Sleep(GameData.ThreadSleep);
                continue;
            }

            Throttlered.executeTaskWithSleep("ProjectsUpdateSleep", std::chrono::milliseconds(4), [] {});

            std::unordered_map<uint64_t, ProjectInfo> CacheProjects = Data::GetCacheProjects();

            if (CacheProjects.size() > 0)
            {
                std::vector<ProjectInfo*> activeProjects;
                for (auto& Item : CacheProjects) {
                    activeProjects.push_back(&Item.second);
                }

                if (activeProjects.size() > 800) activeProjects.resize(800);

                const size_t batchSize = 250;
                for (size_t i = 0; i < activeProjects.size(); i += batchSize) {
                    size_t end = (std::min)(i + batchSize, activeProjects.size());

                    for (size_t j = i; j < end; ++j) {
                        auto* CacheProject = activeProjects[j];
                        mem.AddScatterRead(hScatter, CacheProject->Entity + Offset::RootComponent, (uint64_t*)&CacheProject->RootComponent);
                        mem.AddScatterRead(hScatter, CacheProject->Entity + Offset::TimeTillExplosion, (float*)&CacheProject->TimeTillExplosion);
                        mem.AddScatterRead(hScatter, CacheProject->Entity + Offset::ExplodeState, (EProjectileExplodeState*)&CacheProject->ExplodeState);
                    }
                    mem.ExecuteReadScatter(hScatter);

                    for (size_t j = i; j < end; ++j) {
                        auto* CacheProject = activeProjects[j];
                        if (CacheProject->bVisible == 1) continue;

                        if (CacheProject->EntityName != "ProjGrenade_C")
                        {
                            if (abs(CacheProject->TimeTillExplosion) > 0.0f && CacheProject->TimeTillExplosion == Times[CacheProject->Entity])
                            {
                                CacheProject->bVisible = 1;
                                continue;
                            }
                            else {
                                Times[CacheProject->Entity] = CacheProject->TimeTillExplosion;
                            }
                        }

                        CacheProject->RootComponent = Decrypt::Xe(CacheProject->RootComponent);
                        mem.AddScatterRead(hScatter, CacheProject->RootComponent + Offset::ComponentLocation, (FVector*)&CacheProject->Location);
                    }
                    mem.ExecuteReadScatter(hScatter);
                }

                for (auto& Item : CacheProjects)
                {
                    auto& CacheProject = Item.second;
                    CacheProject.ScreenLocation = VectorHelper::WorldToScreen(CacheProject.Location);
                    CacheProject.Distance = Data::GetCamera().Location.Distance(CacheProject.Location) / 100.0f;
                }
            }

            Data::SetProjects(std::move(CacheProjects));
        }
        mem.CloseScatterHandle(hScatter);
    }
};