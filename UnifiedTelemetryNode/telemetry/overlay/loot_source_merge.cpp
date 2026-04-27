#include "loot_source_merge.hpp"

#include "../sdk/Common/Data.h"
#include <mutex>

namespace {

bool IsSameSourceItem(const ItemData& left, const ItemData& right) {
    if (left.Name != right.Name) return false;
    return left.Position.Distance(right.Position) < 30.0f;
}

ItemData FromRadarItem(const ItemInfo& item) {
    ItemData data{};
    data.Position = { item.Location.X, item.Location.Y, item.Location.Z };
    data.Name = item.Name;
    data.Distance = item.Distance;
    data.IsImportant = item.ItemType == WeaponType::AR ||
        item.ItemType == WeaponType::SR ||
        item.ItemType == WeaponType::DMR;
    return data;
}

} // namespace

namespace LootSourceMerge {

std::vector<ItemData> BuildAllLoot() {
    std::vector<ItemData> items;

    const auto dmaItems = Data::GetItems();
    items.reserve(dmaItems.size());
    for (const auto& pair : dmaItems) {
        items.push_back(FromRadarItem(pair.second));
    }

    const size_t primaryCount = items.size();
    std::vector<bool> consumedPrimary(primaryCount, false);

    std::lock_guard<std::mutex> lock(CachedItemsMutex);
    items.reserve(items.size() + CachedItems.size());
    for (const auto& actorItem : CachedItems) {
        bool duplicateOfPrimary = false;
        for (size_t i = 0; i < primaryCount; ++i) {
            if (consumedPrimary[i]) continue;
            if (!IsSameSourceItem(items[i], actorItem)) continue;

            consumedPrimary[i] = true;
            duplicateOfPrimary = true;
            break;
        }

        if (!duplicateOfPrimary) {
            items.push_back(actorItem);
        }
    }

    return items;
}

} // namespace LootSourceMerge
