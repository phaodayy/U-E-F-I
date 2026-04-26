#pragma once
#include "Data.h"
#include "utils/utils.h"
#include <nlohmann/json.hpp>
#include "imgui_settings.h"

class Config
{
public:
	template <typename T>
	static bool SetConfigItem(const nlohmann::json& config, const std::vector<std::string>& keys, T& value)
	{
		if (keys.empty()) {
			return false;
		}

		const std::string& currentKey = keys[0];

		if (config.count(currentKey) > 0) {
			if (keys.size() == 1) {
				value = config[currentKey].get<T>();
				return true;
			}
			else {
				return SetConfigItem(config[currentKey], std::vector<std::string>(keys.begin() + 1, keys.end()), value);
			}
		}
		else {
			return false;
		}
	}

	static bool Save()
	{
		nlohmann::json Config;

		[&] {
			std::string ConfigName = "Function";
			Config[ConfigName]["ClearKey"] = GameData.Config.Function.ClearKey;
			}();

		[&] {
			std::string ConfigName = "WebRadar";
			Config[ConfigName]["Server IP"] = GameData.Config.WebRadar.IP;
			Config[ConfigName]["Server Port"] = GameData.Config.WebRadar.Port;
			Config[ConfigName]["Server PIN"] = GameData.Config.WebRadar.PIN;

			}();

		[&] {
			std::string ConfigName = "Vehicle";
			Config[ConfigName]["Enable"] = GameData.Config.Vehicle.Enable;
			Config[ConfigName]["Health"] = GameData.Config.Vehicle.Health;
			Config[ConfigName]["Durability"] = GameData.Config.Vehicle.Durability;
			Config[ConfigName]["ShowIcon"] = GameData.Config.Vehicle.ShowIcon;
			Config[ConfigName]["ShowIconAndText"] = GameData.Config.Vehicle.ShowIconAndText;
			Config[ConfigName]["EnableKey"] = GameData.Config.Vehicle.EnableKey;
			Config[ConfigName]["DistanceMax"] = GameData.Config.Vehicle.DistanceMax;
			Config[ConfigName]["FontSize"] = GameData.Config.Vehicle.FontSize;

			Config[ConfigName]["Color"]["0"] = GameData.Config.Vehicle.Color[0];
			Config[ConfigName]["Color"]["1"] = GameData.Config.Vehicle.Color[1];
			Config[ConfigName]["Color"]["2"] = GameData.Config.Vehicle.Color[2];
			Config[ConfigName]["Color"]["3"] = GameData.Config.Vehicle.Color[3];

			Config[ConfigName]["Healthbarcolor"]["0"] = GameData.Config.Vehicle.Healthbarcolor[0];
			Config[ConfigName]["Healthbarcolor"]["1"] = GameData.Config.Vehicle.Healthbarcolor[1];
			Config[ConfigName]["Healthbarcolor"]["2"] = GameData.Config.Vehicle.Healthbarcolor[2];
			Config[ConfigName]["Healthbarcolor"]["3"] = GameData.Config.Vehicle.Healthbarcolor[3];

			Config[ConfigName]["Fuelbarcolor"]["0"] = GameData.Config.Vehicle.Fuelbarcolor[0];
			Config[ConfigName]["Fuelbarcolor"]["1"] = GameData.Config.Vehicle.Fuelbarcolor[1];
			Config[ConfigName]["Fuelbarcolor"]["2"] = GameData.Config.Vehicle.Fuelbarcolor[2];
			Config[ConfigName]["Fuelbarcolor"]["3"] = GameData.Config.Vehicle.Fuelbarcolor[3];
			}();

		[&] {
			std::string ConfigName = "AirDrop";
			Config[ConfigName]["Enable"] = GameData.Config.AirDrop.Enable;
			Config[ConfigName]["EnableKey"] = GameData.Config.AirDrop.EnableKey;
			Config[ConfigName]["DistanceMax"] = GameData.Config.AirDrop.DistanceMax;
			Config[ConfigName]["FontSize"] = GameData.Config.AirDrop.FontSize;
			Config[ConfigName]["ShowItems"] = GameData.Config.AirDrop.ShowItems;

			Config[ConfigName]["Color"]["0"] = GameData.Config.AirDrop.Color[0];
			Config[ConfigName]["Color"]["1"] = GameData.Config.AirDrop.Color[1];
			Config[ConfigName]["Color"]["2"] = GameData.Config.AirDrop.Color[2];
			Config[ConfigName]["Color"]["3"] = GameData.Config.AirDrop.Color[3];
			}();

		[&] {
			std::string ConfigName = "DeadBox";
			Config[ConfigName]["Enable"] = GameData.Config.AirDrop.Enable;
			Config[ConfigName]["EnableKey"] = GameData.Config.AirDrop.EnableKey;
			Config[ConfigName]["DistanceMax"] = GameData.Config.AirDrop.DistanceMax;
			Config[ConfigName]["FontSize"] = GameData.Config.AirDrop.FontSize;
			Config[ConfigName]["ShowItems"] = GameData.Config.AirDrop.ShowItems;

			Config[ConfigName]["Color"]["0"] = GameData.Config.AirDrop.Color[0];
			Config[ConfigName]["Color"]["1"] = GameData.Config.AirDrop.Color[1];
			Config[ConfigName]["Color"]["2"] = GameData.Config.AirDrop.Color[2];
			Config[ConfigName]["Color"]["3"] = GameData.Config.AirDrop.Color[3];
			}();

		[&] {
			std::string ConfigName = "Early";
			Config[ConfigName]["Enable"] = GameData.Config.Early.Enable;
			Config[ConfigName]["DistanceMax"] = GameData.Config.Early.DistanceMax;
			Config[ConfigName]["FontSize"] = GameData.Config.Early.FontSize;
			Config[ConfigName]["ShowDistance"] = GameData.Config.Early.ShowDistance;
			}();

		[&] {
			std::string ConfigName = "PlayerList";
			Config[ConfigName]["RankMode"] = GameData.Config.PlayerList.RankMode;
			}();

		[&] {
			std::string ConfigName = "Project";
			Config[ConfigName]["Enable"] = GameData.Config.Project.Enable;
			Config[ConfigName]["GrenadeEnable"] = GameData.Config.Project.GrenadeEnable;
	
			Config[ConfigName]["MortarShooting"] = GameData.Config.Project.MortarShooting;

			Config[ConfigName]["DistanceMax"] = GameData.Config.Project.DistanceMax;
			Config[ConfigName]["Explosion Range"] = GameData.Config.Project.explosionrange;
			Config[ConfigName]["Grenade Prediction"] = GameData.Config.Project.GrenadePrediction;
			Config[ConfigName]["Grenade Trajectory"] = GameData.Config.Project.GrenadeTrajectory;
			Config[ConfigName]["Explosion Timing"] = GameData.Config.Project.ShowChareTime;
			Config[ConfigName]["FontSize"] = GameData.Config.Project.FontSize;
			Config[ConfigName]["TrajectorySize"] = GameData.Config.Project.TrajectorySize;
			Config[ConfigName]["TextShowChareTime"] = GameData.Config.Project.TextShowChareTime;
			Config[ConfigName]["BarShowChareTime"] = GameData.Config.Project.BarShowChareTime;
			Config[ConfigName]["ChareFontSize"] = GameData.Config.Project.ChareFontSize;

			Config[ConfigName]["ChareColor"]["0"] = GameData.Config.Project.ChareColor[0];
			Config[ConfigName]["ChareColor"]["1"] = GameData.Config.Project.ChareColor[1];
			Config[ConfigName]["ChareColor"]["2"] = GameData.Config.Project.ChareColor[2];
			Config[ConfigName]["ChareColor"]["3"] = GameData.Config.Project.ChareColor[3];

			Config[ConfigName]["Color"]["0"] = GameData.Config.Project.Color[0];
			Config[ConfigName]["Color"]["1"] = GameData.Config.Project.Color[1];
			Config[ConfigName]["Color"]["2"] = GameData.Config.Project.Color[2];
			Config[ConfigName]["Color"]["3"] = GameData.Config.Project.Color[3];

			Config[ConfigName]["explosionrangeColor"]["0"] = GameData.Config.Project.explosionrangeColor[0];
			Config[ConfigName]["explosionrangeColor"]["1"] = GameData.Config.Project.explosionrangeColor[1];
			Config[ConfigName]["explosionrangeColor"]["2"] = GameData.Config.Project.explosionrangeColor[2];
			Config[ConfigName]["explosionrangeColor"]["3"] = GameData.Config.Project.explosionrangeColor[3];

			Config[ConfigName]["TrajectoryColor"]["0"] = GameData.Config.Project.TrajectoryColor[0];
			Config[ConfigName]["TrajectoryColor"]["1"] = GameData.Config.Project.TrajectoryColor[1];
			Config[ConfigName]["TrajectoryColor"]["2"] = GameData.Config.Project.TrajectoryColor[2];
			Config[ConfigName]["TrajectoryColor"]["3"] = GameData.Config.Project.TrajectoryColor[3];


			}();
		[&] {
			std::string ConfigName = "AutoRecoil";
			Config[ConfigName]["Enabled"] = GameData.Config.precision_calibration.Recoilenanlek;
			Config[ConfigName]["Red Dot Amplitude"] = GameData.Config.precision_calibration.yRecoil[0];
			Config[ConfigName]["Double Amplitude"] = GameData.Config.precision_calibration.yRecoil[1];
			Config[ConfigName]["Triple Amplitude"] = GameData.Config.precision_calibration.yRecoil[2];
			Config[ConfigName]["Four Times The Amplitude"] = GameData.Config.precision_calibration.yRecoil[3];
			Config[ConfigName]["Six Times Amplitude"] = GameData.Config.precision_calibration.yRecoil[4];
			Config[ConfigName]["Eight Times Amplitude"] = GameData.Config.precision_calibration.yRecoil[5];
			Config[ConfigName]["Red Dot Delay"] = GameData.Config.precision_calibration.interval[0];
			Config[ConfigName]["Double Latency"] = GameData.Config.precision_calibration.interval[1];
			Config[ConfigName]["Triple Delay"] = GameData.Config.precision_calibration.interval[2];
			Config[ConfigName]["Four Times Delay"] = GameData.Config.precision_calibration.interval[3];
			Config[ConfigName]["Six Times Delay"] = GameData.Config.precision_calibration.interval[4];
			Config[ConfigName]["Eight Times Delay"] = GameData.Config.precision_calibration.interval[5];
			}();

		[&] {
			std::string ConfigName = "Recoil";
			Config[ConfigName]["Enable"] = GameData.Config.Recoil.Enable;
			Config[ConfigName]["RedDot"] = GameData.Config.Recoil.RedDot;
			Config[ConfigName]["x2"] = GameData.Config.Recoil.x2;
			Config[ConfigName]["x3"] = GameData.Config.Recoil.x3;
			Config[ConfigName]["x4"] = GameData.Config.Recoil.x4;
			Config[ConfigName]["x6"] = GameData.Config.Recoil.x6;
			Config[ConfigName]["x8"] = GameData.Config.Recoil.x8;
			Config[ConfigName]["Delay"] = GameData.Config.Recoil.Delay;
			}();

		[&] {
			std::string ConfigName = "Radar";
			Config[ConfigName]["Enable"] = GameData.Config.Radar.Enable;
			Config[ConfigName]["Main"]["ShowPlayer"] = GameData.Config.Radar.Main.ShowPlayer;
			Config[ConfigName]["Main"]["ShowVehicle"] = GameData.Config.Radar.Main.ShowVehicle;
			Config[ConfigName]["Main"]["ShowAirDrop"] = GameData.Config.Radar.Main.ShowAirDrop;
			Config[ConfigName]["Main"]["ShowDeadBox"] = GameData.Config.Radar.Main.ShowDeadBox;
			Config[ConfigName]["Main"]["MapRoom"] = GameData.Config.Radar.Main.MapRoom;
			Config[ConfigName]["Main"]["FontSize"] = GameData.Config.Radar.Main.FontSize;
			Config[ConfigName]["Main"]["MapColor"]["0"] = GameData.Config.Radar.Main.MapColor[0];
			Config[ConfigName]["Main"]["MapColor"]["1"] = GameData.Config.Radar.Main.MapColor[1];
			Config[ConfigName]["Main"]["MapColor"]["2"] = GameData.Config.Radar.Main.MapColor[2];
			Config[ConfigName]["Main"]["MapColor"]["3"] = GameData.Config.Radar.Main.MapColor[3];
			Config[ConfigName]["Main"]["Map_size"] = GameData.Config.Radar.Main.Map_size;

			Config[ConfigName]["Mini"]["ShowPlayer"] = GameData.Config.Radar.Mini.ShowPlayer;
			Config[ConfigName]["Mini"]["ShowVehicle"] = GameData.Config.Radar.Mini.ShowVehicle;
			Config[ConfigName]["Mini"]["ShowAirDrop"] = GameData.Config.Radar.Mini.ShowAirDrop;
			Config[ConfigName]["Mini"]["ShowDeadBox"] = GameData.Config.Radar.Mini.ShowDeadBox;
			Config[ConfigName]["Mini"]["MiniMapRoom"] = GameData.Config.Radar.Mini.MapRoom;
			Config[ConfigName]["Mini"]["FontSize"] = GameData.Config.Radar.Mini.FontSize;
			Config[ConfigName]["Mini"]["MapColor"]["0"] = GameData.Config.Radar.Mini.MapColor[0];
			Config[ConfigName]["Mini"]["MapColor"]["1"] = GameData.Config.Radar.Mini.MapColor[1];
			Config[ConfigName]["Mini"]["MapColor"]["2"] = GameData.Config.Radar.Mini.MapColor[2];
			Config[ConfigName]["Mini"]["MapColor"]["3"] = GameData.Config.Radar.Mini.MapColor[3];
			Config[ConfigName]["Mini"]["Map_size"] = GameData.Config.Radar.Mini.Map_size;
			}();

		[&] {
			std::string ConfigName = "Intelligent Display";
			Config[ConfigName]["AccessoriesFilter"] = GameData.Config.Item.AccessoriesFilter;
			Config[ConfigName]["HandheldWeaponFilter"] = GameData.Config.Item.HandheldWeaponFilter;
			Config[ConfigName]["FilterHelmets"] = GameData.Config.Item.FilterHelmets;
			Config[ConfigName]["ItemLimit"] = GameData.Config.Item.ItemLimit;

			Config[ConfigName]["Bandage"] = GameData.Config.ItemFiltering.Bandage;
			Config[ConfigName]["FirstAidKit"] = GameData.Config.ItemFiltering.FirstAidKit;
			Config[ConfigName]["MedicalKit"] = GameData.Config.ItemFiltering.MedicalKit;
			Config[ConfigName]["Painkiller"] = GameData.Config.ItemFiltering.Painkiller;
			Config[ConfigName]["EnergyDrink"] = GameData.Config.ItemFiltering.EnergyDrink;
			Config[ConfigName]["Adrenaline"] = GameData.Config.ItemFiltering.Adrenaline;
			Config[ConfigName]["C4"] = GameData.Config.ItemFiltering.C4;
			Config[ConfigName]["Grenade"] = GameData.Config.ItemFiltering.Grenade;
			Config[ConfigName]["SmokeGrenade"] = GameData.Config.ItemFiltering.SmokeGrenade;
			Config[ConfigName]["Flashbang"] = GameData.Config.ItemFiltering.Flashbang;
			Config[ConfigName]["Molotov"] = GameData.Config.ItemFiltering.Molotov;
			Config[ConfigName]["BlueZoneGrenade"] = GameData.Config.ItemFiltering.BlueZoneGrenade;
			Config[ConfigName]["StickyBomb"] = GameData.Config.ItemFiltering.StickyBomb;
			}();

		[&] {
			std::string ConfigName = "Item";
			Config[ConfigName]["Enable"] = GameData.Config.Item.Enable;
			Config[ConfigName]["EnableKey"] = GameData.Config.Item.EnableKey;
			//Config[ConfigName]["GroupKey"] = GameData.Config.Item.GroupKey;
			Config[ConfigName]["ShowGroups"] = GameData.Config.Item.ShowGroups;
			Config[ConfigName]["DistanceMax"] = GameData.Config.Item.DistanceMax;
			Config[ConfigName]["FontSize"] = GameData.Config.Item.FontSize;
			Config[ConfigName]["Combination"] = GameData.Config.Item.Combination;
			Config[ConfigName]["ShowIcon"] = GameData.Config.Item.ShowIcon;
			Config[ConfigName]["ShowRay"] = GameData.Config.Item.ShowRay;
			Config[ConfigName]["RayWidth"] = GameData.Config.Item.RayWidth;
			Config[ConfigName]["ShowIconAndText"] = GameData.Config.Item.ShowIconAndText;
			Config[ConfigName]["ShowDistance"] = GameData.Config.Item.ShowDistance;
			Config[ConfigName]["ThresholdX"] = GameData.Config.Item.ThresholdX;
			Config[ConfigName]["ThresholdY"] = GameData.Config.Item.ThresholdY;
			Config[ConfigName]["GroupAKey"] = GameData.Config.Item.GroupAKey;
			Config[ConfigName]["GroupBKey"] = GameData.Config.Item.GroupBKey;
			Config[ConfigName]["GroupCKey"] = GameData.Config.Item.GroupCKey;
			Config[ConfigName]["GroupDKey"] = GameData.Config.Item.GroupDKey;
			Config[ConfigName]["ShowGroup"] = GameData.Config.Item.ShowGroup;
			Config[ConfigName]["Scene"] = GameData.Scene;

			Config[ConfigName]["GroupAColor"]["0"] = GameData.Config.Item.GroupAColor[0];
			Config[ConfigName]["GroupAColor"]["1"] = GameData.Config.Item.GroupAColor[1];
			Config[ConfigName]["GroupAColor"]["2"] = GameData.Config.Item.GroupAColor[2];
			Config[ConfigName]["GroupAColor"]["3"] = GameData.Config.Item.GroupAColor[3];

			Config[ConfigName]["GroupBColor"]["0"] = GameData.Config.Item.GroupBColor[0];
			Config[ConfigName]["GroupBColor"]["1"] = GameData.Config.Item.GroupBColor[1];
			Config[ConfigName]["GroupBColor"]["2"] = GameData.Config.Item.GroupBColor[2];
			Config[ConfigName]["GroupBColor"]["3"] = GameData.Config.Item.GroupBColor[3];

			Config[ConfigName]["GroupCColor"]["0"] = GameData.Config.Item.GroupCColor[0];
			Config[ConfigName]["GroupCColor"]["1"] = GameData.Config.Item.GroupCColor[1];
			Config[ConfigName]["GroupCColor"]["2"] = GameData.Config.Item.GroupCColor[2];
			Config[ConfigName]["GroupCColor"]["3"] = GameData.Config.Item.GroupCColor[3];

			Config[ConfigName]["GroupDColor"]["0"] = GameData.Config.Item.GroupDColor[0];
			Config[ConfigName]["GroupDColor"]["1"] = GameData.Config.Item.GroupDColor[1];
			Config[ConfigName]["GroupDColor"]["2"] = GameData.Config.Item.GroupDColor[2];
			Config[ConfigName]["GroupDColor"]["3"] = GameData.Config.Item.GroupDColor[3];

			Config[ConfigName]["RayColor"]["0"] = GameData.Config.Item.RayColor[0];
			Config[ConfigName]["RayColor"]["1"] = GameData.Config.Item.RayColor[1];
			Config[ConfigName]["RayColor"]["2"] = GameData.Config.Item.RayColor[2];
			Config[ConfigName]["RayColor"]["3"] = GameData.Config.Item.RayColor[3];

			Config[ConfigName]["Lists"] = {};

			for (const auto& pair : GameData.Config.Item.Lists) {
				const std::string& key = pair.first;
				const ItemDetail& detail = pair.second;

				nlohmann::json ItemDetail = {
					{"Name", detail.Name},
					{"DisplayName", Utils::StringToUTF8(detail.DisplayName)},
					{"Type", static_cast<int>(detail.Type)},
					{"Group", detail.Group},
					{"ShowRay", detail.ShowRay }
				};
				Config[ConfigName]["Lists"][key] = ItemDetail;
			}
			}();

		[&] {
			std::string ConfigName = "signal_overlay";

			Config[ConfigName]["ServerIP"] = GameData.Config.WebRadar.IP;
			Config[ConfigName]["ServerPort"] = GameData.Config.WebRadar.Port;
			Config[ConfigName]["ServerPIN"] = GameData.Config.WebRadar.PIN;
			Config[ConfigName]["Mouse"] = GameData.Config.signal_overlay.Mouse;
			Config[ConfigName]["Enable"] = GameData.Config.signal_overlay.Enable;
			Config[ConfigName]["Stroke"] = GameData.Config.signal_overlay.Stroke;
			Config[ConfigName]["DistanceStyle"] = GameData.Config.signal_overlay.DistanceStyle;
			Config[ConfigName]["HealthBarStyle"] = GameData.Config.signal_overlay.HealthBarStyle;
			Config[ConfigName]["VisibleCheck"] = GameData.Config.signal_overlay.VisibleCheck;
			Config[ConfigName]["Bone Visibility"] = GameData.Config.signal_overlay.Skeleton;
			Config[ConfigName]["AimExpandInfo"] = GameData.Config.signal_overlay.AimExpandInfo;
			Config[ConfigName]["TargetedRay"] = GameData.Config.signal_overlay.TargetedRay;
			Config[ConfigName]["VisibleCheckRay"] = GameData.Config.signal_overlay.VisibleCheckRay;
			Config[ConfigName]["LockedHiddenBones"] = GameData.Config.signal_overlay.LockedHiddenBones;
			Config[ConfigName]["PlayerLine"] = GameData.Config.signal_overlay.PlayerLine;
			Config[ConfigName]["Rank Ico"] = GameData.Config.signal_overlay.showico;
			Config[ConfigName]["suodingbianse"] = GameData.Config.signal_overlay.suodingbianse;
			Config[ConfigName]["Weapon"] = GameData.Config.signal_overlay.Weapon;
			Config[ConfigName]["Partner"] = GameData.Config.signal_overlay.Partner;
			Config[ConfigName]["Grade"] = GameData.Config.signal_overlay.Level;
			Config[ConfigName]["KillCount"] = GameData.Config.signal_overlay.Kills;
			Config[ConfigName]["Damage Amount"] = GameData.Config.signal_overlay.Damage;
			Config[ConfigName]["Audience"] = GameData.Config.signal_overlay.Spectate;
			Config[ConfigName]["FPP Rank"] = GameData.Config.signal_overlay.FPP;
			Config[ConfigName]["TPP Rank"] = GameData.Config.signal_overlay.TPP;
			Config[ConfigName]["Offline"] = GameData.Config.signal_overlay.Offline;
			Config[ConfigName]["Downed"] = GameData.Config.signal_overlay.Downed;
			Config[ConfigName]["Display Frame"] = GameData.Config.signal_overlay.DisplayFrame;
			Config[ConfigName]["Show Teammates"] = GameData.Config.signal_overlay.duiyouKey;
			Config[ConfigName]["Playerskey"] = GameData.Config.signal_overlay.Playerskey;
			Config[ConfigName]["DataSwitchkey"] = GameData.Config.signal_overlay.DataSwitchkey;
			Config[ConfigName]["LowModel"] = GameData.Config.signal_overlay.LowModel;
			Config[ConfigName]["MediumModel"] = GameData.Config.signal_overlay.MediumModel;
			Config[ConfigName]["HighModel"] = GameData.Config.signal_overlay.HighModel;
			Config[ConfigName]["PhysxLoadRadius"] = GameData.Config.signal_overlay.PhysxLoadRadius;
			/*for (size_t i = 0; i < 17; i++)
			{
				Config[ConfigName]["ShowInfos"][std::to_string(i)] = GameData.Config.signal_overlay.ShowInfos[i];
			}*/
			Config[ConfigName]["Head Drawing"] = GameData.Config.signal_overlay.HeadDrawing;
			Config[ConfigName]["Data Switch"] = GameData.Config.signal_overlay.DataSwitch;
			Config[ConfigName]["Danger Warning"] = GameData.Config.signal_overlay.DangerWarning;
			Config[ConfigName]["Nickname"] = GameData.Config.signal_overlay.Nickname;
			Config[ConfigName]["TeamID"] = GameData.Config.signal_overlay.TeamID;
			Config[ConfigName]["ClanName"] = GameData.Config.signal_overlay.ClanName;
			Config[ConfigName]["Dis"] = GameData.Config.signal_overlay.Dis;
			Config[ConfigName]["health_bar"] = GameData.Config.signal_overlay.health_bar;
			Config[ConfigName]["Health"] = GameData.Config.signal_overlay.Health;
			Config[ConfigName]["healthBarPosition"] = GameData.Config.signal_overlay.HealthBarPos;
			Config[ConfigName]["healthBarStyle"] = GameData.Config.signal_overlay.HealthBarStyle;
			Config[ConfigName]["healthBarWidth"] = GameData.Config.signal_overlay.HealthBarWidth;
			Config[ConfigName]["healthBarHeight"] = GameData.Config.signal_overlay.HealthBarHeight;
			Config[ConfigName]["healthBarAlpha"] = GameData.Config.signal_overlay.HealthBarAlpha;

			Config[ConfigName]["DistanceMax"] = GameData.Config.signal_overlay.DistanceMax;
			Config[ConfigName]["SkeletonWidth"] = GameData.Config.signal_overlay.SkeletonWidth;
			Config[ConfigName]["FontSize"] = GameData.Config.signal_overlay.FontSize;
			Config[ConfigName]["WeaponDistanceMax"] = GameData.Config.signal_overlay.WeaponDistanceMax;
			Config[ConfigName]["InfoDistanceMax"] = GameData.Config.signal_overlay.InfoDistanceMax;
			Config[ConfigName]["RayWidth"] = GameData.Config.signal_overlay.RayWidth;
			Config[ConfigName]["StrokeSize"] = GameData.Config.signal_overlay.StrokeSize;
			Config[ConfigName]["FocusModeKey"] = GameData.Config.signal_overlay.FocusModeKey;
			Config[ConfigName]["FocusMode"] = GameData.Config.signal_overlay.FocusMode;
			Config[ConfigName]["DeadBox"] = GameData.Config.DeadBox.Enable;
			Config[ConfigName]["Box Distance"] = GameData.Config.DeadBox.DistanceMax;


			Config[ConfigName]["Color"]["aim"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.aim.Skeleton[0];
			Config[ConfigName]["Color"]["aim"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.aim.Skeleton[1];
			Config[ConfigName]["Color"]["aim"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.aim.Skeleton[2];
			Config[ConfigName]["Color"]["aim"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.aim.Skeleton[3];

			Config[ConfigName]["Color"]["Info"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Info.Skeleton[0];
			Config[ConfigName]["Color"]["Info"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Info.Skeleton[1];
			Config[ConfigName]["Color"]["Info"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Info.Skeleton[2];
			Config[ConfigName]["Color"]["Info"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Info.Skeleton[3];

			Config[ConfigName]["Color"]["xuetiaoyanse"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[0];
			Config[ConfigName]["Color"]["xuetiaoyanse"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[1];
			Config[ConfigName]["Color"]["xuetiaoyanse"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[2];
			Config[ConfigName]["Color"]["xuetiaoyanse"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[3];

			Config[ConfigName]["Color"]["Ray"]["Line"]["0"] = GameData.Config.signal_overlay.Color.Ray.Line[0];
			Config[ConfigName]["Color"]["Ray"]["Line"]["1"] = GameData.Config.signal_overlay.Color.Ray.Line[1];
			Config[ConfigName]["Color"]["Ray"]["Line"]["2"] = GameData.Config.signal_overlay.Color.Ray.Line[2];
			Config[ConfigName]["Color"]["Ray"]["Line"]["3"] = GameData.Config.signal_overlay.Color.Ray.Line[3];

			Config[ConfigName]["Color"]["Default"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Default.Info[0];
			Config[ConfigName]["Color"]["Default"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Default.Info[1];
			Config[ConfigName]["Color"]["Default"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Default.Info[2];
			Config[ConfigName]["Color"]["Default"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Default.Info[3];
			Config[ConfigName]["Color"]["Default"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Default.Skeleton[0];
			Config[ConfigName]["Color"]["Default"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Default.Skeleton[1];
			Config[ConfigName]["Color"]["Default"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Default.Skeleton[2];
			Config[ConfigName]["Color"]["Default"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Default.Skeleton[3];

			Config[ConfigName]["Color"]["Visible"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Visible.Skeleton[0];
			Config[ConfigName]["Color"]["Visible"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Visible.Skeleton[1];
			Config[ConfigName]["Color"]["Visible"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Visible.Skeleton[2];
			Config[ConfigName]["Color"]["Visible"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Visible.Skeleton[3];
			Config[ConfigName]["Color"]["Visible"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Visible.Info[0];
			Config[ConfigName]["Color"]["Visible"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Visible.Info[1];
			Config[ConfigName]["Color"]["Visible"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Visible.Info[2];
			Config[ConfigName]["Color"]["Visible"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Visible.Info[3];

			Config[ConfigName]["Color"]["Partner"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Partner.Skeleton[0];
			Config[ConfigName]["Color"]["Partner"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Partner.Skeleton[1];
			Config[ConfigName]["Color"]["Partner"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Partner.Skeleton[2];
			Config[ConfigName]["Color"]["Partner"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Partner.Skeleton[3];
			Config[ConfigName]["Color"]["Partner"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Partner.Info[0];
			Config[ConfigName]["Color"]["Partner"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Partner.Info[1];
			Config[ConfigName]["Color"]["Partner"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Partner.Info[2];
			Config[ConfigName]["Color"]["Partner"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Partner.Info[3];

			Config[ConfigName]["Color"]["Groggy"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Groggy.Skeleton[0];
			Config[ConfigName]["Color"]["Groggy"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Groggy.Skeleton[1];
			Config[ConfigName]["Color"]["Groggy"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Groggy.Skeleton[2];
			Config[ConfigName]["Color"]["Groggy"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Groggy.Skeleton[3];
			Config[ConfigName]["Color"]["Groggy"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Groggy.Info[0];
			Config[ConfigName]["Color"]["Groggy"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Groggy.Info[1];
			Config[ConfigName]["Color"]["Groggy"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Groggy.Info[2];
			Config[ConfigName]["Color"]["Groggy"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Groggy.Info[3];

			Config[ConfigName]["Color"]["Dangerous"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Dangerous.Skeleton[0];
			Config[ConfigName]["Color"]["Dangerous"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Dangerous.Skeleton[1];
			Config[ConfigName]["Color"]["Dangerous"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Dangerous.Skeleton[2];
			Config[ConfigName]["Color"]["Dangerous"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Dangerous.Skeleton[3];
			Config[ConfigName]["Color"]["Dangerous"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Dangerous.Info[0];
			Config[ConfigName]["Color"]["Dangerous"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Dangerous.Info[1];
			Config[ConfigName]["Color"]["Dangerous"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Dangerous.Info[2];
			Config[ConfigName]["Color"]["Dangerous"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Dangerous.Info[3];

			Config[ConfigName]["Color"]["Blacklist"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Blacklist.Skeleton[0];
			Config[ConfigName]["Color"]["Blacklist"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Blacklist.Skeleton[1];
			Config[ConfigName]["Color"]["Blacklist"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Blacklist.Skeleton[2];
			Config[ConfigName]["Color"]["Blacklist"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Blacklist.Skeleton[3];
			Config[ConfigName]["Color"]["Blacklist"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Blacklist.Info[0];
			Config[ConfigName]["Color"]["Blacklist"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Blacklist.Info[1];
			Config[ConfigName]["Color"]["Blacklist"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Blacklist.Info[2];
			Config[ConfigName]["Color"]["Blacklist"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Blacklist.Info[3];

			Config[ConfigName]["Color"]["Whitelist"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.Whitelist.Skeleton[0];
			Config[ConfigName]["Color"]["Whitelist"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.Whitelist.Skeleton[1];
			Config[ConfigName]["Color"]["Whitelist"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.Whitelist.Skeleton[2];
			Config[ConfigName]["Color"]["Whitelist"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.Whitelist.Skeleton[3];
			Config[ConfigName]["Color"]["Whitelist"]["Info"]["0"] = GameData.Config.signal_overlay.Color.Whitelist.Info[0];
			Config[ConfigName]["Color"]["Whitelist"]["Info"]["1"] = GameData.Config.signal_overlay.Color.Whitelist.Info[1];
			Config[ConfigName]["Color"]["Whitelist"]["Info"]["2"] = GameData.Config.signal_overlay.Color.Whitelist.Info[2];
			Config[ConfigName]["Color"]["Whitelist"]["Info"]["3"] = GameData.Config.signal_overlay.Color.Whitelist.Info[3];

			Config[ConfigName]["Color"]["AI"]["Skeleton"]["0"] = GameData.Config.signal_overlay.Color.AI.Skeleton[0];
			Config[ConfigName]["Color"]["AI"]["Skeleton"]["1"] = GameData.Config.signal_overlay.Color.AI.Skeleton[1];
			Config[ConfigName]["Color"]["AI"]["Skeleton"]["2"] = GameData.Config.signal_overlay.Color.AI.Skeleton[2];
			Config[ConfigName]["Color"]["AI"]["Skeleton"]["3"] = GameData.Config.signal_overlay.Color.AI.Skeleton[3];
			Config[ConfigName]["Color"]["AI"]["Info"]["0"] = GameData.Config.signal_overlay.Color.AI.Info[0];
			Config[ConfigName]["Color"]["AI"]["Info"]["1"] = GameData.Config.signal_overlay.Color.AI.Info[1];
			Config[ConfigName]["Color"]["AI"]["Info"]["2"] = GameData.Config.signal_overlay.Color.AI.Info[2];
			Config[ConfigName]["Color"]["AI"]["Info"]["3"] = GameData.Config.signal_overlay.Color.AI.Info[3];
			}();

		[&] {
			std::string ConfigName = "precision_calibration";
			Config[ConfigName]["Enable"] = GameData.Config.precision_calibration.Enable;
			Config[ConfigName]["COM"] = GameData.Config.precision_calibration.COM;


			Config[ConfigName]["Mortar"] = GameData.Config.precision_calibration.Mortar;
			Config[ConfigName]["Mortar2"] = GameData.Config.precision_calibration.Mortar2;
			Config[ConfigName]["MortarPredict"] = GameData.Config.precision_calibration.MortarPredict;
			Config[ConfigName]["Grenade"] = GameData.Config.precision_calibration.Grenade;
			Config[ConfigName]["GrenadePredict"] = GameData.Config.precision_calibration.GrenadePredict;
			Config[ConfigName]["PanzerFaust"] = GameData.Config.precision_calibration.PanzerFaust;
			
			Config[ConfigName]["AutoConnect"] = GameData.Config.precision_calibration.AutoConnect;
			Config[ConfigName]["ConfigIndex"] = GameData.Config.precision_calibration.ConfigIndex;
			Config[ConfigName]["Controller"] = GameData.Config.precision_calibration.Controller;
			Config[ConfigName]["IP"] = GameData.Config.precision_calibration.IP;
			Config[ConfigName]["Port"] = GameData.Config.precision_calibration.Port;
			Config[ConfigName]["UUID"] = GameData.Config.precision_calibration.UUID;
			Config[ConfigName]["ShowFOV"] = GameData.Config.precision_calibration.ShowFOV;
			Config[ConfigName]["ShowWheelFOV"] = GameData.Config.precision_calibration.ShowWheelFOV;
			Config[ConfigName]["AdjustableDistance"] = GameData.Config.signal_overlay.AdjustableDistance;
			Config[ConfigName]["ShowPoint"] = GameData.Config.precision_calibration.ShowPoint;
			Config[ConfigName]["LockMode"] = GameData.Config.precision_calibration.LockMode;
			Config[ConfigName]["PointSize"] = GameData.Config.precision_calibration.PointSize;

			Config[ConfigName]["FOVColor"]["0"] = GameData.Config.precision_calibration.FOVColor[0];
			Config[ConfigName]["FOVColor"]["1"] = GameData.Config.precision_calibration.FOVColor[1];
			Config[ConfigName]["FOVColor"]["2"] = GameData.Config.precision_calibration.FOVColor[2];
			Config[ConfigName]["FOVColor"]["3"] = GameData.Config.precision_calibration.FOVColor[3];

			Config[ConfigName]["WheelFOVColor"]["0"] = GameData.Config.precision_calibration.WheelFOVColor[0];
			Config[ConfigName]["WheelFOVColor"]["1"] = GameData.Config.precision_calibration.WheelFOVColor[1];
			Config[ConfigName]["WheelFOVColor"]["2"] = GameData.Config.precision_calibration.WheelFOVColor[2];
			Config[ConfigName]["WheelFOVColor"]["3"] = GameData.Config.precision_calibration.WheelFOVColor[3];

			Config[ConfigName]["PointColor"]["0"] = GameData.Config.precision_calibration.PointColor[0];
			Config[ConfigName]["PointColor"]["1"] = GameData.Config.precision_calibration.PointColor[1];
			Config[ConfigName]["PointColor"]["2"] = GameData.Config.precision_calibration.PointColor[2];
			Config[ConfigName]["PointColor"]["3"] = GameData.Config.precision_calibration.PointColor[3];

			for (auto& Item : GameData.Config.precision_calibration.Configs)
			{
				std::string Index = std::to_string(Item.first);
				Config[ConfigName]["Configs"][Index]["Key"] = Item.second.Key;

				for (auto& Weapon : Item.second.Weapon)
				{
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["enable"] = Weapon.second.enable;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimAndShot"] = Weapon.second.AimAndShot;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Delay1"] = Weapon.second.Delay1;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Threshold"] = Weapon.second.Threshold;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["banjiAimDistance"] = Weapon.second.banjiAimDistance;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomAim"] = Weapon.second.RandomAim;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomFactor"] = Weapon.second.RandomFactor;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomInterval"] = Weapon.second.RandomInterval;

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomBodyParts"] = Weapon.second.RandomBodyParts;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomBodyPartCount"] = Weapon.second.RandomBodyPartCount;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomSpeed"] = Weapon.second.RandomSpeed;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["FOVenable"] = Weapon.second.FOVenable;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["VisibleCheck"] = Weapon.second.VisibleCheck;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AutoSwitch"] = Weapon.second.AutoSwitch;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["NoBulletNotAim"] = Weapon.second.NoBulletNotAim;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["IsScopeandAim"] = Weapon.second.IsScopeandAim;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["ShowFOV"] = Weapon.second.ShowFOV;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["HotkeyMerge"] = Weapon.second.HotkeyMerge;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimWheel"] = Weapon.second.AimWheel;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["NoRecoil"] = Weapon.second.NoRecoil;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["OriginalRecoil"] = Weapon.second.OriginalRecoil;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Prediction"] = Weapon.second.Prediction;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["DynamicFov"] = Weapon.second.DynamicFov;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["InitialValue"] = Weapon.second.InitialValue;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["FOV"] = Weapon.second.FOV;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["WheelFOV"] = Weapon.second.WheelFOV;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["SwitchingDelay"] = Weapon.second.SwitchingDelay;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimDistance"] = Weapon.second.AimDistance;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["XSpeed"] = Weapon.second.XSpeed;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["YSpeed"] = Weapon.second.YSpeed;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Smooth"] = Weapon.second.Smooth;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimWheelSpeed"] = Weapon.second.AimWheelSpeed;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RecoilTime"] = Weapon.second.RecoilTime;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["FPS"] = Weapon.second.FPS;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimSpeedMaxFactor"] = Weapon.second.AimSpeedMaxFactor;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["IgnoreGroggy"] = Weapon.second.IgnoreGroggy;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AimWheelBone"] = Weapon.second.AimWheelBone;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["PredictionMode"] = Weapon.second.PredictionMode;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["LineTraceSingle"] = Weapon.second.LineTraceSingle;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AutomaticShooting"] = Weapon.second.AutomaticShooting;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AutomaticShootingTime"] = Weapon.second.AutomaticShootingTime;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AutomaticShootingFOV"] = Weapon.second.AutomaticShootingFOV;

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["First"]["Key"] = Weapon.second.First.Key;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["First"]["Mode"] = Weapon.second.First.Mode;

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Second"]["Key"] = Weapon.second.Second.Key;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Second"]["Mode"] = Weapon.second.Second.Mode;

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Groggy"]["Key"] = Weapon.second.Groggy.Key;
					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Groggy"]["Mode"] = Weapon.second.Groggy.Mode;

					for (size_t i = 0; i < 17; i++)
					{
						Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["RandomBodyPartsList"][std::to_string(i)] = Weapon.second.RandomBodyPartsList[i];
					}

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["Wheel"]["Key"] = Weapon.second.Wheel.Key;

					Config[ConfigName]["Configs"][Index]["Weapon"][Weapon.first]["AutomaticShootingKey"]["Key"] = Weapon.second.AutomaticShootingKey;
				}
			}
			}();

		[&] {
			std::string ConfigName = "Overlay";
			Config[ConfigName]["VSync"] = GameData.Config.Overlay.VSync;
			Config[ConfigName]["Enable"] = GameData.Config.Overlay.bEnable;
			Config[ConfigName]["UseThread"] = GameData.Config.Overlay.UseThread;
			Config[ConfigName]["SafeExit"] = GameData.Config.Overlay.Quit_key;
			Config[ConfigName]["ShowMenu"] = GameData.Config.Menu.ShowKey;
			Config[ConfigName]["Fusion mode"] = GameData.Config.Overlay.ModeKey;
			}();
		[&] {
			std::string ConfigName = "Theme";
			Config[ConfigName]["Players"] = GameData.Config.Window.Players;
			Config[ConfigName]["Switch Languages"] = GameData.Config.Project.CurrentLanguage;
			}();
		[&] {
			std::string ConfigName = "Debug";
			Config[ConfigName]["EnablePerformanceMonitor"] = GameData.Config.Debug.EnablePerformanceMonitor;
		}();
		Utils::WriteConfigFile("Config/DHConfig.bak", Config.dump());
		Utils::Log(1, "[CONFIG] Save config thanh cong -> Config/DHConfig.bak");
		return true;
	};

	static void Load()
	{
		Utils::Log(0, "[CONFIG] Dang load config tu Config/DHConfig.bak...");
		std::string ConfigText = Utils::ReadConfigFile("Config/DHConfig.bak");

		if (ConfigText == "")
		{
			Utils::Log(2, "[CONFIG] Khong tim thay file config hoac file rong!");
			return;
		}

		std::string Text = Utils::ReadConfigFile("Config/DHConfig.bak");

		auto Config = nlohmann::json::parse(Text);
		Utils::Log(1, "[CONFIG] Parse config JSON thanh cong");

		[&] {
			std::string ConfigName = "precision_calibration";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.precision_calibration.Enable);
			SetConfigItem(Config, { ConfigName, "COM" }, GameData.Config.precision_calibration.COM);
			SetConfigItem(Config, { ConfigName, "ConfigIndex" }, GameData.Config.precision_calibration.ConfigIndex);
			SetConfigItem(Config, { ConfigName, "Controller" }, GameData.Config.precision_calibration.Controller);
			SetConfigItem(Config, { ConfigName, "AutoConnect" }, GameData.Config.precision_calibration.AutoConnect);



			SetConfigItem(Config, { ConfigName, "Mortar" }, GameData.Config.precision_calibration.Mortar);
			SetConfigItem(Config, { ConfigName, "Mortar2" }, GameData.Config.precision_calibration.Mortar2);
			SetConfigItem(Config, { ConfigName, "MortarPredict" }, GameData.Config.precision_calibration.MortarPredict);
			SetConfigItem(Config, { ConfigName, "MortarKey" }, GameData.Config.precision_calibration.MortarKey);
			SetConfigItem(Config, { ConfigName, "MortarFOV" }, GameData.Config.precision_calibration.MortarFOV);
			SetConfigItem(Config, { ConfigName, "MortarSmooth" }, GameData.Config.precision_calibration.MortarSmooth);
			SetConfigItem(Config, { ConfigName, "Grenade" }, GameData.Config.precision_calibration.Grenade);
			SetConfigItem(Config, { ConfigName, "GrenadePredict" }, GameData.Config.precision_calibration.GrenadePredict);
			SetConfigItem(Config, { ConfigName, "PanzerFaust" }, GameData.Config.precision_calibration.PanzerFaust);

			if (Config[ConfigName].count("IP"))
			{
				snprintf(GameData.Config.precision_calibration.IP, sizeof(GameData.Config.precision_calibration.IP), "%s", Utils::StringToUTF8(Config[ConfigName]["IP"]));
			}

			if (Config[ConfigName].count("Port"))
			{
				snprintf(GameData.Config.precision_calibration.Port, sizeof(GameData.Config.precision_calibration.Port), "%s", Utils::StringToUTF8(Config[ConfigName]["Port"]));
			}

			if (Config[ConfigName].count("UUID"))
			{
				snprintf(GameData.Config.precision_calibration.UUID, sizeof(GameData.Config.precision_calibration.UUID), "%s", Utils::StringToUTF8(Config[ConfigName]["UUID"]));
			}

			SetConfigItem(Config, { ConfigName, "ShowFOV" }, GameData.Config.precision_calibration.ShowFOV);
			SetConfigItem(Config, { ConfigName, "ShowWheelFOV" }, GameData.Config.precision_calibration.ShowWheelFOV);
			SetConfigItem(Config, { ConfigName, "ShowPoint" }, GameData.Config.precision_calibration.ShowPoint);
			SetConfigItem(Config, { ConfigName, "LockMode" }, GameData.Config.precision_calibration.LockMode);
			SetConfigItem(Config, { ConfigName, "PointSize" }, GameData.Config.precision_calibration.PointSize);
			SetConfigItem(Config, { ConfigName, "AdjustableDistance" }, GameData.Config.signal_overlay.AdjustableDistance);
			SetConfigItem(Config, { ConfigName, "FOVColor", "0" }, GameData.Config.precision_calibration.FOVColor[0]);
			SetConfigItem(Config, { ConfigName, "FOVColor", "1" }, GameData.Config.precision_calibration.FOVColor[1]);
			SetConfigItem(Config, { ConfigName, "FOVColor", "2" }, GameData.Config.precision_calibration.FOVColor[2]);
			SetConfigItem(Config, { ConfigName, "FOVColor", "3" }, GameData.Config.precision_calibration.FOVColor[3]);
			SetConfigItem(Config, { ConfigName, "WheelFOVColor", "0" }, GameData.Config.precision_calibration.WheelFOVColor[0]);
			SetConfigItem(Config, { ConfigName, "WheelFOVColor", "1" }, GameData.Config.precision_calibration.WheelFOVColor[1]);
			SetConfigItem(Config, { ConfigName, "WheelFOVColor", "2" }, GameData.Config.precision_calibration.WheelFOVColor[2]);
			SetConfigItem(Config, { ConfigName, "WheelFOVColor", "3" }, GameData.Config.precision_calibration.WheelFOVColor[3]);
			SetConfigItem(Config, { ConfigName, "PointColor", "0" }, GameData.Config.precision_calibration.PointColor[0]);
			SetConfigItem(Config, { ConfigName, "PointColor", "1" }, GameData.Config.precision_calibration.PointColor[1]);
			SetConfigItem(Config, { ConfigName, "PointColor", "2" }, GameData.Config.precision_calibration.PointColor[2]);
			SetConfigItem(Config, { ConfigName, "PointColor", "3" }, GameData.Config.precision_calibration.PointColor[3]);

			for (auto& Item : GameData.Config.precision_calibration.Configs)
			{
				std::string Index = std::to_string(Item.first);
				SetConfigItem(Config, { ConfigName, "Configs", Index, "Key" }, Item.second.Key);

				for (auto& Weapon : Item.second.Weapon)
				{
					
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomAim" }, Weapon.second.RandomAim);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomFactor" }, Weapon.second.RandomFactor);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomInterval" }, Weapon.second.RandomInterval);

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomBodyParts" }, Weapon.second.RandomBodyParts);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomBodyPartCount" }, Weapon.second.RandomBodyPartCount);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomSpeed" }, Weapon.second.RandomSpeed);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "enable" }, Weapon.second.enable);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "FOVenable" }, Weapon.second.FOVenable);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "VisibleCheck" }, Weapon.second.VisibleCheck);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AutoSwitch" }, Weapon.second.AutoSwitch);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "NoBulletNotAim" }, Weapon.second.NoBulletNotAim);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "IsScopeandAim" }, Weapon.second.IsScopeandAim);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "ShowFOV" }, Weapon.second.ShowFOV);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "HotkeyMerge" }, Weapon.second.HotkeyMerge);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimWheel" }, Weapon.second.AimWheel);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "NoRecoil" }, Weapon.second.NoRecoil);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Delay1" }, Weapon.second.Delay1);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "banjiAimDistance" }, Weapon.second.banjiAimDistance);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Threshold" }, Weapon.second.Threshold);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimAndShot" }, Weapon.second.AimAndShot);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "OriginalRecoil" }, Weapon.second.OriginalRecoil);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Prediction" }, Weapon.second.Prediction);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "DynamicFov" }, Weapon.second.DynamicFov);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "InitialValue" }, Weapon.second.InitialValue);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "FOV" }, Weapon.second.FOV);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "WheelFOV" }, Weapon.second.WheelFOV);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "SwitchingDelay" }, Weapon.second.SwitchingDelay);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimDistance" }, Weapon.second.AimDistance);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "XSpeed" }, Weapon.second.XSpeed);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "YSpeed" }, Weapon.second.YSpeed);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Smooth" }, Weapon.second.Smooth);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimWheelSpeed" }, Weapon.second.AimWheelSpeed);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RecoilTime" }, Weapon.second.RecoilTime);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "FPS" }, Weapon.second.FPS);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimSpeedMaxFactor" }, Weapon.second.AimSpeedMaxFactor);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "IgnoreGroggy" }, Weapon.second.IgnoreGroggy);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AimWheelBone" }, Weapon.second.AimWheelBone);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "PredictionMode" }, Weapon.second.PredictionMode);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "LineTraceSingle" }, Weapon.second.LineTraceSingle);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AutomaticShooting" }, Weapon.second.AutomaticShooting);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AutomaticShootingTime" }, Weapon.second.AutomaticShootingTime);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AutomaticShootingFOV" }, Weapon.second.AutomaticShootingFOV);

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "First", "Key" }, Weapon.second.First.Key);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "First", "Mode" }, Weapon.second.First.Mode);

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Second", "Key" }, Weapon.second.Second.Key);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Second", "Mode" }, Weapon.second.Second.Mode);

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Groggy", "Key" }, Weapon.second.Groggy.Key);
					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Groggy", "Mode" }, Weapon.second.Groggy.Mode);

					for (size_t i = 0; i < 17; i++)
					{
						SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "RandomBodyPartsList", std::to_string(i) }, Weapon.second.RandomBodyPartsList[i]);
					}

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "Wheel", "Key" }, Weapon.second.Wheel.Key);

					SetConfigItem(Config, { ConfigName, "Configs", Index, "Weapon", Weapon.first, "AutomaticShootingKey", "Key" }, Weapon.second.AutomaticShootingKey);

				}
			}
			}();

		[&] {
			std::string ConfigName = "Overlay";
			SetConfigItem(Config, { ConfigName, "VSync" }, GameData.Config.Overlay.VSync);
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Overlay.bEnable);
			SetConfigItem(Config, { ConfigName, "UseThread" }, GameData.Config.Overlay.UseThread);
			SetConfigItem(Config, { ConfigName, "Fusion mode" }, GameData.Config.Overlay.ModeKey);
			}();

		[&] {
			std::string ConfigName = "Function";
			SetConfigItem(Config, { ConfigName, "ClearKey" }, GameData.Config.Function.ClearKey);
			}();

		[&] {
			std::string ConfigName = "WebRadar";
			if (Config[ConfigName].count("Server IP"))
			{
				snprintf(GameData.Config.WebRadar.IP, sizeof(GameData.Config.WebRadar.IP), "%s", Utils::StringToUTF8(Config[ConfigName]["Server IP"]));
			}
			if (Config[ConfigName].count("Server Port"))
			{
				snprintf(GameData.Config.WebRadar.Port, sizeof(GameData.Config.WebRadar.Port), "%s", Utils::StringToUTF8(Config[ConfigName]["Server Port"]));
			}
			if (Config[ConfigName].count("Server PIN"))
			{
				snprintf(GameData.Config.WebRadar.PIN, sizeof(GameData.Config.WebRadar.PIN), "%s", Utils::StringToUTF8(Config[ConfigName]["Server PIN"]));
			}

			}();

		[&] {
			std::string ConfigName = "Vehicle";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Vehicle.Enable);
			SetConfigItem(Config, { ConfigName, "Health" }, GameData.Config.Vehicle.Health);
			SetConfigItem(Config, { ConfigName, "Durability" }, GameData.Config.Vehicle.Durability);
			SetConfigItem(Config, { ConfigName, "ShowIcon" }, GameData.Config.Vehicle.ShowIcon);
			SetConfigItem(Config, { ConfigName, "ShowIconAndText" }, GameData.Config.Vehicle.ShowIconAndText);
			SetConfigItem(Config, { ConfigName, "EnableKey" }, GameData.Config.Vehicle.EnableKey);
			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.Vehicle.DistanceMax);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.Vehicle.FontSize);

			SetConfigItem(Config, { ConfigName, "Color", "0" }, GameData.Config.Vehicle.Color[0]);
			SetConfigItem(Config, { ConfigName, "Color", "1" }, GameData.Config.Vehicle.Color[1]);
			SetConfigItem(Config, { ConfigName, "Color", "2" }, GameData.Config.Vehicle.Color[2]);
			SetConfigItem(Config, { ConfigName, "Color", "3" }, GameData.Config.Vehicle.Color[3]);

			SetConfigItem(Config, { ConfigName, "Healthbarcolor", "0" }, GameData.Config.Vehicle.Healthbarcolor[0]);
			SetConfigItem(Config, { ConfigName, "Healthbarcolor", "1" }, GameData.Config.Vehicle.Healthbarcolor[1]);
			SetConfigItem(Config, { ConfigName, "Healthbarcolor", "2" }, GameData.Config.Vehicle.Healthbarcolor[2]);
			SetConfigItem(Config, { ConfigName, "Healthbarcolor", "3" }, GameData.Config.Vehicle.Healthbarcolor[3]);

			SetConfigItem(Config, { ConfigName, "Fuelbarcolor", "0" }, GameData.Config.Vehicle.Fuelbarcolor[0]);
			SetConfigItem(Config, { ConfigName, "Fuelbarcolor", "1" }, GameData.Config.Vehicle.Fuelbarcolor[1]);
			SetConfigItem(Config, { ConfigName, "Fuelbarcolor", "2" }, GameData.Config.Vehicle.Fuelbarcolor[2]);
			SetConfigItem(Config, { ConfigName, "Fuelbarcolor", "3" }, GameData.Config.Vehicle.Fuelbarcolor[3]);
			}();

		[&] {
			std::string ConfigName = "AirDrop";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.AirDrop.Enable);
			SetConfigItem(Config, { ConfigName, "EnableKey" }, GameData.Config.AirDrop.EnableKey);
			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.AirDrop.DistanceMax);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.AirDrop.FontSize);
			SetConfigItem(Config, { ConfigName, "ShowItems" }, GameData.Config.AirDrop.ShowItems);

			SetConfigItem(Config, { ConfigName, "Color", "0" }, GameData.Config.AirDrop.Color[0]);
			SetConfigItem(Config, { ConfigName, "Color", "1" }, GameData.Config.AirDrop.Color[1]);
			SetConfigItem(Config, { ConfigName, "Color", "2" }, GameData.Config.AirDrop.Color[2]);
			SetConfigItem(Config, { ConfigName, "Color", "3" }, GameData.Config.AirDrop.Color[3]);
			}();

		[&] {
			std::string ConfigName = "DeadBox";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.AirDrop.Enable);
			SetConfigItem(Config, { ConfigName, "EnableKey" }, GameData.Config.AirDrop.EnableKey);
			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.AirDrop.DistanceMax);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.AirDrop.FontSize);
			SetConfigItem(Config, { ConfigName, "ShowItems" }, GameData.Config.AirDrop.ShowItems);

			SetConfigItem(Config, { ConfigName, "Color", "0" }, GameData.Config.AirDrop.Color[0]);
			SetConfigItem(Config, { ConfigName, "Color", "1" }, GameData.Config.AirDrop.Color[1]);
			SetConfigItem(Config, { ConfigName, "Color", "2" }, GameData.Config.AirDrop.Color[2]);
			SetConfigItem(Config, { ConfigName, "Color", "3" }, GameData.Config.AirDrop.Color[3]);
			}();

		[&] {
			std::string ConfigName = "Early";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Early.Enable);
			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.Early.DistanceMax);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.Early.FontSize);
			SetConfigItem(Config, { ConfigName, "ShowDistance" }, GameData.Config.Early.ShowDistance);
			}();

		[&] {
			std::string ConfigName = "PlayerList";
			SetConfigItem(Config, { ConfigName, "RankMode" }, GameData.Config.PlayerList.RankMode);
			}();

		[&] {
			std::string ConfigName = "Intelligent Display";
			SetConfigItem(Config, { ConfigName, "AccessoriesFilter" }, GameData.Config.Item.AccessoriesFilter);
			SetConfigItem(Config, { ConfigName, "HandheldWeaponFilter" }, GameData.Config.Item.HandheldWeaponFilter);
			SetConfigItem(Config, { ConfigName, "FilterHelmets" }, GameData.Config.Item.FilterHelmets);
			SetConfigItem(Config, { ConfigName, "ItemLimit" }, GameData.Config.Item.ItemLimit);

			SetConfigItem(Config, { ConfigName, "Bandage" }, GameData.Config.ItemFiltering.Bandage);
			SetConfigItem(Config, { ConfigName, "FirstAidKit" }, GameData.Config.ItemFiltering.FirstAidKit);
			SetConfigItem(Config, { ConfigName, "MedicalKit" }, GameData.Config.ItemFiltering.MedicalKit);
			SetConfigItem(Config, { ConfigName, "Painkiller" }, GameData.Config.ItemFiltering.Painkiller);
			SetConfigItem(Config, { ConfigName, "EnergyDrink" }, GameData.Config.ItemFiltering.EnergyDrink);
			SetConfigItem(Config, { ConfigName, "Adrenaline" }, GameData.Config.ItemFiltering.Adrenaline);
			SetConfigItem(Config, { ConfigName, "C4" }, GameData.Config.ItemFiltering.C4);
			SetConfigItem(Config, { ConfigName, "Grenade" }, GameData.Config.ItemFiltering.Grenade);
			SetConfigItem(Config, { ConfigName, "SmokeGrenade" }, GameData.Config.ItemFiltering.SmokeGrenade);
			SetConfigItem(Config, { ConfigName, "Flashbang" }, GameData.Config.ItemFiltering.Flashbang);
			SetConfigItem(Config, { ConfigName, "Molotov" }, GameData.Config.ItemFiltering.Molotov);
			SetConfigItem(Config, { ConfigName, "BlueZoneGrenade" }, GameData.Config.ItemFiltering.BlueZoneGrenade);
			SetConfigItem(Config, { ConfigName, "StickyBomb" }, GameData.Config.ItemFiltering.StickyBomb);
			}();

		[&] {
			std::string ConfigName = "Item";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Item.Enable);
			SetConfigItem(Config, { ConfigName, "EnableKey" }, GameData.Config.Item.EnableKey);
			//SetConfigItem(Config, { ConfigName, "GroupKey" }, GameData.Config.Item.GroupKey);
			SetConfigItem(Config, { ConfigName, "ShowGroups" }, GameData.Config.Item.ShowGroups);
			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.Item.DistanceMax);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.Item.FontSize);
			SetConfigItem(Config, { ConfigName, "Combination" }, GameData.Config.Item.Combination);
			SetConfigItem(Config, { ConfigName, "RayWidth" }, GameData.Config.Item.RayWidth);
			SetConfigItem(Config, { ConfigName, "ShowRay" }, GameData.Config.Item.ShowRay);
			SetConfigItem(Config, { ConfigName, "ShowIconAndText" }, GameData.Config.Item.ShowIconAndText);
			SetConfigItem(Config, { ConfigName, "ShowIcon" }, GameData.Config.Item.ShowIcon);
			SetConfigItem(Config, { ConfigName, "ShowDistance" }, GameData.Config.Item.ShowDistance);
			SetConfigItem(Config, { ConfigName, "ThresholdX" }, GameData.Config.Item.ThresholdX);
			SetConfigItem(Config, { ConfigName, "ThresholdY" }, GameData.Config.Item.ThresholdY);
			SetConfigItem(Config, { ConfigName, "GroupAKey" }, GameData.Config.Item.GroupAKey);
			SetConfigItem(Config, { ConfigName, "GroupBKey" }, GameData.Config.Item.GroupBKey);
			SetConfigItem(Config, { ConfigName, "GroupCKey" }, GameData.Config.Item.GroupCKey);
			SetConfigItem(Config, { ConfigName, "GroupDKey" }, GameData.Config.Item.GroupDKey);
			SetConfigItem(Config, { ConfigName, "ShowGroup" }, GameData.Config.Item.ShowGroup);
			SetConfigItem(Config, { ConfigName, "Scene" }, GameData.Scene);

			SetConfigItem(Config, { ConfigName, "GroupAColor", "0" }, GameData.Config.Item.GroupAColor[0]);
			SetConfigItem(Config, { ConfigName, "GroupAColor", "1" }, GameData.Config.Item.GroupAColor[1]);
			SetConfigItem(Config, { ConfigName, "GroupAColor", "2" }, GameData.Config.Item.GroupAColor[2]);
			SetConfigItem(Config, { ConfigName, "GroupAColor", "3" }, GameData.Config.Item.GroupAColor[3]);

			SetConfigItem(Config, { ConfigName, "GroupBColor", "0" }, GameData.Config.Item.GroupBColor[0]);
			SetConfigItem(Config, { ConfigName, "GroupBColor", "1" }, GameData.Config.Item.GroupBColor[1]);
			SetConfigItem(Config, { ConfigName, "GroupBColor", "2" }, GameData.Config.Item.GroupBColor[2]);
			SetConfigItem(Config, { ConfigName, "GroupBColor", "3" }, GameData.Config.Item.GroupBColor[3]);

			SetConfigItem(Config, { ConfigName, "GroupCColor", "0" }, GameData.Config.Item.GroupCColor[0]);
			SetConfigItem(Config, { ConfigName, "GroupCColor", "1" }, GameData.Config.Item.GroupCColor[1]);
			SetConfigItem(Config, { ConfigName, "GroupCColor", "2" }, GameData.Config.Item.GroupCColor[2]);
			SetConfigItem(Config, { ConfigName, "GroupCColor", "3" }, GameData.Config.Item.GroupCColor[3]);

			SetConfigItem(Config, { ConfigName, "GroupDColor", "0" }, GameData.Config.Item.GroupDColor[0]);
			SetConfigItem(Config, { ConfigName, "GroupDColor", "1" }, GameData.Config.Item.GroupDColor[1]);
			SetConfigItem(Config, { ConfigName, "GroupDColor", "2" }, GameData.Config.Item.GroupDColor[2]);
			SetConfigItem(Config, { ConfigName, "GroupDColor", "3" }, GameData.Config.Item.GroupDColor[3]);

			SetConfigItem(Config, { ConfigName, "RayColor", "0" }, GameData.Config.Item.RayColor[0]);
			SetConfigItem(Config, { ConfigName, "RayColor", "1" }, GameData.Config.Item.RayColor[1]);
			SetConfigItem(Config, { ConfigName, "RayColor", "2" }, GameData.Config.Item.RayColor[2]);
			SetConfigItem(Config, { ConfigName, "RayColor", "3" }, GameData.Config.Item.RayColor[3]);

			for (auto& pair : GameData.Config.Item.Lists) {
				const std::string& key = pair.first;
				ItemDetail& detail = pair.second;
				SetConfigItem(Config, { ConfigName, "Lists", key, "Group" }, detail.Group);
				SetConfigItem(Config, { ConfigName, "Lists", key, "ShowRay" }, detail.ShowRay);
			}
			}();

		[&] {
			std::string ConfigName = "signal_overlay";

			if (Config[ConfigName].count("ServerIP"))
			{
				snprintf(GameData.Config.WebRadar.IP, sizeof(GameData.Config.WebRadar.IP), "%s", Utils::StringToUTF8(Config[ConfigName]["ServerIP"]));
			}

			if (Config[ConfigName].count("ServerPort"))
			{
				snprintf(GameData.Config.WebRadar.Port, sizeof(GameData.Config.WebRadar.Port), "%s", Utils::StringToUTF8(Config[ConfigName]["ServerPort"]));
			}

			if (Config[ConfigName].count("ServerPIN"))
			{
				snprintf(GameData.Config.WebRadar.PIN, sizeof(GameData.Config.WebRadar.PIN), "%s", Utils::StringToUTF8(Config[ConfigName]["ServerPIN"]));
			}
			
			SetConfigItem(Config, { ConfigName, "Mouse" }, GameData.Config.signal_overlay.Mouse);
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.signal_overlay.Enable);
			SetConfigItem(Config, { ConfigName, "Stroke" }, GameData.Config.signal_overlay.Stroke);
			SetConfigItem(Config, { ConfigName, "DistanceStyle" }, GameData.Config.signal_overlay.DistanceStyle);
			SetConfigItem(Config, { ConfigName, "HealthBarStyle" }, GameData.Config.signal_overlay.HealthBarStyle);
			SetConfigItem(Config, { ConfigName, "VisibleCheck" }, GameData.Config.signal_overlay.VisibleCheck);
			SetConfigItem(Config, { ConfigName, "Bone Visibility" }, GameData.Config.signal_overlay.Skeleton);
			SetConfigItem(Config, { ConfigName, "AimExpandInfo" }, GameData.Config.signal_overlay.AimExpandInfo);
			SetConfigItem(Config, { ConfigName, "TargetedRay" }, GameData.Config.signal_overlay.TargetedRay);
			SetConfigItem(Config, { ConfigName, "VisibleCheckRay" }, GameData.Config.signal_overlay.VisibleCheckRay);
			SetConfigItem(Config, { ConfigName, "LockedHiddenBones" }, GameData.Config.signal_overlay.LockedHiddenBones);
			SetConfigItem(Config, { ConfigName, "PlayerLine" }, GameData.Config.signal_overlay.PlayerLine);
			SetConfigItem(Config, { ConfigName, "Rank Ico" }, GameData.Config.signal_overlay.showico);
			SetConfigItem(Config, { ConfigName, "suodingbianse" }, GameData.Config.signal_overlay.suodingbianse);
			SetConfigItem(Config, { ConfigName, "Weapon" }, GameData.Config.signal_overlay.Weapon);
			SetConfigItem(Config, { ConfigName, "Partner" }, GameData.Config.signal_overlay.Partner);
			SetConfigItem(Config, { ConfigName, "Grade" }, GameData.Config.signal_overlay.Level);
			SetConfigItem(Config, { ConfigName, "KillCount" }, GameData.Config.signal_overlay.Kills);
			SetConfigItem(Config, { ConfigName, "Damage Amount" }, GameData.Config.signal_overlay.Damage);
			SetConfigItem(Config, { ConfigName, "Audience" }, GameData.Config.signal_overlay.Spectate);
			SetConfigItem(Config, { ConfigName, "FPP Rank" }, GameData.Config.signal_overlay.FPP);
			SetConfigItem(Config, { ConfigName, "TPP Rank" }, GameData.Config.signal_overlay.TPP);
			SetConfigItem(Config, { ConfigName, "Offline" }, GameData.Config.signal_overlay.Offline);
			SetConfigItem(Config, { ConfigName, "Downed" }, GameData.Config.signal_overlay.Downed);
			SetConfigItem(Config, { ConfigName, "Display Frame" }, GameData.Config.signal_overlay.DisplayFrame);
			SetConfigItem(Config, { ConfigName, "Show Teammates" }, GameData.Config.signal_overlay.duiyouKey);
			SetConfigItem(Config, { ConfigName, "Playerskey" }, GameData.Config.signal_overlay.Playerskey);
			SetConfigItem(Config, { ConfigName, "DataSwitchkey" }, GameData.Config.signal_overlay.DataSwitchkey);
			SetConfigItem(Config, { ConfigName, "LowModel" }, GameData.Config.signal_overlay.LowModel);
			SetConfigItem(Config, { ConfigName, "MediumModel" }, GameData.Config.signal_overlay.MediumModel);
			SetConfigItem(Config, { ConfigName, "HighModel" }, GameData.Config.signal_overlay.HighModel);
			SetConfigItem(Config, { ConfigName, "PhysxLoadRadius" }, GameData.Config.signal_overlay.PhysxLoadRadius);
			/*for (size_t i = 0; i < 17; i++)
			{
				SetConfigItem(Config, { ConfigName, "ShowInfos", std::to_string(i) }, GameData.Config.signal_overlay.ShowInfos[i]);
			}*/
			SetConfigItem(Config, { ConfigName, "Head Drawing" }, GameData.Config.signal_overlay.HeadDrawing);
			SetConfigItem(Config, { ConfigName, "Data Switch" }, GameData.Config.signal_overlay.DataSwitch);
			SetConfigItem(Config, { ConfigName, "Danger Warning" }, GameData.Config.signal_overlay.DangerWarning);
			SetConfigItem(Config, { ConfigName, "Nickname" }, GameData.Config.signal_overlay.Nickname);
			SetConfigItem(Config, { ConfigName, "TeamID" }, GameData.Config.signal_overlay.TeamID);
			SetConfigItem(Config, { ConfigName, "ClanName" }, GameData.Config.signal_overlay.ClanName);
			SetConfigItem(Config, { ConfigName, "Dis" }, GameData.Config.signal_overlay.Dis);
			SetConfigItem(Config, { ConfigName, "health_bar" }, GameData.Config.signal_overlay.health_bar);
			SetConfigItem(Config, { ConfigName, "Health" }, GameData.Config.signal_overlay.Health);
			SetConfigItem(Config, { ConfigName, "healthBarPosition" }, GameData.Config.signal_overlay.HealthBarPos);
			SetConfigItem(Config, { ConfigName, "healthBarStyle" }, GameData.Config.signal_overlay.HealthBarStyle);
			SetConfigItem(Config, { ConfigName, "healthBarWidth" }, GameData.Config.signal_overlay.HealthBarWidth);
			SetConfigItem(Config, { ConfigName, "healthBarHeight" }, GameData.Config.signal_overlay.HealthBarHeight);
			SetConfigItem(Config, { ConfigName, "healthBarAlpha" }, GameData.Config.signal_overlay.HealthBarAlpha);

			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.signal_overlay.DistanceMax);
			SetConfigItem(Config, { ConfigName, "SkeletonWidth" }, GameData.Config.signal_overlay.SkeletonWidth);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.signal_overlay.FontSize);
			SetConfigItem(Config, { ConfigName, "WeaponDistanceMax" }, GameData.Config.signal_overlay.WeaponDistanceMax);
			SetConfigItem(Config, { ConfigName, "InfoDistanceMax" }, GameData.Config.signal_overlay.InfoDistanceMax);
			SetConfigItem(Config, { ConfigName, "RayWidth" }, GameData.Config.signal_overlay.RayWidth);
			SetConfigItem(Config, { ConfigName, "StrokeSize" }, GameData.Config.signal_overlay.StrokeSize);
			SetConfigItem(Config, { ConfigName, "FocusModeKey" }, GameData.Config.signal_overlay.FocusModeKey);
			SetConfigItem(Config, { ConfigName, "FocusMode" }, GameData.Config.signal_overlay.FocusMode);
			SetConfigItem(Config, { ConfigName, "DeadBox" }, GameData.Config.DeadBox.Enable);
			SetConfigItem(Config, { ConfigName, "Box Distance" }, GameData.Config.DeadBox.DistanceMax);


			SetConfigItem(Config, { ConfigName, "Color", "aim", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.aim.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "aim", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.aim.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "aim", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.aim.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "aim", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.aim.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Info", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Info.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Info", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Info.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Info", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Info.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Info", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Info.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "xuetiaoyanse", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "xuetiaoyanse", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "xuetiaoyanse", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "xuetiaoyanse", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.xuetiaoyanse.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Ray", "Line", "0" }, GameData.Config.signal_overlay.Color.Ray.Line[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Ray", "Line", "1" }, GameData.Config.signal_overlay.Color.Ray.Line[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Ray", "Line", "2" }, GameData.Config.signal_overlay.Color.Ray.Line[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Ray", "Line", "3" }, GameData.Config.signal_overlay.Color.Ray.Line[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Default", "Info", "0" }, GameData.Config.signal_overlay.Color.Default.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Info", "1" }, GameData.Config.signal_overlay.Color.Default.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Info", "2" }, GameData.Config.signal_overlay.Color.Default.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Info", "3" }, GameData.Config.signal_overlay.Color.Default.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Default", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Default.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Default.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Default.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Default", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Default.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Visible.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Visible.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Visible.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Visible.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Info", "0" }, GameData.Config.signal_overlay.Color.Visible.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Info", "1" }, GameData.Config.signal_overlay.Color.Visible.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Info", "2" }, GameData.Config.signal_overlay.Color.Visible.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Visible", "Info", "3" }, GameData.Config.signal_overlay.Color.Visible.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Partner.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Partner.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Partner.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Partner.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Info", "0" }, GameData.Config.signal_overlay.Color.Partner.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Info", "1" }, GameData.Config.signal_overlay.Color.Partner.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Info", "2" }, GameData.Config.signal_overlay.Color.Partner.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Partner", "Info", "3" }, GameData.Config.signal_overlay.Color.Partner.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Groggy.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Groggy.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Groggy.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Groggy.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Info", "0" }, GameData.Config.signal_overlay.Color.Groggy.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Info", "1" }, GameData.Config.signal_overlay.Color.Groggy.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Info", "2" }, GameData.Config.signal_overlay.Color.Groggy.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Groggy", "Info", "3" }, GameData.Config.signal_overlay.Color.Groggy.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Dangerous.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Dangerous.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Dangerous.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Dangerous.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Info", "0" }, GameData.Config.signal_overlay.Color.Dangerous.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Info", "1" }, GameData.Config.signal_overlay.Color.Dangerous.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Info", "2" }, GameData.Config.signal_overlay.Color.Dangerous.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Dangerous", "Info", "3" }, GameData.Config.signal_overlay.Color.Dangerous.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Blacklist.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Blacklist.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Blacklist.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Blacklist.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Info", "0" }, GameData.Config.signal_overlay.Color.Blacklist.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Info", "1" }, GameData.Config.signal_overlay.Color.Blacklist.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Info", "2" }, GameData.Config.signal_overlay.Color.Blacklist.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Blacklist", "Info", "3" }, GameData.Config.signal_overlay.Color.Blacklist.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.Whitelist.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.Whitelist.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.Whitelist.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.Whitelist.Skeleton[3]);

			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Info", "0" }, GameData.Config.signal_overlay.Color.Whitelist.Info[0]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Info", "1" }, GameData.Config.signal_overlay.Color.Whitelist.Info[1]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Info", "2" }, GameData.Config.signal_overlay.Color.Whitelist.Info[2]);
			SetConfigItem(Config, { ConfigName, "Color", "Whitelist", "Info", "3" }, GameData.Config.signal_overlay.Color.Whitelist.Info[3]);

			SetConfigItem(Config, { ConfigName, "Color", "AI", "Skeleton", "0" }, GameData.Config.signal_overlay.Color.AI.Skeleton[0]);
			SetConfigItem(Config, { ConfigName, "Color", "AI", "Skeleton", "1" }, GameData.Config.signal_overlay.Color.AI.Skeleton[1]);
			SetConfigItem(Config, { ConfigName, "Color", "AI", "Skeleton", "2" }, GameData.Config.signal_overlay.Color.AI.Skeleton[2]);
			SetConfigItem(Config, { ConfigName, "Color", "AI", "Skeleton", "3" }, GameData.Config.signal_overlay.Color.AI.Skeleton[3]);
			}();

		[&] {
			std::string ConfigName = "Project";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Project.Enable);

			SetConfigItem(Config, { ConfigName, "GrenadeEnable" }, GameData.Config.Project.GrenadeEnable);

			SetConfigItem(Config, { ConfigName, "MortarShooting" }, GameData.Config.Project.MortarShooting);


			SetConfigItem(Config, { ConfigName, "DistanceMax" }, GameData.Config.Project.DistanceMax);
			SetConfigItem(Config, { ConfigName, "Explosion Range" }, GameData.Config.Project.explosionrange);
			SetConfigItem(Config, { ConfigName, "Grenade Prediction" }, GameData.Config.Project.GrenadePrediction);
			SetConfigItem(Config, { ConfigName, "Grenade Trajectory" }, GameData.Config.Project.GrenadeTrajectory);
			SetConfigItem(Config, { ConfigName, "Explosion Timing" }, GameData.Config.Project.ShowChareTime);
			SetConfigItem(Config, { ConfigName, "FontSize" }, GameData.Config.Project.FontSize);
			SetConfigItem(Config, { ConfigName, "TrajectorySize" }, GameData.Config.Project.TrajectorySize);
			SetConfigItem(Config, { ConfigName, "TextShowChareTime" }, GameData.Config.Project.TextShowChareTime);
			SetConfigItem(Config, { ConfigName, "BarShowChareTime" }, GameData.Config.Project.BarShowChareTime);
			SetConfigItem(Config, { ConfigName, "ChareFontSize" }, GameData.Config.Project.ChareFontSize);

			SetConfigItem(Config, { ConfigName, "ChareColor", "0" }, GameData.Config.Project.ChareColor[0]);
			SetConfigItem(Config, { ConfigName, "ChareColor", "1" }, GameData.Config.Project.ChareColor[1]);
			SetConfigItem(Config, { ConfigName, "ChareColor", "2" }, GameData.Config.Project.ChareColor[2]);
			SetConfigItem(Config, { ConfigName, "ChareColor", "3" }, GameData.Config.Project.ChareColor[3]);

			SetConfigItem(Config, { ConfigName, "Color", "0" }, GameData.Config.Project.Color[0]);
			SetConfigItem(Config, { ConfigName, "Color", "1" }, GameData.Config.Project.Color[1]);
			SetConfigItem(Config, { ConfigName, "Color", "2" }, GameData.Config.Project.Color[2]);
			SetConfigItem(Config, { ConfigName, "Color", "3" }, GameData.Config.Project.Color[3]);

			SetConfigItem(Config, { ConfigName, "explosionrangeColor", "0" }, GameData.Config.Project.explosionrangeColor[0]);
			SetConfigItem(Config, { ConfigName, "explosionrangeColor", "1" }, GameData.Config.Project.explosionrangeColor[1]);
			SetConfigItem(Config, { ConfigName, "explosionrangeColor", "2" }, GameData.Config.Project.explosionrangeColor[2]);
			SetConfigItem(Config, { ConfigName, "explosionrangeColor", "3" }, GameData.Config.Project.explosionrangeColor[3]);

			SetConfigItem(Config, { ConfigName, "TrajectoryColor", "0" }, GameData.Config.Project.TrajectoryColor[0]);
			SetConfigItem(Config, { ConfigName, "TrajectoryColor", "1" }, GameData.Config.Project.TrajectoryColor[1]);
			SetConfigItem(Config, { ConfigName, "TrajectoryColor", "2" }, GameData.Config.Project.TrajectoryColor[2]);
			SetConfigItem(Config, { ConfigName, "TrajectoryColor", "3" }, GameData.Config.Project.TrajectoryColor[3]);

			}();

		[&] {
			std::string ConfigName = "AutoRecoil";
			SetConfigItem(Config, { ConfigName, "Enabled" }, GameData.Config.precision_calibration.Recoilenanlek);
			SetConfigItem(Config, { ConfigName, "Red Dot Amplitude" }, GameData.Config.precision_calibration.yRecoil[0]);
			SetConfigItem(Config, { ConfigName, "Double Amplitude" }, GameData.Config.precision_calibration.yRecoil[1]);
			SetConfigItem(Config, { ConfigName, "Triple Amplitude" }, GameData.Config.precision_calibration.yRecoil[2]);
			SetConfigItem(Config, { ConfigName, "Four Times The Amplitude" }, GameData.Config.precision_calibration.yRecoil[3]);
			SetConfigItem(Config, { ConfigName, "Six Times Amplitude" }, GameData.Config.precision_calibration.yRecoil[4]);
			SetConfigItem(Config, { ConfigName, "Eight Times Amplitude" }, GameData.Config.precision_calibration.yRecoil[5]);
			SetConfigItem(Config, { ConfigName, "Red Dot Delay" }, GameData.Config.precision_calibration.interval[0]);
			SetConfigItem(Config, { ConfigName, "Double Latency" }, GameData.Config.precision_calibration.interval[1]);
			SetConfigItem(Config, { ConfigName, "Triple Delay" }, GameData.Config.precision_calibration.interval[2]);
			SetConfigItem(Config, { ConfigName, "Four Times Delay" }, GameData.Config.precision_calibration.interval[3]);
			SetConfigItem(Config, { ConfigName, "Six Times Delay" }, GameData.Config.precision_calibration.interval[4]);
			SetConfigItem(Config, { ConfigName, "Eight Times Delay" }, GameData.Config.precision_calibration.interval[5]);

			}();

		[&] {
			std::string ConfigName = "Recoil";
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Recoil.Enable);
			SetConfigItem(Config, { ConfigName, "RedDot" }, GameData.Config.Recoil.RedDot);
			SetConfigItem(Config, { ConfigName, "x2" }, GameData.Config.Recoil.x2);
			SetConfigItem(Config, { ConfigName, "x3" }, GameData.Config.Recoil.x3);
			SetConfigItem(Config, { ConfigName, "x4" }, GameData.Config.Recoil.x4);
			SetConfigItem(Config, { ConfigName, "x6" }, GameData.Config.Recoil.x6);
			SetConfigItem(Config, { ConfigName, "x8" }, GameData.Config.Recoil.x8);
			SetConfigItem(Config, { ConfigName, "Delay" }, GameData.Config.Recoil.Delay);
			}();
		[&] {
			std::string ConfigName = "Radar";
			SetConfigItem(Config, { ConfigName, "Main", "ShowPlayer" }, GameData.Config.Radar.Main.ShowPlayer);
			SetConfigItem(Config, { ConfigName, "Main", "ShowVehicle" }, GameData.Config.Radar.Main.ShowVehicle);
			SetConfigItem(Config, { ConfigName, "Main", "ShowAirDrop" }, GameData.Config.Radar.Main.ShowAirDrop);
			SetConfigItem(Config, { ConfigName, "Main", "ShowDeadBox" }, GameData.Config.Radar.Main.ShowDeadBox);
			SetConfigItem(Config, { ConfigName, "Main", "MapRoom" }, GameData.Config.Radar.Main.MapRoom);
			SetConfigItem(Config, { ConfigName, "Main", "FontSize" }, GameData.Config.Radar.Main.FontSize);

			SetConfigItem(Config, { ConfigName, "Main", "MapColor", "0" }, GameData.Config.Radar.Main.MapColor[0]);
			SetConfigItem(Config, { ConfigName, "Main", "MapColor", "1" }, GameData.Config.Radar.Main.MapColor[1]);
			SetConfigItem(Config, { ConfigName, "Main", "MapColor", "2" }, GameData.Config.Radar.Main.MapColor[2]);
			SetConfigItem(Config, { ConfigName, "Main", "MapColor", "3" }, GameData.Config.Radar.Main.MapColor[3]);

			SetConfigItem(Config, { ConfigName, "Main", "Map_size" }, GameData.Config.Radar.Main.Map_size);
			SetConfigItem(Config, { ConfigName, "Mini", "ShowPlayer" }, GameData.Config.Radar.Mini.ShowPlayer);
			SetConfigItem(Config, { ConfigName, "Mini", "ShowVehicle" }, GameData.Config.Radar.Mini.ShowVehicle);
			SetConfigItem(Config, { ConfigName, "Mini", "ShowAirDrop" }, GameData.Config.Radar.Mini.ShowAirDrop);
			SetConfigItem(Config, { ConfigName, "Mini", "ShowDeadBox" }, GameData.Config.Radar.Mini.ShowDeadBox);
			SetConfigItem(Config, { ConfigName, "Mini", "MiniMapRoom" }, GameData.Config.Radar.Mini.MapRoom);
			SetConfigItem(Config, { ConfigName, "Mini", "FontSize" }, GameData.Config.Radar.Mini.FontSize);

			SetConfigItem(Config, { ConfigName, "Mini", "MapColor", "0" }, GameData.Config.Radar.Mini.MapColor[0]);
			SetConfigItem(Config, { ConfigName, "Mini", "MapColor", "1" }, GameData.Config.Radar.Mini.MapColor[1]);
			SetConfigItem(Config, { ConfigName, "Mini", "MapColor", "2" }, GameData.Config.Radar.Mini.MapColor[2]);
			SetConfigItem(Config, { ConfigName, "Mini", "MapColor", "3" }, GameData.Config.Radar.Mini.MapColor[3]);

			SetConfigItem(Config, { ConfigName, "Mini", "Map_size" }, GameData.Config.Radar.Mini.Map_size);
			}();

		[&] {
			std::string ConfigName = "Overlay";
			SetConfigItem(Config, { ConfigName, "VSync" }, GameData.Config.Overlay.VSync);
			SetConfigItem(Config, { ConfigName, "Enable" }, GameData.Config.Overlay.bEnable);
			SetConfigItem(Config, { ConfigName, "UseThread" }, GameData.Config.Overlay.UseThread);
			SetConfigItem(Config, { ConfigName, "SafeExit" }, GameData.Config.Overlay.Quit_key);
			SetConfigItem(Config, { ConfigName, "ShowMenu" }, GameData.Config.Menu.ShowKey);
			SetConfigItem(Config, { ConfigName, "Fusion mode" }, GameData.Config.Overlay.ModeKey);

			}();


		[&] {
			std::string ConfigName = "Theme";
			SetConfigItem(Config, { ConfigName, "Players" }, GameData.Config.Window.Players);
			SetConfigItem(Config, { ConfigName, "Switch Languages" }, GameData.Config.Project.CurrentLanguage);
			}();

		[&] {
			std::string ConfigName = "Debug";
			SetConfigItem(Config, { ConfigName, "EnablePerformanceMonitor" }, GameData.Config.Debug.EnablePerformanceMonitor);
			}();

		Utils::Log(1, "[CONFIG] Load config hoan tat - signal_overlay: %s | precision_calibration: %s | Radar Main: %s",
			GameData.Config.signal_overlay.Enable ? "ON" : "OFF",
			GameData.Config.precision_calibration.Enable ? "ON" : "OFF",
			GameData.Config.Radar.Main.ShowPlayer ? "ON" : "OFF");
	}
};