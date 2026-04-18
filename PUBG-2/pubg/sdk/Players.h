#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/Decrypt.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <Hack/GNames.h>
#include <Utils/Timer.h>
#include <Hack/Process.h>
#include <Hack/LineTrace.h>
#include <Hack/PhysXManager.h>
#include <Common/Offset.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

inline std::queue<std::pair<std::string, PlayerRankList>> RankWorkQueue;
inline std::mutex RankQueueMutex;
inline std::condition_variable RankQueueCondition;
inline int PlayerRankWorkCount = 0;

inline uint8_t* xe_decrypt_container = []() -> uint8_t* {
    auto p = (uint8_t*)VirtualAlloc(nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return p;
}();

class Players
{
public:

	static void DecryptedFloat(float* buffer, unsigned int size, char offset) {
		uint32_t decryptionTable[16] = { Offset::HealthKey0, Offset::HealthKey1, Offset::HealthKey2, Offset::HealthKey3,
							Offset::HealthKey4, Offset::HealthKey5, Offset::HealthKey6, Offset::HealthKey7,
							Offset::HealthKey8, Offset::HealthKey9, Offset::HealthKey10, Offset::HealthKey11,
							Offset::HealthKey12, Offset::HealthKey13, Offset::HealthKey14, Offset::HealthKey15 };

		auto* ptr = reinterpret_cast<unsigned char*>(buffer);
		for (unsigned int i = 0; i < size; ++i) {
			char index = i + offset;
			ptr[i] ^= reinterpret_cast<char*>(decryptionTable)[index & 0x3F];
		}
	}

	static void LoadLists(int type)
	{
		std::unordered_map <std::string, int> playerWhiteLists;
		std::unordered_map <std::string, int> playerBlackLists;
		std::string filename = "C:\\BlackLists.txt";
		if (type == 2) filename = "C:\\WhiteLists.txt";

		std::ifstream file(filename);
		if (!file.is_open()) return;

		std::string line;
		while (std::getline(file, line)) {
			if (type == 2) playerWhiteLists[line] = 1;
			else playerBlackLists[line] = 1;
		}

		if (type == 2) Data::SetPlayerWhiteLists(playerWhiteLists);
		else Data::SetPlayerBlackLists(playerBlackLists);
		file.close();
	}

	static void SaveLists(int type) {
		std::unordered_map <std::string, int> playerWhiteLists = Data::GetPlayerWhiteLists();
		std::unordered_map <std::string, int> playerBlackLists = Data::GetPlayerBlackLists();
		std::string filename;
		std::unordered_map<std::string, int> currentList;

		if (type == 2) {
			filename = "C:\\WhiteLists.txt";
			currentList = playerWhiteLists;
		}
		else {
			filename = "C:\\BlackLists.txt";
			currentList = playerBlackLists;
		}

		std::ofstream file(filename);
		if (!file.is_open()) return;

		for (const auto& entry : currentList) {
			file << entry.first << std::endl;
		}
		file.close();
	}

	static void ReadPlayerLists()
	{
		LoadLists(1);
		LoadLists(2);
	}

	static void UpdateFogPlayers()
	{
		while (true)
		{
			if (GameData.Scene != Scene::Gaming)
			{
				Sleep(GameData.ThreadSleep);
				continue;
			}

			std::unordered_map<uint64_t, FogPlayerInfo> fogPlayerInfos;
			TArray<uint64_t> dormantCharacterClientList = mem.Read<TArray<uint64_t>>(GameData.AntiCheatCharacterSyncManager + Offset::DormantCharacterClientList);
			for (auto fogPlayerEntity : dormantCharacterClientList.GetVector())
			{
				FogPlayerInfo fogPlayer;
				fogPlayerInfos[fogPlayerEntity] = fogPlayer;
			}

			Data::SetFogPlayers(fogPlayerInfos);
			// Tang sleep len 200ms: fog players khong can cap nhat lien tuc
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	static void UpdatePlayerLists()
	{
		if (GameData.Scene == Scene::Gaming)
		{
			auto playerBlackLists = Data::GetPlayerBlackLists();
			auto playerWhiteLists = Data::GetPlayerWhiteLists();
			auto scatterHandle = mem.CreateScatterHandle();

			std::unordered_map<std::string, GamePlayerInfo> gPlayerLists;
			std::unordered_map<std::string, PlayerRankList> playerRankLists = Data::GetPlayerRankLists();

			std::vector<GamePlayerInfo> playerLists;
			GameData.PlayerCount = mem.Read<int>(GameData.GameState + Offset::PlayerArray + 0x8);
			GameData.NumAliveTeams = mem.Read<int>(GameData.GameState + Offset::NumAliveTeams);
			if (GameData.PlayerCount <= 0)
			{
				mem.CloseScatterHandle(scatterHandle);
				return;
			}
			
			uint64_t* playerArrayBuffer = new uint64_t[GameData.PlayerCount];
			mem.Read(mem.Read<uint64_t>(GameData.GameState + Offset::PlayerArray), playerArrayBuffer, sizeof(uint64_t) * GameData.PlayerCount);
			std::vector<uint64_t> playerArray(playerArrayBuffer, playerArrayBuffer + GameData.PlayerCount);
			delete[] playerArrayBuffer;

			for (auto& pPlayerInfo : playerArray)
			{
				GamePlayerInfo player;
				player.pPlayerInfo = pPlayerInfo;
				playerLists.push_back(player);
			}

			std::unordered_map<uint64_t, GamePlayerInfo> cachedGlobalLists = Data::GetGlobalPlayerCache(); 

			const size_t batchSize = 30;
			for (size_t i = 0; i < playerLists.size(); i += batchSize) {
				size_t end = (std::min)(i + batchSize, playerLists.size());
				bool hasNewPlayers = false;

				for (size_t j = i; j < end; ++j) {
					GamePlayerInfo& player = playerLists[j];
					if (cachedGlobalLists.count(player.pPlayerInfo)) {
						player = cachedGlobalLists[player.pPlayerInfo];
						continue;
					}
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::PlayerName, (uint64_t*)&player.pPlayerName);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::PlayerStatusType, (BYTE*)&player.StatusType);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::AccountId, (uint64_t*)&player.pAccountId);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::TeamNumber, (int*)&player.TeamID);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::SquadMemberIndex, (int*)&player.SquadMemberIndex);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::PartnerLevel, (EPartnerLevel*)&player.PartnerLevel);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::PlayerStatistics, (int*)&player.KillCount);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::DamageDealtOnEnemy, (float*)&player.DamageDealtOnEnemy);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::CharacterClanInfo + 0x20, (uint64_t*)&player.pClanName);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::SurvivalTier, (int*)&player.PubgIdData.SurvivalTier);
					mem.AddScatterRead(scatterHandle, player.pPlayerInfo + Offset::SurvivalLevel, (int*)&player.PubgIdData.SurvivalLevel);
					hasNewPlayers = true;
				}

				if (hasNewPlayers) mem.ExecuteReadScatter(scatterHandle);
			}

			for (size_t i = 0; i < playerLists.size(); i += batchSize) {
				size_t end = (std::min)(i + batchSize, playerLists.size());
				bool hasNewPlayers = false;
				for (size_t j = i; j < end; ++j) {
					GamePlayerInfo& player = playerLists[j];
					if (player.PlayerName != "") continue;

					mem.AddScatterRead(scatterHandle, player.pClanName, (FText*)&player.FClanName);
					mem.AddScatterRead(scatterHandle, player.pPlayerName, (FText*)&player.FPlayerName);
					mem.AddScatterRead(scatterHandle, player.pAccountId, (FText*)&player.FAccountId);
					hasNewPlayers = true;
				}
				if (hasNewPlayers) mem.ExecuteReadScatter(scatterHandle);
			}

			for (GamePlayerInfo& player : playerLists)
			{
				if (player.PlayerName == "") {
					player.ClanName = Utils::UnicodeToAnsi(player.FClanName.buffer);
					player.AccountId = Utils::UnicodeToAnsi(player.FAccountId.buffer);
					player.PlayerName = Utils::UnicodeToAnsi(player.FPlayerName.buffer);
					player.TeamID = (player.TeamID >= 100000) ? (player.TeamID - 100000) : player.TeamID;
					Data::SetGlobalPlayerCacheItem(player.pPlayerInfo, player); 
				}

				player.IsMyTeam = GameData.LocalPlayerInfo.TeamID == player.TeamID;

				if (playerBlackLists.find(player.AccountId) != playerBlackLists.end()) player.ListType = 1;
				else if (playerWhiteLists.find(player.AccountId) != playerWhiteLists.end()) player.ListType = 2;

				if (player.PubgIdData.SurvivalTier > 0) player.PubgIdData.SurvivalLevel = (player.PubgIdData.SurvivalTier - 1) * 500 + player.PubgIdData.SurvivalLevel;
			}

			for (GamePlayerInfo& player : playerLists)
			{
				if (player.PlayerName == "") continue;
				gPlayerLists[player.PlayerName] = player;

				if (playerRankLists.count(player.PlayerName) == 0)
				{
					PlayerRankList playerRankList;
					playerRankList.AccountId = player.AccountId;
					playerRankList.PlayerName = player.PlayerName;
					playerRankList.Tem = player.TeamID;
					playerRankList.DamageAmount = player.DamageDealtOnEnemy;
					playerRankList.Survivallevel = player.PubgIdData.SurvivalLevel;
					Data::SetPlayerRankListsItem(playerRankList.AccountId, playerRankList);
				}
			}

			Data::SetPlayerLists(gPlayerLists);
			mem.CloseScatterHandle(scatterHandle);
		}
		else Data::SetPlayerLists({});
	}

