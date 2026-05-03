#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace FlickWeaponCatalog {

struct Weapon {
    const char* label;
    const char* folder;
    const char* asset;
    const char* key;
    const char* category_key;
    std::vector<const char*> aliases;
    bool default_enabled;
};

struct Category {
    const char* label;
    const char* key;
    const char* folder;
    const char* asset;
    bool default_enabled;
};

inline const std::vector<Category>& Categories() {
    static const std::vector<Category> categories = {
        { "Assault Rifle", "ar", "Gun/AR", "Item_Weapon_HK416_C", true },
        { "Marksman Rifle", "dmr", "Gun/DMR", "Item_Weapon_FNFal_C", true },
        { "Sniper Rifle", "sr", "Gun/SR", "Item_Weapon_Kar98k_C", true },
        { "Submachine Gun", "smg", "Gun/SMG", "Item_Weapon_UMP_C", true },
        { "Machine Gun", "lmg", "Gun/LMG", "Item_Weapon_M249_C", true },
        { "Shotgun", "sg", "Gun/SG", "Item_Weapon_Saiga12_C", true },
        { "Handgun", "hg", "Gun/HG", "Item_Weapon_G18_C", true },
        { "Special", "special", "Gun/Special", "Item_Weapon_PanzerFaust100M_C", true },
    };
    return categories;
}

