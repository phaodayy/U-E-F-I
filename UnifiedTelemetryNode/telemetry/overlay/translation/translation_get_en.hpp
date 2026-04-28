#pragma once

#include "translation_strings.hpp"

namespace Translation {

inline void FillEnglish(Strings& s) {

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
            s.NamePos = skCrypt("Name Position");
            s.DistancePos = skCrypt("Distance Position");
            s.WeaponPos = skCrypt("Weapon Position");
            s.RankPos = skCrypt("Rank Position");
            s.SpectatedPos = skCrypt("Spectated Position");

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
            s.ShowcaseRankLayout = skCrypt("Rank Layout");
            s.ShowcaseMarkerLayout = skCrypt("Compact Marker Layout");
            s.ShowcaseControllerMatrix = skCrypt("Controller Matrix");
            s.ShowcaseKeyPresets = skCrypt("Key Presets");
            s.ShowcaseHotkeyMatrix = skCrypt("Hotkey Matrix");
            s.ShowcaseEquipmentTier2 = skCrypt("Equipment Tier II");
            s.ShowcaseEquipmentTier3 = skCrypt("Equipment Tier III");
            s.ShowcaseOpticCatalog = skCrypt("Optic Catalog");
            s.ShowcaseAttachmentCatalog = skCrypt("Attachment Catalog");
            s.ShowcaseWeaponCatalog = skCrypt("Weapon Catalog");
            s.ShowcaseTacticalCatalog = skCrypt("Tactical Catalog");
            s.ShowcaseProfileSlots = skCrypt("Interface Profile Slots");
            s.ShowcaseAnnouncementPanel = skCrypt("Announcement Panel");
            s.ShowcaseMapLayers = skCrypt("Map Layers");
            s.ShowcaseSharedRadarProfile = skCrypt("Shared Map Profile");
            s.PreviewPanel = skCrypt("INTERFACE PREVIEW");
            s.SimulateWall = skCrypt("Simulate Occlusion");
            s.Snaplines = skCrypt("Snaplines");
            s.MaxDistance = skCrypt("Max Distance");
            s.FovColor = skCrypt("FOV Color");
            s.CurrentMethod = skCrypt("Current Method");
            s.CloseOverlay = skCrypt("Close Overlay");
            s.AdsOnly = skCrypt("ADS Only");
            s.OsdColor = skCrypt("OSD Color");
            s.MacroStatusPreview = skCrypt("Macro profile preview active");
            s.ShowVehicles = skCrypt("Show Vehicles");
            s.ShowAirdrops = skCrypt("Show Airdrops");
            s.ShowDeathboxes = skCrypt("Show Deathboxes");
            s.HeaderWeaponry = skCrypt("WEAPONS & ATTACHMENTS");
            s.SpecialWeapons = skCrypt("Special Weapons");
            s.AllWeapons = skCrypt("All Weapons");
            s.MuzzleAccess = skCrypt("Muzzle Attachments");
            s.ExtendedMags = skCrypt("Extended Magazines");
            s.ShowNeutralTargets = skCrypt("Show Neutral Targets");
            s.MiniMapConfig = skCrypt("MINI MAP CONFIG");
            s.LiveSharing = skCrypt("Live Sharing");
            s.StatusOnline = skCrypt("ONLINE");
            s.StatusOffline = skCrypt("OFFLINE");
            s.ShowcaseEspNameplates = skCrypt("Layered Nameplates");
            s.ShowcaseEspThreatBands = skCrypt("Threat Warning Bands");
            s.ShowcaseEspTeamColors = skCrypt("Custom Team Colors");
            s.ShowcaseAimProfiles = skCrypt("Per-Weapon Aim Profiles");
            s.ShowcaseAimCurve = skCrypt("Smoothing Curve");
            s.ShowcaseAimPriority = skCrypt("Target Priority");
            s.ShowcaseMacroWeaponProfiles = skCrypt("Weapon Macro Profiles");
            s.ShowcaseMacroSensitivity = skCrypt("Sensitivity Profiles");
            s.ShowcaseMacroOverlayLayout = skCrypt("Macro OSD Layout");
            s.ShowcaseLootRareItems = skCrypt("Rare Item Filter");
            s.ShowcaseLootConsumables = skCrypt("Detailed Consumable Filter");
            s.ShowcaseLootThrowables = skCrypt("Throwable Filter");
            s.ShowcaseRadarPins = skCrypt("Map Pins");
            s.ShowcaseRadarLegend = skCrypt("Map Legend");
            s.ItemCatalogTitle = skCrypt("Item Image Catalog");
            s.ItemCatalogHint = skCrypt("Selection is menu-only");
            s.ItemCatalogSelected = skCrypt("Selected");
            s.BackpackLv1 = skCrypt("Backpack Level 1");
            s.BackpackLv2 = skCrypt("Backpack Level 2");
            s.BackpackLv3 = skCrypt("Backpack Level 3");
            s.GhillieSuits = skCrypt("Ghillie Suits");
            s.SurvivalUtility = skCrypt("Survival Gear");
            s.RepairKits = skCrypt("Repair Kits");
            s.Grips = skCrypt("Foregrips");
            s.Stocks = skCrypt("Stocks / Pads");
            s.HeaderAttachments = skCrypt("[-] OTHER ATTACHMENTS");
            s.HeaderSurvival = skCrypt("[-] SURVIVAL & UTILITY");
            s.PlayerList = skCrypt("Player List");
            s.Spectators = skCrypt("Spectators");
            s.HoldPanelKey = skCrypt("Hold Panel Key");
            s.PanelKey = skCrypt("Panel Key");
            s.HealthDisplay = skCrypt("Health Display");
            s.HealthColumn = skCrypt("Column");
            s.HealthTextOnly = skCrypt("Text");
            s.Ammo = skCrypt("Ammo");
            s.Damage = skCrypt("Damage");
            s.Speed = skCrypt("Speed");
            s.AimWarn = skCrypt("Aim Warn");
            s.CloseWarn = skCrypt("Close Warn");
            s.ViewRay = skCrypt("View Ray");
            s.FireTrace = skCrypt("Fire Trace");
            s.RayWidth = skCrypt("Ray Width");
            s.BigMap = skCrypt("BigMap");
            s.MiniMap = skCrypt("MiniMap");
            s.EnableMiniMap = skCrypt("Enable MiniMap");
            s.EnableBigMap = skCrypt("Enable BigMap");
            s.NameRadar = skCrypt("Name Radar");
            s.DirectionRadar = skCrypt("Direction Radar");
            s.NameBackground = skCrypt("Name Background");
            s.VehicleRadar = skCrypt("Vehicle Radar");
            s.AirdropRadar = skCrypt("Airdrop Radar");
            s.DeadBoxRadar = skCrypt("DeadBox Radar");
            s.MapLegend = skCrypt("Map Legend");
            s.Enemy = skCrypt("Enemy");
            s.Vehicle = skCrypt("Vehicle");
            s.State = skCrypt("State");
            s.HelmetShort = skCrypt("Helmet");
            s.VestShort = skCrypt("Vest");
            s.BackpackShort = skCrypt("Pack");
            s.HeaderVehicleFilter = skCrypt("[-] VEHICLE FILTER");
            s.VehicleUAZ = skCrypt("UAZ Vehicle");
            s.VehicleDacia = skCrypt("Dacia Vehicle");
            s.VehicleBuggy = skCrypt("Buggy Vehicle");
            s.VehicleBike = skCrypt("Bike / Bicycle");
            s.VehicleBoat = skCrypt("Boat / Ship");
            s.VehicleAir = skCrypt("Aircraft / Glider");
            s.VehicleBRDM = skCrypt("BRDM Armored");
            s.VehicleScooter = skCrypt("Scooter");
            s.VehicleTuk = skCrypt("TukTuk");
            s.VehicleSnow = skCrypt("Snow Vehicle");
            s.VehicleBus = skCrypt("Mini Bus");
            s.VehicleTruck = skCrypt("Truck / Loot Truck");
            s.VehicleTrain = skCrypt("Train");
            s.VehicleMirado = skCrypt("Mirado Vehicle");
            s.VehiclePickup = skCrypt("Pickup Truck");
            s.VehicleRony = skCrypt("Rony Vehicle");
            s.VehicleBlanc = skCrypt("Blanc Vehicle");

            s.HeaderAccount = skCrypt("LOADER ACCOUNT");
            s.Username = skCrypt("Username");
            s.Password = skCrypt("Password");
            s.Login = skCrypt("Login");
            s.Logout = skCrypt("Logout");
            s.Register = skCrypt("Register");
            s.ActivateKey = skCrypt("Activate Key");
            s.Expiry = skCrypt("Expiry Date");
            s.ConfigCode = skCrypt("Config Code");
            s.SaveCloud = skCrypt("Cloud Sync");
            s.ImportCloud = skCrypt("Cloud Import");
            s.CloudConfig = skCrypt("Cloud Configuration");
            s.TabAdmin = skCrypt("Admin");
}

} // namespace Translation
