#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Common/Constant.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <diagnostic_node/GNames.h>
#include <diagnostic_node/Decrypt.h>
#include <algorithm>
#include <Common/Offset.h>

class Radar
{
public:
    static void UpdateWorldMapInfo()
    {
        int WorldOriginLocationX = mem.Read<int>(GameData.UWorld + Offset::WorldToMap);
        int WorldOriginLocationY = mem.Read<int>(GameData.UWorld + Offset::WorldToMap + 0x4);

        GameData.Radar.WorldOriginLocation = { (float)WorldOriginLocationX, (float)WorldOriginLocationY, 0.f };
        GameData.Radar.MapSize = MapsSize[GameData.MapName];
    }

    static bool GetMapGrid()
    {
        uint64_t BlockInputWidgetList = mem.Read<uint64_t>(GameData.MyHUD + Offset::BlockInputWidgetList);
        int BlockInputWidgetListCount = mem.Read<int>(GameData.MyHUD + Offset::BlockInputWidgetList + 0x8);
        
        if (BlockInputWidgetListCount > 0 && BlockInputWidgetListCount < 1000) {
            for (int i = 0; i < BlockInputWidgetListCount; i++)
            {
                uint64_t Widget = mem.Read<uint64_t>(BlockInputWidgetList + i * 8);
                if (!Utils::ValidPtr(Widget)) continue;

                int ID = Decrypt::CIndex(mem.Read<int>(Widget + Offset::ObjID));
                std::string WidgetName = GNames::GetNameByID(ID);

                if (WidgetName == "NewWorldMapWidget_BP_C" || 
                    WidgetName == "UI_NewWorldMapWidget_C" || 
                    WidgetName == "UI_OldErangel_NewWorldMapWidget_BP_C" ||
                    WidgetName == "UI_TrainingWorldMapWidget_C" || 
                    WidgetName == "TDM_NewWorldMapWidget_BP_C" ||
                    WidgetName == "ArcadeNewWorldMapWidget_C" ||
                    WidgetName == "WarModeWorldMapWidget_C") {
                    
                    if (CheckAndSetMapGrid(Widget, WidgetName)) return true;
                }
            }
        }

        auto WidgetStateMap = mem.Read<TMap<FString, FTslWidgetState>>(GameData.MyHUD + Offset::WidgetStateMap);
        auto elements = WidgetStateMap.GetVector();
        for (auto& Elem : elements) {
            auto& Value = Elem.Value.Value;
            if (!Utils::ValidPtr(Value.Widget)) continue; 
            
            ULONG ID = Decrypt::CIndex(mem.Read<ULONG>(Value.Widget + Offset::ObjID));
            std::string WidgetName = GNames::GetName(ID);

            std::string lowerName = WidgetName;
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

            if (lowerName.find("worldmapwidget") != std::string::npos || 
                lowerName.find("newworldmap") != std::string::npos) {
                
                if (CheckAndSetMapGrid(Value.Widget, WidgetName)) return true;
            }
        }
        return false;
    }

    static bool CheckAndSetMapGrid(uint64_t Widget, const std::string& WidgetName) {
        uint64_t FastMapGrid = mem.Read<uint64_t>(Widget + Offset::MapGrid_Map);
        if (Utils::ValidPtr(FastMapGrid)) {
            uint64_t Slot = mem.Read<uint64_t>(FastMapGrid + Offset::Slot);
            if (Utils::ValidPtr(Slot)) {
                FMargin Layout = mem.Read<FMargin>(Slot + Offset::LayoutData + Offset::Offsets);
                if (Layout.Right > 50.f && Layout.Bottom > 50.f) {
                    GameData.Radar.MapGrid = FastMapGrid;
                    GameData.Radar.MapWidget = Widget;
                    Utils::Log(1, "[RADAR] Tim thay Map Grid tai %s", WidgetName.c_str());
                    return true;
                }
            }
        }
        return false;
    }

