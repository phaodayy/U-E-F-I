#include "entity_aliases.hpp"

#include "../../sdk/core/embedded_resources.hpp"
#include "../../../nlohmann/json.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>

namespace {

struct VehicleAlias {
    std::string Canonical;
    VehicleResolver::Category Category = VehicleResolver::Category::Unknown;
    std::vector<std::string> Exact;
    std::vector<std::string> Contains;
};

struct ItemAlias {
    std::string Canonical;
    std::vector<std::string> Exact;
    std::vector<std::string> Contains;
};

struct AliasState {
    bool Loaded = false;
    std::string Path;
    std::vector<VehicleAlias> Vehicles;
    std::vector<ItemAlias> Items;
};

AliasState g_state;

std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string StripPng(std::string value) {
    if (value.size() > 4 && ToLower(value.substr(value.size() - 4)) == ".png") {
        value.resize(value.size() - 4);
    }
    return value;
}

std::vector<std::string> ReadStringArray(const nlohmann::json& obj, const char* key) {
    std::vector<std::string> out;
    if (!obj.contains(key) || !obj[key].is_array()) return out;

    for (const auto& value : obj[key]) {
        if (value.is_string()) out.push_back(ToLower(StripPng(value.get<std::string>())));
    }
    return out;
}

VehicleResolver::Category ParseCategory(const std::string& value) {
    const std::string name = ToLower(value);
    if (name == "uaz") return VehicleResolver::Category::Uaz;
    if (name == "dacia") return VehicleResolver::Category::Dacia;
    if (name == "buggy") return VehicleResolver::Category::Buggy;
    if (name == "bike") return VehicleResolver::Category::Bike;
    if (name == "boat") return VehicleResolver::Category::Boat;
    if (name == "brdm") return VehicleResolver::Category::Brdm;
    if (name == "scooter") return VehicleResolver::Category::Scooter;
    if (name == "snow") return VehicleResolver::Category::Snow;
    if (name == "tuk") return VehicleResolver::Category::Tuk;
    if (name == "bus") return VehicleResolver::Category::Bus;
    if (name == "truck") return VehicleResolver::Category::Truck;
    if (name == "train") return VehicleResolver::Category::Train;
    if (name == "mirado") return VehicleResolver::Category::Mirado;
    if (name == "pickup") return VehicleResolver::Category::Pickup;
    if (name == "rony") return VehicleResolver::Category::Rony;
    if (name == "blanc") return VehicleResolver::Category::Blanc;
    if (name == "air") return VehicleResolver::Category::Air;
    return VehicleResolver::Category::Unknown;
}

bool TryOpen(const char* path, std::ifstream& file) {
    file.open(path);
    if (!file.is_open()) return false;
    g_state.Path = path;
    return true;
}

void LoadAliases() {
    if (g_state.Loaded) return;
    g_state.Loaded = true;
    g_state.Vehicles.clear();
    g_state.Items.clear();
    g_state.Path.clear();

    nlohmann::json root;
    std::string embeddedAliases;
    if (EmbeddedResources::LoadText("Assets/entity_aliases.json", embeddedAliases)) {
        g_state.Path = "embedded:Assets/entity_aliases.json";
        root = nlohmann::json::parse(embeddedAliases, nullptr, false);
    } else {
        std::ifstream file;
        if (!TryOpen("Assets/entity_aliases.json", file) &&
            !TryOpen("../Assets/entity_aliases.json", file) &&
            !TryOpen("../../Assets/entity_aliases.json", file)) {
            return;
        }

        root = nlohmann::json::parse(file, nullptr, false);
    }
    if (root.is_discarded()) return;

    if (root.contains("vehicles") && root["vehicles"].is_array()) {
        for (const auto& item : root["vehicles"]) {
            if (!item.contains("canonical") || !item["canonical"].is_string()) continue;
            VehicleAlias alias;
            alias.Canonical = StripPng(item["canonical"].get<std::string>());
            alias.Category = ParseCategory(item.value("category", std::string{}));
            alias.Exact = ReadStringArray(item, "aliases");
            alias.Contains = ReadStringArray(item, "contains");
            alias.Exact.push_back(ToLower(alias.Canonical));
            g_state.Vehicles.push_back(alias);
        }
    }

    if (root.contains("items") && root["items"].is_array()) {
        for (const auto& item : root["items"]) {
            if (!item.contains("canonical") || !item["canonical"].is_string()) continue;
            ItemAlias alias;
            alias.Canonical = StripPng(item["canonical"].get<std::string>());
            alias.Exact = ReadStringArray(item, "aliases");
            alias.Contains = ReadStringArray(item, "contains");
            alias.Exact.push_back(ToLower(alias.Canonical));
            g_state.Items.push_back(alias);
        }
    }
}

bool MatchExact(const std::vector<std::string>& values, const std::string& name) {
    return std::find(values.begin(), values.end(), name) != values.end();
}

bool MatchContains(const std::vector<std::string>& values, const std::string& name) {
    for (const std::string& value : values) {
        if (!value.empty() && name.find(value) != std::string::npos) return true;
    }
    return false;
}

} // namespace

namespace EntityAliases {

std::string ResolveItemAsset(const std::string& rawName) {
    LoadAliases();
    const std::string name = ToLower(StripPng(rawName));
    for (const ItemAlias& alias : g_state.Items) {
        if (MatchExact(alias.Exact, name) || MatchContains(alias.Contains, name)) {
            return alias.Canonical;
        }
    }
    return StripPng(rawName);
}

bool ResolveVehicle(const std::string& rawName, std::string& iconName,
                    VehicleResolver::Category& category) {
    LoadAliases();
    const std::string name = ToLower(StripPng(rawName));
    for (const VehicleAlias& alias : g_state.Vehicles) {
        if (MatchExact(alias.Exact, name) || MatchContains(alias.Contains, name)) {
            iconName = alias.Canonical;
            category = alias.Category;
            return true;
        }
    }
    return false;
}

void Reload() {
    g_state.Loaded = false;
    LoadAliases();
}

const std::string& LoadedPath() {
    LoadAliases();
    return g_state.Path;
}

} // namespace EntityAliases
