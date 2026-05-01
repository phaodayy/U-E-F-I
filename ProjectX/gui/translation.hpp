#pragma once

namespace Translation {
    inline int CurrentLanguage = 0; // 0: EN, 1: VN

    struct Strings {
        const char* MainTitle;
        const char* Subtitle;
        const char* TabAimbot;
        const char* TabVisuals;
        const char* TabSettings;
        const char* ExitCheat;

        // Aimbot
        const char* AimbotHeader;
        const char* EnableFlickbot;
        const char* TargetFOV;
        const char* SmoothFactor;
        const char* AimStrength;
        const char* Deadzone;
        const char* TargetBone;
        const char* TriggerKey;
        const char* AutoFire;
        const char* MacroHeader;
        const char* EnableMacro;
        const char* VerticalIntensity;
        const char* HorizontalIntensity;
        const char* MacroDelay;
        const char* MacroNote;

        // Visuals
        const char* VisualsHeader;
        const char* MasterESP;
        const char* PlayerBoxes;
        const char* Skeleton;
        const char* Health;
        const char* HeadMarker;
        const char* Names;
        const char* Distance;
        const char* TeamCheck;
        const char* BoxStyle;
        const char* MaxDistance;

        // Misc
        const char* SystemHeader;
        const char* KernelTest;
        const char* RunMouse;
        const char* RunClick;
        const char* StreamProof;
        const char* MiniRadar;
        const char* SaveConfig;
        const char* LoadConfig;
        const char* CoreInfo;

        // Colors
        const char* ColorTree;
        const char* TargetItem;
        const char* ColorVisible;
        const char* ColorInvisible;
        const char* SyncGlobal;
    };

    inline Strings Get() {
        if (CurrentLanguage == 1) { // Vietnamese
            return {
                "GZ-ProjectX", "BAN QUYEN GZ", "Dieu khien Aimbot", "Hien thi hinh anh", "Cai dat he thong", "THOAT INTERNAL",
                "HIEU NANG AIMBOT", "Bat Flickbot", "Tam nhin (FOV)", "Do muot (Smooth)", "Do manh Aim", "Vung chet di chuyen", "Vi tri Aim (Bone)", "Phim kich hoat", "Tu dong ban (Hit muc tieu)",
                "MACRO GHIM TAM", "Bat Macro tuy chinh", "Cuong do doc", "Cuong do ngang", "Do tre Macro (ms)", "Luu y: Ghi chuot xuong khi giu CHUOT TRAI",
                "CAU HINH HINH ANH", "Bat ESP tong", "Khung nguoi choi", "Ve xuong", "Hien thi mau", "Dau dau", "Hien thi ten", "Khoang cach", "Bo qua dong doi", "Kieu khung", "KC ve toi da",
                "CHAN DOAN HE THONG", "Kiem tra ket noi Kernel:", "CHAY THU CHUOT", "CHAY THU CLICK", "An menu khi stream", "Bat Radar thu nho", "LUU CAU HINH", "TAI CAU HINH",
                "GZ-ProjectX v2.1 | Shared-Memory Core | AMD/Intel Support",
                "Mau sac & Do trong suot", "Muc tieu", "Mau nhin thay", "Mau bi che", "Dong bo mau an"
            };
        }
        // Default English
        return {
            "GZ-ProjectX", "PROJECT CORE", "Aimbot Control", "Visual Overlays", "System Settings", "EXIT INTERNAL",
            "AIMBOT PERFORMANCE", "Enable Flickbot", "Target FOV", "Smooth Factor", "Aim Strength", "Movement Deadzone", "Target Bone", "Trigger Key", "Auto Fire (Magnetic Snap)",
            "RECOIL MACRO (PULL-DOWN)", "Enable Custom Macro", "Vertical Intensity", "Horizontal Intensity", "Macro Delay (ms)", "Note: Pulls mouse down while holding LEFT CLICK",
            "VISUAL CONFIGURATION", "Enable Master ESP", "Player Boxes", "Skeleton Trace", "Health Indicator", "Head Marker", "Display Names", "Distance Labels", "Ignore Teammates", "Box Style", "Max Render Dist",
            "SYSTEM DIAGNOSTICS", "Kernel Connection Tests:", "RUN MOUSE TEST", "RUN CLICK TEST", "Stream Proof (Hide Overlay)", "Enable Mini-Radar", "SAVE CONFIGURATION", "LOAD CONFIGURATION",
            "GZ-ProjectX v2.1 | Shared-Memory Core | AMD/Intel Support",
            "Individual Colors & Alpha", "Target Item", "Color Visible", "Color Invisible", "Sync Invisible to Global"
        };
    }
}