    static bool GetMiniMapGrid() {
        auto WidgetStateMap = mem.Read<TMap<FString, FTslWidgetState>>(GameData.MyHUD + Offset::WidgetStateMap);
        auto elements = WidgetStateMap.GetVector();
        
        for (auto& Elem : elements) {
            auto& Value = Elem.Value.Value;
            if (!Utils::ValidPtr(Value.Widget)) continue; 
            
            ULONG ID = Decrypt::CIndex(mem.Read<ULONG>(Value.Widget + Offset::ObjID));
            std::string WidgetPtrName = GNames::GetName(ID);

            if (WidgetPtrName == "MinimapOriginalType_C")
            {
                GameData.Radar.MiniMapWidget = Value.Widget;
                Utils::Log(1, "[RADAR] Tim thay Mini-map Widget");
                return true;
            }
        }
        return false;
    }

    static FVector2D WorldToRadarLocation(FVector Location)
    {
        const FVector2D WorldLocation = { Location.X + GameData.Radar.WorldOriginLocation.X, Location.Y + GameData.Radar.WorldOriginLocation.Y };
        const FVector2D RadarPos = WorldLocation - GameData.Radar.WorldCenterLocation;
        FVector2D ScreenCenter = FVector2D(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2);
        return ScreenCenter + FVector2D{ RadarPos.X / GameData.Radar.MapSizeFactored * ScreenCenter.Y, RadarPos.Y / GameData.Radar.MapSizeFactored * ScreenCenter.Y };
    }

    static FVector2D WorldToMiniRadarLocation(FVector Location)
    {
        const FVector2D ScreenCenter = { (float)GameData.Config.Overlay.ScreenWidth / 2, (float)GameData.Config.Overlay.ScreenHeight / 2 };
        const FVector2D RadarSize = GameData.Radar.bRadarExtended ? GameData.Radar.MiniRadarSizeLarge : GameData.Radar.MiniRadarSizeNormal;
        const FVector2D RadarFrom = GameData.Radar.bRadarExtended ? GameData.Radar.MiniRadarFromLarge : GameData.Radar.MiniRadarFromNormal;
        const FVector MiniRadarPos = (Location - GameData.LocalPlayerInfo.Location) * 0.01f;
        FVector2D MiniRadarScreenLocation = {
            RadarFrom.X * ScreenCenter.X * 2 + ((1.0f + MiniRadarPos.X / GameData.Radar.MiniRadarDistance) * RadarSize.X / 2.0f) * ScreenCenter.X * 2,
            RadarFrom.Y * ScreenCenter.Y * 2 + ((1.0f + MiniRadarPos.Y / GameData.Radar.MiniRadarDistance) * RadarSize.Y / 2.0f) * ScreenCenter.Y * 2
        };
        return MiniRadarScreenLocation;
    }

