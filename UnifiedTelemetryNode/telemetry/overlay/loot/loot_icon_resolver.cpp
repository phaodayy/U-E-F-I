#include "loot_icon_resolver.hpp"
#include "../entities/entity_aliases.hpp"

#include <map>
#include <string>

TextureInfo* GetPreviewIcon(std::string folder, std::string asset);

namespace {

std::string StripPngExtension(std::string name) {
    constexpr const char* suffix = ".png";
    constexpr size_t suffixLen = 4;
    if (name.size() >= suffixLen &&
        name.compare(name.size() - suffixLen, suffixLen, suffix) == 0) {
        name.resize(name.size() - suffixLen);
    }
    return name;
}

const char* const* ItemAssetFolders(size_t& count) {
    static const char* folders[] = {
        "Gun/AR",
        "Gun/SR",
        "Gun/DMR",
        "Gun/SMG",
        "Gun/LMG",
        "Gun/SG",
        "Gun/HG",
        "Gun/Special",
        "Gun/Throw",
        "Gun/Melee",
        "Ammo",
        "Medicine",
        "Armor",
        "Helmet",
        "Backpack",
        "Ghillie",
        "Utility",
        "Repair",
        "Key",
        "Attachment/Scope",
        "Attachment/Muzzle",
        "Attachment/Grip",
        "Attachment/Mag",
        "Attachment/Stock"
    };
    count = sizeof(folders) / sizeof(folders[0]);
    return folders;
}

} // namespace

namespace LootIconResolver {

TextureInfo* GetItemIcon(const std::string& itemName) {
    static std::map<std::string, TextureInfo*> resolvedIcons;
    static TextureInfo missingIcon{};

    if (itemName.empty()) return &missingIcon;

    const std::string asset = EntityAliases::ResolveItemAsset(StripPngExtension(itemName));
    auto cached = resolvedIcons.find(asset);
    if (cached != resolvedIcons.end()) {
        return cached->second ? cached->second : &missingIcon;
    }

    size_t folderCount = 0;
    const char* const* folders = ItemAssetFolders(folderCount);
    for (size_t i = 0; i < folderCount; ++i) {
        TextureInfo* icon = GetPreviewIcon(folders[i], asset);
        if (icon && icon->SRV) {
            resolvedIcons[asset] = icon;
            return icon;
        }
    }

    resolvedIcons[asset] = nullptr;
    return &missingIcon;
}

} // namespace LootIconResolver
