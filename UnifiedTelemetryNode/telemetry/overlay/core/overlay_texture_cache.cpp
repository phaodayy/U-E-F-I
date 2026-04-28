#include "overlay_texture_cache.hpp"

#include "../loot/loot_icon_resolver.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include "../../sdk/core/embedded_resources.hpp"
#include <protec/skCrypt.h>

#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../../sdk/third_party/stb/stb_image.h"

namespace {

ID3D11Device* g_device = nullptr;
std::map<std::string, TextureInfo> g_weaponImages;
std::map<std::string, TextureInfo> g_mapIcons;
std::map<std::string, TextureInfo> g_vehicleIcons;
std::map<std::string, TextureInfo> g_itemIcons;
constexpr int kAnimatedSpriteFrames = 8;

char ToLowerAscii(char value) {
    if (value >= 'A' && value <= 'Z') return static_cast<char>(value + ('a' - 'A'));
    return value;
}

bool EndsWithPng(const std::string& value) {
    constexpr const char* suffix = ".png";
    constexpr std::size_t suffixLen = 4;
    if (value.size() < suffixLen) return false;
    for (std::size_t i = 0; i < suffixLen; ++i) {
        if (ToLowerAscii(value[value.size() - suffixLen + i]) != suffix[i]) return false;
    }
    return true;
}

bool StartsWith(const std::string& value, const char* prefix) {
    const std::size_t prefixLen = std::char_traits<char>::length(prefix);
    return value.size() >= prefixLen && value.compare(0, prefixLen, prefix) == 0;
}

bool EndsWith(const std::string& value, const char* suffix) {
    const std::size_t suffixLen = std::char_traits<char>::length(suffix);
    return value.size() >= suffixLen &&
        value.compare(value.size() - suffixLen, suffixLen, suffix) == 0;
}

std::string StripPngExtension(std::string value) {
    if (EndsWithPng(value)) value.resize(value.size() - 4);
    return value;
}

std::string Basename(std::string value) {
    const std::size_t slash = value.find_last_of("/\\");
    if (slash != std::string::npos) value.erase(0, slash + 1);
    return StripPngExtension(value);
}

std::string StripWeaponClassAffixes(std::string value) {
    value = Basename(value);
    if (StartsWith(value, "Item_Weapon_")) value.erase(0, 12);
    if (StartsWith(value, "Weap")) value.erase(0, 4);
    if (EndsWith(value, "_C")) value.resize(value.size() - 2);
    return value;
}

std::string AliasKey(const std::string& value) {
    std::string stripped = StripWeaponClassAffixes(value);
    std::string key;
    key.reserve(stripped.size());
    for (char c : stripped) {
        if (c == '_' || c == '-' || c == ' ') continue;
        key.push_back(ToLowerAscii(c));
    }
    return key;
}

void AddCandidate(std::vector<std::string>& out, std::string value) {
    value = Basename(value);
    if (value.empty()) return;
    if (std::find(out.begin(), out.end(), value) == out.end()) {
        out.push_back(value);
    }
}

std::string WeaponAssetAlias(const std::string& weaponName) {
    struct Alias {
        const char* Key;
        const char* Asset;
    };
    static const Alias aliases[] = {
        { "ace32", "ACE" },
        { "ak47", "AKM" },
        { "berreta686", "S686" },
        { "bizon", "野牛PP19" },
        { "bizonpp19", "野牛PP19" },
        { "cowbar", "撬棍" },
        { "crossbow", "弩" },
        { "crowbar", "撬棍" },
        { "deagle", "沙漠之鹰" },
        { "decoygrenade", "诱饵手雷" },
        { "deserteagle", "沙漠之鹰" },
        { "dp12", "DBS" },
        { "duncanshk416", "M416" },
        { "famas", "FAMASI" },
        { "famasg2", "FAMASI" },
        { "flashbang", "闪光弹" },
        { "flaregun", "信号枪" },
        { "fnfal", "SLR" },
        { "fraggrenade", "手雷" },
        { "g18", "P18C" },
        { "grenade", "手雷" },
        { "hk416", "M416" },
        { "l6", "Lynx" },
        { "lunchmeatsak47", "AKM" },
        { "m9", "P92" },
        { "madsfnfal", "SLR" },
        { "madsqbu88", "QBU" },
        { "machete", "砍刀" },
        { "microuzi", "UZI" },
        { "mk47mutant", "Mk47" },
        { "molotov", "燃烧瓶" },
        { "mosin", "莫辛甘纳" },
        { "mosinnagant", "莫辛甘纳" },
        { "mutant", "Mk47" },
        { "nagantm1895", "R1895" },
        { "origin12", "O12" },
        { "origins12", "O12" },
        { "pan", "平底锅" },
        { "panzerfaust", "RPG" },
        { "pp19", "野牛PP19" },
        { "qbu88", "QBU" },
        { "qbz", "QBZ95" },
        { "rhino", "R45" },
        { "saiga12", "S12K" },
        { "sawedoff", "SawedOff" },
        { "sawnoff", "SawedOff" },
        { "scarl", "SCAR-L" },
        { "sickle", "镰刀" },
        { "skorpion", "蝎式手枪" },
        { "smokebomb", "SmokeGrenade" },
        { "smokegrenade", "SmokeGrenade" },
        { "spiketrap", "SpikeTrap" },
        { "stickygrenade", "粘性炸弹" },
        { "thompson", "汤姆逊" },
        { "tommygun", "汤姆逊" },
        { "ump45", "UMP" },
        { "win1894", "Win94" },
        { "winchester", "Win94" },
        { "vz61skorpion", "蝎式手枪" },
    };

    const std::string key = AliasKey(weaponName);
    for (const Alias& alias : aliases) {
        if (key == alias.Key) return alias.Asset;
    }
    return {};
}

std::vector<std::string> WeaponIconCandidates(const std::string& weaponName) {
    std::vector<std::string> candidates;
    AddCandidate(candidates, weaponName);
    AddCandidate(candidates, StripWeaponClassAffixes(weaponName));
    AddCandidate(candidates, WeaponAssetAlias(weaponName));
    return candidates;
}

std::vector<std::string> VehicleIconCandidates(const std::string& name) {
    std::vector<std::string> candidates;
    AddCandidate(candidates, name);
    const VehicleResolver::VehicleInfo info = VehicleResolver::Resolve(name);
    AddCandidate(candidates, info.IconName);
    return candidates;
}

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** outSrv,
                         int* outWidth, int* outHeight, int* outFrames = nullptr);