private:
	static void UpdatePlayerCache(std::unordered_map<uint64_t, Player>& cachePlayers, std::unordered_map<uint64_t, Player>& playersData, std::vector<Player*>& playersNeedingPointers, std::vector<Player*>& allPlayers) {
		for (auto& item : cachePlayers) {
			Player& p = item.second;
			allPlayers.push_back(&p);

			if (playersData.count(p.Entity)) {
				const Player& cached = playersData[p.Entity];
				if (cached.RootComponent != 0) {
					p.RootComponent = cached.RootComponent;
					p.CharacterMovement = cached.CharacterMovement;
					p.PlayerState = cached.PlayerState;
					p.WeaponProcessor = cached.WeaponProcessor;
					p.pCharacterName = cached.pCharacterName;
					p.bEncryptedHealth = cached.bEncryptedHealth;
					p.EncryptedHealthOffset = cached.EncryptedHealthOffset;
					p.DecryptedHealthOffset = cached.DecryptedHealthOffset;
					p.StaticMesh = cached.StaticMesh;
					p.TeamID = cached.TeamID;
					p.CharacterState = cached.CharacterState;
					p.SpectatedCount = cached.SpectatedCount;
					p.EquippedWeapons = cached.EquippedWeapons;
					continue;
				}
			}
			playersNeedingPointers.push_back(&p);
		}
	}

	static void ReadPlayerPointers(VMMDLL_SCATTER_HANDLE hScatter, std::vector<Player*>& playersNeedingPointers, std::vector<Player*>& allPlayers) {
		const size_t batchSize = 30;

		if (!playersNeedingPointers.empty()) {
			for (size_t i = 0; i < playersNeedingPointers.size(); i += batchSize) {
				size_t end = (std::min)(i + batchSize, playersNeedingPointers.size());
				for (size_t j = i; j < end; ++j) {
					Player* p = playersNeedingPointers[j];
					mem.AddScatterRead(hScatter, p->Entity + Offset::RootComponent, (uint64_t*)&p->RootComponent);
					mem.AddScatterRead(hScatter, p->Entity + Offset::CharacterMovement, (uint64_t*)&p->CharacterMovement);
					mem.AddScatterRead(hScatter, p->Entity + Offset::PlayerState, (uint64_t*)&p->PlayerState);
					mem.AddScatterRead(hScatter, p->Entity + Offset::WeaponProcessor, (uint64_t*)&p->WeaponProcessor);
					mem.AddScatterRead(hScatter, p->Entity + Offset::CharacterName, (uint64_t*)&p->pCharacterName);
					mem.AddScatterRead(hScatter, p->Entity + Offset::bEncryptedHealth, (BYTE*)&p->bEncryptedHealth);
					mem.AddScatterRead(hScatter, p->Entity + Offset::EncryptedHealthOffset, (unsigned char*)&p->EncryptedHealthOffset);
					mem.AddScatterRead(hScatter, p->Entity + Offset::DecryptedHealthOffset, (unsigned char*)&p->DecryptedHealthOffset);
					
					mem.AddScatterRead(hScatter, p->Entity + Offset::MortarLocation, (uint64_t*)&p->MortarEntity);
					mem.AddScatterRead(hScatter, p->Entity + Offset::VehicleRiderComponent, (uint64_t*)&p->VehicleRiderComponent);
				}
				mem.ExecuteReadScatter(hScatter);
			}
		}

		for (size_t i = 0; i < allPlayers.size(); i += batchSize) {
			size_t end = (std::min)(i + batchSize, allPlayers.size());
			for (size_t j = i; j < end; ++j) {
				Player* p = allPlayers[j];
				mem.AddScatterRead(hScatter, p->Entity + Offset::Mesh, (uint64_t*)&p->MeshComponent);
			}
			mem.ExecuteReadScatter(hScatter);
		}

		for (size_t i = 0; i < allPlayers.size(); i += batchSize) {
			size_t end = (std::min)(i + batchSize, allPlayers.size());
			for (size_t j = i; j < end; ++j) {
				Player* p = allPlayers[j];
				if (p->MeshComponent) {
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::BoneCount, (int*)&p->BoneCount);
				}
			}
			mem.ExecuteReadScatter(hScatter);
		}

		if (!playersNeedingPointers.empty()) {
			for (Player* p : playersNeedingPointers) {
				p->RootComponent = Decrypt::Xe(p->RootComponent);
				p->CharacterMovement = Decrypt::Xe(p->CharacterMovement);
				p->PlayerState = Decrypt::Xe(p->PlayerState);
			}
		}
	}

	static CameraData ReadPlayerData(VMMDLL_SCATTER_HANDLE hScatter, std::vector<Player*>& allPlayers, std::unordered_map<uint64_t, Player>& playersData, bool bSlowUpdate) {
		// Sync Camera Data - Dùng CameraCache để khớp với hệ thống tọa độ game
		CameraData currentCamera;
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheLocation, &currentCamera.Location);
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheRotation, &currentCamera.Rotation);
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheFOV, &currentCamera.FOV);
		mem.AddScatterRead(hScatter, GameData.UWorld + Offset::TimeSeconds, &GameData.WorldTimeSeconds);
		// mem.ExecuteReadScatter(hScatter); // Combine with first batch

		// Kiem tra aimbot active de quyet dinh doc AimOffsets hay khong
		bool needAimOffsets = GameData.Config.AimBot.Enable;

		const size_t batchSize = 30;
		for (size_t i = 0; i < allPlayers.size(); i += batchSize) {
			size_t end = (std::min)(i + batchSize, allPlayers.size());
			for (size_t j = i; j < end; ++j) {
				Player* p = allPlayers[j];
				if (p->RootComponent) {
					mem.AddScatterRead(hScatter, p->RootComponent + Offset::ComponentLocation, (FVector*)&p->Location);
					mem.AddScatterRead(hScatter, p->RootComponent + Offset::ComponentVelocity, (FVector*)&p->ComponentVelocity);
				}
				if (p->bEncryptedHealth == 0) mem.AddScatterRead(hScatter, p->Entity + Offset::Health, (float*)&p->Health);
				else mem.AddScatterRead(hScatter, p->Entity + Offset::Health + p->EncryptedHealthOffset, (float*)&p->Health);
				
				mem.AddScatterRead(hScatter, p->Entity + Offset::GroggyHealth, (float*)&p->GroggyHealth);

				if (bSlowUpdate) {
					mem.AddScatterRead(hScatter, p->Entity + Offset::LastTeamNum, (int*)&p->TeamID);
					mem.AddScatterRead(hScatter, p->Entity + Offset::CharacterState, (ECharacterState*)&p->CharacterState);
					mem.AddScatterRead(hScatter, p->Entity + Offset::SpectatedCount, (int*)&p->SpectatedCount);
					mem.AddScatterRead(hScatter, p->Entity + Offset::Gender, (EGender*)&p->Gender);
				}
				// Chi doc AimOffsets khi aimbot bat (tiet kiem ~N*8 bytes/frame khi aimbot tat)
				if (needAimOffsets)
					mem.AddScatterRead(hScatter, p->Entity + Offset::AimOffsets, (FRotator*)&p->AimOffsets);

				if (p->CharacterMovement)
					mem.AddScatterRead(hScatter, p->CharacterMovement + Offset::LastUpdateVelocity, (FVector*)&p->Velocity);

				if (p->MeshComponent) {
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::AnimScriptInstance, (uint64_t*)&p->AnimScriptInstance);
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::ComponentToWorld, (FTransform*)&p->ComponentToWorld);
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::StaticMesh, (uint64_t*)&p->StaticMesh);
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::LastSubmitTime, (float*)&p->LastSubmitTime);
					mem.AddScatterRead(hScatter, p->MeshComponent + Offset::LastRenderTimeOnScreen, (float*)&p->LastRenderTimeOnScreen);

					if (bSlowUpdate) {
						mem.AddScatterRead(hScatter, p->MeshComponent + Offset::bAlwaysCreatePhysicsState, (UCHAR*)&p->bAlwaysCreatePhysicsState);
					}
				}
				if (p->VehicleRiderComponent) {
					mem.AddScatterRead(hScatter, p->VehicleRiderComponent + Offset::SeatIndex, (int*)&p->SeatIndex);
					mem.AddScatterRead(hScatter, p->VehicleRiderComponent + Offset::LastVehiclePawn, (uint64_t*)&p->LastVehiclePawn);
				}
				bool needsName = !playersData.count(p->Entity) || playersData[p->Entity].Name.empty();
				if (needsName && p->pCharacterName) mem.AddScatterRead(hScatter, p->pCharacterName, (FText*)&p->CharacterName);

				if (p->WeaponProcessor) {
					if (bSlowUpdate) mem.AddScatterRead(hScatter, p->WeaponProcessor + Offset::EquippedWeapons, (uint64_t*)&p->EquippedWeapons);
					mem.AddScatterRead(hScatter, p->WeaponProcessor + Offset::CurrentWeaponIndex, (BYTE*)&p->CurrentWeaponIndex);
				}

				if (bSlowUpdate && p->Entity == GameData.AcknowledgedPawn) {
					mem.AddScatterRead(hScatter, p->Entity + Offset::MortarLocation, (uint64_t*)&p->MortarEntity);
				}
				if (p->AnimScriptInstance) {
					mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::PreEvalPawnState, (EAnimPawnState*)&p->PreEvalPawnState);
					mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::bIsScoping_CP, (bool*)&p->IsScoping);
					mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::ControlRotation_CP, (FRotator*)&p->ControlRotation);
					mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::LeanLeftAlpha_CP, (float*)&p->LeanLeftAlpha_CP);
					mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::LeanRightAlpha_CP, (float*)&p->LeanRightAlpha_CP);
					if (p->IsMe) {
						mem.AddScatterRead(hScatter, p->AnimScriptInstance + Offset::RecoilADSRotation_CP, (FRotator*)&p->RecoilADSRotation);
					}
				}
			}
			auto dmaStart = std::chrono::high_resolution_clock::now();
			mem.ExecuteReadScatter(hScatter);
			auto dmaEnd = std::chrono::high_resolution_clock::now();
			GameData.Performance.DmaLatencyMs = std::chrono::duration<float, std::milli>(dmaEnd - dmaStart).count();

			if (i == 0) { // Get camera data from first batch
				// currentCamera populated by scatter read at start
			}
		}
		return currentCamera;
	}

	static void ProcessPlayerData(std::unordered_map<uint64_t, Player>& cachePlayers, std::unordered_map<uint64_t, Player>& playersData, int& fogPlayerCount, std::vector<Player*>& playersNeedingBones, std::vector<Player*>& playersNeedingWeaponPtr, const CameraData& camera) {
		static auto gamePlayerLists = Data::GetPlayerLists();
		static uint64_t lastPlayerListUpdateTime = 0;
		uint64_t currentTime = GetTickCount64();

		if (currentTime - lastPlayerListUpdateTime > 1000) {
			gamePlayerLists = Data::GetPlayerLists();
			lastPlayerListUpdateTime = currentTime;
		}
		
		const FMatrix cameraRotationMatrix = camera.Rotation.GetMatrix();

		for (auto& item : cachePlayers)
		{
			Player& player = item.second;
			if (player.bEncryptedHealth != 0) DecryptedFloat(&player.Health, sizeof(player.Health), player.DecryptedHealthOffset);

			if (player.CharacterName.buffer[0] != 0) player.Name = Utils::RemoveBracketsAndTrim(Utils::UnicodeToAnsi(player.CharacterName.buffer));
			else if (playersData.count(player.Entity)) player.Name = playersData[player.Entity].Name;

			if (!player.Name.empty() && gamePlayerLists.count(player.Name)) {
				GamePlayerInfo playerInfo = gamePlayerLists[player.Name];
				player.ClanName = playerInfo.ClanName;
				player.SurvivalLevel = playerInfo.PubgIdData.SurvivalLevel;
				player.PartnerLevel = playerInfo.PartnerLevel;
				player.DamageDealtOnEnemy = playerInfo.DamageDealtOnEnemy;
				player.KillCount = playerInfo.KillCount;
				player.ListType = playerInfo.ListType;
				player.SquadMemberIndex = playerInfo.SquadMemberIndex;
				player.AccountId = playerInfo.AccountId;
			}

			if (player.Entity == GameData.AcknowledgedPawn) player.IsMe = true;
			player.TeamID = (player.TeamID >= 100000) ? (player.TeamID - 100000) : player.TeamID;
			if (GameData.Config.ESP.duiyou) player.IsMyTeam = false;
			else player.IsMyTeam = player.TeamID == GameData.LocalPlayerTeamID;

			// TÍNH KHOẢNG CÁCH ĐỒNG BỘ TẠI ĐÂY
			player.Distance = camera.Location.Distance(player.Location) / 100.0f;

			FVector2D worldToScreen = VectorHelper::WorldToScreen(player.Location, camera.Location, cameraRotationMatrix, camera.FOV);
			if (player.bAlwaysCreatePhysicsState == 4) {
				player.InFog = true; player.InScreen = false; fogPlayerCount++;
			}
			else {
				if (!player.IsMe && (worldToScreen.X < -100 || worldToScreen.X > GameData.Config.Overlay.ScreenWidth + 100 || worldToScreen.Y < -100 || worldToScreen.Y > GameData.Config.Overlay.ScreenHeight + 100)) player.InScreen = false;
				else player.InScreen = true;
			}

			if (player.InScreen && !player.IsMe && !player.IsMyTeam) {
				if (player.StaticMesh) {
					// Nâng cấp Hiệu năng (Adaptive Scatter Read)
					uint64_t currentTick = GetTickCount64();
					uint64_t msSinceLastBoneUpdate = currentTick - player.LastBoneUpdateTick;
					
					float CenterX = GameData.Config.Overlay.ScreenWidth / 2.0f;
					float CenterY = GameData.Config.Overlay.ScreenHeight / 2.0f;
					float distToCrosshair = std::sqrt(std::pow(worldToScreen.X - CenterX, 2) + std::pow(worldToScreen.Y - CenterY, 2));

					float distance = Data::GetCamera().Location.Distance(player.Location) / 100.0f;
					bool shouldUpdateBones = false;

					// ƯU TIÊN TUYỆT ĐỐI CHO MỤC TIÊU ĐANG BỊ LOCK
					bool isAimTarget = (GameData.AimBot.Target != 0 && GameData.AimBot.Target == player.Entity);

					if (isAimTarget || distance <= 150.0f || distToCrosshair <= 400.0f) { // Nới rộng khoảng nạp xương liên tục (150m thay vì 50m)
						shouldUpdateBones = true;
					} 
					else if (distance > 150.0f && distance <= 300.0f) {
						if (msSinceLastBoneUpdate >= 4) shouldUpdateBones = true; // Tăng tần suất (4ms thay vì 8ms)
					}
					else {
						if (msSinceLastBoneUpdate >= 50) shouldUpdateBones = true; // (50ms thay vì 100ms)
					}

					if (shouldUpdateBones) {
						player.LastBoneUpdateTick = currentTick;
						playersNeedingBones.push_back(&player);
					}
				}
				if (player.EquippedWeapons && player.CurrentWeaponIndex >= 0 && player.CurrentWeaponIndex < 8) playersNeedingWeaponPtr.push_back(&player);
			}
			if (player.IsMe && player.EquippedWeapons && player.CurrentWeaponIndex >= 0 && player.CurrentWeaponIndex < 8) playersNeedingWeaponPtr.push_back(&player);
		}
	}

	static void UpdateBonesAndVisibility(VMMDLL_SCATTER_HANDLE hScatter, std::vector<Player*>& players, CameraData& camera) {
		if (players.empty()) return;

		// Đọc lại Camera ngay trong đợt này để đảm bảo Sync tuyệt đối với Bone
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheLocation, &camera.Location);
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheRotation, &camera.Rotation);
		mem.AddScatterRead(hScatter, GameData.PlayerCameraManager + Offset::CameraCacheFOV, &camera.FOV);

		const size_t batchSize = 32;
		for (size_t i = 0; i < players.size(); i += batchSize) {
			size_t end = (std::min)(i + batchSize, players.size());
			for (size_t j = i; j < end; ++j) {
				Player* p = players[j];
				p->Skeleton = std::make_shared<PlayerSkeleton>();
				for (EBoneIndex bone : SkeletonLists::Bones) {
					int actualBoneIndex = GetGenderBoneIndex(p->Gender, bone);
					mem.AddScatterRead(hScatter, p->StaticMesh + (static_cast<unsigned long long>(actualBoneIndex) * 0x30), (FTransform*)&p->Skeleton->Bones[bone]);
				}
			}
			mem.ExecuteReadScatter(hScatter);
		}

		const float myRenderTime = GameData.LocalPlayerInfo.LastRenderTimeOnScreen;
		constexpr float kRenderVisibilityThreshold = 0.05f;
		const FMatrix cameraRotationMatrix = camera.Rotation.GetMatrix();

		for (size_t j = 0; j < players.size(); ++j) {
			Player* p = players[j];

			// Visibility check theo LastRenderTimeOnScreen
			bool isMeshVisible = false;
			if (myRenderTime > 0.0f) {
				isMeshVisible = ((p->LastRenderTimeOnScreen + kRenderVisibilityThreshold) >= myRenderTime);
			} else {
				isMeshVisible = (std::abs(p->LastSubmitTime - p->LastRenderTimeOnScreen) <= kRenderVisibilityThreshold);
			}

			// Tinh toan vi tri xuong va screen position (Toi uu matrix math)
			const FMatrix componentToWorldMatrix = p->ComponentToWorld.ToMatrixWithScale();
			for (EBoneIndex bone : SkeletonLists::Bones) {
				p->Skeleton->LocationBones[bone] = VectorHelper::GetBoneWithRotation(p->Skeleton->Bones[bone], componentToWorldMatrix);
				p->Skeleton->ScreenBones[bone]   = VectorHelper::WorldToScreen(p->Skeleton->LocationBones[bone], camera.Location, cameraRotationMatrix, camera.FOV);
				p->Skeleton->VisibleBones[bone]  = isMeshVisible;
			}

			if (isMeshVisible) {
				p->Skeleton->VisibleBones[EBoneIndex::Head]    = true;
				p->Skeleton->VisibleBones[EBoneIndex::Neck_01] = true;
				p->IsVisible = true;
			} else {
				p->IsVisible = false;
			}
		}
	}

	static void ReadPlayerBonesAndWeapon(VMMDLL_SCATTER_HANDLE hScatter, std::vector<Player*>& playersNeedingBones, std::vector<Player*>& playersNeedingWeaponPtr, std::vector<Player*>& playersNeedingWeaponDetails, CameraData& camera) {
		UpdateBonesAndVisibility(hScatter, playersNeedingBones, camera);

		for (Player* p : playersNeedingWeaponPtr) mem.AddScatterRead(hScatter, p->EquippedWeapons + p->CurrentWeaponIndex * 8, (uint64_t*)&p->CurrentWeapon);
		mem.ExecuteReadScatter(hScatter);

		for (Player* p : playersNeedingWeaponPtr) {
			if (p->CurrentWeapon > 0) {
				mem.AddScatterRead(hScatter, p->CurrentWeapon + Offset::ObjID, (int*)&p->WeaponID);
				mem.AddScatterRead(hScatter, p->CurrentWeapon + Offset::WeaponConfig_WeaponClass, (BYTE*)&p->WeaponClassByte);
				if (p->IsMe) {
					mem.AddScatterRead(hScatter, p->CurrentWeapon + Offset::ElapsedCookingTime, (float*)&p->ElapsedCookingTime);
					if (p->CurrentWeapon) {
						mem.AddScatterRead(hScatter, p->CurrentWeapon + Offset::RecoilValueVector, (FVector*)&p->RecoilValueVector);
					}
				}
				playersNeedingWeaponDetails.push_back(p);
			}
		}
		mem.ExecuteReadScatter(hScatter);
	}

	static void FinalizePlayerUpdate(std::unordered_map<uint64_t, Player>& cachePlayers, std::unordered_map<uint64_t, Player>& playersData, std::unordered_map<uint64_t, tMapInfo>& enemyInfoMap, AimBotConfig& config, bool isWheelKeyDown, const CameraData& camera) {
		for (auto& item : cachePlayers) {
			Player& player = item.second;
			if (player.CurrentWeapon > 0) {
				player.WeaponID = Decrypt::CIndex(player.WeaponID);
				auto newInfo = Data::GetGNameListsByIDItem(player.WeaponID);
				if (newInfo.DisplayName != "" && newInfo.DisplayName != "Unknown") {
					player.WeaponEntityInfo = newInfo;
					player.WeaponName = newInfo.DisplayName;
				}
			}
			playersData[player.Entity] = player;
		}

		for (auto& item : cachePlayers)
		{
			Player& player = item.second;
			player.Distance = camera.Location.Distance(player.Location) / 100.0f;
			player.IsAimMe = false;
			player.State = player.Health > 0.0f ? CharacterState::Alive : player.GroggyHealth > 0.0f ? CharacterState::Groggy : CharacterState::Dead;

			if (player.CharacterState == ECharacterState::Offline) player.RadarState = ECharacterIconType::Quitter;
			else if (player.State == CharacterState::Groggy) player.RadarState = ECharacterIconType::Groggy;

			player.Location = player.ComponentToWorld.Translation;
			float timeStampDelta = GameData.WorldTimeSeconds - enemyInfoMap[player.Entity].TimeStamp;
			enemyInfoMap[player.Entity].TimeStamp = GameData.WorldTimeSeconds;

			[&] {
				auto& posInfo = enemyInfoMap[player.Entity].PosInfo.Info;
				if (player.State == CharacterState::Dead) posInfo.clear();
				else {
					if (player.Velocity.Length() < 0.1f) { // Neu Engine Velocity bang 0, thu uoc tinh (co the dang dung im hoac lag)
						if (timeStampDelta) posInfo.push_front({ GameData.WorldTimeSeconds, player.Location });
						if (posInfo.size() > 100) posInfo.pop_back();
						float sumTimeDelta = 0.0f; FVector sumPosDif;
						for (size_t i = 1; i < posInfo.size(); i++) {
							const float deltaTime = posInfo[i - 1].Time - posInfo[i].Time;
							const FVector deltaPos = posInfo[i - 1].Pos - posInfo[i].Pos;
							const FVector deltaVelocity = deltaPos * (1.0f / deltaTime);
							if (deltaTime > 0.05f) posInfo.clear();
							else {
								sumTimeDelta += deltaTime; sumPosDif = sumPosDif + deltaPos;
								if (sumTimeDelta > 0.15f) break;
							}
						}
						if (sumTimeDelta > 0.1f) player.Velocity = sumPosDif * (1.0f / sumTimeDelta);
					}
					// Neu player.Velocity da co du lieu tu Engine (0x3D0), ta khong can tinh lai qua phuc tap o day
				}
			}();

			player.LastUpdateTime = GameData.WorldTimeSeconds;
			player.LastUpdateTick = GetTickCount64();
			playersData[player.Entity] = player;
			if (player.IsMe) { 
				GameData.LocalPlayerInfo = player; 
				GameData.LocalPlayerTeamID = player.TeamID;
				Data::SetLocalAimData(player.ControlRotation, player.RecoilADSRotation, player.RecoilValueVector);
			}
			if (!player.InScreen || player.IsMe || player.IsMyTeam) continue;

			FVector aimDirection = FRotator(0.0f, player.AimOffsets.Yaw, 0.0f).GetUnitVector();
			FVector aimFov = VectorHelper::CalculateAngles(player.Location, GameData.LocalPlayerInfo.Location);
			FRotator amiMz = player.AimOffsets; amiMz.Clamp();
			if (abs(int32_t(abs(aimFov.X - amiMz.Yaw))) <= 2 && abs(int32_t(abs(aimFov.Y - amiMz.Pitch))) <= 2) player.IsAimMe = true;

			if (!isWheelKeyDown)
			{
				AimBotConfig& config_ref = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];
				bool isFirstKey = GameData.Keyboard.IsKeyDown(config_ref.First.Key);
				bool isSecondKey = GameData.Keyboard.IsKeyDown(config_ref.Second.Key);
				bool isConfigKey = isFirstKey || isSecondKey;
				
				if (player.State != CharacterState::Dead || isConfigKey)
				{
					// LOGIC LOCK MUC TIEU: Neu da Lock thi chi update xuong cho muc tieu do, khong switch sang thang khac
					bool isCurrentTarget = (GameData.AimBot.Target != 0 && GameData.AimBot.Target == player.Entity);
					bool canSelectNew = !GameData.AimBot.Lock || GameData.AimBot.Target == 0;

					if (isCurrentTarget || canSelectNew) {
						float searchFov = config_ref.FOV;
						if (isCurrentTarget && GameData.AimBot.Lock) searchFov *= 1.5f; // FOV stickiness: nới rộng FOV khi đã khóa

						// CHẾ ĐỘ AIM TỰ ĐỘNG MỚI (CỐ ĐỊNH TRONG CODE)
						int aimMode = isSecondKey ? 1 : 0; // Force Key = AIM HEAD, Primary Key = AIM MAIN
						
						static const std::vector<int> headBones = { EBoneIndex::ForeHead, EBoneIndex::Head, EBoneIndex::Neck_01 }; // Forehead, Head, Neck
						static const std::vector<int> bodyBones = { EBoneIndex::Spine_03, EBoneIndex::Spine_01, EBoneIndex::Pelvis, EBoneIndex::Upperarm_L, EBoneIndex::Upperarm_R, EBoneIndex::Thigh_L, EBoneIndex::Thigh_R }; // Chest, Spine, Pelvis, Shoulders, Thighs
						
						int bestBone = -1;
						float minBoneDist = FLT_MAX;
						FVector2D screenCenter = { GameData.Config.Overlay.ScreenWidth / 2.0f, GameData.Config.Overlay.ScreenHeight / 2.0f };

						// 1. Nếu là AIM HEAD, kiểm tra vùng đầu trước
						if (aimMode == 1) {
							for (int b : headBones) {
								if (!player.Skeleton->VisibleBones[b]) continue; // Vùng nào lộ diện (VisibleBones đã được gán = isMeshVisible ở trên)
								bestBone = b;
								break; // Ưu tiên đầu tiên trong list (thường là Forehead)
							}
						}

						// 2. Nếu chưa tìm thấy (Mode Main hoặc Head bị che), tìm vùng gần tâm nhất
						if (bestBone == -1) {
							std::vector<int> candidates = headBones;
							candidates.insert(candidates.end(), bodyBones.begin(), bodyBones.end());

							for (int b : candidates) {
								if (!player.Skeleton->VisibleBones[b]) continue;
								FVector2D sPos = player.Skeleton->ScreenBones[b];
								if (sPos.X == 0 || sPos.Y == 0) continue;

								float dist = Utils::CalculateDistance(screenCenter.X, screenCenter.Y, sPos.X, sPos.Y);
								if (dist < minBoneDist) {
									minBoneDist = dist;
									bestBone = b;
								}
							}
						}

						// 3. Cập nhật mục tiêu nếu tìm thấy xương hợp lệ
						if (bestBone != -1) {
							float finalDist = (GameData.Config.AimBot.LockMode) ? player.Distance : minBoneDist;
							
							if (finalDist <= (GameData.Config.AimBot.LockMode ? (float)config_ref.AimDistance : searchFov)) {
								if (finalDist < GameData.AimBot.ScreenDistance || isCurrentTarget) {
									GameData.AimBot.ScreenDistance = finalDist;
									GameData.AimBot.Target = player.Entity;
									GameData.AimBot.Bone = bestBone;
									GameData.AimBot.Type = EntityType::Player;
								}
							}
						}
					}
				}
			}
			if (GameData.AimBot.Target == player.Entity) GameData.AimBot.TargetPlayerInfo = player;
		}
	}

