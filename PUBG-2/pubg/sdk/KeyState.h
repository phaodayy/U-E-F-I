#pragma once
#include <Winsock2.h>
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Utils/KmBox.h>
#include <Utils/KmBoxNet.h>
#include <Utils/Utils.h>
#include <Hack/Players.h>

namespace KeyState
{
	void Init()
	{
		Utils::Log(0, "[KEYBOARD] Dang khoi tao keyboard qua hyper backend...");
		GameData.Keyboard = mem.GetKeyboard();

		if (!GameData.Keyboard.InitKeyboard())
		{
			Utils::Log(0, "[KEYBOARD] Chua the khoi tao keyboard (game chua san sang, se thu lai sau)");
		}
		else {
			Utils::Log(1, "[KEYBOARD] Khoi tao keyboard thanh cong!");
			GameData.Keyboard.InitCursorPosition();
			Utils::Log(1, "[KEYBOARD] Dia chi keyboard: 0x%llX", GameData.Keyboard.GetAddrss());
			Utils::Log(1, "[KEYBOARD] Cursor position da khoi tao");
		}
	}
	void Update() {
		Utils::Log(1, "[KEYBOARD] Bat dau vong lap update phim & trang thai");
		while (true)
		{
			GameData.Keyboard.UpdateKeys();

			std::unordered_map<int, std::vector<std::string>> Keys;
			Keys[GameData.Config.Menu.ShowKey].push_back("Menu");
			Keys[GameData.Config.Overlay.Quit_key].push_back("DEAD");
			Keys[GameData.Config.AimBot.Configs[0].Key].push_back("AimBotConfig0");
			Keys[GameData.Config.AimBot.Configs[1].Key].push_back("AimBotConfig1");
			Keys[VK_DELETE].push_back("RecoverOverlay");
			Keys[GameData.Config.Function.ClearKey].push_back("Clear");
			Keys[GameData.Config.Item.GroupKey].push_back("GroupKey");
			Keys[GameData.Config.Item.GroupAKey].push_back("GroupAKey");
			Keys[GameData.Config.Item.GroupBKey].push_back("GroupBKey");
			Keys[GameData.Config.Item.GroupCKey].push_back("GroupCKey");
			Keys[GameData.Config.Item.GroupDKey].push_back("GroupDKey");
			Keys[GameData.Config.Vehicle.EnableKey].push_back("VehicleEnable");
			Keys[GameData.Config.PlayerList.MarkKey].push_back("PlayerListMarkType");
			Keys[GameData.Config.ESP.FocusModeKey].push_back("FocusModeKey");
			Keys[GameData.Config.AirDrop.EnableKey].push_back("AirDropEnableKey");
			Keys[GameData.Config.DeadBox.EnableKey].push_back("DeadBoxEnableKey");
			Keys[GameData.Config.Overlay.ModeKey].push_back("FusionModeKEY");
			Keys[GameData.Config.ESP.duiyouKey].push_back("duiyouKey");
			Keys[GameData.Config.ESP.DataSwitchkey].push_back("DataSwitchkey");
			Keys[GameData.Config.ESP.Playerskey].push_back("Playerskey");
			Keys[VK_F5].push_back("SceneGaming");
			Keys[VK_F6].push_back("SceneLobby");
			Keys[VK_F7].push_back("ReloadOffsets");
			//Keys[GameData.Config.ESP.Mousekey].push_back("Mousekey");
			
			for (auto Key : Keys)
			{
				if (GameData.Keyboard.WasKeyPressed(Key.first))
				{
					for (auto KeyName : Key.second)
					{
						if (KeyName == "ReloadOffsets")
						{
							Utils::Log(0, "[OFFSET] Dang tai lai offset tu file...");
							Offset::Sever_Init();
						}

						if (KeyName == "SceneGaming")
						{
							if (GameData.Scene != Scene::Gaming && GameData.PID != 0) {
								bool IsLobbyMap = Utils::IsLobby(GameData.MapName);
								if (!IsLobbyMap && GameData.MapName != "None" && GameData.MapName != "fail") {
									Utils::Log(1, "[SCENE] Phim F5: Map hop le (%s) -> Chuyen sang Gaming!", GameData.MapName.c_str());
									GameData.Scene = Scene::Gaming;
								} else {
									Utils::Log(2, "[SCENE] Phim F5: Chua vao ban do hoac dang load Map (%s)", GameData.MapName.c_str());
								}
							}
						}

						if (KeyName == "SceneLobby")
						{
							if (GameData.Scene != Scene::Lobby) {
								Utils::Log(1, "[SCENE] Phim F6: Dung quet tran & chuyen ve sanh.");
								GameData.Scene = Scene::Lobby;
							}
						}

						if (KeyName == "AirDropEnableKey")
						{
							GameData.Config.AirDrop.Enable = !GameData.Config.AirDrop.Enable;
						}

						if (KeyName == "duiyouKey")
						{
							GameData.Config.ESP.duiyou = !GameData.Config.ESP.duiyou;
						}

						if (KeyName == "DataSwitchkey")
						{
							GameData.Config.ESP.DataSwitch = !GameData.Config.ESP.DataSwitch;
						}
						/*if (KeyName == "Mousekey")
						{
							GameData.Config.ESP.Mouse = !GameData.Config.ESP.Mouse;
						}*/
						if (KeyName == "Playerskey")
						{
							GameData.Config.Window.Players = !GameData.Config.Window.Players;
						}

						if (KeyName == "FusionModeKEY")
						{
							// �л� FusionMode ״̬
							GameData.Config.Overlay.FusionMode = !GameData.Config.Overlay.FusionMode;

						}


						if (KeyName == "DeadBoxEnableKey")
						{
							GameData.Config.DeadBox.Enable = !GameData.Config.DeadBox.Enable;
						}

						if (KeyName == "FocusModeKey")
						{
							GameData.Config.ESP.FocusMode = !GameData.Config.ESP.FocusMode;
						}

						if (KeyName == "DEAD")//  �����ǽ���
						{

							HWND Progman = FindWindowA("Progman", NULL);
							HWND TrayWnd = FindWindowA("Shell_TrayWnd", NULL);
							ShowWindow(Progman, SW_SHOW);
							ShowWindow(TrayWnd, SW_SHOW);

							ExitProcess(1);
							exit(1);
						}
						if (KeyName == "Clear")
						{
							bool shouldLog = true; // ������Ҫʱ��¼��־

							if (GameData.Config.AimBot.Connected) {
								if (GameData.Config.AimBot.Controller == 0) {
									KmBox::Clear();
									if (shouldLog) Utils::Log(1, "KMBOX Clear Success");
								}
								else if (GameData.Config.AimBot.Controller == 1) {
									KmBoxNet::Clear();
									if (shouldLog) Utils::Log(1, "KMBOXNET Clear Success");
								}
							}

							Data::SetCacheEntitys({});
							Data::SetCachePlayers({});
							Data::SetPlayers({});
							Data::SetPlayersData({});
							Data::SetCacheVehicles({});
							Data::SetVehicles({});
							Data::SetVehiclWheels({});
							Data::SetItems({});
							Data::SetCacheDroppedItems({});
							Data::SetCacheDroppedItemGroups({});
							GameData.AimBot.Target = 0;
							GameData.AimBot.Lock = false;
							bool shouldRefresh = false;
							// ֻ�е�ȷʵ��Ҫˢ��ʱ�ŵ���
							if (shouldRefresh) {
								mem.RefreshAll();
							}
						}
						

						if (KeyName == "GroupKey")
						{
							if (GameData.Config.Item.ShowGroup != 6)
							{
								GameData.Config.Item.Enable = 1;
								GameData.Config.Item.ShowGroup++;
							}
							else {
								GameData.Config.Item.ShowGroup = 0;
								GameData.Config.Item.Enable = 0;
							}
						}

						if (KeyName == "GroupAKey")
						{
							GameData.Config.Item.Enable = 1;
							GameData.Config.Item.ShowGroup = 2;
						}

						if (KeyName == "GroupBKey")
						{
							GameData.Config.Item.Enable = 1;
							GameData.Config.Item.ShowGroup = 3;
						}

						if (KeyName == "GroupCKey")
						{
							GameData.Config.Item.Enable = 1;
							GameData.Config.Item.ShowGroup = 4;
						}

						if (KeyName == "GroupDKey")
						{
							GameData.Config.Item.Enable = 1;
							GameData.Config.Item.ShowGroup = 5;
						}

						//if (KeyName == "GroupAKey")
						//{
						//	GameData.Config.Item.Enable = 1;
						//	//GameData.Config.Item.ShowGroup = 2;
						//	if (GameData.Config.Item.ShowGroups.count(1))
						//		GameData.Config.Item.ShowGroups.erase(1);
						//	else
						//		GameData.Config.Item.ShowGroups.insert(1);
						//}

						//if (KeyName == "GroupBKey")
						//{
						//	GameData.Config.Item.Enable = 1;
						//	//GameData.Config.Item.ShowGroup = 3;2
						//	if (GameData.Config.Item.ShowGroups.count(2))
						//		GameData.Config.Item.ShowGroups.erase(2);
						//	else
						//		GameData.Config.Item.ShowGroups.insert(2);
						//}

						//if (KeyName == "GroupCKey")
						//{
						//	GameData.Config.Item.Enable = 1;
						//	//GameData.Config.Item.ShowGroup = 4;
						//	if (GameData.Config.Item.ShowGroups.count(3))
						//		GameData.Config.Item.ShowGroups.erase(3);
						//	else
						//		GameData.Config.Item.ShowGroups.insert(3);
						//}

						//if (KeyName == "GroupDKey")
						//{
						//	GameData.Config.Item.Enable = 1;
						//	//GameData.Config.Item.ShowGroup = 5;
						//	if (GameData.Config.Item.ShowGroups.count(4))
						//		GameData.Config.Item.ShowGroups.erase(4);
						//	else
						//		GameData.Config.Item.ShowGroups.insert(4);
						//}

						if (KeyName == "VehicleEnable")
						{
							GameData.Config.Vehicle.Enable = !GameData.Config.Vehicle.Enable;
						}
						if (KeyName == "Menu")
						{
							GameData.Config.Menu.Show = !GameData.Config.Menu.Show;

							if (GameData.Config.Menu.Show)
							{
								SetForegroundWindow(GameData.Config.Overlay.hWnd);
							}
							else {
								SetForegroundWindow(GetDesktopWindow());
							}
						}
						if (KeyName == "AimBotConfig0")
						{
							GameData.Config.AimBot.ConfigIndex = 0;
						}
						if (KeyName == "AimBotConfig1")
						{
							GameData.Config.AimBot.ConfigIndex = 1;
						}
					}
				}
			}

			Sleep(10);
		}
	}
}