bool LoadTextureFromAssetFolders(const char* folder, const std::string& asset,
                                 ID3D11ShaderResourceView** outSrv,
                                 int* outWidth, int* outHeight, int* outFrames = nullptr) {
    if (asset.empty()) return false;
    const std::string fileName = asset + skCrypt(".png");
    const std::string path1 = skCrypt("Assets/") + std::string(folder) + skCrypt("/") + fileName;
    const std::string path2 = skCrypt("../") + path1;
    const std::string path3 = skCrypt("../../") + path1;
    return LoadTextureFromFile(path1.c_str(), outSrv, outWidth, outHeight, outFrames) ||
        LoadTextureFromFile(path2.c_str(), outSrv, outWidth, outHeight, outFrames) ||
        LoadTextureFromFile(path3.c_str(), outSrv, outWidth, outHeight, outFrames);
}

std::string AnimatedPathFor(const char* filename) {
    if (!filename) return {};
    std::string path(filename);
    std::string lower = path;
    for (char& c : lower) {
        c = ToLowerAscii(c);
        if (c == '\\') c = '/';
    }

    const std::string needle = skCrypt("assets/");
    const std::size_t pos = lower.find(needle);
    if (pos == std::string::npos) return {};

    path.replace(pos, needle.size(), skCrypt("AssetsAnimated/"));
    return path;
}

