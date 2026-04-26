#pragma once
#include "../../protec/skCrypt.h"

namespace Translation {
    inline int CurrentLanguage = 0; // 0: EN, 1: VN

    struct Strings {
        const char* MainTitle;
        const char* TabVisuals;
        const char* Tabprecision_calibration;
        const char* TabMacro;
        const char* TabSettings;
        const char* TabRadar;
        const char* TabLoot; // NEW
        
        const char* VisualPerformance;
        const char* MasterToggle;
        const char* EnemyESP;
        const char* TeammateESP;
        const char* PlayerESP;
        const char* InfoESP;
        const char* Box;
        const char* Skeleton;
        const char* Interpolate;
        const char* HeadCircle;
        const char* VisCheck;
        const char* HealthBar;
        const char* HealthPos;
        const char* Distance;
        const char* Name;
        const char* ESP_Spectated;
        const char* ESP_Offscreen;
        const char* IndicatorRadius;
        const char* IndicatorSize;
        const char* IndicatorStyle;
        const char* ColorMode;
        const char* ColorNear;
        const char* ColorFar;
        const char* Weapon;
        const char* WeaponType;
        const char* ItemsVehicles;
        const char* DistThresholds;
        const char* ColorsTitle;
        const char* RenderDist;
        const char* RadarOffsetX;
        const char* RadarOffsetY;
        const char* ShowCrosshair;
        const char* RadarZoom;
        const char* RadarRotation;
        const char* RadarEnable;
        const char* RadarDotSize;
        const char* TabDebug;
        
        const char* precision_calibrationSoon;
        const char* MacroSoon;
        
        const char* AimEnabled;
        const char* AimFOV;
        const char* AimSmooth;
        const char* AimKey;
        const char* AimBone;
        const char* AimVisible;
        const char* AimPrediction;
        
        const char* MacroEnabled;
        const char* MacroStrength;
        const char* MacroHumanize;
        const char* MacroOSD;
        const char* RescanAttach;
        const char* CurrentWeapon;
        const char* DetectedAttach;
        const char* NoWeapon;
        
        const char* Language;
        const char* ResetColors;
        const char* SaveConfig;
        const char* LoadConfig;
        const char* Version;
        const char* AntiScreenshot;
        const char* ShowMacroOSD;
        const char* RescaleX;
        const char* RescaleY;

        // Visuals LOD Labels
        const char* SmartLOD;
        const char* BoxMax;
        const char* HealthMax;
        const char* SkeletonMax;
        const char* NameMax;
        const char* DistMax;
        const char* WeaponMax;
        
        const char* VisBox;
        const char* InvBox;
        const char* VisSkel;
        const char* InvSkel;
        const char* ColorNames;
        const char* ColorDist;
        const char* ColorWeapon;
        
        // Premium Headers
        const char* HeaderSystemConfig;
        const char* HeaderPrecisionSettings;
        const char* HeaderAimStructure;
        const char* HeaderVisualCore;
        const char* HeaderRenderStyle;
        const char* HeaderOverlayHUD;
        const char* HeaderDangerScan;
        const char* HeaderLootEngine;
        const char* HeaderPickupFilter;
        const char* HeaderWorldEntities;
        const char* HeaderSystemCore;
        const char* HeaderAntiTracking;
        const char* HeaderEngineUtils;
        
        const char* MainTelemetry;
        const char* SafeStatus;
        const char* PasteLabel;
    };

