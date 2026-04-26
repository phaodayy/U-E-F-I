#pragma once
#include <Common/Constant.h>
#include <Common/Data.h>
#include <Common/Config.h>
#include <Common/Offset.h>
#include "Players.h"
#include "Items.h"
#include "Vehicles.h"
#include "precision_calibration.h"
#include "Radar.h"
#include "Projects.h"
#include "Decrypt.h"
#include "LineTrace.h"
#include "SceneExporter.h"
#include "LineTraceHook.h"
#include "MeshPatcher.h"
#include "MouseDispatcher.h"
#include "Utils/MacroEngine.h"

#include <iostream>
#include <thread>
#include <chrono>

namespace diagnostic_node {
	static inline Throttler throttler;
	static inline bool GameProcessFound = false;
	static inline bool PrevGameProcessFound = false;
	static inline DWORD PrevPID = 0;

	static void EntityInit() {
		mem.ScatterRead_Init();
	}

	static void UpdatePID() {
		// This is managed by the throttler inside Update()
	}

	static void Update() {
		static Throttler throttler;
		while (true)
		{
			if (!mem.DMA_INITIALIZED) {
				if (mem.Init("TslGame.exe", false, true)) {
					GameData.DMAConnected = true;
					Utils::Log(1, "[HYPER] CONNECTED :)");
				} else {
					GameData.DMAConnected = false;
					Sleep(2000);
					continue;
				}
			}

			// 1. DONG BO PID VA BASE ADDRESS (CHO CA DMA VA DRIVER)
			throttler.executeTask("UpdatePID", std::chrono::milliseconds(500), [&]() {
				DWORD PID = mem.GetTslGamePID();
				GameData.PID = PID;

				// LUÔN ĐỒNG BỘ PID VÀ BASE CHO DRIVER (SYSTEM B) DÙ GAME ĐÃ TÌM THẤY HAY CHƯA
				if (GameData.PID != 0 && mem.GetProcessBase() != 0) {
					GameData.GameBase = mem.GetProcessBase();
					telemetryMemory::g_ProcessId = GameData.PID;
					telemetryMemory::g_BaseAddress = GameData.GameBase;
				}

				if (GameProcessFound && GameData.PID != 0) {
					char buffer[2];
					if (!VMMDLL_MemReadEx(mem.vHandle, GameData.PID, GameData.GameBase, (PBYTE)buffer, 2, NULL, VMMDLL_FLAG_NOCACHE)) {
						GameProcessFound = false; GameData.PID = 0; mem.PROCESS_INITIALIZED = false;
					}
					return;
				}

				if (PID == 0) {
					mem.RefreshAll(); 
					GameData.Scene = Scene::FindProcess; GameProcessFound = false; GameData.PID = 0; Decrypt::DestroyXe();
				} else {
					GameProcessFound = true;
					if (PID != PrevPID) {
						mem.Init("TslGame.exe", false, false);
						GameData.GameBase = mem.GetProcessBase();
						EntityInit(); Data::SetGNameLists(EntityLists);
						Utils::Log(1, "[GAME] PID: %d | Base: 0x%llX", GameData.PID, GameData.GameBase);
						GameData.Scene = Scene::Lobby;
						PrevPID = PID;
					}
				}
				PrevGameProcessFound = GameProcessFound;
			});

			// 2. DOC CAC POINTER TOAN CUC (UWORLD, GAMEINSTANCE...)
			if (GameData.PID != 0) {
				uint64_t UWorldEnc = mem.Read<uint64_t>(GameData.GameBase + Offset::UWorld);
				GameData.UWorld = Decrypt::Xe(UWorldEnc);

				if (Utils::ValidPtr(GameData.UWorld)) {
					uint64_t GameInstanceEnc = mem.Read<uint64_t>(GameData.UWorld + Offset::GameInstance);
					GameData.GameInstance = Decrypt::Xe(GameInstanceEnc);

					uint64_t GameStateEnc = mem.Read<uint64_t>(GameData.UWorld + Offset::GameState);
					GameData.GameState = Decrypt::Xe(GameStateEnc);

					uint64_t LocalPlayersEnc = mem.Read<uint64_t>(GameData.GameInstance + Offset::LocalPlayer);
					uint64_t LocalPlayers = Decrypt::Xe(LocalPlayersEnc);
					uint64_t LocalPlayer = mem.Read<uint64_t>(LocalPlayers);
					
					uint64_t PlayerControllerEnc = mem.Read<uint64_t>(LocalPlayer + Offset::PlayerController);
					GameData.PlayerController = Decrypt::Xe(PlayerControllerEnc);
					
					uint64_t AcknowledgedPawnEnc = mem.Read<uint64_t>(GameData.PlayerController + Offset::AcknowledgedPawn);
					GameData.AcknowledgedPawn = Decrypt::Xe(AcknowledgedPawnEnc);

					uint64_t CurrentLevelEnc = mem.Read<uint64_t>(GameData.UWorld + Offset::CurrentLevel);
					GameData.CurrentLevel = Decrypt::Xe(CurrentLevelEnc);

					// SYNC VOI MACRO ENGINE (System B) TAI DAY DE CO TOC DO CAO 
					if (GameData.AcknowledgedPawn != 0) {
						G_LocalPawn = GameData.AcknowledgedPawn;
						G_UWorld = GameData.UWorld;
					}
				}

				int MapID = Decrypt::CIndex(mem.Read<int>(GameData.UWorld + Offset::ObjID));
				GameData.MapName = GNames::GetNameByID(MapID);

				if (telemetryMemory::IsKeyDown(VK_F5)) {
					if (GameData.Scene != Scene::Gaming && !Utils::IsLobby(GameData.MapName)) GameData.Scene = Scene::Gaming;
					Sleep(300);
				} else if (telemetryMemory::IsKeyDown(VK_F6)) {
					if (GameData.Scene != Scene::Lobby) GameData.Scene = Scene::Lobby;
					Sleep(300);
				}
			}

			// 3. CAP NHAT SCENE (LOBBY <-> GAMING)
			if (GameData.Scene != GameData.PreviousScene) {
				GameData.PreviousScene = GameData.Scene;
				if (GameData.Scene == Scene::Lobby) { 
					// Release resources
				} else if (GameData.Scene == Scene::Gaming) { 
					Players::ReadPlayerLists(); 
					Sleep(3000); 
				}
			}
			telemetryMemory::StealthSleep(10); // Loop speed for main data
		}
	}

