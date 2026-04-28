#include "../core/overlay_menu.hpp"
#include "../entities/entity_aliases.hpp"
#include "loot_cluster_renderer.hpp"
#include "loot_debug_renderer.hpp"
#include "loot_source_merge.hpp"
#include "../core/overlay_texture_cache.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include "../../sdk/context.hpp"
#include "../../../protec/skCrypt.h"

namespace {

bool ShouldDrawVehicle(const OverlayMenu& menu, const std::string& name) {
    const VehicleResolver::VehicleInfo info = VehicleResolver::Resolve(name);
    switch (info.CategoryId) {
    case VehicleResolver::Category::Uaz: return menu.loot_vehicle_uaz;
    case VehicleResolver::Category::Dacia: return menu.loot_vehicle_dacia;
    case VehicleResolver::Category::Buggy: return menu.loot_vehicle_buggy;
    case VehicleResolver::Category::Bike: return menu.loot_vehicle_bike;
    case VehicleResolver::Category::Boat: return menu.loot_vehicle_boat;
    case VehicleResolver::Category::Brdm: return menu.loot_vehicle_brdm;
    case VehicleResolver::Category::Scooter: return menu.loot_vehicle_scooter;
    case VehicleResolver::Category::Snow: return menu.loot_vehicle_snow;
    case VehicleResolver::Category::Tuk: return menu.loot_vehicle_tuk;
    case VehicleResolver::Category::Bus: return menu.loot_vehicle_bus;
    case VehicleResolver::Category::Truck: return menu.loot_vehicle_truck;
    case VehicleResolver::Category::Train: return menu.loot_vehicle_train;
    case VehicleResolver::Category::Mirado: return menu.loot_vehicle_mirado;
    case VehicleResolver::Category::Pickup: return menu.loot_vehicle_pickup;
    case VehicleResolver::Category::Rony: return menu.loot_vehicle_rony;
    case VehicleResolver::Category::Blanc: return menu.loot_vehicle_blanc;
    case VehicleResolver::Category::Air: return menu.loot_vehicle_air;
    default: return true;
    }
}

} // namespace

