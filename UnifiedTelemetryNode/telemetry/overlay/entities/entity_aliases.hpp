#pragma once

#include "../vehicle/vehicle_resolver.hpp"
#include <string>

namespace EntityAliases {

std::string ResolveItemAsset(const std::string& rawName);
bool ResolveVehicle(const std::string& rawName, std::string& iconName,
                    VehicleResolver::Category& category);
void Reload();
const std::string& LoadedPath();

} // namespace EntityAliases