    inline Strings Get() {
        Strings s;
        if (CurrentLanguage == 1) { // Vietnamese
            s.MainTitle = skCrypt("PHAN MEM GZ-integrity_monitor EXTERNAL");
            s.TabVisuals = skCrypt("\xEF\x91\x81 Hi\xEC\x83\x9Bn th\xEC\x83\x8B"); // 👁️ Hiển thị (UTF-8 encoding might be tricky, using symbol fallback if needed)
            s.Tabprecision_calibration = skCrypt("\xEF\x8E\xAF T\xEC\x8B\xB1 \xEC\x83\x91\xEC\x83\x91ng ng\xEC\x83\x91m"); // 🎯 Tự động ngắm
            s.TabMacro = skCrypt("\xEF\x9B\xB0 Macro");
            s.TabSettings = skCrypt("\xEF\x98\x93 C\xEC\x83\x81i \xEC\x83\x91\xEC\x83\x8Bt"); // ⚙️ Cài đặt
            s.TabRadar = skCrypt("\xEF\x97\xBA B\xEC\x83\x83n \xEC\x83\x91\xEC\x83\xBB"); // 🗺️ Bản đồ
            s.TabLoot = skCrypt("\xEF\x8E\x92 V\xEC\x83\x83t ph\xEC\x83\x83m"); // 🎒 Vật phẩm
            
            s.VisualPerformance = skCrypt("HIEU NANG HIEN THI");
            s.MasterToggle = skCrypt("Bat tat signal_overlay");
            s.EnemyESP = skCrypt("signal_overlay Ke dich");
            s.TeammateESP = skCrypt("signal_overlay Dong doi");
            s.PlayerESP = skCrypt("signal_overlay Nguoi choi");
            s.InfoESP = skCrypt("Thong tin");
            s.Box = skCrypt("Khung");
            s.Skeleton = skCrypt("Xuong");
            s.Interpolate = skCrypt("Noi diem (Muot)");
            s.HeadCircle = skCrypt("Vong dau");
            s.VisCheck = skCrypt("Cach vat can");
            s.HealthBar = skCrypt("Thanh mau");
            s.HealthPos = skCrypt("Vi tri thanh mau");
            s.Distance = skCrypt("Khoang cach");
            s.Name = skCrypt("Ten");
            s.ESP_Spectated = skCrypt("Hien thi nguoi xem");
            s.ESP_Offscreen = skCrypt("Canh bao ngoai man hinh");
            s.IndicatorRadius = skCrypt("Khoang cach mui ten");
            s.IndicatorSize = skCrypt("Kich thuoc mui ten");
            s.IndicatorStyle = skCrypt("Kieu mui ten");
            s.ColorMode = skCrypt("Che do mau");
            s.ColorNear = skCrypt("Mau o gan");
            s.ColorFar = skCrypt("Mau o xa");
            s.Weapon = skCrypt("Vu khi");
            s.WeaponType = skCrypt("Kieu ve vu khi");
            s.ItemsVehicles = skCrypt("Vat pham & Xe");
            s.DistThresholds = skCrypt("Gioi han khoang cach (Smart LOD)");
            s.ColorsTitle = skCrypt("Mau sac rieng biet");
            s.RenderDist = skCrypt("Khoang cach render");
            s.RadarOffsetX = skCrypt("Le ngang Radar (X)");
            s.RadarOffsetY = skCrypt("Le doc Radar (Y)");
            s.ShowCrosshair = skCrypt("Hien tam Radar");
            s.RadarZoom = skCrypt("Ti le Zoom Radar");
            s.RadarRotation = skCrypt("Goc xoay Radar");
            s.RadarEnable = skCrypt("Bat tat Radar");
            s.RadarDotSize = skCrypt("Kich co cham");
            s.TabDebug = skCrypt("Vat pham");
            
            s.precision_calibrationSoon = skCrypt("precision_calibration dang phat trien");
            s.MacroSoon = skCrypt("Macro dang phat trien");
            
            s.AimEnabled = skCrypt("Bat tu dong ngam");
            s.AimFOV = skCrypt("Vung ngam (FOV)");
            s.AimSmooth = skCrypt("Do muot (Smooth)");
            s.AimKey = skCrypt("Phim bam");
            s.AimBone = skCrypt("Vi tri ngam");
            s.AimVisible = skCrypt("Chi ngam khi thay");
            s.AimPrediction = skCrypt("Du doan quy dao");
            
            s.MacroEnabled = skCrypt("Bat Ghi tam (No-Recoil)");
            s.MacroStrength = skCrypt("Suc manh Ghi tam");
            s.MacroHumanize = skCrypt("Nhan ban (Humanize)");
            s.MacroOSD = skCrypt("Hien thi OSD");
            s.RescanAttach = skCrypt("Quet lai phu kien");
            s.CurrentWeapon = skCrypt("Vu khi: ");
            s.DetectedAttach = skCrypt("Phu kien: ");
            s.NoWeapon = skCrypt("Khong cam sung");
            
            s.Language = skCrypt("Ngon ngu");
            s.ResetColors = skCrypt("Reset mau");
            s.SaveConfig = skCrypt("Luu cau hinh");
            s.LoadConfig = skCrypt("Tai cau hinh");
            s.Version = skCrypt("Phien ban");
            s.AntiScreenshot = skCrypt("Stream Proof");
            s.ShowMacroOSD = skCrypt("Hien thi OSD");
            s.RescaleX = skCrypt("Ti le X");
            s.RescaleY = skCrypt("Ti le Y");

            s.SmartLOD = skCrypt("Smart LOD");
            s.BoxMax = skCrypt("Max Khung");
            s.HealthMax = skCrypt("Max Mau");
            s.SkeletonMax = skCrypt("Max Xuong");
            s.NameMax = skCrypt("Max Ten");
            s.DistMax = skCrypt("Max Khoang cach");
            s.WeaponMax = skCrypt("Max Vu khi");
            
            s.VisBox = skCrypt("Mau Khung (Thay)");
            s.InvBox = skCrypt("Mau Khung (An)");
            s.VisSkel = skCrypt("Mau Xuong (Thay)");
            s.InvSkel = skCrypt("Mau Xuong (An)");
            s.ColorNames = skCrypt("Mau Ten");
            s.ColorDist = skCrypt("Mau Khoang cach");
            s.ColorWeapon = skCrypt("Mau Vu khi");

            s.HeaderSystemConfig = skCrypt("\xE2\x97\x89 CAU HINH HE THONG"); // ◉
            s.HeaderPrecisionSettings = skCrypt("\xE2\x97\x88 THONG SO CHINH XAC"); // ◈
            s.HeaderAimStructure = skCrypt("\xE2\x9C\xA4 CAU TRUC NGAM"); // ❖
            s.HeaderVisualCore = skCrypt("\xE2\x97\x89 LOI HIEN THI");
            s.HeaderRenderStyle = skCrypt("\xE2\x96\xA3 KIEU VE");
            s.HeaderOverlayHUD = skCrypt("\xE2\x96\xA0 GIAO DIEN HUD");
            s.HeaderDangerScan = skCrypt("\xE2\x9A\xA0 QUET NGUY HIEM");
            s.HeaderLootEngine = skCrypt("\xE2\x9C\x96 HE THONG LOOT");
            s.HeaderPickupFilter = skCrypt("\xE2\x96\xBD BO LOC VAT PHAM");
            s.HeaderWorldEntities = skCrypt("\xE2\x97\xBD DOI TUONG THE GIOI");
            s.HeaderSystemCore = skCrypt("\xE2\x9A\x99 LOI HE THONG"); // ⚙
            s.HeaderAntiTracking = skCrypt("\xE2\x9B\xA1 CHONG TRUY VET");
            s.HeaderEngineUtils = skCrypt("\xE2\x9A\x92 TIEN ICH ENGINE");
            
            s.MainTelemetry = skCrypt("PHAN MEM GIAM SAT");
            s.SafeStatus = skCrypt("AN TOAN");
            s.PasteLabel = skCrypt("Dan tu bo nho");
        } else { // English
            s.MainTitle = skCrypt("GZ-integrity_monitor EXTERNAL");
            s.TabVisuals = skCrypt("\xE2\x97\x89 Visuals");
            s.Tabprecision_calibration = skCrypt("\xE2\x97\x88 precision_calibration");
            s.TabMacro = skCrypt("\xE2\x9C\xA4 Macro");
            s.TabSettings = skCrypt("\xE2\x9A\x99 Settings");
            s.TabRadar = skCrypt("\xE2\x94\xB3 Radar");
            s.TabLoot = skCrypt("\xE2\x9C\x96 Items");
            
            s.VisualPerformance = skCrypt("VISUAL PERFORMANCE");
            s.MasterToggle = skCrypt("Master signal_overlay Toggle");
            s.EnemyESP = skCrypt("Enemy signal_overlay");
            s.TeammateESP = skCrypt("Teammate signal_overlay");
            s.PlayerESP = skCrypt("Player signal_overlay");
            s.InfoESP = skCrypt("Information");
            s.Box = skCrypt("Box");
            s.Skeleton = skCrypt("Skeleton");
            s.Interpolate = skCrypt("Interpolate Joints");
            s.HeadCircle = skCrypt("Head Circle");
            s.VisCheck = skCrypt("Visibility Check");
            s.HealthBar = skCrypt("Health Bar");
            s.HealthPos = skCrypt("Health Bar Position");
            s.Distance = skCrypt("Distance");
            s.Name = skCrypt("Player Name");
            s.ESP_Spectated = skCrypt("Spectated Count");
            s.ESP_Offscreen = skCrypt("Off-screen Indicators");
            s.IndicatorRadius = skCrypt("Indicator Radius");
            s.IndicatorSize = skCrypt("Indicator Size");
            s.IndicatorStyle = skCrypt("Indicator Style");
            s.ColorMode = skCrypt("Color Mode");
            s.ColorNear = skCrypt("Color (Near)");
            s.ColorFar = skCrypt("Color (Far)");
            s.Weapon = skCrypt("Weapon Info");
            s.WeaponType = skCrypt("Weapon Draw Mode");
            s.ItemsVehicles = skCrypt("Items & Vehicles");
            s.DistThresholds = skCrypt("Distance Thresholds (Culling)");
            s.ColorsTitle = skCrypt("Individual Colors");
            s.RenderDist = skCrypt("Render Distance");
            s.RadarOffsetX = skCrypt("Radar X Offset");
            s.RadarOffsetY = skCrypt("Radar Y Offset");
            s.ShowCrosshair = skCrypt("Show Radar Center");
            s.RadarZoom = skCrypt("Radar Zoom Scale");
            s.RadarRotation = skCrypt("Radar Rotation Offset");
            s.RadarEnable = skCrypt("Enable Radar");
            s.RadarDotSize = skCrypt("Dot Size");
            s.TabDebug = skCrypt("Debug");
            
            s.precision_calibrationSoon = skCrypt("precision_calibration Soon");
            s.MacroSoon = skCrypt("Macro Soon");
            
            s.AimEnabled = skCrypt("Enable precision_calibration");
            s.AimFOV = skCrypt("precision_calibration FOV");
            s.AimSmooth = skCrypt("Smoothness");
            s.AimKey = skCrypt("Aim Key");
            s.AimBone = skCrypt("Target Bone");
            s.AimVisible = skCrypt("Visible Only");
            s.AimPrediction = skCrypt("Enable Prediction");
            
            s.MacroEnabled = skCrypt("Enable No-Recoil");
            s.MacroStrength = skCrypt("Recoil Strength");
            s.MacroHumanize = skCrypt("Humanize");
            s.MacroOSD = skCrypt("Show OSD");
            s.RescanAttach = skCrypt("Rescan Attachments");
            s.CurrentWeapon = skCrypt("Weapon: ");
            s.DetectedAttach = skCrypt("Attachments: ");
            s.NoWeapon = skCrypt("Holstered");
            
            s.Language = skCrypt("Language");
            s.ResetColors = skCrypt("Reset Colors");
            s.SaveConfig = skCrypt("Save Config");
            s.LoadConfig = skCrypt("Load Config");
            s.Version = skCrypt("Version");
            s.AntiScreenshot = skCrypt("Stream Proof");
            s.ShowMacroOSD = skCrypt("Show OSD");
            s.RescaleX = skCrypt("Rescale X");
            s.RescaleY = skCrypt("Rescale Y");

            s.SmartLOD = skCrypt("Smart LOD");
            s.BoxMax = skCrypt("Box Max");
            s.HealthMax = skCrypt("Health Max");
            s.SkeletonMax = skCrypt("Skeleton Max");
            s.NameMax = skCrypt("Name Max");
            s.DistMax = skCrypt("Dist Max");
            s.WeaponMax = skCrypt("Weapon Max");
            
            s.VisBox = skCrypt("Visible Box Color");
            s.InvBox = skCrypt("Invisible Box Color");
            s.VisSkel = skCrypt("Visible Skeleton Color");
            s.InvSkel = skCrypt("Invisible Skeleton Color");
            s.ColorNames = skCrypt("Names Color");
            s.ColorDist = skCrypt("Distance Color");
            s.ColorWeapon = skCrypt("Weapon Color");

            s.HeaderSystemConfig = skCrypt("\xE2\x9A\x99 SYSTEM CONFIG");
            s.HeaderPrecisionSettings = skCrypt("\xE2\x97\x88 PRECISION SETTINGS");
            s.HeaderAimStructure = skCrypt("\xE2\x9C\xA4 AIM STRUCTURE");
            s.HeaderVisualCore = skCrypt("\xE2\x97\x89 VISUAL CORE");
            s.HeaderRenderStyle = skCrypt("\xE2\x96\xA3 RENDER STYLE");
            s.HeaderOverlayHUD = skCrypt("\xE2\x96\xA0 OVERLAY HUD");
            s.HeaderDangerScan = skCrypt("\xE2\x9A\xA0 DANGER SCAN");
            s.HeaderLootEngine = skCrypt("\xE2\x9C\x96 LOOT ENGINE");
            s.HeaderPickupFilter = skCrypt("\xE2\x96\xBD PICKUP FILTER");
            s.HeaderWorldEntities = skCrypt("\xE2\x97\xBD WORLD ENTITIES");
            s.HeaderSystemCore = skCrypt("\xE2\x9A\x9B SYSTEM CORE");
            s.HeaderAntiTracking = skCrypt("\xE2\x9B\xA1 ANTI-TRACKING");
            s.HeaderEngineUtils = skCrypt("\xE2\x9A\x92 ENGINE UTILS");
            
            s.MainTelemetry = skCrypt("GZ TELEMETRY");
            s.SafeStatus = skCrypt("SAFE");
            s.PasteLabel = skCrypt("Paste Key");
        }
        return s;
    }