bool DecodeTextureFromFile(const char* filename, ID3D11ShaderResourceView** outSrv,
                           int* outWidth, int* outHeight, int* outFrames, bool animatedSheet) {
    if (!g_device || !outSrv || !outWidth || !outHeight) return false;

    int imageWidth = 0;
    int imageHeight = 0;
    unsigned char* imageData = nullptr;

    const unsigned char* resourceData = nullptr;
    std::size_t resourceSize = 0;
    if (EmbeddedResources::LoadBinary(filename, resourceData, resourceSize) &&
        resourceSize <= static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        imageData = stbi_load_from_memory(resourceData, static_cast<int>(resourceSize),
            &imageWidth, &imageHeight, nullptr, 4);
    }

    if (!imageData) {
        imageData = stbi_load(filename, &imageWidth, &imageHeight, nullptr, 4);
    }
    if (!imageData) return false;

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = imageWidth;
    desc.Height = imageHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subResource = {};
    subResource.pSysMem = imageData;
    subResource.SysMemPitch = desc.Width * 4;

    ID3D11Texture2D* texture = nullptr;
    if (FAILED(g_device->CreateTexture2D(&desc, &subResource, &texture))) {
        stbi_image_free(imageData);
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    const bool ok = SUCCEEDED(g_device->CreateShaderResourceView(texture, &srvDesc, outSrv));
    texture->Release();
    stbi_image_free(imageData);
    if (!ok) return false;

    int frames = animatedSheet ? kAnimatedSpriteFrames : 1;
    if (frames > 1 && imageWidth >= frames && (imageWidth % frames) == 0) {
        *outWidth = imageWidth / frames;
    } else {
        frames = 1;
        *outWidth = imageWidth;
    }
    *outHeight = imageHeight;
    if (outFrames) *outFrames = frames;
    return true;
}

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** outSrv,
                         int* outWidth, int* outHeight, int* outFrames) {
    if (outFrames) *outFrames = 1;

    const std::string animatedPath = AnimatedPathFor(filename);
    if (!animatedPath.empty() &&
        DecodeTextureFromFile(animatedPath.c_str(), outSrv, outWidth, outHeight, outFrames, true)) {
        return true;
    }

    return DecodeTextureFromFile(filename, outSrv, outWidth, outHeight, outFrames, false);
}

} // namespace

TextureInfo PreviewInstructor;