public:
	static void Update()
	{
		Throttler throttlered; auto hScatter = mem.CreateScatterHandle();
		std::unordered_map<uint64_t, tMapInfo> enemyInfoMap;
		float timeSeconds = 0.f; static int loopCount = 0;
		MemCache<int> cachePlayerCount{3000}; MemCache<int> cacheNumAliveTeams{3000};

		while (true)
		{
			auto startTime = std::chrono::high_resolution_clock::now();
			loopCount++;

			if (GameData.Scene != Scene::Gaming)
			{
				timeSeconds = 0.f; GameData.FogPlayerCount = 0; enemyInfoMap.clear();
				GameData.PlayerSegmentLists.clear(); GameData.PlayerRankLists.clear();
				Data::SetPlayerLists({}); GameData.LocalPlayerInfo = Player();
				// Ngoai tran dau: reset hieu nang ve 0 de Performance Monitor khong bi "dong bang" so cu
				GameData.Performance.DataThreadMs = 0.0f;
				Sleep(GameData.ThreadSleep); continue;
			}

			int fogPlayerCount = 0; int cachedCount = 0;
			if (!MEM_READ_CACHED(GameData.GameState + Offset::PlayerArray + 0x8, cachedCount, cachePlayerCount)) GameData.PlayerCount = cachedCount;
			else GameData.PlayerCount = cachedCount;

			int cachedTeams = 0;
			if (MEM_READ_CACHED(GameData.GameState + Offset::NumAliveTeams, cachedTeams, cacheNumAliveTeams)) GameData.NumAliveTeams = cachedTeams;

			std::unordered_map<uint64_t, Player> cachePlayers, playersData;
			Data::CopyCachePlayers(cachePlayers);
			Data::CopyPlayersData(playersData);

			std::vector<Player*> playersNeedingPointers, allPlayers;

			UpdatePlayerCache(cachePlayers, playersData, playersNeedingPointers, allPlayers);
			ReadPlayerPointers(hScatter, playersNeedingPointers, allPlayers);
			
			// DOC CAMERA DAU TIEN DE DUNG CHO CONFIG/AIM
			CameraData currentCamera = ReadPlayerData(hScatter, allPlayers, playersData, (loopCount % 60 == 0));
			AimBotConfig config = GameData.Config.AimBot.Configs[GameData.Config.AimBot.ConfigIndex].Weapon[WeaponTypeToString[GameData.LocalPlayerInfo.WeaponEntityInfo.WeaponType]];

			if (config.DynamicFov) { config.FOV *= (90.0f / (currentCamera.FOV <= 0 ? 90.0f : currentCamera.FOV)); config.WheelFOV *= (90.0f / (currentCamera.FOV <= 0 ? 90.0f : currentCamera.FOV)); }
			
			// Reset ScreenDistance chi khi Lock = false de tim BEST target
			if (!GameData.AimBot.Lock) {
				GameData.AimBot.ScreenDistance = 9999.0f;
			}

			bool isAimKeyDown = GameData.Keyboard.IsKeyDown(config.First.Key) || GameData.Keyboard.IsKeyDown(config.Second.Key);
			bool isWheelKeyDown = GameData.Keyboard.IsKeyDown(config.Wheel.Key);

			// Tu dong reset Target neu khong co phím nhan (Tranh reset o dau loop gay race condition)
			if (!isAimKeyDown && !isWheelKeyDown) {
				GameData.AimBot.Target = 0;
				GameData.AimBot.Lock = false;
				GameData.AimBot.ScreenDistance = 9999.0f;
			}

			std::vector<Player*> playersNeedingBones, playersNeedingWeaponPtr;
			ProcessPlayerData(cachePlayers, playersData, fogPlayerCount, playersNeedingBones, playersNeedingWeaponPtr, currentCamera);

			std::vector<Player*> playersNeedingWeaponDetails;
			ReadPlayerBonesAndWeapon(hScatter, playersNeedingBones, playersNeedingWeaponPtr, playersNeedingWeaponDetails, currentCamera);



			FinalizePlayerUpdate(cachePlayers, playersData, enemyInfoMap, config, isWheelKeyDown, currentCamera);

			if ((!GameData.AimBot.Lock || Utils::ValidPtr(GameData.AimBot.Target)) && isWheelKeyDown && config.AimWheel)
			{
				std::unordered_map<uint64_t, VehicleWheelInfo> vehicleWheels = Data::GetVehicleWheels();
				const FMatrix cameraRotationMatrix = currentCamera.Rotation.GetMatrix();
				for (auto& item : vehicleWheels) {
					auto& wheel = item.second; 
					FVector2D screenPos = VectorHelper::WorldToScreen(wheel.Location, currentCamera.Location, cameraRotationMatrix, currentCamera.FOV);
					float dist = Utils::CalculateDistance(GameData.Config.Overlay.ScreenWidth / 2, GameData.Config.Overlay.ScreenHeight / 2, screenPos.X, screenPos.Y);
					if (dist <= config.WheelFOV && dist < GameData.AimBot.ScreenDistance) {
						GameData.AimBot.ScreenDistance = dist; GameData.AimBot.Target = wheel.Wheel; GameData.AimBot.Bone = 0; GameData.AimBot.Type = EntityType::Wheel;
					}
				}
			}

			Data::SetCamera(currentCamera);
			if (!cachePlayers.empty()) { 
				Data::SetPlayers(std::move(cachePlayers)); 
				Data::SetPlayersData(std::move(playersData)); 
			}
			GameData.FogPlayerCount = fogPlayerCount;

			// Ket thuc do DataThread
			auto endTime = std::chrono::high_resolution_clock::now();
			GameData.Performance.DataThreadMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

			// Adaptive sleep nang cao:
			// - Luon uu tien tan suat update vua phai de giam tai cho card DMA
			// - Khong player gan: 15ms (~66fps)
			// - Co player: 8ms (~125fps)
			// - Neu DataThreadMs > 10ms: Sleep them 5ms de xa stress PCIe
			int adaptiveSleep = 8;
			if (playersNeedingBones.empty()) adaptiveSleep = 15;
			if (GameData.Performance.DataThreadMs > 10.0f) adaptiveSleep += 5;
			
			std::this_thread::sleep_for(std::chrono::milliseconds(adaptiveSleep));
		}
		mem.CloseScatterHandle(hScatter);
	}
};