inline const std::vector<Weapon>& All() {
    static const std::vector<Weapon> weapons = {
        { "ACE32", "Gun/AR", "Item_Weapon_ACE32_C", "ace32", "ar", { "ace32" }, true },
        { "AKM", "Gun/AR", "Item_Weapon_AK47_C", "akm", "ar", { "akm", "ak47", "lunchmeats_ak47" }, true },
        { "AUG", "Gun/AR", "Item_Weapon_AUG_C", "aug", "ar", { "aug" }, true },
        { "Beryl M762", "Gun/AR", "Item_Weapon_BerylM762_C", "m762", "ar", { "m762", "berylm762" }, true },
        { "M416", "Gun/AR", "Item_Weapon_HK416_C", "m416", "ar", { "m416", "hk416", "duncans_m416" }, true },
        { "FAMAS", "Gun/AR", "Item_Weapon_FAMASG2_C", "famasg2", "ar", { "famasg2", "famas" }, true },
        { "G36C", "Gun/AR", "Item_Weapon_G36C_C", "g36c", "ar", { "g36c" }, true },
        { "Groza", "Gun/AR", "Item_Weapon_Groza_C", "groza", "ar", { "groza" }, true },
        { "K2", "Gun/AR", "Item_Weapon_K2_C", "k2", "ar", { "k2" }, true },
        { "M16A4", "Gun/AR", "Item_Weapon_M16A4_C", "m16a4", "ar", { "m16a4" }, true },
        { "Mk47", "Gun/AR", "Item_Weapon_Mk47Mutant_C", "mk47mutant", "ar", { "mk47mutant", "mk47" }, true },
        { "QBZ95", "Gun/AR", "Item_Weapon_QBZ95_C", "qbz95", "ar", { "qbz95", "qbz" }, true },
        { "SCAR-L", "Gun/AR", "Item_Weapon_SCAR-L_C", "scar-l", "ar", { "scar-l", "scarl" }, true },

        { "Dragunov", "Gun/DMR", "Item_Weapon_Dragunov_C", "dragunov", "dmr", { "dragunov" }, true },
        { "SLR", "Gun/DMR", "Item_Weapon_FNFal_C", "slr", "dmr", { "slr", "fnfal", "madsfnfal", "mads_fnfal" }, true },
        { "Mini14", "Gun/DMR", "Item_Weapon_Mini14_C", "mini14", "dmr", { "mini14" }, true },
        { "Mk12", "Gun/DMR", "Item_Weapon_Mk12_C", "mk12", "dmr", { "mk12" }, true },
        { "Mk14", "Gun/DMR", "Item_Weapon_Mk14_C", "mk14", "dmr", { "mk14" }, true },
        { "QBU", "Gun/DMR", "Item_Weapon_QBU88_C", "qbu", "dmr", { "qbu", "qbu88", "madsqbu88", "mads_qbu88" }, true },
        { "SKS", "Gun/DMR", "Item_Weapon_SKS_C", "sks", "dmr", { "sks" }, true },
        { "VSS", "Gun/DMR", "Item_Weapon_VSS_C", "vss", "dmr", { "vss" }, true },

        { "AWM", "Gun/SR", "Item_Weapon_AWM_C", "awm", "sr", { "awm" }, true },
        { "Kar98k", "Gun/SR", "Item_Weapon_Kar98k_C", "kar98k", "sr", { "kar98k", "julieskar98k" }, true },
        { "Lynx AMR", "Gun/SR", "Item_Weapon_L6_C", "lynx", "sr", { "lynx", "l6" }, true },
        { "M24", "Gun/SR", "Item_Weapon_M24_C", "m24", "sr", { "m24", "juliesm24" }, true },
        { "Mosin", "Gun/SR", "Item_Weapon_Mosin_C", "mosin", "sr", { "mosin", "mosinnagant" }, true },
        { "Win94", "Gun/SR", "Item_Weapon_Win1894_C", "win94", "sr", { "win94", "win1894" }, true },
        { "Crossbow", "Gun/Special", "Item_Weapon_Crossbow_C", "crossbow", "special", { "crossbow" }, true },

        { "P90", "Gun/SMG", "Item_Weapon_P90_C", "p90", "smg", { "p90" }, true },
        { "Vector", "Gun/SMG", "Item_Weapon_Vector_C", "vector", "smg", { "vector" }, true },
        { "UZI", "Gun/SMG", "Item_Weapon_UZI_C", "uzi", "smg", { "uzi" }, true },
        { "UMP45", "Gun/SMG", "Item_Weapon_UMP_C", "ump45", "smg", { "ump45", "ump" }, true },
        { "Thompson", "Gun/SMG", "Item_Weapon_Thompson_C", "tommygun", "smg", { "tommygun", "thompson" }, true },
        { "PP-19", "Gun/SMG", "Item_Weapon_BizonPP19_C", "pp19", "smg", { "pp19", "bizonpp19" }, true },
        { "JS9", "Gun/SMG", "Item_Weapon_JS9_C", "js9", "smg", { "js9" }, true },
        { "MP5K", "Gun/SMG", "Item_Weapon_MP5K_C", "mp5k", "smg", { "mp5k" }, true },
        { "MP9", "Gun/SMG", "Item_Weapon_MP9_C", "mp9", "smg", { "mp9" }, true },

        { "DP28", "Gun/LMG", "Item_Weapon_DP28_C", "dp28", "lmg", { "dp28" }, true },
        { "M249", "Gun/LMG", "Item_Weapon_M249_C", "m249", "lmg", { "m249" }, true },
        { "MG3", "Gun/LMG", "Item_Weapon_MG3_C", "mg3", "lmg", { "mg3" }, true },

        { "S686", "Gun/SG", "Item_Weapon_Berreta686_C", "s686", "sg", { "s686", "berreta686" }, true },
        { "S12K", "Gun/SG", "Item_Weapon_Saiga12_C", "s12k", "sg", { "s12k", "saiga12" }, true },
        { "S1897", "Gun/SG", "Item_Weapon_Winchester_C", "s1897", "sg", { "s1897", "winchester" }, true },
        { "DBS", "Gun/SG", "Item_Weapon_DP12_C", "dbs", "sg", { "dbs", "dp12" }, true },
        { "O12", "Gun/SG", "Item_Weapon_OriginS12_C", "o12", "sg", { "o12", "origin12", "origins12" }, true },
        { "Sawed-off", "Gun/SG", "Item_Weapon_Sawnoff_C", "sawnoff", "sg", { "sawnoff", "sawed-off" }, true },

        { "P18C", "Gun/HG", "Item_Weapon_G18_C", "g18", "hg", { "g18", "p18c" }, true },
        { "P1911", "Gun/HG", "Item_Weapon_M1911_C", "m1911", "hg", { "m1911", "p1911" }, true },
        { "P92", "Gun/HG", "Item_Weapon_M9_C", "m9", "hg", { "m9", "p92" }, true },
        { "R1895", "Gun/HG", "Item_Weapon_NagantM1895_C", "nagantm1895", "hg", { "nagantm1895", "r1895" }, true },
        { "R45", "Gun/HG", "Item_Weapon_Rhino_C", "rhino", "hg", { "rhino", "r45" }, true },
        { "Deagle", "Gun/HG", "Item_Weapon_DesertEagle_C", "deserteagle", "hg", { "deserteagle", "deagle" }, true },
        { "Skorpion", "Gun/HG", "Item_Weapon_vz61Skorpion_C", "vz61skorpion", "hg", { "vz61skorpion", "skorpion" }, true },
        { "Flare Gun", "Gun/Special", "Item_Weapon_FlareGun_C", "flaregun", "special", { "flaregun" }, true },
        { "M79", "Gun/Special", "Item_Weapon_M79_C", "m79", "special", { "m79" }, true },
        { "Panzer", "Gun/Special", "Item_Weapon_PanzerFaust100M_C", "panzerfaust", "special", { "panzerfaust", "panzer" }, true },
    };
    return weapons;
}

