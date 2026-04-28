#include "../core/overlay_menu.hpp"
#include "../core/overlay_asset_animation.hpp"
#include "../core/overlay_hotkeys.hpp"
#include "../translation/translation.hpp"
#include <protec/skCrypt.h>
#include <string>

struct VisualLootTile {
    const char* label;
    const char* folder;
    const char* asset;
    bool* enabled;
};

// Logic rendering cho tab Loot

void OverlayMenu::RenderTabLoot(ImVec2 windowSize) {
    auto Lang = Translation::Get();
    float totalWidth = windowSize.x - 60;
    ImGui::Columns(5, skCrypt("ItemColumns"), false);
    struct VisualLootTile {
        const char* label;
        const char* folder;
        const char* asset;
        bool* enabled;
    };

    auto DrawVisualLootTile = [&](const VisualLootTile& item, const ImVec2& tileSize) {
        ImGui::PushID(item.asset);
        ImVec2 tileMin = ImGui::GetCursorScreenPos();
        ImVec2 tileMax(tileMin.x + tileSize.x, tileMin.y + tileSize.y);

        if (ImGui::InvisibleButton(skCrypt("##VisualLootTile"), tileSize)) {
            *item.enabled = !*item.enabled;
        }

        bool hovered = ImGui::IsItemHovered();
        ImDrawList* tileDraw = ImGui::GetWindowDrawList();
        ImU32 bg = *item.enabled ? IM_COL32(0, 150, 255, 55) : IM_COL32(12, 24, 45, 135);
        ImU32 border = *item.enabled ? IM_COL32(0, 210, 255, 190) : IM_COL32(80, 110, 150, 85);
        if (hovered) bg = IM_COL32(0, 190, 255, 75);

        tileDraw->AddRectFilled(tileMin, tileMax, bg, 8.0f);
        tileDraw->AddRect(tileMin, tileMax, border, 8.0f, 0, *item.enabled ? 1.6f : 1.0f);

        TextureInfo* icon = GetPreviewIcon(item.folder, item.asset);
        const float iconTargetSize = 38.0f;

        if (icon && icon->SRV && icon->Width > 0 && icon->Height > 0) {
            OverlayAssetAnimation::DrawOptions anim{};
            anim.hovered = hovered;
            anim.selected = *item.enabled;
            anim.important = *item.enabled;
            anim.strength = hovered || *item.enabled ? 1.18f : 0.86f;
            OverlayAssetAnimation::DrawAnimatedImage(tileDraw, icon,
                ImVec2(tileMin.x + tileSize.x * 0.5f, tileMin.y + 6.0f + iconTargetSize * 0.5f),
                iconTargetSize,
                IM_COL32(255, 255, 255, *item.enabled ? 255 : 226),
                anim);
        } else {
            ImVec2 iconMin(tileMin.x + (tileSize.x - iconTargetSize) * 0.5f, tileMin.y + 6.0f);
            ImVec2 iconMax(iconMin.x + iconTargetSize, iconMin.y + iconTargetSize);
            tileDraw->AddRect(iconMin, iconMax, IM_COL32(110, 140, 180, 170), 5.0f);
            tileDraw->AddText(ImVec2(iconMin.x + 13.0f, iconMin.y + 10.0f), IM_COL32(170, 200, 235, 220), skCrypt("?"));
        }

        ImVec4 textClip(tileMin.x + 5.0f, tileMin.y + 48.0f, tileMax.x - 5.0f, tileMax.y - 5.0f);
        tileDraw->AddText(ImGui::GetFont(), 12.0f, ImVec2(textClip.x, textClip.y),
                            IM_COL32(225, 238, 255, 235), item.label, nullptr, tileSize.x - 10.0f, &textClip);

        if (*item.enabled) {
            tileDraw->AddCircleFilled(ImVec2(tileMax.x - 12.0f, tileMin.y + 12.0f), 6.0f, IM_COL32(0, 220, 255, 230));
        }

        if (hovered) {
            ImGui::SetTooltip("%s", *item.enabled ? Lang.ItemCatalogSelected : Lang.ItemCatalogHint);
        }

        ImGui::PopID();
    };

    auto DrawVisualLootGrid = [&](const VisualLootTile* items, int count, int preferredPerRow = 3) {
        const float tileGap = 8.0f;
        const float tileHeight = 76.0f;
        int perRow = preferredPerRow;
        float available = ImGui::GetContentRegionAvail().x;

        // Standardize on 3 per row for better density as requested
        perRow = 3;

        float tileWidth = (available - (perRow - 1) * tileGap) / perRow;
        if (tileWidth > 96.0f) tileWidth = 96.0f;
        if (tileWidth < 68.0f) tileWidth = 68.0f;
        ImVec2 tileSize(tileWidth, tileHeight);

        for (int i = 0; i < count; ++i) {
            DrawVisualLootTile(items[i], tileSize);
            if ((i % perRow) != (perRow - 1) && i != count - 1) {
                ImGui::SameLine(0, tileGap);
            }
        }
    };
    ImGui::SetColumnWidth(0, totalWidth / 5.0f);
    ImGui::SetColumnWidth(1, totalWidth / 5.0f);
    ImGui::SetColumnWidth(2, totalWidth / 5.0f);
    ImGui::SetColumnWidth(3, totalWidth / 5.0f);
    ImGui::SetColumnWidth(4, totalWidth / 5.0f);

    // Column 1: engine and gear
    BeginGlassCard(skCrypt("##ItemCol1"), Lang.HeaderLootEngine, ImVec2(totalWidth / 5.0f - 12, 0));
    ImGui::Checkbox(Lang.TabLoot, &g_Menu.esp_items);
    OverlayHotkeys::DrawKeyBind(skCrypt("Items Toggle Key"), &g_Menu.esp_items_toggle_key, g_Menu.waiting_for_key);
    ImGui::Checkbox(skCrypt("Loot Resolver Debug"), &g_Menu.debug_loot_resolver);
    ImGui::SliderInt(Lang.RenderDist, &g_Menu.loot_max_dist, 10, 300, skCrypt("%d m"));
    ImGui::SliderFloat(skCrypt("Item Icon Size"), &g_Menu.item_icon_size, 12.0f, 48.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(skCrypt("Group Icon Size"), &g_Menu.item_group_icon_size, 10.0f, 38.0f, skCrypt("%.0f px"));
    ImGui::SliderFloat(skCrypt("Loot Text Size"), &g_Menu.loot_distance_font_size, 8.0f, 20.0f, skCrypt("%.1f px"));
    ImGui::Checkbox(skCrypt("Animated Assets"), &g_Menu.asset_animation_enabled);
    ImGui::SameLine();
    ImGui::Checkbox(skCrypt("Glow"), &g_Menu.asset_animation_glow);
    ImGui::SameLine();
    ImGui::Checkbox(skCrypt("Shine"), &g_Menu.asset_animation_shine);
    ImGui::SliderFloat(skCrypt("Anim Strength"), &g_Menu.asset_animation_strength, 0.0f, 2.0f, skCrypt("%.2f"));
    ImGui::SliderFloat(skCrypt("Anim Speed"), &g_Menu.asset_animation_speed, 0.10f, 3.0f, skCrypt("%.2f"));
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), Lang.HeaderGearFilter);
    VisualLootTile gearTiles[] = {
        { Lang.HelmetLv1, skCrypt("Helmet"), skCrypt("Item_Head_E_01_Lv1_C"), &g_Menu.loot_helmet_lv1 },
        { Lang.HelmetLv2, skCrypt("Helmet"), skCrypt("Item_Head_F_01_Lv2_C"), &g_Menu.loot_helmet_lv2 },
        { Lang.HelmetLv3, skCrypt("Helmet"), skCrypt("Item_Head_G_01_Lv3_C"), &g_Menu.loot_helmet_lv3 },
        { Lang.ArmorLv1, skCrypt("Armor"), skCrypt("Item_Armor_E_01_Lv1_C"), &g_Menu.loot_armor_lv1 },
        { Lang.ArmorLv2, skCrypt("Armor"), skCrypt("Item_Armor_D_01_Lv2_C"), &g_Menu.loot_armor_lv2 },
        { Lang.ArmorLv3, skCrypt("Armor"), skCrypt("Item_Armor_C_01_Lv3_C"), &g_Menu.loot_armor_lv3 },
        { Lang.BackpackLv1, skCrypt("Backpack"), skCrypt("Item_Back_E_01_Lv1_C"), &g_Menu.loot_backpack_lv1 },
        { Lang.BackpackLv2, skCrypt("Backpack"), skCrypt("Item_Back_F_01_Lv2_C"), &g_Menu.loot_backpack_lv2 },
        { Lang.BackpackLv3, skCrypt("Backpack"), skCrypt("Item_Back_C_01_Lv3_C"), &g_Menu.loot_backpack_lv3 }
    };
    DrawVisualLootGrid(gearTiles, IM_ARRAYSIZE(gearTiles), 3);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), skCrypt("SPECIAL REPAIR & GHILLIE"));
    VisualLootTile specialGearTiles[] = {
        { skCrypt("Rep:A"), skCrypt("Repair"), skCrypt("Armor_Repair_Kit_C"), &g_Menu.loot_repair_armor },
        { skCrypt("Rep:H"), skCrypt("Repair"), skCrypt("Helmet_Repair_Kit_C"), &g_Menu.loot_repair_helmet },
        { skCrypt("Rep:V"), skCrypt("Repair"), skCrypt("Vehicle_Repair_Kit_C"), &g_Menu.loot_repair_vehicle },
        { skCrypt("G:Arc"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_01_C"), &g_Menu.loot_ghillie_arctic },
        { skCrypt("G:Des"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_02_C"), &g_Menu.loot_ghillie_desert },
        { skCrypt("G:Jun"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_03_C"), &g_Menu.loot_ghillie_jungle },
        { skCrypt("G:For"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_04_C"), &g_Menu.loot_ghillie_forest },
        { skCrypt("G:Mos"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_05_C"), &g_Menu.loot_ghillie_mossy },
        { skCrypt("G:Brn"), skCrypt("Ghillie"), skCrypt("Item_Ghillie_06_C"), &g_Menu.loot_ghillie_brown }
    };
    DrawVisualLootGrid(specialGearTiles, IM_ARRAYSIZE(specialGearTiles), 3);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), skCrypt("TACTICAL UTILITY"));
    VisualLootTile tacticalTiles[] = {
        { skCrypt("Drone"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_Drone_C"), &g_Menu.loot_utility_drone },
        { skCrypt("Spot"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_Spotter_Scope_C"), &g_Menu.loot_utility_scope },
        { skCrypt("Jamr"), skCrypt("Backpack"), skCrypt("Item_Back_BlueBlocker"), &g_Menu.loot_utility_jammer },
        { skCrypt("BlueC"), skCrypt("Utility"), skCrypt("Item_Bluechip_C"), &g_Menu.loot_utility_bluechip },
        { skCrypt("Trans"), skCrypt("Utility"), skCrypt("Item_Revival_Transmitter_C"), &g_Menu.loot_utility_vtransmitter },
        { skCrypt("Shld"), skCrypt("Utility"), skCrypt("Item_BulletproofShield_C"), &g_Menu.loot_utility_shield }
    };
    DrawVisualLootGrid(tacticalTiles, IM_ARRAYSIZE(tacticalTiles), 3);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 2: Medicines & Attachments
    BeginGlassCard(skCrypt("##ItemCol2"), Lang.HeaderHealFilter, ImVec2(totalWidth / 5.0f - 12, 0));
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), skCrypt("MEDICINES"));
    VisualLootTile medTiles[] = {
        { Lang.Healing, skCrypt("Medicine"), skCrypt("Item_Heal_FirstAid_C"), &g_Menu.loot_meds_healing },
        { Lang.Boosters, skCrypt("Medicine"), skCrypt("Item_Boost_EnergyDrink_C"), &g_Menu.loot_meds_boosts }
    };
    DrawVisualLootGrid(medTiles, IM_ARRAYSIZE(medTiles), 2);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), skCrypt("OPTICS (SCOPES)"));
    VisualLootTile scopeTiles[] = {
        { skCrypt("RDS"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_DotSight_01_C"), &g_Menu.loot_scope_reddot },
        { skCrypt("Holo"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_Holosight_C"), &g_Menu.loot_scope_holo },
        { skCrypt("2x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_Aimpoint_C"), &g_Menu.loot_scope_2x },
        { skCrypt("3x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_Scope3x_C"), &g_Menu.loot_scope_3x },
        { skCrypt("4x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_ACOG_01_C"), &g_Menu.loot_scope_4x },
        { skCrypt("6x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_Scope6x_C"), &g_Menu.loot_scope_6x },
        { skCrypt("8x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_PM2_01_C"), &g_Menu.loot_scope_8x },
        { skCrypt("15x"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_CQBSS_C"), &g_Menu.loot_scope_15x },
        { skCrypt("Trm"), skCrypt("Attachment/Scope"), skCrypt("Item_Attach_Weapon_Upper_Thermal_C"), &g_Menu.loot_scope_thermal }
    };
    DrawVisualLootGrid(scopeTiles, IM_ARRAYSIZE(scopeTiles), 3);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), skCrypt("MUZZLES & GRIPS"));
    VisualLootTile attachTiles[] = {
        { skCrypt("Comp"), skCrypt("Attachment/Muzzle"), skCrypt("Item_Attach_Weapon_Muzzle_Compensator_Large_C"), &g_Menu.loot_muzzle_comp },
        { skCrypt("Supp"), skCrypt("Attachment/Muzzle"), skCrypt("Item_Attach_Weapon_Muzzle_Suppressor_Large_C"), &g_Menu.loot_muzzle_supp },
        { skCrypt("Vert"), skCrypt("Attachment/Grip"), skCrypt("Item_Attach_Weapon_Lower_Foregrip_C"), &g_Menu.loot_grip_vertical },
        { skCrypt("Angle"), skCrypt("Attachment/Grip"), skCrypt("Item_Attach_Weapon_Lower_AngledForeGrip_C"), &g_Menu.loot_grip_angled },
        { skCrypt("Thumb"), skCrypt("Attachment/Grip"), skCrypt("Item_Attach_Weapon_Lower_ThumbGrip_C"), &g_Menu.loot_grip_thumb },
        { skCrypt("Ext"), skCrypt("Attachment/Mag"), skCrypt("Item_Attach_Weapon_Magazine_ExtendedQuickDraw_Large_C"), &g_Menu.loot_mag_ext_quick }
    };
    DrawVisualLootGrid(attachTiles, IM_ARRAYSIZE(attachTiles), 3);

    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 3: Ammo - Exhaustive List
    BeginGlassCard(skCrypt("##ItemCol3"), Lang.HeaderAmmoScope, ImVec2(totalWidth / 5.0f - 12, 0));
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), skCrypt("AMMUNITION"));
    VisualLootTile ammoTiles[] = {
        { skCrypt("5.56"), skCrypt("Ammo"), skCrypt("Item_Ammo_556mm_C"), &g_Menu.loot_ammo_556 },
        { skCrypt("7.62"), skCrypt("Ammo"), skCrypt("Item_Ammo_762mm_C"), &g_Menu.loot_ammo_762 },
        { skCrypt("9mm"), skCrypt("Ammo"), skCrypt("Item_Ammo_9mm_C"), &g_Menu.loot_ammo_9mm },
        { skCrypt(".45"), skCrypt("Ammo"), skCrypt("Item_Ammo_45ACP_C"), &g_Menu.loot_ammo_45 },
        { skCrypt("5.7mm"), skCrypt("Ammo"), skCrypt("Item_Ammo_57mm_C"), &g_Menu.loot_ammo_57 },
        { skCrypt("Slug"), skCrypt("Ammo"), skCrypt("Item_Ammo_12GuageSlug_C"), &g_Menu.loot_ammo_slug },
        { skCrypt("SG"), skCrypt("Ammo"), skCrypt("Item_Ammo_12Guage_C"), &g_Menu.loot_ammo_12g },
        { skCrypt(".300"), skCrypt("Ammo"), skCrypt("Item_Ammo_300Magnum_C"), &g_Menu.loot_ammo_300 },
        { skCrypt("40mm"), skCrypt("Ammo"), skCrypt("Item_Ammo_40mm_C"), &g_Menu.loot_ammo_40 },
        { skCrypt("Bolt"), skCrypt("Ammo"), skCrypt("Item_Ammo_Bolt_C"), &g_Menu.loot_ammo_bolt },
        { skCrypt("Flare"), skCrypt("Ammo"), skCrypt("Item_Ammo_Flare_C"), &g_Menu.loot_ammo_flare },
        { skCrypt("Mortar"), skCrypt("Ammo"), skCrypt("Item_Ammo_Mortar_C"), &g_Menu.loot_ammo_mortar }
    };
    DrawVisualLootGrid(ammoTiles, IM_ARRAYSIZE(ammoTiles), 3);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.0f, 1.0f), skCrypt("SECURITY CARDS & KEYS"));
    VisualLootTile keyTiles[] = {
        { skCrypt("SecC"), skCrypt("Key"), skCrypt("Item_Secuity_Keycard_C"), &g_Menu.loot_key_security },
        { skCrypt("Room"), skCrypt("Key"), skCrypt("Item_BTSecretRoom_Key_C"), &g_Menu.loot_key_secret },
        { skCrypt("Tiger"), skCrypt("Key"), skCrypt("Item_Tiger_Key_C"), &g_Menu.loot_key_taego },
        { skCrypt("Vikn"), skCrypt("Key"), skCrypt("Item_DihorOtok_Key_C"), &g_Menu.loot_key_vikendi },
        { skCrypt("Chim"), skCrypt("Key"), skCrypt("Item_Chimera_Key_C"), &g_Menu.loot_key_chimera },
        { skCrypt("Havn"), skCrypt("Key"), skCrypt("Item_Heaven_Key_C"), &g_Menu.loot_key_haven }
    };
    DrawVisualLootGrid(keyTiles, IM_ARRAYSIZE(keyTiles), 3);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 1.0f, 1.0f), skCrypt("GRENADES & THROWABLES"));
    VisualLootTile throwTiles[] = {
        { skCrypt("Frag"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_Grenade_C"), &g_Menu.loot_throw_frag },
        { skCrypt("Smok"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_SmokeBomb_C"), &g_Menu.loot_throw_smoke },
        { skCrypt("Flsh"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_FlashBang_C"), &g_Menu.loot_throw_flash },
        { skCrypt("Molo"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_Molotov_C"), &g_Menu.loot_throw_molotov },
        { skCrypt("C4"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_C4_C"), &g_Menu.loot_throw_c4 },
        { skCrypt("Stky"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_StickyGrenade_C"), &g_Menu.loot_throw_sticky },
        { skCrypt("BZ"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_BluezoneGrenade_C"), &g_Menu.loot_throw_bz },
        { skCrypt("Dcy"), skCrypt("Gun/Throw"), skCrypt("Item_Weapon_DecoyGrenade_C"), &g_Menu.loot_throw_decoy }
    };
    DrawVisualLootGrid(throwTiles, IM_ARRAYSIZE(throwTiles), 3);
    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 4: Weapons & Attach - Exhaustive List
    BeginGlassCard(skCrypt("##ItemCol4"), Lang.HeaderWeaponry, ImVec2(totalWidth / 5.0f - 12, 0));

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), skCrypt("ASSAULT RIFLES"));
    VisualLootTile arTiles[] = {
        { skCrypt("M416"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_HK416_C"), &g_Menu.loot_weapon_hk416 },
        { skCrypt("AKM"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_AK47_C"), &g_Menu.loot_weapon_ak47 },
        { skCrypt("Beryl"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_BerylM762_C"), &g_Menu.loot_weapon_beryl },
        { skCrypt("ACE32"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_ACE32_C"), &g_Menu.loot_weapon_ace32 },
        { skCrypt("AUG"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_AUG_C"), &g_Menu.loot_weapon_aug },
        { skCrypt("Groza"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_Groza_C"), &g_Menu.loot_weapon_groza },
        { skCrypt("SCAR-L"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_SCAR-L_C"), &g_Menu.loot_weapon_scar },
        { skCrypt("M16A4"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_M16A4_C"), &g_Menu.loot_weapon_m16 },
        { skCrypt("QBZ"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_QBZ95_C"), &g_Menu.loot_weapon_qbz },
        { skCrypt("K2"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_K2_C"), &g_Menu.loot_weapon_k2 },
        { skCrypt("FAMAS"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_FAMASG2_C"), &g_Menu.loot_weapon_famas },
        { skCrypt("G36C"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_G36C_C"), &g_Menu.loot_weapon_g36c },
        { skCrypt("Mutant"), skCrypt("Gun/AR"), skCrypt("Item_Weapon_Mk47Mutant_C"), &g_Menu.loot_weapon_mutant }
    };
    DrawVisualLootGrid(arTiles, IM_ARRAYSIZE(arTiles), 3);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), skCrypt("SNIPERS & DMRS"));
    VisualLootTile srTiles[] = {
        { skCrypt("AWM"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_AWM_C"), &g_Menu.loot_weapon_awm },
        { skCrypt("M24"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_M24_C"), &g_Menu.loot_weapon_m24 },
        { skCrypt("Kar98k"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Kar98k_C"), &g_Menu.loot_weapon_kar98 },
        { skCrypt("Mosin"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Mosin_C"), &g_Menu.loot_weapon_mosin },
        { skCrypt("Win94"), skCrypt("Gun/SR"), skCrypt("Item_Weapon_Win1894_C"), &g_Menu.loot_weapon_win94 },
        { skCrypt("Mk14"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mk14_C"), &g_Menu.loot_weapon_mk14 },
        { skCrypt("Mk12"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mk12_C"), &g_Menu.loot_weapon_mk12 },
        { skCrypt("SLR"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_FNFal_C"), &g_Menu.loot_weapon_slr },
        { skCrypt("SKS"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_SKS_C"), &g_Menu.loot_weapon_sks },
        { skCrypt("Mini14"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Mini14_C"), &g_Menu.loot_weapon_mini14 },
        { skCrypt("Dragunov"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_Dragunov_C"), &g_Menu.loot_weapon_dragunov },
        { skCrypt("VSS"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_VSS_C"), &g_Menu.loot_weapon_vss },
        { skCrypt("QBU"), skCrypt("Gun/DMR"), skCrypt("Item_Weapon_QBU88_C"), &g_Menu.loot_weapon_qbu }
    };
    DrawVisualLootGrid(srTiles, IM_ARRAYSIZE(srTiles), 3);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), skCrypt("SMGS & MACHINE GUNS"));
    VisualLootTile smgTiles[] = {
        { skCrypt("MP5K"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_MP5K_C"), &g_Menu.loot_weapon_mp5 },
        { skCrypt("UMP"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_UMP_C"), &g_Menu.loot_weapon_ump },
        { skCrypt("Vector"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_Vector_C"), &g_Menu.loot_weapon_vector },
        { skCrypt("UZI"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_UZI_C"), &g_Menu.loot_weapon_uzi },
        { skCrypt("P90"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_P90_C"), &g_Menu.loot_weapon_p90 },
        { skCrypt("JS9"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_JS9_C"), &g_Menu.loot_weapon_js9 },
        { skCrypt("MP9"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_MP9_C"), &g_Menu.loot_weapon_mp9 },
        { skCrypt("Bizon"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_BizonPP19_C"), &g_Menu.loot_weapon_bizon },
        { skCrypt("Thompson"), skCrypt("Gun/SMG"), skCrypt("Item_Weapon_Thompson_C"), &g_Menu.loot_weapon_thompson },
        { skCrypt("M249"), skCrypt("Gun/LMG"), skCrypt("Item_Weapon_M249_C"), &g_Menu.loot_weapon_m249 },
        { skCrypt("MG3"), skCrypt("Gun/LMG"), skCrypt("Item_Weapon_MG3_C"), &g_Menu.loot_weapon_mg3 },
        { skCrypt("DP-28"), skCrypt("Gun/LMG"), skCrypt("Item_Weapon_DP28_C"), &g_Menu.loot_weapon_dp28 }
    };
    DrawVisualLootGrid(smgTiles, IM_ARRAYSIZE(smgTiles), 3);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), skCrypt("SHOTGUNS & SIDEARMS"));
    VisualLootTile sgTiles[] = {
        { skCrypt("DBS"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_DP12_C"), &g_Menu.loot_weapon_dp12 },
        { skCrypt("S12K"), skCrypt("Gun/SG"), skCrypt("Item_Weapon_Saiga12_C"), &g_Menu.loot_weapon_saiga },
        { skCrypt("D-Eagle"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_DesertEagle_C"), &g_Menu.loot_weapon_deagle },
        { skCrypt("P18C"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_G18_C"), &g_Menu.loot_weapon_p92 },
        { skCrypt("Skorpion"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_vz61Skorpion_C"), &g_Menu.loot_weapon_skorpion },
        { skCrypt("Nagant"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_NagantM1895_C"), &g_Menu.loot_weapon_nagant },
        { skCrypt("Rhino"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_Rhino_C"), &g_Menu.loot_weapon_rhino },
        { skCrypt("Stun"), skCrypt("Gun/HG"), skCrypt("Item_Weapon_StunGun_C"), &g_Menu.loot_weapon_stungun }
    };
    DrawVisualLootGrid(sgTiles, IM_ARRAYSIZE(sgTiles), 3);
    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.0f, 1.0f), skCrypt("SPECIALS & MELEE"));
    VisualLootTile miscWeaponTiles[] = {
        { skCrypt("Flare"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_FlareGun_C"), &g_Menu.loot_weapon_flare },
        { skCrypt("Cross"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_Crossbow_C"), &g_Menu.loot_weapon_crossbow },
        { skCrypt("Panzer"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_PanzerFaust100M_C"), &g_Menu.loot_weapon_panzer },
        { skCrypt("M79"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_M79_C"), &g_Menu.loot_weapon_m79 },
        { skCrypt("Pan"), skCrypt("Gun/Melee"), skCrypt("Item_Weapon_Pan_C"), &g_Menu.loot_weapon_pan },
        { skCrypt("Spike"), skCrypt("Gun/Special"), skCrypt("Item_Weapon_SpikeTrap_C"), &g_Menu.loot_weapon_spike }
    };
    DrawVisualLootGrid(miscWeaponTiles, IM_ARRAYSIZE(miscWeaponTiles), 3);

    ImGui::EndChild();

    ImGui::NextColumn();
    // Col 5: Vehicles Dedicated
    BeginGlassCard(skCrypt("##ItemCol5"), Lang.HeaderVehicleFilter, ImVec2(totalWidth / 5.0f - 12, 0));

    // Main Vehicle Toggle
    ImGui::Checkbox(Lang.ShowVehicles, &g_Menu.esp_vehicles);
    OverlayHotkeys::DrawKeyBind(skCrypt("Vehicle Toggle Key"), &g_Menu.esp_vehicles_toggle_key, g_Menu.waiting_for_key);
    ImGui::SliderInt(skCrypt("##VehDist"), &g_Menu.vehicle_max_dist, 50, 1000, skCrypt("%d m"));
    ImGui::SliderFloat(skCrypt("Vehicle Icon Size"), &g_Menu.vehicle_icon_size, 16.0f, 80.0f, skCrypt("%.0f px"));
    ImGui::Separator();

    // Specific Vehicle Sub-filters using the grid system for visual consistency
    VisualLootTile vehicleTiles[] = {
        { Lang.VehicleUAZ, skCrypt("Vehicle"), skCrypt("Uaz_A_00_C"), &g_Menu.loot_vehicle_uaz },
        { Lang.VehicleDacia, skCrypt("Vehicle"), skCrypt("Dacia_A_00_v2_C"), &g_Menu.loot_vehicle_dacia },
        { Lang.VehicleBuggy, skCrypt("Vehicle"), skCrypt("Buggy_A_01_C"), &g_Menu.loot_vehicle_buggy },
        { Lang.VehicleBike, skCrypt("Vehicle"), skCrypt("BP_Motorbike_04_C"), &g_Menu.loot_vehicle_bike },
        { Lang.VehicleBoat, skCrypt("Vehicle"), skCrypt("Boat_PG117_C"), &g_Menu.loot_vehicle_boat },
        { Lang.VehicleBRDM, skCrypt("Vehicle"), skCrypt("BP_BRDM_C"), &g_Menu.loot_vehicle_brdm },
        { Lang.VehicleScooter, skCrypt("Vehicle"), skCrypt("BP_Scooter_00_A_C"), &g_Menu.loot_vehicle_scooter },
        { Lang.VehicleSnow, skCrypt("Vehicle"), skCrypt("BP_Snowbike_00_C"), &g_Menu.loot_vehicle_snow },
        { Lang.VehicleTuk, skCrypt("Vehicle"), skCrypt("BP_TukTukTuk_A_00_C"), &g_Menu.loot_vehicle_tuk },
        { Lang.VehicleBus, skCrypt("Vehicle"), skCrypt("BP_MiniBus_C"), &g_Menu.loot_vehicle_bus },
        { Lang.VehicleTruck, skCrypt("Vehicle"), skCrypt("BP_LootTruck_C"), &g_Menu.loot_vehicle_truck },
        { Lang.VehicleTrain, skCrypt("Vehicle"), skCrypt("BP_DO_Circle_Train_Merged_C"), &g_Menu.loot_vehicle_train },
        { Lang.VehicleMirado, skCrypt("Vehicle"), skCrypt("BP_Mirado_A_00_C"), &g_Menu.loot_vehicle_mirado },
        { Lang.VehiclePickup, skCrypt("Vehicle"), skCrypt("BP_PickupTruck_A_00_C"), &g_Menu.loot_vehicle_pickup },
        { Lang.VehicleRony, skCrypt("Vehicle"), skCrypt("BP_M_Rony_A_00_C"), &g_Menu.loot_vehicle_rony },
        { Lang.VehicleBlanc, skCrypt("Vehicle"), skCrypt("BP_Blanc_C"), &g_Menu.loot_vehicle_blanc },
        { Lang.VehicleAir, skCrypt("Vehicle"), skCrypt("BP_Motorglider_C"), &g_Menu.loot_vehicle_air }
    };

    // Draw 3 items per row as requested
    DrawVisualLootGrid(vehicleTiles, IM_ARRAYSIZE(vehicleTiles), 3);
    ImGui::EndChild();

    ImGui::Columns(1);
}