TextureInfo* GetPreviewIcon(std::string folder, std::string asset) {
    const std::string cacheKey = folder + skCrypt(":") + asset;
    auto cached = g_itemIcons.find(cacheKey);
    if (cached != g_itemIcons.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    int frames = 1;
    const std::string fileName = asset + skCrypt(".png");
    const std::string basePath = skCrypt("Assets/") + folder + skCrypt("/");

    const std::string p1 = basePath + fileName;
    const std::string p2 = skCrypt("../") + p1;
    const std::string p3 = skCrypt("../../") + p1;

    if (LoadTextureFromFile(p1.c_str(), &srv, &w, &h, &frames) ||
        LoadTextureFromFile(p2.c_str(), &srv, &w, &h, &frames) ||
        LoadTextureFromFile(p3.c_str(), &srv, &w, &h, &frames)) {
        g_itemIcons[cacheKey] = { srv, w, h, frames };
        return &g_itemIcons[cacheKey];
    }

    g_itemIcons[cacheKey] = {};
    return &g_itemIcons[cacheKey];
}

namespace OverlayTextures {

void SetDevice(ID3D11Device* device) {
    g_device = device;
}

bool LoadInstructor() {
    return LoadTextureFromFile(skCrypt("Assets/All/Instructor.png"),
        &PreviewInstructor.SRV, &PreviewInstructor.Width, &PreviewInstructor.Height, &PreviewInstructor.Frames);
}

TextureInfo* GetMapIcon(const std::string& name) {
    auto cached = g_mapIcons.find(name);
    if (cached != g_mapIcons.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    int frames = 1;
    if (LoadTextureFromAssetFolders("Map", name, &srv, &w, &h, &frames)) {
        g_mapIcons[name] = { srv, w, h, frames };
        return &g_mapIcons[name];
    }

    g_mapIcons[name] = {};
    return &g_mapIcons[name];
}

TextureInfo* GetVehicleIcon(const std::string& name) {
    auto cached = g_vehicleIcons.find(name);
    if (cached != g_vehicleIcons.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    int frames = 1;

    for (const std::string& candidate : VehicleIconCandidates(name)) {
        if (LoadTextureFromAssetFolders("Vehicle", candidate, &srv, &w, &h, &frames)) {
            g_vehicleIcons[name] = { srv, w, h, frames };
            return &g_vehicleIcons[name];
        }
    }

    g_vehicleIcons[name] = {};
    return &g_vehicleIcons[name];
}

TextureInfo* GetItemIcon(const std::string& name) {
    TextureInfo* resolvedIcon = LootIconResolver::GetItemIcon(name);
    if (resolvedIcon && resolvedIcon->SRV) return resolvedIcon;

    auto cached = g_itemIcons.find(name);
    if (cached != g_itemIcons.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    int frames = 1;
    std::string fileName;

    if (name.find(skCrypt("Helmet")) != std::string::npos) {
        if (name.find(skCrypt("Lv3")) != std::string::npos) fileName = skCrypt("Item_Head_G_01_Lv3_C");
        else if (name.find(skCrypt("Lv2")) != std::string::npos) fileName = skCrypt("Item_Head_F_01_Lv2_C");
        else fileName = skCrypt("Item_Head_E_01_Lv1_C");
    } else if (name.find(skCrypt("Vest")) != std::string::npos ||
               name.find(skCrypt("Armor")) != std::string::npos) {
        if (name.find(skCrypt("Lv3")) != std::string::npos) fileName = skCrypt("Item_Armor_C_01_Lv3_C");
        else if (name.find(skCrypt("Lv2")) != std::string::npos) fileName = skCrypt("Item_Armor_D_01_Lv2_C");
        else fileName = skCrypt("Item_Armor_E_01_Lv1_C");
    } else if (name.find(skCrypt("First Aid")) != std::string::npos) fileName = skCrypt("Item_Heal_FirstAid_C");
    else if (name.find(skCrypt("Med Kit")) != std::string::npos) fileName = skCrypt("Item_Heal_MedKit_C");
    else if (name.find(skCrypt("Drink")) != std::string::npos) fileName = skCrypt("Item_Boost_EnergyDrink_C");
    else if (name.find(skCrypt("5.56")) != std::string::npos) fileName = skCrypt("Item_Ammo_556mm_C");
    else if (name.find(skCrypt("7.62")) != std::string::npos) fileName = skCrypt("Item_Ammo_762mm_C");
    else if (name.find(skCrypt("8x")) != std::string::npos) fileName = skCrypt("Item_Attach_Weapon_Upper_PM2_01_C");
    else if (name.find(skCrypt("6x")) != std::string::npos) fileName = skCrypt("Item_Attach_Weapon_Upper_Scope6x_C");
    else if (name.find(skCrypt("4x")) != std::string::npos) fileName = skCrypt("Item_Attach_Weapon_Upper_ACOG_01_C");

    if (!fileName.empty()) {
        const std::string path = skCrypt("Assets/All/") + fileName + skCrypt(".png");
        if (LoadTextureFromFile(path.c_str(), &srv, &w, &h, &frames)) {
            g_itemIcons[name] = { srv, w, h, frames };
            return &g_itemIcons[name];
        }
    }

    g_itemIcons[name] = {};
    return &g_itemIcons[name];
}

TextureInfo* GetWeaponImage(const std::string& weaponName) {
    auto cached = g_weaponImages.find(weaponName);
    if (cached != g_weaponImages.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    int frames = 1;

    for (const std::string& candidate : WeaponIconCandidates(weaponName)) {
        if (LoadTextureFromAssetFolders("Weapon", candidate, &srv, &w, &h, &frames)) {
            g_weaponImages[weaponName] = { srv, w, h, frames };
            return &g_weaponImages[weaponName];
        }
    }

    g_weaponImages[weaponName] = {};
    return &g_weaponImages[weaponName];
}

} // namespace OverlayTextures