	static void Init() {
		try {
			Offset::Sever_Init();
			EntityInit();
			Config::Load();
			
			if (!mem.Init("TslGame.exe", false, false)) { 
				Utils::Log(2, "[HYPER] Init fail, but overlay will show.");
			} else {
				Utils::Log(1, "[HYPER] OK.");
			}

			KeyState::Init();
			MacroEngine::Initialize();
			Data::SetGNameLists(EntityLists);
			
			std::thread(Update).detach();
			std::thread(KeyState::Update).detach();
			std::thread(Actors::Update).detach();
			std::thread(Players::Update).detach();
			std::thread(Vehicles::Update).detach();
			std::thread(precision_calibration::Run).detach();
			std::thread(Items::Update).detach();
			std::thread(Projects::Update).detach();
			std::thread(Radar::Update).detach();

			// Chay MacroEngine thread (1ms sleep)
			std::thread([]() {
				while (true) {
					MacroEngine::Update();
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}).detach();
			
			std::thread(MouseDispatcher::DispatchThreadFunc).detach();

			// Tach list update ra khoi loop chinh
			std::thread([]() {
				while (true) {
					if (GameData.Scene == Scene::Gaming && GameData.PID != 0 && Utils::ValidPtr(GameData.UWorld)) {
						Players::UpdatePlayerLists();
					}
					Sleep(2500);
				}
			}).detach();

		} catch (...) {
			Utils::Log(2, "Critical error in diagnostic_node::Init");
		}
	}
}
