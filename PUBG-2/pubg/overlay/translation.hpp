#pragma once
#include "../sdk/skCrypt.h"

namespace Translation {
    inline int CurrentLanguage = 0; // 0: EN, 1: VN

    struct Strings {
        const char* MainTitle;
        const char* TabVisuals;
        const char* TabAimbot;
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
        
        const char* AimbotSoon;
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
        
        // Colors Labels
        const char* VisBox;
        const char* InvBox;
        const char* VisSkel;
        const char* InvSkel;
        const char* ColorNames;
        const char* ColorDist;
        const char* ColorWeapon;
    };

    inline Strings Get() {
        if (CurrentLanguage == 1) { // Vietnamese
            return {
                skCrypt("PHAN MEM GZ-CHEAT EXTERNAL"), skCrypt("Hien thi"), skCrypt("Tu ngam"), skCrypt("Macro"), skCrypt("Cai dat"), skCrypt("Radar"), skCrypt("Vat pham & Xe"),
                skCrypt("HIEU NANG HIEN THI"), skCrypt("Bat tat ESP"), skCrypt("ESP Ke dich"), skCrypt("ESP Dong doi"), skCrypt("ESP Nguoi choi"), skCrypt("Thong tin"), skCrypt("Khung"), skCrypt("Xuong"), skCrypt("Noi diem (Muot)"), skCrypt("Vong dau"), skCrypt("Cach vat can"),
                skCrypt("Thanh mau"), skCrypt("Vi tri thanh mau"), skCrypt("Khoang cach"), skCrypt("Ten"), skCrypt("Hien thi nguoi xem"), skCrypt("Canh bao ngoai man hinh"), skCrypt("Khoang cach mui ten"), skCrypt("Kich thuoc mui ten"), skCrypt("Kieu mui ten"), skCrypt("Che do mau"), skCrypt("Mau o gan"), skCrypt("Mau o xa"), skCrypt("Vu khi"), skCrypt("Kieu ve vu khi"), skCrypt("Vat pham & Xe"), skCrypt("Gioi han khoang cach (Smart LOD)"), skCrypt("Mau sac rieng biet"), skCrypt("Khoang cach render"),
                skCrypt("Le ngang Radar (X)"), skCrypt("Le doc Radar (Y)"), skCrypt("Hien tam Radar"), skCrypt("Ti le Zoom Radar"), skCrypt("Goc xoay Radar"), skCrypt("Bat tat Radar"), skCrypt("Kich co cham"), skCrypt("Vat pham"),
                skCrypt("Aimbot dang phat trien"), skCrypt("Macro dang phat trien"),
                skCrypt("Bat tu dong ngam"), skCrypt("Vung ngam (FOV)"), skCrypt("Do muot (Smooth)"), skCrypt("Phim bam"), skCrypt("Vi tri ngam"), skCrypt("Chi ngam khi thay"), skCrypt("Du doan quy dao"),
                skCrypt("Bat Ghi tam (No-Recoil)"), skCrypt("Suc manh Ghi tam"), skCrypt("Nhan ban (Humanize)"), skCrypt("Hien thi OSD"), skCrypt("Quet lai phu kien"), 
                skCrypt("Vu khi: "), skCrypt("Phu kien: "), skCrypt("Khong cam sung"),
                skCrypt("Ngon ngu"), skCrypt("Reset mau"), skCrypt("Luu cau hinh"), skCrypt("Tai cau hinh"), skCrypt("Phien ban"),
                skCrypt("Stream Proof"), skCrypt("Hien thi OSD"), skCrypt("Ti le X"), skCrypt("Ti le Y"),
                skCrypt("Smart LOD"), skCrypt("Max Khung"), skCrypt("Max Mau"), skCrypt("Max Xuong"), skCrypt("Max Ten"), skCrypt("Max Khoang cach"), skCrypt("Max Vu khi"),
                skCrypt("Mau Khung (Thay)"), skCrypt("Mau Khung (An)"), skCrypt("Mau Xuong (Thay)"), skCrypt("Mau Xuong (An)"), skCrypt("Mau Ten"), skCrypt("Mau Khoang cach"), skCrypt("Mau Vu khi")
            };
        }
        return {
            skCrypt("GZ-CHEAT EXTERNAL"), skCrypt("Visuals"), skCrypt("Aimbot"), skCrypt("Macro"), skCrypt("Settings"), skCrypt("Radar"), skCrypt("Items & Vehicles"),
            skCrypt("VISUAL PERFORMANCE"), skCrypt("Master ESP Toggle"), skCrypt("Enemy ESP"), skCrypt("Teammate ESP"), skCrypt("Player ESP"), skCrypt("Information"), skCrypt("Box"), skCrypt("Skeleton"), skCrypt("Interpolate Joints"), skCrypt("Head Circle"), skCrypt("Visibility Check"),
            skCrypt("Health Bar"), skCrypt("Health Bar Position"), skCrypt("Distance"), skCrypt("Player Name"), skCrypt("Spectated Count"), skCrypt("Off-screen Indicators"), skCrypt("Indicator Radius"), skCrypt("Indicator Size"), skCrypt("Indicator Style"), skCrypt("Color Mode"), skCrypt("Color (Near)"), skCrypt("Color (Far)"), skCrypt("Weapon Info"), skCrypt("Weapon Draw Mode"), skCrypt("Items & Vehicles"), skCrypt("Distance Thresholds (Culling)"), skCrypt("Individual Colors"), skCrypt("Render Distance"),
            skCrypt("Radar X Offset"), skCrypt("Radar Y Offset"), skCrypt("Show Radar Center"), skCrypt("Radar Zoom Scale"), skCrypt("Radar Rotation Offset"), skCrypt("Enable Radar"), skCrypt("Dot Size"), skCrypt("Debug"),
            skCrypt("Aimbot Soon"), skCrypt("Macro Soon"), 
            skCrypt("Enable Aimbot"), skCrypt("Aimbot FOV"), skCrypt("Smoothness"), skCrypt("Aim Key"), skCrypt("Target Bone"), skCrypt("Visible Only"), skCrypt("Enable Prediction"),
            skCrypt("Enable No-Recoil"), skCrypt("Recoil Strength"), skCrypt("Humanize"), skCrypt("Show OSD"), skCrypt("Rescan Attachments"),
            skCrypt("Weapon: "), skCrypt("Attachments: "), skCrypt("Holstered"),
            skCrypt("Language"), skCrypt("Reset Colors"), skCrypt("Save Config"), skCrypt("Load Config"), skCrypt("Version"),
            skCrypt("Stream Proof"), skCrypt("Show OSD"), skCrypt("Rescale X"), skCrypt("Rescale Y"),
            skCrypt("Smart LOD"), skCrypt("Box Max"), skCrypt("Health Max"), skCrypt("Skeleton Max"), skCrypt("Name Max"), skCrypt("Dist Max"), skCrypt("Weapon Max"),
            skCrypt("Visible Box Color"), skCrypt("Invisible Box Color"), skCrypt("Visible Skeleton Color"), skCrypt("Invisible Skeleton Color"), skCrypt("Names Color"), skCrypt("Distance Color"), skCrypt("Weapon Color")
        };
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

