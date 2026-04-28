#pragma once

#include "overlay_menu.hpp"
#include <d3d11.h>
#include <string>

namespace OverlayTextures {

void SetDevice(ID3D11Device* device);
bool LoadInstructor();
TextureInfo* GetVehicleIcon(const std::string& name);
TextureInfo* GetItemIcon(const std::string& name);
TextureInfo* GetWeaponImage(const std::string& weaponName);

} // namespace OverlayTextures