void OverlayMenu::RenderLootEsp(ImDrawList* draw) {
    if (!draw || !esp_toggle) return;

            std::vector<ItemData> allLoot = LootSourceMerge::BuildAllLoot();

            struct VehicleDrawKey {
                std::string IconName;
                Vector2 Screen;
            };
            std::vector<VehicleDrawKey> seenVehicles;
            auto HasDuplicateVehicle = [&](const std::string& iconName, const Vector2& screen) {
                constexpr float kDuplicateRadiusSq = 36.0f * 36.0f;
                for (const auto& seen : seenVehicles) {
                    if (seen.IconName != iconName) continue;
                    const float dx = seen.Screen.x - screen.x;
                    const float dy = seen.Screen.y - screen.y;
                    if ((dx * dx) + (dy * dy) <= kDuplicateRadiusSq) return true;
                }
                return false;
            };
            auto MarkVehicleSeen = [&](const std::string& iconName, const Vector2& screen) {
                seenVehicles.push_back({ iconName, screen });
            };

            std::vector<LootClusterRenderer::Entry> lootEntries;
            for (const auto& item : allLoot) {
                if (item.Distance <= 0) continue;

                bool should_draw = false;
                ImU32 col = IM_COL32(200, 200, 200, 255); // Default Loot

                if (item.RenderType == ItemRenderType::Vehicle) {
                    if (g_Menu.esp_vehicles &&
                        !VehicleResolver::ShouldHideAtDistance(item.Distance) &&
                        item.Distance < static_cast<float>(g_Menu.vehicle_max_dist) &&
                        ShouldDrawVehicle(*this, item.Name)) {
                        should_draw = true; col = IM_COL32(0, 255, 255, 255);
                    }
                } else if (item.RenderType == ItemRenderType::AirDrop) {
                    if (g_Menu.esp_airdrops) { should_draw = true; col = IM_COL32(255, 50, 50, 255); }
                } else if (item.RenderType == ItemRenderType::DeadBox) {
                    if (g_Menu.esp_deadboxes && item.Distance < 200.0f) { should_draw = true; col = IM_COL32(255, 140, 0, 255); }
                } else if (item.RenderType == ItemRenderType::Projectile) {
                    should_draw = true; col = IM_COL32(255, 0, 0, 255); // BRIGHT RED FOR DANGER
                } else { // Generic items
                    if (g_Menu.esp_items && item.Distance > 5.0f && item.Distance < static_cast<float>(g_Menu.loot_max_dist)) {
                        const std::string& id = item.Name; // e.g. Item_Weapon_AK47_C

                        // --- 1. WEAPONS: ASSAULT RIFLES ---
                        if      (id == skCrypt("Item_Weapon_HK416_C")) { if(g_Menu.loot_weapon_hk416) { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_AK47_C"))  { if(g_Menu.loot_weapon_ak47)  { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_BerylM762_C")) { if(g_Menu.loot_weapon_beryl) { should_draw = true; col = IM_COL32(255, 120, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_SCAR-L_C")) { if(g_Menu.loot_weapon_scar)  { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_AUG_C"))    { if(g_Menu.loot_weapon_aug)   { should_draw = true; col = IM_COL32(255, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_Groza_C"))  { if(g_Menu.loot_weapon_groza) { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_ACE32_C"))  { if(g_Menu.loot_weapon_ace32) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_FAMASG2_C")) { if(g_Menu.loot_weapon_famas) { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_G36C_C"))   { if(g_Menu.loot_weapon_g36c)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_QBZ95_C"))  { if(g_Menu.loot_weapon_qbz)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_K2_C"))     { if(g_Menu.loot_weapon_k2)    { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Mk47Mutant_C")) { if(g_Menu.loot_weapon_mutant) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M16A4_C"))  { if(g_Menu.loot_weapon_m16)   { should_draw = true; } }

                        // --- 2. WEAPONS: SNIPERS & DMRS ---
                        else if (id == skCrypt("Item_Weapon_AWM_C"))    { if(g_Menu.loot_weapon_awm)   { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_M24_C"))    { if(g_Menu.loot_weapon_m24)   { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Kar98k_C")) { if(g_Menu.loot_weapon_kar98) { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mosin_C"))  { if(g_Menu.loot_weapon_mosin) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Win1894_C")) { if(g_Menu.loot_weapon_win94) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Mk14_C"))   { if(g_Menu.loot_weapon_mk14)  { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_FNFal_C"))  { if(g_Menu.loot_weapon_slr)   { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_SKS_C"))    { if(g_Menu.loot_weapon_sks)   { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mk12_C"))   { if(g_Menu.loot_weapon_mk12)  { should_draw = true; col = IM_COL32(0, 255, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_Dragunov_C")) { if(g_Menu.loot_weapon_dragunov) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_Mini14_C")) { if(g_Menu.loot_weapon_mini14) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_QBU88_C"))  { if(g_Menu.loot_weapon_qbu)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_VSS_C"))    { if(g_Menu.loot_weapon_vss)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_L6_C"))     { if(g_Menu.loot_weapon_all)   { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }

                        // --- 3. WEAPONS: SMG & LMG ---
                        else if (id == skCrypt("Item_Weapon_P90_C"))    { if(g_Menu.loot_weapon_p90)   { should_draw = true; col = IM_COL32(255, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_UMP_C"))    { if(g_Menu.loot_weapon_ump)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Vector_C")) { if(g_Menu.loot_weapon_vector){ should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_UZI_C"))    { if(g_Menu.loot_weapon_uzi)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MP5K_C"))   { if(g_Menu.loot_weapon_mp5)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MP9_C"))    { if(g_Menu.loot_weapon_mp9)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_JS9_C"))    { if(g_Menu.loot_weapon_js9)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_BizonPP19_C")) { if(g_Menu.loot_weapon_bizon) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Thompson_C")) { if(g_Menu.loot_weapon_thompson) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_MG3_C"))    { if(g_Menu.loot_weapon_mg3)   { should_draw = true; col = IM_COL32(255, 100, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_M249_C"))   { if(g_Menu.loot_weapon_m249)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_DP28_C"))   { if(g_Menu.loot_weapon_dp28)  { should_draw = true; } }

                        // --- 4. WEAPONS: SHOTGUNS & HANDGUNS ---
                        else if (id == skCrypt("Item_Weapon_DP12_C"))   { if(g_Menu.loot_weapon_dp12)  { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_Saiga12_C")) { if(g_Menu.loot_weapon_s12k)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_OriginS12_C")) { if(g_Menu.loot_weapon_saiga) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Berreta686_C")) { if(g_Menu.loot_weapon_db)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_DesertEagle_C")) { if(g_Menu.loot_weapon_deagle) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_vz61Skorpion_C")) { if(g_Menu.loot_weapon_skorpion) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M1911_C"))  { if(g_Menu.loot_weapon_m1911) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_M9_C"))     { if(g_Menu.loot_weapon_p92)   { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Rhino_C"))  { if(g_Menu.loot_weapon_rhino) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_NagantM1895_C")) { if(g_Menu.loot_weapon_nagant) { should_draw = true; } }

                        // --- 5. AMMO: INDIVIDUAL FILTERS ---
                        else if (id == skCrypt("Item_Ammo_556mm_C"))    { if(g_Menu.loot_ammo_556) { should_draw = true; col = IM_COL32(100, 255, 100, 220); } }
                        else if (id == skCrypt("Item_Ammo_762mm_C"))    { if(g_Menu.loot_ammo_762) { should_draw = true; col = IM_COL32(255, 120, 100, 220); } }
                        else if (id == skCrypt("Item_Ammo_300Magnum_C")) { if(g_Menu.loot_ammo_300) { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Ammo_9mm_C"))      { if(g_Menu.loot_ammo_9mm) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_45ACP_C"))    { if(g_Menu.loot_ammo_45)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_12Guage_C"))  { if(g_Menu.loot_ammo_12g) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_12GuageSlug_C")) { if(g_Menu.loot_ammo_slug) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_57mm_C"))     { if(g_Menu.loot_ammo_57)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_40mm_C"))     { if(g_Menu.loot_ammo_40)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Bolt_C"))     { if(g_Menu.loot_ammo_bolt) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Flare_C"))    { if(g_Menu.loot_ammo_flare) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ammo_Mortar_C"))   { if(g_Menu.loot_ammo_mortar) { should_draw = true; } }

                        // --- 6. ATTACHMENTS: SCOPES ---
                        else if (id.find(skCrypt("DotSight")) != std::string::npos) { if(g_Menu.loot_scope_reddot) { should_draw = true; } }
                        else if (id.find(skCrypt("Holosight")) != std::string::npos) { if(g_Menu.loot_scope_holo) { should_draw = true; } }
                        else if (id.find(skCrypt("Aimpoint")) != std::string::npos) { if(g_Menu.loot_scope_2x) { should_draw = true; } }
                        else if (id.find(skCrypt("Scope3x")) != std::string::npos) { if(g_Menu.loot_scope_3x) { should_draw = true; } }
                        else if (id.find(skCrypt("ACOG")) != std::string::npos) { if(g_Menu.loot_scope_4x) { should_draw = true; } }
                        else if (id.find(skCrypt("Scope6x")) != std::string::npos) { if(g_Menu.loot_scope_6x) { should_draw = true; } }
                        else if (id.find(skCrypt("CQBSS")) != std::string::npos) { if(g_Menu.loot_scope_8x) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("PM2")) != std::string::npos) { if(g_Menu.loot_scope_15x) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("Thermal")) != std::string::npos) { if(g_Menu.loot_scope_thermal) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }

                        // --- 7. ATTACHMENTS: MUZZLES ---
                        else if (id.find(skCrypt("Compensator")) != std::string::npos) { if(g_Menu.loot_muzzle_comp) { should_draw = true; } }
                        else if (id.find(skCrypt("FlashHider")) != std::string::npos) { if(g_Menu.loot_muzzle_flash) { should_draw = true; } }
                        else if (id.find(skCrypt("Suppressor")) != std::string::npos) { if(g_Menu.loot_muzzle_supp) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id.find(skCrypt("Choke")) != std::string::npos) { if(g_Menu.loot_muzzle_choke) { should_draw = true; } }

                        // --- 8. ATTACHMENTS: GRIPS & STOCKS ---
                        else if (id.find(skCrypt("Foregrip")) != std::string::npos) { if(g_Menu.loot_grip_vertical) { should_draw = true; } }
                        else if (id.find(skCrypt("AngledForeGrip")) != std::string::npos) { if(g_Menu.loot_grip_angled) { should_draw = true; } }
                        else if (id.find(skCrypt("HalfGrip")) != std::string::npos) { if(g_Menu.loot_grip_half) { should_draw = true; } }
                        else if (id.find(skCrypt("ThumbGrip")) != std::string::npos) { if(g_Menu.loot_grip_thumb) { should_draw = true; } }
                        else if (id.find(skCrypt("Lightweight")) != std::string::npos) { if(g_Menu.loot_grip_light) { should_draw = true; } }
                        else if (id.find(skCrypt("Stock_AR_Heavy")) != std::string::npos) { if(g_Menu.loot_stock_heavy) { should_draw = true; } }
                        else if (id.find(skCrypt("CheekPad")) != std::string::npos) { if(g_Menu.loot_stock_cheek) { should_draw = true; } }

                        // --- 9. ATTACHMENTS: MAGAZINES ---
                        else if (id.find(skCrypt("ExtendedQuickDraw")) != std::string::npos) { if(g_Menu.loot_mag_ext_quick) { should_draw = true; } }
                        else if (id.find(skCrypt("Extended_")) != std::string::npos) { if(g_Menu.loot_mag_ext) { should_draw = true; } }
                        else if (id.find(skCrypt("QuickDraw_")) != std::string::npos) { if(g_Menu.loot_mag_quick) { should_draw = true; } }

                        // --- 10. THROWABLES & TACTICAL ---
                        else if (id == skCrypt("Item_Weapon_Grenade_C")) { if(g_Menu.loot_throw_frag) { should_draw = true; col = IM_COL32(255, 50, 50, 255); } }
                        else if (id == skCrypt("Item_Weapon_SmokeBomb_C")) { if(g_Menu.loot_throw_smoke) { should_draw = true; col = IM_COL32(200, 200, 200, 255); } }
                        else if (id == skCrypt("Item_Weapon_FlashBang_C")) { if(g_Menu.loot_throw_flash) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_Molotov_C")) { if(g_Menu.loot_throw_molotov) { should_draw = true; col = IM_COL32(255, 150, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_C4_C"))      { if(g_Menu.loot_throw_c4)    { should_draw = true; col = IM_COL32(255, 0, 0, 255); } }
                        else if (id == skCrypt("Item_Weapon_StickyGrenade_C")) { if(g_Menu.loot_throw_sticky) { should_draw = true; } }
                        else if (id == skCrypt("Item_Weapon_BluezoneGrenade_C")) { if(g_Menu.loot_throw_bz) { should_draw = true; col = IM_COL32(0, 100, 255, 255); } }
                        else if (id == skCrypt("Item_Weapon_DecoyGrenade_C")) { if(g_Menu.loot_throw_decoy) { should_draw = true; } }

                        // --- 11. GEAR: ARMOR & HELMETS & PACKS ---
                        else if (id.find(skCrypt("Lv3")) != std::string::npos) { if(g_Menu.loot_armor_lv3 || g_Menu.loot_helmet_lv3 || g_Menu.loot_backpack_lv3) { should_draw = true; col = IM_COL32(255, 0, 255, 255); } }
                        else if (id.find(skCrypt("Lv2")) != std::string::npos) { if(g_Menu.loot_armor_lv2 || g_Menu.loot_helmet_lv2 || g_Menu.loot_backpack_lv2) { should_draw = true; col = IM_COL32(0, 200, 255, 255); } }
                        else if (id == skCrypt("Item_Back_BlueBlocker")) { if(g_Menu.loot_utility_jammer) { should_draw = true; col = IM_COL32(0, 255, 255, 255); } }

                        // --- 12. RECOVERY: MEDS & BOOSTS ---
                        else if (id == skCrypt("Item_Heal_MedKit_C") || id == skCrypt("Item_Heal_FirstAid_C")) { if(g_Menu.loot_meds_healing) { should_draw = true; col = IM_COL32(100, 255, 100, 255); } }
                        else if (id.find(skCrypt("Item_Boost")) != std::string::npos) { if(g_Menu.loot_meds_boosts) { should_draw = true; col = IM_COL32(255, 255, 0, 255); } }

                        // --- 13. SPECIAL: GHILLIE & KEYS & REPAIR ---
                        else if (id == skCrypt("Item_Ghillie_01_C")) { if(g_Menu.loot_ghillie_arctic) { should_draw = true; col = IM_COL32(255, 255, 255, 255); } }
                        else if (id == skCrypt("Item_Ghillie_02_C")) { if(g_Menu.loot_ghillie_desert) { should_draw = true; col = IM_COL32(255, 200, 100, 255); } }
                        else if (id == skCrypt("Item_Ghillie_03_C")) { if(g_Menu.loot_ghillie_jungle) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }
                        else if (id == skCrypt("Item_Ghillie_04_C")) { if(g_Menu.loot_ghillie_forest) { should_draw = true; } }
                        else if (id == skCrypt("Item_Ghillie_05_C")) { if(g_Menu.loot_ghillie_mossy)  { should_draw = true; } }
                        else if (id == skCrypt("Item_Ghillie_06_C")) { if(g_Menu.loot_ghillie_brown)  { should_draw = true; } }
                        else if (id.find(skCrypt("Key")) != std::string::npos || id.find(skCrypt("Keycard")) != std::string::npos) { if(g_Menu.loot_key_security || g_Menu.loot_key_secret) { should_draw = true; col = IM_COL32(255, 215, 0, 255); } }
                        else if (id.find(skCrypt("Repair_Kit")) != std::string::npos) { if(g_Menu.loot_repair_armor || g_Menu.loot_repair_helmet) { should_draw = true; col = IM_COL32(0, 255, 0, 255); } }

                        // --- 14. CATCH-ALL & IMPORTANT ---
                        else if (g_Menu.loot_weapon_all || item.IsImportant) should_draw = true;
                    }
                }

                Vector2 itemScreen;
                if (telemetryContext::WorldToScreen(item.Position, itemScreen)) {
                    std::string resolvedIconName = EntityAliases::ResolveItemAsset(item.Name);
                    bool duplicateVehicle = false;
                    if (item.RenderType == ItemRenderType::Vehicle) {
                        resolvedIconName = VehicleResolver::Resolve(item.Name).IconName;
                        duplicateVehicle = HasDuplicateVehicle(resolvedIconName, itemScreen);
                    }

                    if (g_Menu.debug_loot_resolver) {
                        LootDebugRenderer::Draw(draw, item, itemScreen, resolvedIconName,
                            duplicateVehicle, should_draw && !duplicateVehicle);
                    }

                    if (should_draw) {
                        TextureInfo* icon = nullptr;
                        float iconSize = g_Menu.item_icon_size;
                        if (g_Menu.esp_icons) {
                           if (item.RenderType == ItemRenderType::Vehicle) {
                               if (duplicateVehicle) continue;
                               MarkVehicleSeen(resolvedIconName, itemScreen);
                               icon = OverlayTextures::GetVehicleIcon(resolvedIconName);
                               iconSize = g_Menu.vehicle_icon_size;
                           } else {
                               icon = OverlayTextures::GetItemIcon(item.Name);
                           }
                        } else if (item.RenderType == ItemRenderType::Vehicle) {
                            if (duplicateVehicle) continue;
                            MarkVehicleSeen(resolvedIconName, itemScreen);
                            iconSize = g_Menu.vehicle_icon_size;
                        }

                        const bool groupable =
                            item.RenderType == ItemRenderType::Loot;
                        lootEntries.push_back({ item.Name, itemScreen, item.Distance, col, icon, groupable, item.IsImportant, iconSize });
                    }
                }
            }

            LootClusterRenderer::Draw(draw, lootEntries, {
                g_Menu.item_icon_size,
                g_Menu.vehicle_icon_size,
                g_Menu.item_group_icon_size,
                g_Menu.loot_distance_font_size
            });
}
