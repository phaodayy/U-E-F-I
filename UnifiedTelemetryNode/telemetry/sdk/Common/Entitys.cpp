#include "common/Entitys.h"
#include "common/Data.h"
#include <Utils/FNVHash.h>

std::unordered_map<std::string, EntityInfo, FnvHash> EntityPlayerLists = {
	//??
	{"PlayerMale_A", {"type.player", EntityType::Player, 0}},
	{"PlayerMale_A_C", {"type.player", EntityType::Player, 0}},
	{"PlayerFemale_A", {"type.player", EntityType::Player, 0}},
	{"PlayerFemale_A_C", {"type.player", EntityType::Player, 0}},
	{"AIPawn_Base_C", {"type.bot", EntityType::AI, 0}},
	{"AIPawn_Base_Female_C", {"type.bot", EntityType::AI, 0}},
	{"AIPawn_Base_Male_C", {"type.bot", EntityType::AI, 0}},
	{"AIPawn_Base_Pillar_C", {"type.bot", EntityType::AI, 0}},
	{"AIPawn_Base_Female_Pillar_C", {"type.bot", EntityType::AI, 0}},
	{"AIPawn_Base_Male_Pillar_C", {"type.bot", EntityType::AI, 0}},
	{"UltAIPawn_Base_C", {"type.bot", EntityType::AI, 0}},
	{"UltAIPawn_Base_Female_C", {"type.bot", EntityType::AI, 0}},
	{"UltAIPawn_Base_Male_C", {"type.bot", EntityType::AI, 0}},
	{"ZDF2_NPC_Runner_C", {"type.bot", EntityType::AI, 0}},
	{"ZDF2_NPC_Burning_C", {"type.bot", EntityType::AI, 0}},
	{"ZDF2_NPC_Tanker_C", {"type.bot", EntityType::AI, 0}},
	{"ZDF2_NPC_Female_C", {"type.bot", EntityType::AI, 0}},
	{"ZombieNpcNewPawn_Tanker_C", {"type.bot", EntityType::AI, 0}},
	{"UltAIPawn_Base_Pillar_C", {"type.bot", EntityType::AI, 0}},
    {"UltAIPawn_Base_Female_Pillar_C", {"type.bot", EntityType::AI, 0}},
    {"UltAIPawn_Base_Male_Pillar_C", {"type.bot", EntityType::AI, 0}},
    {"BP_MarketAI_Pawn_C", {"type.bot", EntityType::AI, 0}},
};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityItemLists = {
	//??
	{"DeathDropItemPackage_C", {"menu.box_perspective", EntityType::DeadBox, 0}},
	{"Carapackage_RedBox_C", {"menu.airdrop_perspective", EntityType::AirDrop, 0}},
	{"Carapackage_SmallPackage_C", {"menu.airdrop_perspective", EntityType::AirDrop, 0}},
	{"Carapackage_FlareGun_C", {"menu.airdrop_perspective", EntityType::AirDrop, 0}},
	{"Carapackage_SmallPackage_SLB_C", {"menu.airdrop_perspective", EntityType::AirDrop, 0}},
	{"Carapackage_SmallPackage_DihorOtok_C", {"menu.airdrop_perspective", EntityType::AirDrop, 0}},

	//??
	{"DroppedItem", {"DroppedItem", EntityType::DroppedItem, 0}},
	{"DroppedItemGroup", {"DroppedItemGroup", EntityType::DroppedItemGroup, 0}},

	//????
	{"Item_Weapon_Groza_C", {"Groza", EntityType::Item, 0, WeaponType::AR, "", "Item_Weapon_Groza_C.png"}},
	{"Item_Weapon_BerylM762_C", {"Beryl M762", EntityType::Item, 0, WeaponType::AR, "", "Item_Weapon_BerylM762_C.png"}},
	{"Item_Weapon_ACE32_C", {"ACE32", EntityType::Item, 0, WeaponType::AR, "", "Item_Weapon_ACE32_C.png"}},
	{"Item_Weapon_HK416_C", {"M416", EntityType::Item, 0, WeaponType::AR, "", "Item_Weapon_HK416_C.png"}},
	{"Item_Weapon_FAMASG2_C", {"FAMASI", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_AUG_C", {"AUG", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_AK47_C", {"AKM", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_SCAR-L_C", {"SCAR-L", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_G36C_C", {"G36C", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_QBZ95_C", {"QBZ95", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_K2_C", {"K2", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_Mk47Mutant_C", {"Mk47", EntityType::Item, 0, WeaponType::AR}},
	{"Item_Weapon_M16A4_C", {"M16A4", EntityType::Item, 0, WeaponType::AR}},

	{"Item_Weapon_MG3_C", {"MG3", EntityType::Item, 0, WeaponType::LMG}},
	{"Item_Weapon_DP28_C", {"DP28", EntityType::Item, 0, WeaponType::LMG}},
	{"Item_Weapon_M249_C", {"M249", EntityType::Item, 0, WeaponType::LMG}},

	{"Item_Weapon_L6_C", {"Lynx AMR", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_AWM_C", {"AWM", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_M24_C", {"M24", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_Kar98k_C", {"Kar98k", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_Mosin_C", {"Mosin Nagant", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_Win1894_C", {"Win94", EntityType::Item, 0, WeaponType::SR}},
	{"Item_Weapon_Crossbow_C", {"Crossbow", EntityType::Item, 0, WeaponType::SR}},

	{"Item_Weapon_Mk14_C", {"Mk14", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_FNFal_C", {"SLR", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_Mk12_C", {"Mk12", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_SKS_C", {"SKS", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_QBU88_C", {"QBU", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_Dragunov_C", {"Dragunov", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_Mini14_C", {"Mini14", EntityType::Item, 0, WeaponType::DMR}},
	{"Item_Weapon_VSS_C", {"VSS", EntityType::Item, 0, WeaponType::DMR}},

	{"Item_Weapon_OriginS12_C", {"O12", EntityType::Item, 0, WeaponType::SG}},
	{"Item_Weapon_DP12_C", {"DBS", EntityType::Item, 0, WeaponType::SG}},
	{"Item_Weapon_Saiga12_C", {"S12K", EntityType::Item, 0, WeaponType::SG}},
	{"Item_Weapon_Winchester_C", {"S1897", EntityType::Item, 0, WeaponType::SG}},
	{"Item_Weapon_Berreta686_C", {"S686", EntityType::Item, 0, WeaponType::SG}},


	{"Item_Weapon_P90_C", {"P90", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_Vector_C", {"Vector", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_UZI_C", {"UZI", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_UMP_C", {"UMP", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_Thompson_C", {"Thompson", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_BizonPP19_C", {"PP-19 Bizon", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_JS9_C", {"JS9", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_MP5K_C", {"MP5K", EntityType::Item, 0, WeaponType::SMG}},
	{"Item_Weapon_MP9_C", {"MP9", EntityType::Item, 0, WeaponType::SMG}},

	{"Item_Weapon_FlareGun_C", {"Flare Gun", EntityType::Item, 0, WeaponType::HG} },
	{"Item_Weapon_G18_C", {"P18C", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_StunGun_C", {"Stun Gun", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_M1911_C", {"P1911", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_M9_C", {"P92", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_NagantM1895_C", {"R1895", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_Rhino_C", {"R45", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_DesertEagle_C", {"Desert Eagle", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_vz61Skorpion_C", {"Skorpion", EntityType::Item, 0, WeaponType::HG}},
	{"Item_Weapon_Sawnoff_C", {"Sawed-off", EntityType::Item, 0, WeaponType::HG}},


	//??
	{"Item_Heal_MedKit_C", {"Med Kit", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Heal_FirstAid_C", {"First Aid", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Heal_Bandage_C", {"Bandage", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Weapon_TraumaBag_C", {"Trauma Bag", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Weapon_TacPack_C", {"Tactical Pack", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Boost_AdrenalineSyringe_C", {"Adrenaline", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Boost_EnergyDrink_C", {"Energy Drink", EntityType::Item, 0, WeaponType::Drug}},
	{"Item_Boost_PainKiller_C", {"Painkiller", EntityType::Item, 0, WeaponType::Drug}},

	//??
	{"Item_Head_E_01_Lv1_C", {"Helmet Lv.1", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Head_E_02_Lv1_C", {"Helmet Lv.1", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Head_F_01_Lv2_C", {"Helmet Lv.2", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Head_F_02_Lv2_C", {"Helmet Lv.2", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Head_G_01_Lv3_C", {"Helmet Lv.3", EntityType::Item, 0, WeaponType::Armor, "", "Item_Head_G_01_Lv3_C.png"}},

	{"Item_Armor_E_01_Lv1_C", {"Vest Lv.1", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Armor_D_01_Lv2_C", {"Vest Lv.2", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Armor_C_01_Lv3_C", {"Vest Lv.3", EntityType::Item, 0, WeaponType::Armor, "", "Item_Armor_C_01_Lv3_C.png"}},

	{"Item_Back_BlueBlocker", {"Jammer Pack", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_E_02_Lv1_C", {"Backpack Lv.1", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_E_01_Lv1_C", {"Backpack Lv.1", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_F_02_Lv2_C", {"Backpack Lv.2", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_F_01_Lv2_C", {"Backpack Lv.2", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_C_02_Lv3_C", {"Backpack Lv.3", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_C_01_Lv3_C", {"Backpack Lv.3", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Back_B_08_Lv3_C", {"Backpack Lv.3", EntityType::Item, 0, WeaponType::Armor, "", "Item_Back_B_08_Lv3_C.png"}},

	{"Item_Ghillie_01_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Ghillie_02_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Ghillie_03_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Ghillie_04_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Ghillie_05_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},
	{"Item_Ghillie_06_C", {"Ghillie Suit", EntityType::Item, 0, WeaponType::Armor}},

	//??
	{ "Item_Attach_Weapon_Muzzle_Compensator_Large_C", {"Compensator (AR)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_FlashHider_Large_C", {"Flash Hider (AR)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_Suppressor_Large_C", {"Suppressor (AR)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_AR_MuzzleBrake_C", {"Muzzle Brake (AR)", EntityType::Item, 0, WeaponType::Muzzle} },

	{ "Item_Attach_Weapon_Muzzle_Compensator_SniperRifle_C", {"Compensator (SR)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_FlashHider_SniperRifle_C", {"Flash Hider (SR)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_Suppressor_SniperRifle_C", {"Suppressor (SR)", EntityType::Item, 0, WeaponType::Muzzle} },

	{ "Item_Attach_Weapon_Muzzle_Compensator_Medium_C", {"Compensator (SMG)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_FlashHider_Medium_C", {"Flash Hider (SMG)", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_Suppressor_Medium_C", {"Suppressor (SMG)", EntityType::Item, 0, WeaponType::Muzzle} },

	{ "Item_Attach_Weapon_Muzzle_Choke_C", {"Choke", EntityType::Item, 0, WeaponType::Muzzle} },
	{ "Item_Attach_Weapon_Muzzle_Duckbill_C", {"Duckbill", EntityType::Item, 0, WeaponType::Muzzle} },

	//??
	{ "Item_Attach_Weapon_SideRail_DotSight_RMR_C", {"Canted Sight", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_DotSight_01_C", {"Red Dot", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_Holosight_C", {"Holographic", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_Aimpoint_C", {"2x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_Scope3x_C", {"3x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_ACOG_01_C", {"4x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_Scope6x_C", {"6x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_CQBSS_C", {"8x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_PM2_01_C", {"15x Scope", EntityType::Item, 0, WeaponType::Sight} },
	{ "Item_Attach_Weapon_Upper_Thermal_C", {"Thermal Scope", EntityType::Item, 0, WeaponType::Sight} },

	//??
	{ "Item_Attach_Weapon_Stock_AR_Composite_C", {"Tactical Stock", EntityType::Item, 0, WeaponType::GunButt} },
	{ "Item_Attach_Weapon_Stock_AR_HeavyStock_C", {"Heavy Stock", EntityType::Item, 0, WeaponType::GunButt} },
	{ "Item_Attach_Weapon_Stock_SniperRifle_CheekPad_C", {"Cheek Pad", EntityType::Item, 0, WeaponType::GunButt} },
	{ "Item_Attach_Weapon_Stock_SniperRifle_BulletLoops_C", {"Bullet Loops", EntityType::Item, 0, WeaponType::GunButt} },
	{ "Item_Attach_Weapon_Stock_UZI_C", {"Folding Stock", EntityType::Item, 0, WeaponType::GunButt} },
	//??
	{"Item_Attach_Weapon_Lower_Foregrip_C", {"Vertical Foregrip", EntityType::Item, 0, WeaponType::Grip}},
	{ "Item_Attach_Weapon_Lower_AngledForeGrip_C", {"Angled Foregrip", EntityType::Item, 0, WeaponType::Grip} },
	{ "Item_Attach_Weapon_Lower_HalfGrip_C", {"Half Grip", EntityType::Item, 0, WeaponType::Grip} },
	{ "Item_Attach_Weapon_Lower_LightweightForeGrip_C", {"Lightweight Grip", EntityType::Item, 0, WeaponType::Grip} },
	{ "Item_Attach_Weapon_Lower_ThumbGrip_C", {"Thumb Grip", EntityType::Item, 0, WeaponType::Grip} },
	{ "Item_Attach_Weapon_Lower_QuickDraw_Large_Crossbow_C", {"Quiver", EntityType::Item, 0, WeaponType::Grip} },
	{ "Item_Attach_Weapon_Lower_LaserPointer_C", {"Laser Sight", EntityType::Item, 0, WeaponType::Grip} },

	//??
	{"Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Large_C", {"Extended QuickDraw Mag (AR)", EntityType::Item, 0, WeaponType::Magazine}},
	{"Item_Attach_Weapon_Magazine_Extended_Large_C", {"Extended Mag (AR)", EntityType::Item, 0, WeaponType::Magazine}},
	{"Item_Attach_Weapon_Magazine_QuickDraw_Large_C", {"QuickDraw Mag (AR)", EntityType::Item, 0, WeaponType::Magazine} },

	{ "Item_Attach_Weapon_Magazine_ExtendedQuickDraw_SniperRifle_C", {"Extended QuickDraw Mag (SR)", EntityType::Item, 0, WeaponType::Magazine} },
	{ "Item_Attach_Weapon_Magazine_Extended_SniperRifle_C", {"Extended Mag (SR)", EntityType::Item, 0, WeaponType::Magazine} },
	{ "Item_Attach_Weapon_Magazine_QuickDraw_SniperRifle_C", {"QuickDraw Mag (SR)", EntityType::Item, 0, WeaponType::Magazine} },

	{ "Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Medium_C", {"Extended QuickDraw Mag (SMG)", EntityType::Item, 0, WeaponType::Magazine} },
	{ "Item_Attach_Weapon_Magazine_Extended_Medium_C", {"Extended Mag (SMG)", EntityType::Item, 0, WeaponType::Magazine} },
	{ "Item_Attach_Weapon_Magazine_QuickDraw_Medium_C", {"QuickDraw Mag (SMG)", EntityType::Item, 0, WeaponType::Magazine} },


	//??
	{ "Item_Ammo_Mortar_C", {"60mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_Bolt_C", {"Bolt", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_Flare_C", {"Flare", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_57mm_C", {"5.7mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_300Magnum_C", {".300 Magnum", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_556mm_C", {"5.56mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_762mm_C", {"7.62mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_40mm_C", {"40mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_9mm_C", {"9mm", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_12Guage_C", {"12 Gauge", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_12GuageSlug_C", {"12 Gauge Slug", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_45ACP_C", {".45 ACP", EntityType::Item, 0, WeaponType::Bullet} },
	{ "Item_Ammo_ZiplinegunHook_C", {"Zip Line Hook", EntityType::Item, 0, WeaponType::Bullet} },

	//???
	{ "Item_Weapon_C4_C", {"C4", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_BluezoneGrenade_C", {"Bluezone Grenade", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_Grenade_C", {"Frag Grenade", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_FlashBang_C", {"Flashbang", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_StickyGrenade_C", {"Sticky Bomb", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_Molotov_C", {"Molotov", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_SmokeBomb_C", {"Smoke Grenade", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_SpikeTrap_C", {"Spike Trap", EntityType::Item, 0, WeaponType::Grenade} },
	{ "Item_Weapon_DecoyGrenade_C", {"Decoy Grenade", EntityType::Item, 0, WeaponType::Grenade} },

	//??
	{ "Item_Weapon_Pan_C", { "Pan", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Cowbar_C", { "Crowbar", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Sickle_C", { "Sickle", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Machete_C", { "Machete", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Pickaxe_C", { "Pickaxe", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Mortar_C", {"Mortar", EntityType::Item, 0, WeaponType::Mortar} },
	{ "Item_Weapon_Ziplinegun_C", {"Zip Line Gun", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Heal_BattleReadyKit_C", {"Battle Ready Kit", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Weapon_M79_C", {"M79", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Weapon_PanzerFaust100M_C", {"Panzerfaust", EntityType::Item, 0, WeaponType::PanzerFaust100M1} },

	{ "Item_JerryCan_C", {"Gas Can", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_EmergencyPickup_C", {"Emergency Pickup", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Bluechip_C", {"Blue Chip", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Revival_Transmitter_C", {"Blue Chip Transmitter", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_BulletproofShield_C", {"Folded Shield", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Tiger_SelfRevive_C", {"Self-AED", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Mountainbike_C", {"Mountain Bike", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Rubberboat_C", { "Rubber Boat", EntityType::Item, 0, WeaponType::Other } },
	{ "Item_Weapon_Drone_C", {"Drone", EntityType::Item, 0, WeaponType::Other} },

	{ "Vehicle_Repair_Kit_C", {"Vehicle Repair Kit", EntityType::Item, 0, WeaponType::Other} },
	{ "Helmet_Repair_Kit_C", {"Helmet Repair Kit", EntityType::Item, 0, WeaponType::Other} },
	{ "Armor_Repair_Kit_C", {"Armor Repair Kit", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Weapon_IntegratedRepair_C", {"All-in-one Repair Kit", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Weapon_Spotter_Scope_C", {"Spotter Scope", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Weapon_TacPack_C", {"Tactical Backpack", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Neon_Gold_C", {"Gold Bar", EntityType::Item, 0, WeaponType::Other} },
	{ "Item_Neon_Coin_C", {"Coins", EntityType::Item, 0, WeaponType::Other} },

	//??
	{ "Item_Heaven_Key_C", {"Haven Key", EntityType::Item, 0, WeaponType::key} },
	{ "Item_Chimera_Key_C", {"Paramo Key", EntityType::Item, 0, WeaponType::key} },
	{ "Item_Tiger_Key_C", {"Taego Key", EntityType::Item, 0, WeaponType::key} },
	{ "Item_BTSecretRoom_Key_C", {"Erangel Key", EntityType::Item, 0, WeaponType::key} },
	{ "Item_DihorOtok_Key_C", {"Vikendi Keycard", EntityType::Item, 0, WeaponType::key} },
	{ "Item_Secuity_Keycard_C", {"Deston Keycard", EntityType::Item, 0, WeaponType::key} },
};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityWeaponLists = {
	//??
	{"WeapFlashBang_C", {"Flashbang", EntityType::Weapon, 0}},
	{"WeapBluezoneGrenade_C", {"Bluezone Grenade", EntityType::Weapon, 0}},
	{"WeapGrenade_C", {"Frag Grenade", EntityType::Weapon, 0,WeaponType::Grenade}},
	{"WeapMortar_C", {"Mortar", EntityType::Weapon, 0,WeaponType::Other}},
	{"WeapStickyGrenade_C", {"Sticky Bomb", EntityType::Weapon, 0}},
	{"WeapC4_C", {"C4", EntityType::Weapon, 0}},
	{"WeapMolotov_C", {"Molotov", EntityType::Weapon, 0}},
	{"WeapSmokeBomb_C", {"Smoke Grenade", EntityType::Weapon, 0}},
	{"WeapDecoyGrenade_C", {"Decoy Grenade", EntityType::Weapon, 0}},
	{"WeapBluezoneGrenade_C", {"Bluezone Grenade", EntityType::Weapon, 0}},
	{"WeapSpikeTrap_C", {"Spike Trap", EntityType::Weapon, 0}},

	// Melee
	{"WeapCowbar_C", {"Crowbar", EntityType::Weapon, 0}},
	{"WeapPan_C", {"Pan", EntityType::Weapon, 0}},
	{"WeapSickle_C", {"Sickle", EntityType::Weapon, 0}},
	{"WeapMachete_C", {"Machete", EntityType::Weapon, 0}},
	{"WeapCowbarProjectile_C", {"Crowbar", EntityType::Weapon, 0}},
	{"WeapMacheteProjectile_C", {"Machete", EntityType::Weapon, 0}},
	{"WeapPanProjectile_C", {"Pan", EntityType::Weapon, 0}},
	{"WeapSickleProjectile_C", {"Sickle", EntityType::Weapon, 0}},

	// AR
	{"WeapLunchmeatsAK47_C", {"AKM", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapAK47_C", {"AKM", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapGroza_C", {"Groza", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapDuncansHK416_C", {"M416", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapHK416_C", {"M416", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapM16A4_C", {"M16A4", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapSCAR-L_C", {"SCAR-L", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapACE32_C", {"ACE", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapAUG_C", {"AUG", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapBerylM762_C", {"Beryl M762", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapG36C_C", {"G36C", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapQBZ95_C", {"QBZ95", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapK2_C", {"K2", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapMk47Mutant_C", {"Mk47", EntityType::Weapon, 0, WeaponType::AR}},
	{"WeapFamasG2_C", {"FAMAS", EntityType::Weapon, 0, WeaponType::AR}},
	// SR
	{"WeapAWM_C", {"AWM", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapJuliesM24_C", {"M24", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapM24_C", {"M24", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapJuliesKar98k_C", {"Kar98k", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapKar98k_C", {"Kar98k", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapWin94_C", {"Win94", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapL6_C", {"Lynx AMR", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapMosinNagant_C", {"Mosin Nagant", EntityType::Weapon, 0, WeaponType::SR}},
	{"WeapCrossbow_1_C", {"Crossbow", EntityType::Weapon, 0, WeaponType::SR}},

	// SG
	{"WeapOriginS12_C", {"O12", EntityType::Weapon, 0, WeaponType::SG}},
	{"WeapBerreta686_C", {"S686", EntityType::Weapon, 0, WeaponType::SG}},
	{"WeapSaiga12_C", {"S12K", EntityType::Weapon, 0, WeaponType::SG}},
	{"WeapWinchester_C", {"S1897", EntityType::Weapon, 0, WeaponType::SG}},
	{"WeapDP12_C", {"DBS", EntityType::Weapon, 0, WeaponType::SG}},


	// PISTOL
	{"WeapG18_C", {"P18C", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapM1911_C", {"P1911", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapM9_C", {"P92", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapNagantM1895_C", {"R1895", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapRhino_C", {"R45", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapDesertEagle_C", {"Desert Eagle", EntityType::Weapon, 0, WeaponType::HG}},
	{"Weapvz61Skorpion_C", {"Skorpion", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapStunGun_C", {"Stun Gun", EntityType::Weapon, 0, WeaponType::HG}},
	{"WeapSawnoff_C", {"Sawed-off", EntityType::Weapon, 0, WeaponType::HG}},

	// LMG
	{"WeapM249_C", {"M249", EntityType::Weapon, 0, WeaponType::LMG}},
	{"WeapMG3_C", {"MG3", EntityType::Weapon, 0, WeaponType::LMG}},
	{"WeapDP28_C", {"DP28", EntityType::Weapon, 0, WeaponType::LMG}},

	// DMR
	{"WeapDragunov_C", {"Dragunov", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapMini14_C", {"Mini14", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapMk14_C", {"Mk14", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapSKS_C", {"SKS", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapFNFal_C", {"SLR", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapMadsFNFal_C", {"SLR", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapMadsQBU88_C", {"QBU", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapQBU88_C", {"QBU", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapMk12_C", {"Mk12", EntityType::Weapon, 0, WeaponType::DMR}},
	{"WeapVSS_C", {"VSS", EntityType::Weapon, 0, WeaponType::DMR}},

	// SMG
	{"WeapThompson_C", {"Thompson", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapUMP_C", {"UMP", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapUZI_C", {"UZI", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapUziPro_C", {"UZI", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapVector_C", {"Vector", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapBizonPP19_C", {"PP-19 Bizon", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapMP5K_C", {"MP5K", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapP90_C", {"P90", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapJS9_C", {"JS9", EntityType::Weapon, 0, WeaponType::SMG}},
	{"WeapMP9_C", {"MP9", EntityType::Weapon, 0, WeaponType::SMG}},

	// Special
	{"WeapMortar_C", {"Mortar", EntityType::Weapon, 0, WeaponType::Mortar}},
	{"WeapFlareGun_C", {"Flare Gun", EntityType::Weapon, 0, WeaponType::HG}},
	{ "WeapPanzerFaust100M1_C", {"Panzerfaust", EntityType::Weapon, 0, WeaponType::PanzerFaust100M1} },
	{"WeapJerryCan_C", {"Gas Can", EntityType::Weapon, 0}},
	{"WeapDrone_C", {"Drone", EntityType::Weapon, 0}},
	{"WeapTraumaBag_C", {"Trauma Bag", EntityType::Weapon, 0}},
	{"WeapSpotterScope_C", {"Spotter Scope", EntityType::Weapon, 0}},
	{"WeapTacPack_C", {"Tactical Pack", EntityType::Weapon, 0}},
	{"WeapM79_C", {"M79", EntityType::Weapon, 0}},
};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityVehicleLists = {
	//??
	{"BP_EmPickup_Aircraft_C", {"vehicle.helicopter", EntityType::Vehicle, 0}},
	{"BP_EmergencyPickupVehicle_C", {"vehicle.emergency_pickup", EntityType::Vehicle, 0}},
	{"TransportAircraft_Chimera_C", {"vehicle.airplane", EntityType::Vehicle, 0}},
	{"BP_Bicycle_C", {"vehicle.bicycle", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Bicycle_C.png"}},
	{"BP_BRDM_C", {"vehicle.brdm", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_BRDM_C.png"}},//??
	{"Uaz_Armored_C", {"vehicle.uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_A_00_C.png"}},
	{"Uaz_Pillar_C", {"vehicle.armored_uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_C_00_C.png"}},
	{"BP_Motorglider_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},
	{"BP_Motorglider_Blue_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},//??
	{"BP_Motorglider_Green_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},
	{"BP_Motorglider_Orange_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},
	{"BP_Motorglider_Red_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},
	{"BP_Motorglider_Teal_C", {"vehicle.glider", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorglider_C.png"}},
	{"BP_LootTruck_C", {"vehicle.loot_truck", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_LootTruck_C.png"}},
	{"AquaRail_A_01_C", {"vehicle.aquarail", EntityType::Vehicle, 0, WeaponType::Other, "", "AquaRail_A_00_C.png"}},
	{"AquaRail_A_02_C", {"vehicle.aquarail", EntityType::Vehicle, 0, WeaponType::Other, "", "AquaRail_A_00_C.png"}},
	{"AquaRail_A_03_C", {"vehicle.aquarail", EntityType::Vehicle, 0, WeaponType::Other, "", "AquaRail_A_00_C.png"}},
	{"Boat_PG117_C", {"vehicle.boat", EntityType::Vehicle, 0, WeaponType::Other, "", "Boat_PG117_C.png"}},
	{"PG117_A_01_C", {"vehicle.boat", EntityType::Vehicle, 0, WeaponType::Other, "", "Boat_PG117_C.png"}},
	{"AirBoat_V2_C", {"vehicle.airboat", EntityType::Vehicle, 0, WeaponType::Other, "", "Boat_PG117_C.png"}},
	{"BP_M_Rony_A_01_C", {"vehicle.rony", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_M_Rony_A_00_C.png"}},
	{"BP_M_Rony_A_02_C", {"vehicle.rony", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_M_Rony_A_00_C.png"}},
	{"BP_M_Rony_A_03_C", {"vehicle.rony", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_M_Rony_A_00_C.png"}},
	{"BP_Mirado_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_A_01_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_A_02_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_A_03_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_A_03_Esports_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_A_04_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_A_00_C.png"}},
	{"BP_Mirado_Open_05_C", {"vehicle.mirado", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Mirado_Open_C", {"vehicle.mirado_open", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Mirado_Open_01_C", {"vehicle.mirado_open", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Mirado_Open_02_C", {"vehicle.mirado_open", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Mirado_Open_03_C", {"vehicle.mirado_open", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Mirado_Open_04_C", {"vehicle.mirado_open", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Mirado_Open_00_C.png"}},
	{"BP_Motorbike_04_C", {"vehicle.motorbike", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorbike_04_C.png"}},
	{"BP_Motorbike_04_Desert_C", {"vehicle.motorbike", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorbike_04_Desert_C.png"}},
	{"BP_Motorbike_Solitario_C", {"vehicle.motorbike", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorbike_Solitario_C.png"}},
	{"BP_Motorbike_04_SideCar_C", {"vehicle.motorbike", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorbike_04_SideCar_C.png"}},
	{"BP_Motorbike_04_SideCar_Desert_C", {"vehicle.motorbike", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Motorbike_04_SideCar_Desert_C.png"}},
	{"BP_Niva_01_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_02_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_03_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_04_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_05_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_06_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_07_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_Niva_Esports_C", {"vehicle.zima", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Niva_00_C.png"}},
	{"BP_PickupTruck_A_01_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_A_02_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_A_03_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_A_04_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_A_05_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_A_esports_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_A_00_C.png"}},
	{"BP_PickupTruck_B_01_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_B_00_C.png"}},
	{"BP_PickupTruck_B_02_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_B_00_C.png"}},
	{"BP_PickupTruck_B_03_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_B_00_C.png"}},
	{"BP_PickupTruck_B_04_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_B_00_C.png"}},
	{"BP_PickupTruck_B_05_C", {"vehicle.pickup", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PickupTruck_B_00_C.png"}},
	{"BP_TukTukTuk_A_01_C", {"vehicle.tukshai", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_TukTukTuk_A_00_C.png"}},
	{"BP_TukTukTuk_A_02_C", {"vehicle.tukshai", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_TukTukTuk_A_00_C.png"}},
	{"BP_TukTukTuk_A_03_C", {"vehicle.tukshai", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_TukTukTuk_A_00_C.png"}},
	{"BP_Van_A_01_C", {"vehicle.bus", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Van_A_00_C.png"}},
	{"BP_Van_A_02_C", {"vehicle.bus", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Van_A_00_C.png"}},
	{"BP_Van_A_03_C", {"vehicle.bus", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Van_A_00_C.png"}},
	{"BP_MiniBus_C", {"vehicle.bus", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_MiniBus_C.png"}},
	{"BP_Scooter_01_A_C", {"vehicle.scooter", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Scooter_00_A_C.png"}},
	{"BP_Scooter_02_A_C", {"vehicle.scooter", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Scooter_00_A_C.png"}},
	{"BP_Scooter_03_A_C", {"vehicle.scooter", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Scooter_00_A_C.png"}},
	{"BP_Scooter_04_A_C", {"vehicle.scooter", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Scooter_00_A_C.png"}},
	{"BP_Snowbike_01_C", {"vehicle.snowmobile", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Snowbike_01_C.png"}},
	{"BP_Snowbike_02_C", {"vehicle.snowmobile", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Snowbike_02_C.png"}},
	{"BP_Snowmobile_01_C", {"vehicle.snowmobile", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Snowmobile_01_C.png"}},
	{"BP_Snowmobile_02_C", {"vehicle.snowmobile", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Snowmobile_02_C.png"}},
	{"BP_Snowmobile_03_C", {"vehicle.snowmobile", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Snowmobile_03_C.png"}},
	{"Buggy_A_01_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_01_C.png"}},
	{"Buggy_A_02_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_02_C.png"}},
	{"Buggy_A_03_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_03_C.png"}},
	{"Buggy_A_04_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_04_C.png"}},
	{"Buggy_A_05_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_05_C.png"}},
	{"Buggy_A_06_C", {"vehicle.buggy", EntityType::Vehicle, 0, WeaponType::Other, "", "Buggy_A_06_C.png"}},
	{"Dacia_A_01_v2_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Dacia_A_01_v2_snow_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Dacia_A_02_v2_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Dacia_A_03_v2_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Dacia_A_03_v2_Esports_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Dacia_A_04_v2_C", {"vehicle.dacia", EntityType::Vehicle, 0, WeaponType::Other, "", "Dacia_A_00_v2_C.png"}},
	{"Uaz_A_01_C", {"vehicle.uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_A_00_C.png"}},
	{"Uaz_B_01_C", {"vehicle.uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_B_00_C.png"}},
	{"Uaz_B_01_esports_C", {"vehicle.uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_B_00_C.png"}},
	{"Uaz_C_01_C", {"vehicle.uaz", EntityType::Vehicle, 0, WeaponType::Other, "", "Uaz_C_00_C.png"}},
		{"BP_Dirtbike_C", {"vehicle.dirtbike", EntityType::Vehicle, 0}},
		{"BP_CoupeRB_C", {"vehicle.coupe_rb", EntityType::Vehicle, 0}},
		{"BP_ATV_C", {"vehicle.atv", EntityType::Vehicle, 0}},
		{"BP_PonyCoupe_C", {"vehicle.mirado", EntityType::Vehicle, 0}},
		{"BP_Porter_C", {"vehicle.porter", EntityType::Vehicle, 0}},
		{"BP_Pillar_Car_C", {"vehicle.pillar_car", EntityType::Vehicle, 0}},
		{"BP_Food_Truck_C", {"vehicle.food_truck", EntityType::Vehicle, 0}},
		{"BP_Blanc_C", {"vehicle.blanc", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Blanc_C.png"}},
		{"BP_Blanc_Esports_C", {"vehicle.blanc", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_Blanc_Esports_C.png"}},
		{"BP_McLarenGT_C", {"vehicle.mclaren", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_McLarenGT_C.png"}},
		{"ABP_McLarenGT_C", {"vehicle.mclaren", EntityType::Vehicle, 0, WeaponType::Other, "", "ABP_McLarenGT_C.png"} },
		{"BP_McLarenGT_Lx_Yellow_C", {"vehicle.mclaren", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_McLarenGT_Lx_Yellow_C.png"} },
		{"BP_McLarenGT_St_black_C", {"vehicle.mclaren", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_McLarenGT_St_black_C.png"} },
		{"BP_McLarenGT_St_white_C", {"vehicle.mclaren", EntityType::Vehicle, 0} },
		{"BP_DBX_LGD_C", {"vehicle.aston_martin_suv", EntityType::Vehicle, 0} },
		{"BP_Vantage_EP_C", {"vehicle.aston_martin_coupe", EntityType::Vehicle, 0} },
		{"BP_Vantage_LGD_C", {"vehicle.aston_martin_luxe", EntityType::Vehicle, 0} },
		{"BP_PicoBus_C", {"vehicle.electric_bus", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PicoBus_C.png"} },
		{"BP_PanigaleV4S_LGD04_C", {"vehicle.ducati_gold", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PanigaleV4S_LGD04_C.png"} },
		{"BP_PanigaleV4S_LGD02_C", {"vehicle.ducati_rose", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PanigaleV4S_LGD02_C.png"} },
		{"BP_PanigaleV4S_LGD01_C", {"vehicle.ducati_green", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PanigaleV4S_LGD01_C.png"} },
		{"BP_PanigaleV4S_LGD03_C", {"vehicle.ducati_pink", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PanigaleV4S_LGD03_C.png"} },
		{"BP_PanigaleV4S_EP01_C", {"vehicle.ducati_red", EntityType::Vehicle, 0, WeaponType::Other, "", "BP_PanigaleV4S_EP01_C.png"} },
		{"BP_PanigaleV4S_EP02_C", {"vehicle.ducati_black", EntityType::Vehicle, 0} },
		{"BP_Urus_EP_C", {"vehicle.lambo_suv", EntityType::Vehicle, 0} },
		{"BP_Urus_LGD_C", {"vehicle.lambo_suv_luxe", EntityType::Vehicle, 0} },
		{"BP_Countach_ULT_C", {"vehicle.lambo_coupe", EntityType::Vehicle, 0} },
		{"BP_Classic_02_C", {"vehicle.classic_car", EntityType::Vehicle, 0} },
		{"BP_Rubber_boat_C", {"vehicle.rubber_boat", EntityType::Vehicle, 0}},
	{"BP_BearV2_C", {"vehicle.bear", EntityType::Vehicle, 0} },
	{"StrongBoxBP_C", {"vehicle.safe", EntityType::Vehicle, 0} },

};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityProjectLists = {
	//???
	{"ProjGrenade_C", {"aim.grenade", EntityType::Project, 0}},
	{"ProjBluezoneGrenade_C", {"smart.bluezone", EntityType::Project, 0}},
	{"ProjBZGrenade_C", {"smart.bluezone", EntityType::Project, 0}},
	{"ProjFlashBang_C", {"smart.flash", EntityType::Project, 0}},
	{"ProjMolotov_C", {"smart.molotov", EntityType::Project, 0}},
};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityOtherLists = {
	{"ScopeAimCamera", {"ScopeAimCamera", EntityType::Other, 0} },
	{"MouseX", {"MouseX", EntityType::Other, 0} },
	{"MouseY", {"MouseY", EntityType::Other, 0} },
};

std::unordered_map<std::string, EntityInfo, FnvHash> EntityLists = {


};


EntityInfo findEntityInfoByID(int id)
{
	return EntityInfo();
}

bool hasEntityWithIDZero()
{
	return false;
}

void EntityInit()
{
	EntityLists.clear();
	EntityLists.insert(EntityOtherLists.begin(), EntityOtherLists.end());
	EntityLists.insert(EntityPlayerLists.begin(), EntityPlayerLists.end());
	EntityLists.insert(EntityWeaponLists.begin(), EntityWeaponLists.end());
	EntityLists.insert(EntityItemLists.begin(), EntityItemLists.end());
	EntityLists.insert(EntityVehicleLists.begin(), EntityVehicleLists.end());
	EntityLists.insert(EntityProjectLists.begin(), EntityProjectLists.end());
}
