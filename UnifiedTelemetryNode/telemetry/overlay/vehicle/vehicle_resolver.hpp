#pragma once

#include <string>

namespace VehicleResolver {

enum class Category {
    Unknown,
    Uaz,
    Dacia,
    Buggy,
    Bike,
    Boat,
    Brdm,
    Scooter,
    Snow,
    Tuk,
    Bus,
    Truck,
    Train,
    Mirado,
    Pickup,
    Rony,
    Blanc,
    Air
};

struct VehicleInfo {
    std::string IconName;
    Category CategoryId = Category::Unknown;
    bool Known = false;
};

VehicleInfo Resolve(const std::string& className);
float CloseHideDistanceMeters();
bool ShouldHideAtDistance(float distanceMeters);

} // namespace VehicleResolver
