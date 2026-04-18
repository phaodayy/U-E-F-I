#pragma once
#include <mutex>
#include <thread>
#include <chrono>
#include "utils/Driver.h"
#include "utils/UsermodeMouse.h"
#include "common/Data.h"

class MouseDispatcher {
private:
    static inline float pendingAimX = 0.0f;
    static inline float pendingAimY = 0.0f;
    static inline float pendingRecoilX = 0.0f;
    static inline float pendingRecoilY = 0.0f;

    static inline float accumX = 0.0f;
    static inline float accumY = 0.0f;

    static inline std::mutex mtx;

public:
    static void AddAim(float x, float y) {
        std::lock_guard<std::mutex> lock(mtx);
        pendingAimX += x;
        pendingAimY += y;
    }

    static void AddRecoil(float x, float y) {
        std::lock_guard<std::mutex> lock(mtx);
        pendingRecoilX += x;
        pendingRecoilY += y;
    }

    static void Reset() {
        std::lock_guard<std::mutex> lock(mtx);
        pendingAimX = 0; pendingAimY = 0;
        pendingRecoilX = 0; pendingRecoilY = 0;
        accumX = 0; accumY = 0;
    }

    static void DispatchThreadFunc() {
        while (true) {
            float moveX = 0, moveY = 0;
            {
                std::lock_guard<std::mutex> lock(mtx);
                // Gộp Aim và Recoil theo sơ đồ
                moveX = pendingAimX + pendingRecoilX + accumX;
                moveY = pendingAimY + pendingRecoilY + accumY;

                pendingAimX = 0; pendingAimY = 0;
                pendingRecoilX = 0; pendingRecoilY = 0;
            }

            int iX = (int)moveX;
            int iY = (int)moveY;

            // Lưu lại phần dư cho frame sau
            {
                std::lock_guard<std::mutex> lock(mtx);
                accumX = moveX - iX;
                accumY = moveY - iY;
            }

            if (iX != 0 || iY != 0) {
                switch (GameData.Config.AimBot.Controller) {
                    case 4: Driver::Move(iX, iY); break;
                    case 5: UsermodeMouse::Move(iX, iY); break;
                }
            }

            // Dong bo voi AimBot o muc 250Hz: dispatch moi 4ms de muot ma va on dinh nhat
            std::this_thread::sleep_for(std::chrono::milliseconds(4));
        }
    }
};
