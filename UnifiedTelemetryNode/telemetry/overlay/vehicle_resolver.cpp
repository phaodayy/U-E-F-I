#include "vehicle_resolver.hpp"

namespace {

constexpr float kCloseHideDistanceMeters = 20.0f;

bool Contains(const std::string& value, const char* token) {
    return value.find(token) != std::string::npos;
}

std::string StripPng(std::string value) {
    if (value.size() > 4 && value.compare(value.size() - 4, 4, ".png") == 0) {
        value.resize(value.size() - 4);
    }
    return value;
}

VehicleResolver::VehicleInfo Make(const char* iconName, VehicleResolver::Category category) {
    return { iconName, category, true };
}

} // namespace

namespace VehicleResolver {

VehicleInfo Resolve(const std::string& className) {
    const std::string name = StripPng(className);

    if (Contains(name, "AquaRail")) return Make("AquaRail_A_00_C", Category::Boat);
    if (Contains(name, "Rubber_boat") || Contains(name, "Rubberboat")) return Make("Item_Rubberboat_C", Category::Boat);
    if (Contains(name, "Boat") || Contains(name, "PG117") || Contains(name, "AirBoat")) return Make("Boat_PG117_C", Category::Boat);

    if (Contains(name, "BRDM")) return Make("BP_BRDM_C", Category::Brdm);
    if (Contains(name, "Motorglider") || Contains(name, "Aircraft") || Contains(name, "TransportAircraft")) {
        return Make("BP_Motorglider_C", Category::Air);
    }

    if (Contains(name, "Uaz_B")) return Make("Uaz_B_00_C", Category::Uaz);
    if (Contains(name, "Uaz_C") || Contains(name, "Uaz_Pillar")) return Make("Uaz_C_00_C", Category::Uaz);
    if (Contains(name, "Uaz")) return Make("Uaz_A_00_C", Category::Uaz);

    if (Contains(name, "Dacia")) return Make("Dacia_A_00_v2_C", Category::Dacia);
    if (Contains(name, "Buggy")) return Make("Buggy_A_01_C", Category::Buggy);
    if (Contains(name, "ATV")) return Make("Buggy_A_01_C", Category::Buggy);

    if (Contains(name, "Motorbike_04_SideCar")) return Make("BP_Motorbike_00_SideCar_C", Category::Bike);
    if (Contains(name, "Motorbike") || Contains(name, "Dirtbike") || Contains(name, "Panigale")) {
        return Make("BP_Motorbike_04_C", Category::Bike);
    }
    if (Contains(name, "Bicycle") || Contains(name, "Mountainbike")) return Make("Item_Mountainbike_C", Category::Bike);

    if (Contains(name, "Scooter")) return Make("BP_Scooter_00_A_C", Category::Scooter);
    if (Contains(name, "Snowbike")) return Make("BP_Snowbike_00_C", Category::Snow);
    if (Contains(name, "Snowmobile")) return Make("BP_Snowmobile_00_C", Category::Snow);
    if (Contains(name, "Niva")) return Make("BP_Niva_00_C", Category::Snow);
    if (Contains(name, "Tuk")) return Make("BP_TukTukTuk_A_00_C", Category::Tuk);

    if (Contains(name, "PicoBus")) return Make("BP_PicoBus_C", Category::Bus);
    if (Contains(name, "MiniBus")) return Make("BP_MiniBus_C", Category::Bus);
    if (Contains(name, "Van")) return Make("BP_Van_A_00_C", Category::Bus);

    if (Contains(name, "Porter")) return Make("BP_PickupTruck_B_00_C", Category::Pickup);
    if (Contains(name, "Pillar_Car")) return Make("BP_PickupTruck_A_00_C", Category::Pickup);
    if (Contains(name, "LootTruck") || Contains(name, "Food_Truck")) return Make("BP_LootTruck_C", Category::Truck);
    if (Contains(name, "Circle_Train")) return Make("BP_DO_Circle_Train_Merged_C", Category::Train);
    if (Contains(name, "Line_Train_Dino")) return Make("BP_DO_Line_Train_Dino_Merged_C", Category::Train);
    if (Contains(name, "Line_Train") || Contains(name, "Train")) return Make("BP_DO_Line_Train_Merged_C", Category::Train);

    if (Contains(name, "Mirado_Open")) return Make("BP_Mirado_Open_00_C", Category::Mirado);
    if (Contains(name, "Mirado") || Contains(name, "PonyCoupe")) return Make("BP_Mirado_A_00_C", Category::Mirado);
    if (Contains(name, "PickupTruck_B")) return Make("BP_PickupTruck_B_00_C", Category::Pickup);
    if (Contains(name, "PickupTruck")) return Make("BP_PickupTruck_A_00_C", Category::Pickup);
    if (Contains(name, "Rony")) return Make("BP_M_Rony_A_00_C", Category::Rony);
    if (Contains(name, "Blanc")) return Make("BP_Blanc_C", Category::Blanc);

    if (Contains(name, "CoupeRB")) return Make("BP_Mirado_A_00_C", Category::Mirado);
    if (Contains(name, "BP_McLaren") || Contains(name, "BP_DBX") || Contains(name, "BP_Vantage") ||
        Contains(name, "BP_Urus") || Contains(name, "BP_Countach") || Contains(name, "BP_Classic")) {
        return Make("BP_Mirado_A_00_C", Category::Mirado);
    }

    return { name, Category::Unknown, false };
}

float CloseHideDistanceMeters() {
    return kCloseHideDistanceMeters;
}

bool ShouldHideAtDistance(float distanceMeters) {
    return distanceMeters > 0.0f && distanceMeters <= kCloseHideDistanceMeters;
}

} // namespace VehicleResolver