    static void Update()
    {
        Throttler Throttlered;
        const FVector2D DefaultSize = { 1080.0f, 1080.0f };
        int SleepTime = 4;

        while (true)
        {
            auto startTime = std::chrono::high_resolution_clock::now();

            if (!GameData.Config.Radar.Enable || GameData.Scene != Scene::Gaming || GameData.PID == 0) {
                GameData.Radar.MapGrid = 0; GameData.Radar.MiniMapWidget = 0;
                GameData.Radar.Visibility = false;
                GameData.Radar.MiniRadarVisibility = false;
                GameData.Performance.RadarThreadMs = 0.0f;
                Sleep(100);
                continue;
            }
            
            if (GameData.Radar.MapSize.Size <= 0) {
                GameData.Radar.MapSize = MapsSize.count(GameData.MapName) ? MapsSize[GameData.MapName] : MapsSize["Baltic_Main"];
            }

            Throttlered.executeTaskWithSleep("RadarUpdateSleep", std::chrono::milliseconds(SleepTime), [] {});

            if (!Utils::ValidPtr(GameData.Radar.MapGrid)) Throttlered.executeTask("FindMapGrid", std::chrono::milliseconds(1000), []() { Radar::GetMapGrid(); });
            if (!Utils::ValidPtr(GameData.Radar.MiniMapWidget)) Throttlered.executeTask("FindMiniMapGrid", std::chrono::milliseconds(1000), []() { Radar::GetMiniMapGrid(); });

            // CHỈ ĐỌC DỮ LIỆU CHI TIẾT KHI BẢN ĐỒ ĐANG MỞ HOẶC MINIMAP HIỆN HÀNH
            auto hScatter = mem.CreateScatterHandle();
            uint64_t Slot = 0, FeatureRepObject = 0, FeatureRep = 0;
            int RepObjectCount = 0, SelectMinimapSizeIndex = 0;
            ESlateVisibility Visibility = ESlateVisibility::Hidden, MiniRadarVisibility = ESlateVisibility::Hidden;

            // 1. Kiểm tra trạng thái hiển thị trước (Lượt đọc cực nhẹ)
            if (Utils::ValidPtr(GameData.Radar.MiniMapWidget)) mem.AddScatterRead(hScatter, GameData.Radar.MiniMapWidget + Offset::Visibility, (ESlateVisibility*)&MiniRadarVisibility);
            if (Utils::ValidPtr(GameData.Radar.MapWidget)) mem.AddScatterRead(hScatter, GameData.Radar.MapWidget + Offset::Visibility, (ESlateVisibility*)&Visibility);
            mem.ExecuteReadScatter(hScatter);

            bool isWorldMapVisible = (Visibility == ESlateVisibility::Visible || Visibility == ESlateVisibility::HitTestInvisible || Visibility == ESlateVisibility::SelfHitTestInvisible);
            bool isMiniMapVisible = (MiniRadarVisibility == ESlateVisibility::Visible || MiniRadarVisibility == ESlateVisibility::HitTestInvisible || MiniRadarVisibility == ESlateVisibility::SelfHitTestInvisible);

            // 2. Chỉ nạp dữ liệu nặng khi thực sự cần thiết
            if (isWorldMapVisible || isMiniMapVisible) {
                mem.AddScatterRead(hScatter, GameData.GameState + Offset::FeatureRepObject, (uint64_t*)&FeatureRepObject);
                mem.AddScatterRead(hScatter, GameData.GameState + Offset::FeatureRepObject + 8, (int*)&RepObjectCount);

                if (isMiniMapVisible && Utils::ValidPtr(GameData.Radar.MiniMapWidget)) {
                    mem.AddScatterRead(hScatter, GameData.Radar.MiniMapWidget + Offset::SelectMinimapSizeIndex, (int*)&SelectMinimapSizeIndex);
                }
                
                if (isWorldMapVisible && Utils::ValidPtr(GameData.Radar.MapGrid)) {
                    mem.AddScatterRead(hScatter, GameData.Radar.MapGrid + Offset::Slot, (uint64_t*)&Slot);
                }
                mem.ExecuteReadScatter(hScatter);
            }

            // 3. Xử lý dữ liệu nạp được
            FMargin Layout; FVector2D Alignment;
            FVector SafetyZonePosition, BlueZonePosition;
            float SafetyZoneRadius = 0, BlueZoneRadius = 0;

            if (Utils::ValidPtr(Slot) && isWorldMapVisible) {
                mem.AddScatterRead(hScatter, Slot + Offset::LayoutData + Offset::Offsets, (FMargin*)&Layout);
                mem.AddScatterRead(hScatter, Slot + Offset::LayoutData + Offset::Offsets + Offset::Alignment, (FVector2D*)&Alignment);
                mem.ExecuteReadScatter(hScatter);
            }

            if (FeatureRepObject > 0 && RepObjectCount > 0 && (isWorldMapVisible || isMiniMapVisible)) {
                mem.AddScatterRead(hScatter, FeatureRepObject + (RepObjectCount - 1) * 8, (uint64_t*)&FeatureRep);
                mem.ExecuteReadScatter(hScatter);
                if (Utils::ValidPtr(FeatureRep)) {
                    mem.AddScatterRead(hScatter, FeatureRep + Offset::SafetyZonePosition, (FVector*)&BlueZonePosition);
                    mem.AddScatterRead(hScatter, FeatureRep + Offset::SafetyZoneRadius, (float*)&BlueZoneRadius);
                    mem.AddScatterRead(hScatter, FeatureRep + Offset::BlueZonePosition, (FVector*)&SafetyZonePosition);
                    mem.AddScatterRead(hScatter, FeatureRep + Offset::BlueZoneRadius, (float*)&SafetyZoneRadius);
                    mem.ExecuteReadScatter(hScatter);
                }
            }

            GameData.Radar.Slot = Slot;
            GameData.Radar.Visibility = isWorldMapVisible;
            GameData.Radar.MiniRadarVisibility = isMiniMapVisible;
            GameData.Radar.Layout = Layout; 
            GameData.Radar.SafetyZonePosition = SafetyZonePosition; 
            GameData.Radar.SafetyZoneRadius = SafetyZoneRadius;
            GameData.Radar.BlueZonePosition = BlueZonePosition; 
            GameData.Radar.BlueZoneRadius = BlueZoneRadius; 
            GameData.Radar.SelectMinimapSizeIndex = SelectMinimapSizeIndex;

            if (isWorldMapVisible && Layout.Right > 0.001f) {
                const FVector2D CurrentPos = { Layout.Right * (Alignment.X - 0.5f) - Layout.Left, Layout.Bottom * (Alignment.Y - 0.5f) - Layout.Top };
                GameData.Radar.Position = { CurrentPos.X / DefaultSize.X / (Layout.Right / DefaultSize.X) * 2.0f, CurrentPos.Y / DefaultSize.Y / (Layout.Right / DefaultSize.X) * 2.0f };
                GameData.Radar.MapSizeFactored = GameData.Radar.MapSize.Size / (Layout.Right / DefaultSize.X);
                GameData.Radar.WorldCenterLocation = { GameData.Radar.MapSize.Size * (1.0f + GameData.Radar.Position.X), GameData.Radar.MapSize.Size * (1.0f + GameData.Radar.Position.Y) };
                SleepTime = 0;
            } else {
                SleepTime = 4;
            }

            if (GameData.Radar.MiniRadarVisibility) {
                FVector2D FHD = { (float)GameData.Config.Overlay.ScreenWidth, (float)GameData.Config.Overlay.ScreenHeight };
                FVector2D RadarTo_FHD = { GameData.Config.Overlay.ScreenWidth * 0.846875f, GameData.Config.Overlay.ScreenHeight * 0.7351851851852f };
                float RSWidth = GameData.Config.Overlay.ScreenHeight * 0.2407407407407f;
                RadarTo_FHD = { RadarTo_FHD.X + RSWidth + 1.0f, RadarTo_FHD.Y + RSWidth };
                const float RSN_FHD = RSWidth + 1.0f, RSL_FHD = RSWidth * 1.769230769230769f + 1.0f;
                const FVector2D RadarTo = { RadarTo_FHD.X / FHD.X, RadarTo_FHD.Y / FHD.Y }, RSN = { RSN_FHD / FHD.X, RSN_FHD / FHD.Y }, RFN = { RadarTo.X - RSN.X, RadarTo.Y - RSN.Y }, RSL = { RSL_FHD / FHD.X, RSL_FHD / FHD.Y }, RFL = { RadarTo.X - RSL.X, RadarTo.Y - RSL.Y };
                const float Speed = GameData.Radar.VehicleSpeed;
                float RadarDist = Speed < 30.0f ? 200.0f : Speed < 70.0f ? 250.0f : Speed < 95.0f ? 300.0f : 400.0f;
                if (SelectMinimapSizeIndex > 0) RadarDist *= RSL_FHD / RSN_FHD;
                GameData.Radar.MiniRadarDistance = RadarDist; GameData.Radar.bRadarExtended = SelectMinimapSizeIndex > 0;
                GameData.Radar.MiniRadarSizeLarge = RSL; GameData.Radar.MiniRadarFromLarge = RFL; GameData.Radar.MiniRadarSizeNormal = RSN; GameData.Radar.MiniRadarFromNormal = RFN;
            }
            mem.CloseScatterHandle(hScatter);

            auto endTime = std::chrono::high_resolution_clock::now();
            GameData.Performance.RadarThreadMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        }
    }
};