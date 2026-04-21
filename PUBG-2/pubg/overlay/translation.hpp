#pragma once

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
        
        const char* MasterToggle;
        const char* PlayerESP;
        const char* InfoESP;
        const char* Box;
        const char* Skeleton;
        const char* HeadCircle;
        const char* VisCheck;
        const char* HealthBar;
        const char* Distance;
        const char* Name;
        const char* ESP_Spectated;
        const char* Weapon;
        const char* RenderDist;
        const char* RadarOffsetX;
        const char* RadarOffsetY;
        const char* ShowCrosshair;
        const char* RadarZoom;
        const char* RadarRotation;
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
                "PHAN MEM GZ-CHEAT EXTERNAL", "Hien thi", "Tu ngam", "Macro", "Cai dat", "Radar", "Vat pham & Xe",
                "Bat tat ESP", "ESP Nguoi choi", "Thong tin", "Khung", "Xuong", "Vong dau", "Cach vat can",
                "Thanh mau", "Khoang cach", "Ten", "Hien thi nguoi xem", "Vu khi", "Khoang cach render",
                "Le ngang Radar (X)", "Le doc Radar (Y)", "Hien tam Radar", "Ti le Zoom Radar", "Goc xoay Radar", "Vat pham",
                "Aimbot dang phat trien", "Macro dang phat trien",
                "Bat tu dong ngam", "Vung ngam (FOV)", "Do muot (Smooth)", "Phim bam", "Vi tri ngam", "Chi ngam khi thay", "Du doan quy dao",
                "Bat Ghi tam (No-Recoil)", "Suc manh Ghi tam", "Nhan ban (Humanize)", "Hien thi OSD", "Quet lai phu kien", 
                "Vu khi: ", "Phu kien: ", "Khong cam sung",
                "Ngon ngu", "Reset mau", "Luu cau hinh", "Tai cau hinh", "Phien ban",
                "Stream Proof", "Hien thi OSD", "Ti le X", "Ti le Y",
                "Smart LOD", "Max Khung", "Max Mau", "Max Xuong", "Max Ten", "Max Khoang cach", "Max Vu khi",
                "Mau Khung (Thay)", "Mau Khung (An)", "Mau Xuong (Thay)", "Mau Xuong (An)", "Mau Ten", "Mau Khoang cach", "Mau Vu khi"
            };
        }
        return {
            "GZ-CHEAT EXTERNAL", "Visuals", "Aimbot", "Macro", "Settings", "Radar", "Items & Vehicles",
            "Master ESP Toggle", "Player ESP", "Information", "Box", "Skeleton", "Head Circle", "Visibility Check",
            "Health Bar", "Distance", "Player Name", "Spectated Count", "Weapon Info", "Render Distance",
            "Radar X Offset", "Radar Y Offset", "Show Radar Center", "Radar Zoom Scale", "Radar Rotation Offset", "Debug",
            "Aimbot Soon", "Macro Soon", 
            "Enable Aimbot", "Aimbot FOV", "Smoothness", "Aim Key", "Target Bone", "Visible Only", "Enable Prediction",
            "Enable No-Recoil", "Recoil Strength", "Humanize", "Show OSD", "Rescan Attachments",
            "Weapon: ", "Attachments: ", "Holstered",
            "Language", "Reset Colors", "Save Config", "Load Config", "Version",
            "Stream Proof", "Show OSD", "Rescale X", "Rescale Y",
            "Smart LOD", "Box Max", "Health Max", "Skeleton Max", "Name Max", "Dist Max", "Weapon Max",
            "Visible Box Color", "Invisible Box Color", "Visible Skeleton Color", "Invisible Skeleton Color", "Names Color", "Distance Color", "Weapon Color"
        };
    }

    inline const char* GetAttachmentName(int type, int id) {
        if (id == 0) return (CurrentLanguage == 1) ? "Khong" : "None";

        if (CurrentLanguage == 1) { // Vietnamese
            if (type == 0) { // Scope
                switch(id) {
                    case 1: return "Cham do";
                    case 2: return "Holo";
                    case 3: return "2X";
                    case 4: return "3X";
                    case 5: return "4X";
                    case 6: return "6X";
                    case 7: return "8X";
                    case 8: return "15X";
                }
            } else if (type == 1) { // Muzzle
                switch(id) {
                    case 1: return "Lua";
                    case 2: return "Thanh";
                    case 4: return "Giat";
                }
            } else if (type == 2) { // Grip
                switch(id) {
                    case 1: return "Doc";
                    case 2: return "Ngang";
                    case 3: return "Nua";
                    case 4: return "Nhe";
                }
            }
        } else { // English
            if (type == 0) { // Scope
                switch(id) {
                    case 1: return "RedDot";
                    case 2: return "Holo";
                    case 3: return "2X";
                    case 4: return "3X";
                    case 5: return "4X";
                    case 6: return "6X";
                    case 7: return "8X";
                    case 8: return "15X";
                }
            } else if (type == 1) { // Muzzle
                switch(id) {
                    case 1: return "Flash";
                    case 2: return "Supp";
                    case 4: return "Comp";
                }
            } else if (type == 2) { // Grip
                switch(id) {
                    case 1: return "Vertical";
                    case 2: return "Angled";
                    case 3: return "Half";
                    case 4: return "Light";
                }
            }
        }
        return (CurrentLanguage == 1) ? "Khong" : "None";
    }
}

