#include "overlay_texture_cache.hpp"

#include "../loot/loot_icon_resolver.hpp"
#include "../vehicle/vehicle_resolver.hpp"
#include <protec/skCrypt.h>

#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "../../sdk/third_party/stb/stb_image.h"

namespace {

ID3D11Device* g_device = nullptr;
std::map<std::string, TextureInfo> g_weaponImages;
std::map<std::string, TextureInfo> g_vehicleIcons;
std::map<std::string, TextureInfo> g_itemIcons;

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** outSrv,
                         int* outWidth, int* outHeight) {
    if (!g_device || !outSrv || !outWidth || !outHeight) return false;

    int imageWidth = 0;
    int imageHeight = 0;
    unsigned char* imageData = stbi_load(filename, &imageWidth, &imageHeight, nullptr, 4);
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

    *outWidth = imageWidth;
    *outHeight = imageHeight;
    return true;
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
    const std::string fileName = asset + skCrypt(".png");
    const std::string basePath = skCrypt("Assets/") + folder + skCrypt("/");

    const std::string p1 = basePath + fileName;
    const std::string p2 = skCrypt("../") + p1;
    const std::string p3 = skCrypt("../../") + p1;

    if (LoadTextureFromFile(p1.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(p2.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(p3.c_str(), &srv, &w, &h)) {
        g_itemIcons[cacheKey] = { srv, w, h };
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
        &PreviewInstructor.SRV, &PreviewInstructor.Width, &PreviewInstructor.Height);
}

TextureInfo* GetVehicleIcon(const std::string& name) {
    auto cached = g_vehicleIcons.find(name);
    if (cached != g_vehicleIcons.end()) return &cached->second;

    ID3D11ShaderResourceView* srv = nullptr;
    int w = 0;
    int h = 0;
    const VehicleResolver::VehicleInfo info = VehicleResolver::Resolve(name);
    const std::string cleanName = info.IconName;

    const std::string path1 = skCrypt("Assets/Vehicle/") + cleanName + skCrypt(".png");
    const std::string path2 = skCrypt("../Assets/Vehicle/") + cleanName + skCrypt(".png");

    if (LoadTextureFromFile(path1.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(path2.c_str(), &srv, &w, &h)) {
        g_vehicleIcons[name] = { srv, w, h };
        return &g_vehicleIcons[name];
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
        if (LoadTextureFromFile(path.c_str(), &srv, &w, &h)) {
            g_itemIcons[name] = { srv, w, h };
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
    const std::string fileName = weaponName + skCrypt(".png");
    const std::string path1 = skCrypt("Assets/Weapon/") + fileName;
    const std::string path2 = skCrypt("../Assets/Weapon/") + fileName;
    const std::string path3 = skCrypt("../../Assets/Weapon/") + fileName;

    if (LoadTextureFromFile(path1.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(path2.c_str(), &srv, &w, &h) ||
        LoadTextureFromFile(path3.c_str(), &srv, &w, &h)) {
        g_weaponImages[weaponName] = { srv, w, h };
        return &g_weaponImages[weaponName];
    }

    g_weaponImages[weaponName] = {};
    return &g_weaponImages[weaponName];
}

} // namespace OverlayTextures
