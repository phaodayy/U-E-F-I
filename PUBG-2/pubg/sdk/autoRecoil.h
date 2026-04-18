#pragma once
#include <DMALibrary/Memory/Memory.h>
#include "common/Data.h"      // 假设这些头文件路径正确
#include "common/Entitys.h"
#include "utils/KmBox.h"
#include "utils/Lurker.h"
#include "utils/KmBoxNet.h"
#include "utils/MoBox.h"
#include <cmath> // 用于 fabsf
#include "MouseDispatcher.h"

class Recoil
{
public:
	/**
	 * @brief 根据配置将鼠标移动指令分发给对应的硬件控制器
	 * @param X 横向移动距离
	 * @param Y 纵向移动距离
	 */
	static void Move(float X, float Y)
	{
		MouseDispatcher::AddRecoil(X, Y);
	}

	static void autoRecoil()
	{
		static bool isCompensating = false; 
		static float initialYaw = 0.0f;     
		static float initialPitch = 0.0f;   
		static FRotator lastRotation;       
		
		// Bộ theo dõi tổng pixels đã di chuyển trong đợt sấy này
		static float totalMovedPixelsX = 0.0f;
		static float totalMovedPixelsY = 0.0f;

		const float YAW_RESET_THRESHOLD = 0.35f; 
		auto recolHandle = mem.CreateScatterHandle();

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			if (!GameData.Config.AimBot.Recoilenanlek)
			{
				isCompensating = false;
				totalMovedPixelsX = 0.0f;
				totalMovedPixelsY = 0.0f;
				continue;
			}

			if (GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::AR &&
				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::SMG &&
				GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType != WeaponType::LMG)
			{
				isCompensating = false; 
				continue;
			}

			if (GameData.Keyboard.IsKeyDown(1))
			{
				FRotator recoilRotation;
				mem.AddScatterRead(recolHandle, GameData.LocalPlayerInfo.AnimScriptInstance + Offset::RecoilADSRotation_CP, &recoilRotation);
				mem.ExecuteReadScatter(recolHandle);

				if (!isCompensating)
				{
					isCompensating = true;
					initialYaw = recoilRotation.Yaw;
					initialPitch = recoilRotation.Pitch;
					lastRotation = recoilRotation;
					totalMovedPixelsX = 0.0f;
					totalMovedPixelsY = 0.0f;
					continue; 
				}

				float recoilDelta = fabsf(recoilRotation.Pitch - lastRotation.Pitch) + fabsf(recoilRotation.Yaw - lastRotation.Yaw);
				
				float fov = Data::GetCamera().FOV;
				if (fov <= 0) fov = 90.0f;

				// Chon cấu hình recoil dựa trên FOV (Scope)
				float currentRecoilValue = 10.0f; 
				int msGap = 1;

				if (fov >= 70) { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[0];
					msGap = GameData.Config.AimBot.interval[0];
				} else if (fov >= 40) { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[1];
					msGap = GameData.Config.AimBot.interval[1];
				} else if (fov >= 25) { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[2];
					msGap = GameData.Config.AimBot.interval[2];
				} else if (fov >= 18) { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[3];
					msGap = GameData.Config.AimBot.interval[3];
				} else if (fov >= 12) { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[4];
					msGap = GameData.Config.AimBot.interval[4];
				} else { 
					currentRecoilValue = (float)GameData.Config.AimBot.yRecoil[5];
					msGap = GameData.Config.AimBot.interval[5];
				}

				float yMult = currentRecoilValue / 10.0f; 
				float xMult = yMult * 0.4f; 

				// Tính độ chênh lệch so với lúc bắt đầu sấy (Reset điểm neo nếu ngừng giật)
				float yawOff = recoilRotation.Yaw - initialYaw;
				float pitchOff = recoilRotation.Pitch - initialPitch;

				if (yawOff > 180.f) yawOff -= 360.f; else if (yawOff < -180.f) yawOff += 360.f;
				if (pitchOff > 180.f) pitchOff -= 360.f; else if (pitchOff < -180.f) pitchOff += 360.f;

				// Nếu độ giật về 0 (đã hồi tâm xong), reset điểm neo
				if (fabsf(yawOff) < 0.01f && fabsf(pitchOff) < 0.01f) {
					initialYaw = recoilRotation.Yaw;
					initialPitch = recoilRotation.Pitch;
				}

				// Tính toán pixels cần bù (Pitch âm là giật lên -> Cần move mouse dương là ghì xuống)
				float targetNeededX = -yawOff * 20.0f * xMult;
				float targetNeededY = -pitchOff * 20.0f * yMult;

				float mX = targetNeededX - totalMovedPixelsX;
				float mY = targetNeededY - totalMovedPixelsY;

				if (fabsf(mX) > 0.001f || fabsf(mY) > 0.001f)
				{
					Move(mX, mY);
					totalMovedPixelsX += mX;
					totalMovedPixelsY += mY;
				}

				if (msGap > 0) std::this_thread::sleep_for(std::chrono::milliseconds(msGap));
				lastRotation = recoilRotation;
			}
			else
			{
				isCompensating = false;
				totalMovedPixelsX = 0.0f;
				totalMovedPixelsY = 0.0f;
			}
		}
		mem.CloseScatterHandle(recolHandle);
	}
};