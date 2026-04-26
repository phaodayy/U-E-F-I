#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <diagnostic_node/GNames.h>
#include <diagnostic_node/Decrypt.h>
#include <Common/Offset.h>

class Items
{
public:
    static std::unordered_map<int, int> GetWeaponAttachedItems(VMMDLL_SCATTER_HANDLE hScatter)
    {
        struct FWeaponAttachmentItem
        {
            uint64_t Item;
            uint64_t WeaponAttachmentData;
            int ItemID;
            EntityInfo EntityInfo;
        };

        uint64_t CurrentWeapon = GameData.LocalPlayerInfo.CurrentWeapon;
        std::vector<FWeaponAttachmentItem> WeaponAttachmentItems;
        std::unordered_map<int, int> WeaponAttachmentItemID;

        //�ֳֹ���
        if (!GameData.Config.Item.HandheldWeaponFilter)
        {

        }

        //ͷ�׹���
        if (!GameData.Config.Item.FilterHelmets)
        {

        }
        // ����������Ʒ����
        {
            uint64_t InventoryFacade = Decrypt::Xe(mem.Read<uint64_t>(GameData.AcknowledgedPawn + Offset::InventoryFacade));
            uint64_t Inventory = mem.Read<uint64_t>(InventoryFacade + Offset::Inventory);
            auto InventoryItems = mem.Read<TArray<uint64_t>>(Inventory + Offset::InventoryItems);
            struct InventoryItemInfo
            {
                uint64_t Item;
                uint64_t ItemTable;
                int ItemID;
                int Count;
            };

            std::unordered_map<uint64_t, InventoryItemInfo> CacheInventoryItemInfos;
            std::unordered_map<uint64_t, InventoryItemInfo> InventoryItemInfos;

            for (auto& Item : InventoryItems.GetVector())
            {
                InventoryItemInfo ItemInfo;
                ItemInfo.Item = Item;
                CacheInventoryItemInfos[Item] = ItemInfo;
            }

            for (auto& Item : CacheInventoryItemInfos)
            {
                mem.AddScatterRead(hScatter, Item.first + Offset::ItemTable, (uint64_t*)&Item.second.ItemTable);
                mem.AddScatterRead(hScatter, Item.first + Offset::InventoryItemTagItemCount, (uint64_t*)&Item.second.Count);
            }
            mem.ExecuteReadScatter(hScatter);
            for (auto& Item : CacheInventoryItemInfos)
            {
                mem.AddScatterRead(hScatter, Item.second.ItemTable + Offset::ItemID, (int*)&Item.second.ItemID);
            }
            mem.ExecuteReadScatter(hScatter);

            //������Ʒ���޹���
            if (GameData.Config.Item.ItemLimit)
            {
                for (auto& Item : CacheInventoryItemInfos)
                {
                    if (Item.second.ItemID == 0) continue;

                    EntityInfo EntityInfo = Data::GetGNameListsByIDItem(Item.second.ItemID);

                    // ���� 
                    if (EntityInfo.Name == skCrypt("Item_Heal_Bandage_C") && Item.second.Count >= GameData.Config.ItemFiltering.Bandage)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //���Ȱ�
                    if (EntityInfo.Name == skCrypt("Item_Heal_FirstAid_C") && Item.second.Count >= GameData.Config.ItemFiltering.FirstAidKit)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //ҽ����
                    if (EntityInfo.Name == skCrypt("Item_Heal_MedKit_C") && Item.second.Count >= GameData.Config.ItemFiltering.MedicalKit)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //ֹʹҩ
                    if (EntityInfo.Name == skCrypt("Item_Boost_PainKiller_C") && Item.second.Count >= GameData.Config.ItemFiltering.Painkiller)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //��������
                    if (EntityInfo.Name == skCrypt("Item_Boost_EnergyDrink_C") && Item.second.Count >= GameData.Config.ItemFiltering.EnergyDrink)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //��������
                    if (EntityInfo.Name == skCrypt("Item_Boost_AdrenalineSyringe_C") && Item.second.Count >= GameData.Config.ItemFiltering.Adrenaline)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //C4
                    if (EntityInfo.Name == skCrypt("Item_Weapon_C4_C") && Item.second.Count >= GameData.Config.ItemFiltering.C4)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //����
                    if (EntityInfo.Name == skCrypt("Item_Weapon_Grenade_C") && Item.second.Count >= GameData.Config.ItemFiltering.Grenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //������
                    if (EntityInfo.Name == skCrypt("Item_Weapon_SmokeBomb_C") && Item.second.Count >= GameData.Config.ItemFiltering.SmokeGrenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //���ⵯ
                    if (EntityInfo.Name == skCrypt("Item_Weapon_FlashBang_C") && Item.second.Count >= GameData.Config.ItemFiltering.Flashbang)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //ȼ��ƿ
                    if (EntityInfo.Name == skCrypt("Item_Weapon_Molotov_C") && Item.second.Count >= GameData.Config.ItemFiltering.Molotov)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //��Ȧ����
                    if (EntityInfo.Name == skCrypt("Item_Weapon_BluezoneGrenade_C") && Item.second.Count >= GameData.Config.ItemFiltering.BlueZoneGrenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //ճ��ը��
                    if (EntityInfo.Name == skCrypt("Item_Weapon_StickyGrenade_C") && Item.second.Count >= GameData.Config.ItemFiltering.StickyBomb)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                }
            }
        }
        //�������
        if (GameData.Config.Item.AccessoriesFilter)
        {
            if (!Utils::ValidPtr(CurrentWeapon))
            {
                TArray<uint64_t> AttachedItems = mem.Read<TArray<uint64_t>>(CurrentWeapon + Offset::AttachedItems);
                for (const auto& Item : AttachedItems.GetVector())
                {
                    if (Utils::ValidPtr(Item)) continue;
                    FWeaponAttachmentItem WeaponAttachmentItem;
                    WeaponAttachmentItem.Item = Item;
                    WeaponAttachmentItems.emplace_back(WeaponAttachmentItem);
                }

                if (WeaponAttachmentItems.size() > 0)
                {
                    for (auto& Item : WeaponAttachmentItems)
                    {
                        mem.AddScatterRead(hScatter, Item.Item + Offset::WeaponAttachmentData, &Item.WeaponAttachmentData);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : WeaponAttachmentItems)
                    {
                        mem.AddScatterRead(hScatter, Item.WeaponAttachmentData + Offset::ItemID, &Item.ItemID);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    std::vector<int> NeedGetNameIDs;
                    for (auto& Item : WeaponAttachmentItems)
                    {
                        WeaponAttachmentItemID[Item.ItemID] = Item.ItemID;

                    }
                }
            }

        }


        return WeaponAttachmentItemID;
    }

    static void Update()
    {
        auto hScatter = mem.CreateScatterHandle();
        std::unordered_map<uint64_t, ItemInfo> CacheItems;
        Throttler Throttlered;
        while (true)
        {
            auto startTime = std::chrono::high_resolution_clock::now();
            if (GameData.Scene != Scene::Gaming)
            {
                CacheItems.clear();
                Data::SetItems({});
                // Ngoai tran: reset thoi gian ItemThread ve 0
                GameData.Performance.ItemThreadMs = 0.0f;
                Sleep(GameData.ThreadSleep);
                continue;
            }

            if (!GameData.Config.Item.Enable)
            {
                CacheItems.clear();
                Data::SetItems({});
                GameData.Performance.ItemThreadMs = 0.0f;
                Sleep(400);
                continue;
            }

            // Cache WeaponAttachmentItemID - chi refresh moi 3s thay vi moi 400ms
            // Dung thread rieng de doc DMA (tranh block Item thread)
            static std::unordered_map<int, int> WeaponAttachmentItemID;
            static std::mutex weaponMutex;
            std::unordered_map<int, int> weaponAttachmentLocal;
            {
                std::lock_guard<std::mutex> lock(weaponMutex);
                weaponAttachmentLocal = WeaponAttachmentItemID;
            }

            Throttlered.executeTask("RefreshWeaponAttach", std::chrono::milliseconds(3000), []() {
                std::thread([]() {
                    auto hScatterWeapon = mem.CreateScatterHandle();
                    auto items = GetWeaponAttachedItems(hScatterWeapon);
                    mem.CloseScatterHandle(hScatterWeapon);
                    
                    std::lock_guard<std::mutex> lock(weaponMutex);
                    WeaponAttachmentItemID = std::move(items);
                }).detach();
            });

            Throttlered.executeTask("UpdateItemsData", std::chrono::milliseconds(2500), [&hScatter, &weaponAttachmentLocal, &CacheItems] {
                //Timer time("Update");

                std::unordered_map<uint64_t, DroppedItemInfo> CacheDroppedItems = Data::GetCacheDroppedItems();
                std::unordered_map<uint64_t, DroppedItemGroupInfo> CacheDroppedItemGroups = Data::GetCacheDroppedItemGroups();

                //��Ʒ��
                {
                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        mem.AddScatterRead(hScatter, Item.second.Entity + Offset::DroppedItemGroup, (uint64_t*)&Item.second.pDroppedItemGroup);
                        mem.AddScatterRead(hScatter, Item.second.Entity + Offset::DroppedItemGroup + 0x8, (int*)&Item.second.Count);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        if (Item.second.Count <= 0 || Item.second.Count > 100)
                        {
                            continue;
                        }

                        for (int i = 0; i < Item.second.Count; i++)
                        {
                            DroppedItemGroupItemInfo DroppedItemGroupItem;
                            DroppedItemGroupItem.pItemGroupComponent = Item.second.pDroppedItemGroup + i * 0x10;
                            Item.second.Items.push_back(DroppedItemGroupItem);
                        }
                    }

                    std::vector<DroppedItemGroupItemInfo*> groupItems;
                    for (auto& Item : CacheDroppedItemGroups) {
                        for (auto& DroppedItemGroupItem : Item.second.Items) {
                            groupItems.push_back(&DroppedItemGroupItem);
                        }
                    }

                    // [FIX STUTTER]: Limit items on table in training room to prevent DMA timeout
                    if (groupItems.size() > 800) groupItems.resize(800);

                    const size_t batchSize = 250;
                    for (size_t i = 0; i < groupItems.size(); i += batchSize) {
                        size_t end = (std::min)(i + batchSize, groupItems.size());
                        
                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, groupItems[j]->pItemGroupComponent, (uint64_t*)&groupItems[j]->ItemGroupComponent);
                        }
                        mem.ExecuteReadScatter(hScatter);
                        
                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, groupItems[j]->ItemGroupComponent + Offset::DroppedItemGroupUItem, (uint64_t*)&groupItems[j]->ItemGroupUItem);
                        }
                        mem.ExecuteReadScatter(hScatter);
                        
                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, groupItems[j]->ItemGroupUItem + Offset::ItemTable, (uint64_t*)&groupItems[j]->ItemTable);
                        }
                        mem.ExecuteReadScatter(hScatter);
                        
                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, groupItems[j]->ItemTable + Offset::ItemID, (int*)&groupItems[j]->ItemID);
                        }
                        mem.ExecuteReadScatter(hScatter);
                    }
                }

                {
                    std::vector<DroppedItemInfo*> droppedItems;
                    for (auto& Item : CacheDroppedItems) {
                        droppedItems.push_back(&Item.second);
                    }

                    if (droppedItems.size() > 800) droppedItems.resize(800);

                    const size_t batchSize = 250;
                    for (size_t i = 0; i < droppedItems.size(); i += batchSize) {
                        size_t end = (std::min)(i + batchSize, droppedItems.size());
                        
                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, droppedItems[j]->Entity + Offset::DroppedItem, (uint64_t*)&droppedItems[j]->pDroppedItem);
                            mem.AddScatterRead(hScatter, droppedItems[j]->Entity + Offset::RootComponent, (uint64_t*)&droppedItems[j]->RootComponent);
                        }
                        mem.ExecuteReadScatter(hScatter);

                        for (size_t j = i; j < end; ++j) {
                            droppedItems[j]->RootComponent = Decrypt::Xe(droppedItems[j]->RootComponent);
                            droppedItems[j]->pDroppedItem = Decrypt::Xe(droppedItems[j]->pDroppedItem);
                            mem.AddScatterRead(hScatter, droppedItems[j]->pDroppedItem + Offset::ItemTable, (uint64_t*)&droppedItems[j]->ItemTable);
                        }
                        mem.ExecuteReadScatter(hScatter);

                        for (size_t j = i; j < end; ++j) {
                            mem.AddScatterRead(hScatter, droppedItems[j]->ItemTable + Offset::ItemID, (int*)&droppedItems[j]->ID);
                        }
                        mem.ExecuteReadScatter(hScatter);
                    }
                }

                std::unordered_map<uint64_t, ItemInfo> AddCacheItems;

                for (auto& Item : CacheDroppedItemGroups)
                {
                    for (auto& DroppedItemGroupItem : Item.second.Items)
                    {
                        if (weaponAttachmentLocal.count(DroppedItemGroupItem.ItemID) > 0)
                        {
                            continue;
                        }

                        if (DroppedItemGroupItem.ItemID == 0) continue;

                        ItemInfo ItemInfo;
                        ItemInfo.Entity = DroppedItemGroupItem.pItemGroupComponent;
                        ItemInfo.ID = DroppedItemGroupItem.ItemID;
                        ItemInfo.RootComponent = DroppedItemGroupItem.ItemGroupComponent;
                        ItemInfo.ItemTable = DroppedItemGroupItem.ItemTable;

                        AddCacheItems[ItemInfo.Entity] = ItemInfo;
                    }
                }

                for (auto& Item : CacheDroppedItems)
                {
                    if (weaponAttachmentLocal.count(Item.second.ID) > 0)
                    {
                        continue;
                    }

                    if (Item.second.ID == 0) continue;

                    ItemInfo ItemInfo;
                    ItemInfo.Entity = Item.second.Entity;
                    ItemInfo.ID = Item.second.ID;
                    ItemInfo.RootComponent = Item.second.RootComponent;
                    ItemInfo.ItemTable = Item.second.ItemTable;

                    AddCacheItems[ItemInfo.Entity] = ItemInfo;
                }

                CacheItems.clear();
                std::vector<int> NeedGetNameIDs;
                for (auto& Item : AddCacheItems)
                {
                    EntityInfo EntityInfo = Data::GetGNameListsByIDItem(Item.second.ID);

                    //printf(U8("EntityInfo [%s]  [%s]  [%d]\n"), EntityInfo.DisplayName.c_str(), EntityInfo.Name.c_str(), Item.second.ID);

                    if (EntityInfo.ID == 0)
                    {
                        NeedGetNameIDs.push_back(Item.second.ID);
                    }
                    else {
                        if (EntityInfo.Type != EntityType::Item) continue;

                        Item.second.Type = EntityInfo.Type;
                        Item.second.DisplayName = EntityInfo.DisplayName;
                        Item.second.Name = EntityInfo.Name;
                        Item.second.ItemType = EntityInfo.WeaponType;

                        CacheItems[Item.second.Entity] = Item.second;
                    }
                }

                if (NeedGetNameIDs.size() > 0) {
                    std::sort(NeedGetNameIDs.begin(), NeedGetNameIDs.end());
                    NeedGetNameIDs.erase(std::unique(NeedGetNameIDs.begin(), NeedGetNameIDs.end()), NeedGetNameIDs.end());

                    if (NeedGetNameIDs.size() > 200) NeedGetNameIDs.resize(200);
                    std::vector<int> localIDs = NeedGetNameIDs;
                    std::thread([localIDs]() {
                        GNames::ReadGNames(const_cast<std::vector<int>&>(localIDs));
                    }).detach();
                }

                //std::cout << time.get() << std::endl;
                });

            std::vector<ItemInfo*> activeItems;
            for (auto& Item : CacheItems)
            {
                if (weaponAttachmentLocal.count(Item.second.ID) > 0)
                {
                    Item.second.bHidden = true;
                    continue;
                }
                activeItems.push_back(&Item.second);
            }

            // [FIX STUTTER]: Batch ScatterReads into chunks of 250 to avoid locking DMA PCIe bus
            const size_t batchSize = 250;
            for (size_t i = 0; i < activeItems.size(); i += batchSize) {
                size_t end = (std::min)(i + batchSize, activeItems.size());
                for (size_t j = i; j < end; ++j) {
                    ItemInfo* pItem = activeItems[j];
                    mem.AddScatterRead(hScatter, pItem->RootComponent + Offset::ComponentLocation, (FVector*)&pItem->Location);
                }
                if (end > i) mem.ExecuteReadScatter(hScatter);
            }

            for (auto& Item : CacheItems)
            {
                Item.second.ScreenLocation = VectorHelper::WorldToScreen(Item.second.Location);
                Item.second.Distance = Data::GetCamera().Location.Distance(Item.second.Location) / 100.0f;
            }

            Data::SetItems(std::move(CacheItems));

            auto endTime = std::chrono::high_resolution_clock::now();
            GameData.Performance.ItemThreadMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

            // Items thay doi cham - giam sleep xuong 100ms de update chi so mem va Item scan min khung nhat co the
            Sleep(100);
        }
        mem.CloseScatterHandle(hScatter);
    }
};