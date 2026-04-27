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
        const char* ESP_Icons;
        const char* ESP_SkeletonDots;
        const char* ESP_SpectatorList;
        const char* KillCount;
        const char* TeamID;
        const char* Rank;
        const char* SurvivalLevel;
        const char* ESP_Spectated;
        const char* ESP_Offscreen;

        const char* FontSize;
        const char* BoxThick;
        const char* SmoothRNG;
        const char* AimKey2;

        const char* IndicatorRadius;

        const char* IndicatorSize;

        const char* IndicatorStyle;

        const char* ColorMode;

        const char* ColorNear;

        const char* ColorFar;

        const char* HeaderTactical;
        const char* GrenadeLine;
        const char* Projectiles;
        const char* ThreatWarning;

        const char* HeaderGearFilter;
        const char* HelmetLv1;
        const char* HelmetLv2;
        const char* HelmetLv3;
        const char* ArmorLv1;
        const char* ArmorLv2;
        const char* ArmorLv3;

        const char* HeaderHealFilter;
        const char* Boosters;
        const char* Healing;

        const char* HeaderAmmoScope;
        const char* AmmoAll;
        const char* AmmoHigh;
        const char* ScopeAll;
        const char* ScopeHigh;

        const char* HeaderShareRadar;
        const char* RadarShare;
        const char* RadarIP;

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
        
        // Health & Weapon HUD additions
        const char* HealthMode;
        const char* ModeDynamic;
        const char* ModeStatic;
        const char* PosLeft;
        const char* PosRight;
        const char* PosTop;
        const char* PosBottom;
        const char* TypeIcon;
        const char* TypeText;
        const char* ColorHealth;
        const char* DisplayOnlyNote;
        const char* ShowcaseVisualProfile;
        const char* ShowcasePrecisionProfile;
        const char* ShowcaseInputProfile;
        const char* ShowcaseInventoryProfile;
        const char* ShowcaseMapProfile;
        const char* ShowcaseVehicleProfile;
        const char* ShowcaseConfigProfile;
        const char* ShowcaseStreamerProfile;
    };



    inline Strings Get() {

        Strings s;

        if (CurrentLanguage == 1) { // Vietnamese

            s.MainTitle = (const char*)u8"PHẦN MỀM GZ-TELEMETRY EXTERNAL";

            s.TabVisuals = (const char*)u8"[+] Hiển thị";

            s.Tabprecision_calibration = (const char*)u8"[*] Tự động ngắm";

            s.TabMacro = (const char*)u8"[!] Macro";

            s.TabSettings = (const char*)u8"[#] Cài đặt";

            s.TabRadar = (const char*)u8"[&] Bản đồ";

            s.TabLoot = (const char*)u8"[$] Vật phẩm";

            

            s.VisualPerformance = (const char*)u8"HIỆU NĂNG HIỂN THỊ";

            s.MasterToggle = (const char*)u8"Bật tắt ESP";

            s.EnemyESP = (const char*)u8"ESP Kẻ địch";

            s.TeammateESP = (const char*)u8"ESP đồng đội";

            s.PlayerESP = (const char*)u8"ESP Người chơi";

            s.InfoESP = (const char*)u8"Thông tin";

            s.Box = (const char*)u8"Khung";

            s.Skeleton = (const char*)u8"Xương";

            s.Interpolate = (const char*)u8"Nối điểm";

            s.HeadCircle = (const char*)u8"Vòng đầu";

            s.VisCheck = (const char*)u8"Kiểm tra vật cản";

            s.HealthBar = (const char*)u8"Thanh máu";

            s.HealthPos = (const char*)u8"Vị trí máu";

            s.Distance = (const char*)u8"Khoảng cách";

            s.Name = (const char*)u8"Tên người chơi";
            s.ESP_Icons = (const char*)u8"Sử dụng Icon";
            s.KillCount = (const char*)u8"Số mạng";
            s.TeamID = (const char*)u8"ID Đội";
            s.Rank = (const char*)u8"Bậc hạng";
            s.SurvivalLevel = (const char*)u8"Cấp độ";
            s.ESP_Spectated = (const char*)u8"Hiện người xem";

            s.ESP_Offscreen = (const char*)u8"Cảnh báo mũi tên";

            s.IndicatorRadius = (const char*)u8"KC mũi tên";

            s.IndicatorSize = (const char*)u8"Size mũi tên";

            s.IndicatorStyle = (const char*)u8"Kiểu mũi tên";

            s.ColorMode = (const char*)u8"Chế độ màu";

            s.ColorNear = (const char*)u8"Màu gần";

            s.ColorFar = (const char*)u8"Màu xa";
            s.HeaderTactical = (const char*)u8"Dữ liệu Chiến thuật";
            s.GrenadeLine = (const char*)u8"Quỹ đạo lựu đạn";
            s.Projectiles = (const char*)u8"Quỹ đạo đạn";
            s.ThreatWarning = (const char*)u8"Cảnh báo đe dọa";
            s.HeaderGearFilter = (const char*)u8"Lọc Trang bị";
            s.HelmetLv1 = (const char*)u8"Mũ 1";
            s.HelmetLv2 = (const char*)u8"Mũ 2";
            s.HelmetLv3 = (const char*)u8"Mũ 3";
            s.ArmorLv1 = (const char*)u8"Giáp 1";
            s.ArmorLv2 = (const char*)u8"Giáp 2";
            s.ArmorLv3 = (const char*)u8"Giáp 3";
            s.HeaderHealFilter = (const char*)u8"Lọc Tiêu hao";
            s.Boosters = (const char*)u8"Nước & Thuốc";
            s.Healing = (const char*)u8"Máu & Sơ cứu";
            s.HeaderAmmoScope = (const char*)u8"Đạn & Ống ngắm";
            s.AmmoAll = (const char*)u8"Tất cả đạn";
            s.AmmoHigh = (const char*)u8"Đạn hiếm";
            s.ScopeAll = (const char*)u8"Tất cả ống ngắm";
            s.ScopeHigh = (const char*)u8"Ống ngắm cao (4x+)";
            s.HeaderShareRadar = (const char*)u8"Chia sẻ Radar";
            s.RadarShare = (const char*)u8"Bật chia sẻ";
            s.RadarIP = (const char*)u8"Địa chỉ IP";
            s.Weapon = (const char*)u8"Vũ khí";

            s.WeaponType = (const char*)u8"Kiểu vẽ";

            s.ItemsVehicles = (const char*)u8"Vật phẩm & Xe";

            s.DistThresholds = (const char*)u8"Khoảng cách tối đa";

            s.ColorsTitle = (const char*)u8"Màu sắc";

            s.RenderDist = (const char*)u8"KC render";

            s.RadarOffsetX = (const char*)u8"Radar X";

            s.RadarOffsetY = (const char*)u8"Radar Y";

            s.ShowCrosshair = (const char*)u8"Hiện tâm";

            s.RadarZoom = (const char*)u8"Tỉ lệ zoom";

            s.RadarRotation = (const char*)u8"Góc xoay";

            s.RadarEnable = (const char*)u8"Bật Radar";

            s.RadarDotSize = (const char*)u8"Kích cỡ chấm";

            s.TabDebug = (const char*)u8"Loot";

            

            s.precision_calibrationSoon = (const char*)u8"Tính năng đang phát triển";

            s.MacroSoon = (const char*)u8"Tính năng đang phát triển";

            

            s.AimEnabled = (const char*)u8"Bật Tự động ngắm";

            s.AimFOV = (const char*)u8"Phạm vi quét (FOV)";

            s.AimSmooth = (const char*)u8"Độ mượt";

            s.AimKey = (const char*)u8"Phím kích hoạt";

            s.AimBone = (const char*)u8"Vùng ưu tiên";

            s.AimVisible = (const char*)u8"Chỉ nhắm mục tiêu nhìn thấy";

            s.AimPrediction = (const char*)u8"Dự đoán quỹ đạo";

            

            s.MacroEnabled = (const char*)u8"Bật Chống giật";

            s.MacroStrength = (const char*)u8"Lực chống giật";

            s.MacroHumanize = (const char*)u8"Mô phỏng người";

            s.MacroOSD = (const char*)u8"Hiện thông số";

            s.RescanAttach = (const char*)u8"Quét lại phụ kiện";

            s.CurrentWeapon = (const char*)u8"Vũ khí: ";

            s.DetectedAttach = (const char*)u8"Phụ kiện: ";

            s.NoWeapon = (const char*)u8"Cất vũ khí";

            

            s.Language = (const char*)u8"Ngôn ngữ";

            s.ResetColors = (const char*)u8"Đặt lại màu";

            s.SaveConfig = (const char*)u8"Lưu cấu hình";

            s.LoadConfig = (const char*)u8"Tải cấu hình";

            s.Version = (const char*)u8"Phiên bản";

            s.AntiScreenshot = (const char*)u8"Stream Proof";

            s.ShowMacroOSD = (const char*)u8"Hiện OSD";

            s.RescaleX = (const char*)u8"Tỉ lệ X";

            s.RescaleY = (const char*)u8"Tỉ lệ Y";



            s.SmartLOD = (const char*)u8"Smart LOD";

            s.BoxMax = (const char*)u8"Khung xa nhất";

            s.HealthMax = (const char*)u8"Máu xa nhất";

            s.SkeletonMax = (const char*)u8"Xương xa nhất";

            s.NameMax = (const char*)u8"Tên xa nhất";
            s.ESP_SkeletonDots = (const char*)u8"Hiện khớp xương";
            s.ESP_SpectatorList = (const char*)u8"Danh sách người xem";
            s.FontSize = (const char*)u8"Cỡ chữ";
            s.BoxThick = (const char*)u8"Độ dày khung";
            s.SmoothRNG = (const char*)u8"Độ mượt ngẫu nhiên";
            s.AimKey2 = (const char*)u8"Phím nhắm 2";

            s.DistMax = (const char*)u8"KC xa nhất";

            s.WeaponMax = (const char*)u8"Vũ khí xa nhất";

            

            s.VisBox = (const char*)u8"Màu khung nhìn thấy";

            s.InvBox = (const char*)u8"Màu khung bị che";

            s.VisSkel = (const char*)u8"Màu xương nhìn thấy";

            s.InvSkel = (const char*)u8"Màu xương bị che";

            s.ColorNames = (const char*)u8"Màu tên";

            s.ColorDist = (const char*)u8"Màu khoảng cách";

            s.ColorWeapon = (const char*)u8"Màu vũ khí";



            s.HeaderSystemConfig = (const char*)u8"[#] CẤU HÌNH HỆ THỐNG";

            s.HeaderPrecisionSettings = (const char*)u8"[*] THÔNG SỐ CHÍNH XÁC";

            s.HeaderAimStructure = (const char*)u8"[!] CẤU TRÚC NGẮM";

            s.HeaderVisualCore = (const char*)u8"[+] LÕI HIỂN THỊ";

            s.HeaderRenderStyle = (const char*)u8"[@] KIỂU VẼ";

            s.HeaderOverlayHUD = (const char*)u8"[%] GIAO DIỆN HUD";

            s.HeaderDangerScan = (const char*)u8"[^] QUÉT NGUY HIỂM";

            s.HeaderLootEngine = (const char*)u8"[$] HỆ THỐNG LOOT";

            s.HeaderPickupFilter = (const char*)u8"[-] BỘ LỌC VẬT PHẨM";

            s.HeaderWorldEntities = (const char*)u8"[~] ĐỐI TƯỢNG THẾ GIỚI";

            s.HeaderSystemCore = (const char*)u8"[#] LÕI HỆ THỐNG";

            s.HeaderAntiTracking = (const char*)u8"[-] CHỐNG TRUY VÉT";

            s.HeaderEngineUtils = (const char*)u8"[^] TIỆN ÍCH ENGINE";

            

            s.MainTelemetry = (const char*)u8"GIAVNI-Z TELEMETRY";

            s.SafeStatus = (const char*)u8"AN TOÀN";

            s.PasteLabel = (const char*)u8"Dán Key";

            s.HealthMode = (const char*)u8"Kiểu màu máu";
            s.ModeDynamic = (const char*)u8"Động (Máu %)";
            s.ModeStatic = (const char*)u8"Cố định";
            s.PosLeft = (const char*)u8"Trái";
            s.PosRight = (const char*)u8"Phải";
            s.PosTop = (const char*)u8"Trên";
            s.PosBottom = (const char*)u8"Dưới";
            s.TypeIcon = (const char*)u8"Hình ảnh";
            s.TypeText = (const char*)u8"Chữ";
            s.ColorHealth = (const char*)u8"Màu máu";
            s.DisplayOnlyNote = (const char*)u8"Chỉ hiển thị trên menu";
            s.ShowcaseVisualProfile = (const char*)u8"Hồ sơ hiển thị nâng cao";
            s.ShowcasePrecisionProfile = (const char*)u8"Hồ sơ chính xác nâng cao";
            s.ShowcaseInputProfile = (const char*)u8"Hồ sơ input nâng cao";
            s.ShowcaseInventoryProfile = (const char*)u8"Bộ lọc kho đồ mở rộng";
            s.ShowcaseMapProfile = (const char*)u8"Hồ sơ bản đồ mở rộng";
            s.ShowcaseVehicleProfile = (const char*)u8"Hiển thị phương tiện nâng cao";
            s.ShowcaseConfigProfile = (const char*)u8"Hồ sơ cấu hình";
            s.ShowcaseStreamerProfile = (const char*)u8"Bố cục trình chiếu";

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
            s.ESP_Icons = skCrypt("Vivid Icons");
            s.KillCount = skCrypt("Kill Count");
            s.TeamID = skCrypt("Team ID");
            s.Rank = skCrypt("Rank Tier");
            s.SurvivalLevel = skCrypt("Survival Level");
            s.ESP_Spectated = skCrypt("Spectated Count");

            s.ESP_Offscreen = skCrypt("Off-screen Indicators");

            s.IndicatorRadius = skCrypt("Indicator Radius");

            s.IndicatorSize = skCrypt("Indicator Size");

            s.IndicatorStyle = skCrypt("Indicator Style");

            s.ColorMode = skCrypt("Color Mode");

            s.ColorNear = skCrypt("Color (Near)");

            s.ColorFar = skCrypt("Color (Far)");
            s.HeaderTactical = skCrypt("Tactical Intelligence");
            s.GrenadeLine = skCrypt("Grenade Prediction");
            s.Projectiles = skCrypt("Bullet Tracers");
            s.ThreatWarning = skCrypt("Threat Warnings");
            s.HeaderGearFilter = skCrypt("Armor/Gear Filter");
            s.HelmetLv1 = skCrypt("Helmet Level 1");
            s.HelmetLv2 = skCrypt("Helmet Level 2");
            s.HelmetLv3 = skCrypt("Helmet Level 3");
            s.ArmorLv1 = skCrypt("Vest Level 1");
            s.ArmorLv2 = skCrypt("Vest Level 2");
            s.ArmorLv3 = skCrypt("Vest Level 3");
            s.HeaderHealFilter = skCrypt("Medical/Boost Filter");
            s.Boosters = skCrypt("Boosters/Energy");
            s.Healing = skCrypt("Healing/First Aid");
            s.HeaderAmmoScope = skCrypt("Ammo & Scope Filter");
            s.AmmoAll = skCrypt("All Ammo Types");
            s.AmmoHigh = skCrypt("High-Caliber Only");
            s.ScopeAll = skCrypt("All Scopes");
            s.ScopeHigh = skCrypt("Long Range (4x+)");
            s.HeaderShareRadar = skCrypt("External Radar Share");
            s.RadarShare = skCrypt("Enable Sharing");
            s.RadarIP = skCrypt("Target Device IP");

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
            s.ESP_SkeletonDots = skCrypt("Draw Joints");
            s.ESP_SpectatorList = skCrypt("Spectators List");
            s.FontSize = skCrypt("Global Text Scale");
            s.BoxThick = skCrypt("Box Thickness");
            s.SmoothRNG = skCrypt("Smoothing RNG");
            s.AimKey2 = skCrypt("Secondary Aim Key");

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

            s.HealthMode = skCrypt("Health Color Mode");
            s.ModeDynamic = skCrypt("Dynamic (%)");
            s.ModeStatic = skCrypt("Static Color");
            s.PosLeft = skCrypt("Left");
            s.PosRight = skCrypt("Right");
            s.PosTop = skCrypt("Top");
            s.PosBottom = skCrypt("Bottom");
            s.TypeIcon = skCrypt("Icon");
            s.TypeText = skCrypt("Text");
            s.ColorHealth = skCrypt("Health Color");
            s.DisplayOnlyNote = skCrypt("Menu display only");
            s.ShowcaseVisualProfile = skCrypt("Advanced Visual Profile");
            s.ShowcasePrecisionProfile = skCrypt("Advanced Precision Profile");
            s.ShowcaseInputProfile = skCrypt("Advanced Input Profile");
            s.ShowcaseInventoryProfile = skCrypt("Expanded Inventory Filter");
            s.ShowcaseMapProfile = skCrypt("Expanded Map Profile");
            s.ShowcaseVehicleProfile = skCrypt("Advanced Vehicle Display");
            s.ShowcaseConfigProfile = skCrypt("Configuration Profiles");
            s.ShowcaseStreamerProfile = skCrypt("Presentation Layout");

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