inline void EnsureDefaults(std::unordered_map<std::string, bool>& values) {
    for (const auto& weapon : All()) {
        if (values.find(weapon.key) == values.end()) {
            values[weapon.key] = weapon.default_enabled;
        }
    }
}

inline void EnsureCategoryDefaults(std::unordered_map<std::string, bool>& values) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = category.default_enabled;
        }
    }
}

inline void EnsureCategoryBoolDefaults(std::unordered_map<std::string, bool>& values, bool default_value) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_value;
        }
    }
}

inline void EnsureCategoryFloatDefaults(std::unordered_map<std::string, float>& values, float default_value) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_value;
        }
    }
}

inline void EnsureCategoryIntDefaults(std::unordered_map<std::string, int>& values, int default_value) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_value;
        }
    }
}

inline void EnsureCategoryMoveSpeedDefaults(std::unordered_map<std::string, float>& values) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = 50.0f;
        }
    }
}

inline void EnsureCategoryFovDefaults(std::unordered_map<std::string, float>& values, float default_fov = 100.0f) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_fov;
        }
    }
}

inline void EnsureCategorySmoothnessDefaults(std::unordered_map<std::string, float>& values, float default_val = 1.0f) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_val;
        }
    }
}

inline void EnsureCategoryShotDelayDefaults(std::unordered_map<std::string, float>& values, float default_val = 0.0f) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_val;
        }
    }
}

inline void EnsureCategoryJitterDefaults(std::unordered_map<std::string, float>& values, float default_val = 0.0f) {
    for (const auto& category : Categories()) {
        if (values.find(category.key) == values.end()) {
            values[category.key] = default_val;
        }
    }
}

inline const Weapon* FindWeapon(const std::string& weapon_name) {
    for (const auto& weapon : All()) {
        if (weapon_name == weapon.key) return &weapon;
        for (const char* alias : weapon.aliases) {
            if (weapon_name == alias) return &weapon;
        }
    }
    return nullptr;
}

inline bool IsEnabled(const std::unordered_map<std::string, bool>& values, const std::string& weapon_name) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return false;

    const auto it = values.find(weapon->category_key);
    return it == values.end() ? weapon->default_enabled : it->second;
}

inline bool BoolForWeapon(const std::unordered_map<std::string, bool>& values, const std::string& weapon_name, bool fallback) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return fallback;

    const auto it = values.find(weapon->category_key);
    return it == values.end() ? fallback : it->second;
}

inline float FloatForWeapon(const std::unordered_map<std::string, float>& values, const std::string& weapon_name,
    float fallback, float min_value, float max_value) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return fallback;

    const auto it = values.find(weapon->category_key);
    if (it == values.end()) return fallback;
    if (it->second < min_value) return min_value;
    if (it->second > max_value) return max_value;
    return it->second;
}

inline int IntForWeapon(const std::unordered_map<std::string, int>& values, const std::string& weapon_name,
    int fallback, int min_value, int max_value) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return fallback;

    const auto it = values.find(weapon->category_key);
    if (it == values.end()) return fallback;
    if (it->second < min_value) return min_value;
    if (it->second > max_value) return max_value;
    return it->second;
}

inline float MoveSpeedForWeapon(const std::unordered_map<std::string, float>& values, const std::string& weapon_name) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return 1.0f;

    const auto it = values.find(weapon->category_key);
    if (it == values.end()) return 1.0f;
    return std::clamp(it->second / 50.0f, 0.001f, 5.0f);
}

inline float FovForWeapon(const std::unordered_map<std::string, float>& values, const std::string& weapon_name, float fallback_fov) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return fallback_fov;

    const auto it = values.find(weapon->category_key);
    if (it == values.end()) return fallback_fov;
    if (it->second < 1.0f) return 1.0f;
    if (it->second > 100.0f) return 100.0f;
    return it->second;
}

inline float SmoothnessForWeapon(const std::unordered_map<std::string, float>& values, const std::string& weapon_name, float fallback) {
    const Weapon* weapon = FindWeapon(weapon_name);
    if (!weapon) return fallback;

    const auto it = values.find(weapon->category_key);
    if (it == values.end()) return fallback;
    return it->second;
}

inline void BuildCategoriesFromWeaponSettings(const std::unordered_map<std::string, bool>& weapon_values,
    std::unordered_map<std::string, bool>& category_values) {
    category_values.clear();
    for (const auto& category : Categories()) {
        bool any_enabled = false;
        bool any_known = false;
        for (const auto& weapon : All()) {
            if (std::string(weapon.category_key) != category.key) continue;
            const auto it = weapon_values.find(weapon.key);
            if (it != weapon_values.end()) {
                any_known = true;
                any_enabled = any_enabled || it->second;
            }
        }
        category_values[category.key] = any_known ? any_enabled : category.default_enabled;
    }
}

} // namespace FlickWeaponCatalog