    inline const char* GetAttachmentName(int type, int id) {
        if (id == 0) return (CurrentLanguage == 1) ? skCrypt("Khong") : skCrypt("None");

        if (CurrentLanguage == 1) { // Vietnamese
            if (type == 0) { // Scope
                switch(id) {
                    case 1: return skCrypt("Cham do");
                    case 2: return skCrypt("Holo");
                    case 3: return skCrypt("2X");
                    case 4: return skCrypt("3X");
                    case 5: return skCrypt("4X");
                    case 6: return skCrypt("6X");
                    case 7: return skCrypt("8X");
                    case 8: return skCrypt("15X");
                }
            } else if (type == 1) { // Muzzle
                switch(id) {
                    case 1: return skCrypt("Lua");
                    case 2: return skCrypt("Thanh");
                    case 4: return skCrypt("Giat");
                }
            } else if (type == 2) { // Grip
                switch(id) {
                    case 1: return skCrypt("Doc");
                    case 2: return skCrypt("Ngang");
                    case 3: return skCrypt("Nua");
                    case 4: return skCrypt("Nhe");
                }
            }
        } else { // English
            if (type == 0) { // Scope
                switch(id) {
                    case 1: return skCrypt("RedDot");
                    case 2: return skCrypt("Holo");
                    case 3: return skCrypt("2X");
                    case 4: return skCrypt("3X");
                    case 5: return skCrypt("4X");
                    case 6: return skCrypt("6X");
                    case 7: return skCrypt("8X");
                    case 8: return skCrypt("15X");
                }
            } else if (type == 1) { // Muzzle
                switch(id) {
                    case 1: return skCrypt("Flash");
                    case 2: return skCrypt("Supp");
                    case 4: return skCrypt("Comp");
                }
            } else if (type == 2) { // Grip
                switch(id) {
                    case 1: return skCrypt("Vertical");
                    case 2: return skCrypt("Angled");
                    case 3: return skCrypt("Half");
                    case 4: return skCrypt("Light");
                }
            }
        }
        return (CurrentLanguage == 1) ? skCrypt("Khong") : skCrypt("None");
    }
}

