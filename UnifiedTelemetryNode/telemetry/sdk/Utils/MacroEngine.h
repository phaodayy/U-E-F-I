#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include "../core/context.hpp"
#include "../../overlay/core/overlay_menu.hpp"
#include "../core/embedded_resources.hpp"
#include "../core/fname.hpp"
#include "../memory/memory.hpp"
#include "../core/offsets.hpp"
#include "../core/telemetry_decrypt.hpp"
#include "Driver.h"
#include "SendInputMacro.h"
#include "Utils.h"
#include "../Common/Data.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

enum WeaponCategory
{
    CAT_AR, CAT_SR, CAT_DMR, CAT_SMG, CAT_LMG, CAT_SG, CAT_PT, CAT_PANZER, CAT_OTHER, CAT_NONE
};

class MacroEngine
{
public:
    static inline json current_gun_data;
    static inline json global_config;
    static inline std::string current_weapon_name = "";
    static inline WeaponCategory current_category = CAT_NONE;
    static inline int bullet_index = 0;
    static inline ULONGLONG last_fire_time = 0;

    static inline int current_scope = 0;
    static inline int current_muzzle = 0;
    static inline int current_grip = 0;

    static inline float global_multiplier = 1.0f;
    static inline bool macro_enabled = false;
    static inline bool macro_humanize = true;
    static inline bool ads_only = true;
    static inline int macro_mode = 1;
    static inline float res_scale_x = 1.0f;
    static inline float res_scale_y = 1.0f;

    // Runtime knobs to avoid fixed hardcoded behavior.
    static inline int recoil_reset_ms = 300;
    static inline int pull_delay_ms = 9;
    static inline int pull_delay_jitter_ms = 2;
    static inline int x_jitter_range = 1;
    static inline float humanize_y_min = 0.95f;
    static inline float humanize_y_max = 1.05f;
    static inline float angle_to_pixel_x = 35.0f; // Increased from 20.0
    static inline float angle_to_pixel_y = 35.0f; // Increased from 20.0
    static inline float yaw_gain = 0.50f;
    static inline float pitch_gain = 1.20f; // Increased from 1.0
    static inline float angle_deadzone = 0.01f;
    static inline float ar_max_step_x = 50.0f;
    static inline float ar_max_step_y = 50.0f;
    static inline float sr_max_step_x = 10.0f;
    static inline float sr_max_step_y = 10.0f;
    static inline float dmr_max_step_x = 15.0f;
    static inline float dmr_max_step_y = 15.0f;
    static inline bool ar_trigger_enabled = false;
    static inline bool sr_trigger_enabled = false;
    static inline bool dmr_trigger_enabled = false;
    static inline bool sg_trigger_enabled = false;
    static inline float ar_trigger_fov = 2.5f;
    static inline float sr_trigger_fov = 1.5f;
    static inline float dmr_trigger_fov = 2.0f;
    static inline float sg_trigger_fov = 4.0f;
    static inline int trigger_delay_ms = 120;
    static inline int sr_autoshot_delay1 = 50;
    static inline int sr_autoshot_delay2 = 1200;
    static inline float ar_base_smoothing = 1.0f;
    static inline float sr_base_smoothing = 1.2f;
    static inline float dmr_base_smoothing = 1.1f;
    static inline float max_smooth_increase = 0.25f;
    static inline float smooth_fov = 10.0f;

    static inline uint64_t sr_sequence_start = 0;

    struct AngleRotation
    {
        float Pitch;
        float Yaw;
        float Roll;
    };

    static inline bool angle_anchor_valid = false;
    static inline float angle_anchor_yaw = 0.0f;
    static inline float angle_anchor_pitch = 0.0f;
    static inline float angle_moved_x = 0.0f;
    static inline float angle_moved_y = 0.0f;
    static inline float pixel_remainder_x = 0.0f;
    static inline float pixel_remainder_y = 0.0f;
    static inline float last_recoil_pitch = 0.0f;
    static inline float last_recoil_yaw = 0.0f;
    static inline float last_vec_y = 0.0f;
    static inline bool trigger_firing = false;
    static inline ULONGLONG last_debug_state_tick = 0;
    static inline ULONGLONG last_debug_gate_tick = 0;
    static inline ULONGLONG last_debug_sendinput_tick = 0;
    static inline bool last_debug_macro_enabled = false;

    static inline std::string external_base_path = "";
    static inline bool prefer_external_data = true;

    static inline bool has_config_write_time = false;
    static inline fs::file_time_type config_write_time = {};
    static inline ULONGLONG last_config_check_tick = 0;

    static inline std::unordered_map<std::string, fs::file_time_type> weapon_write_times;
    static inline ULONGLONG last_weapon_check_tick = 0;

    static bool FileExists(const std::string& path)
    {
        std::ifstream f(path.c_str());
        return f.good();
    }

    static std::string EnsureTrailingSlash(std::string path)
    {
        if (!path.empty() && path.back() != '\\' && path.back() != '/')
        {
            path.push_back('\\');
        }
        return path;
    }

    static std::string GetExecutableDir()
    {
        char exePath[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);

        std::string dir = exePath;
        const size_t pos = dir.find_last_of("\\/");
        if (pos != std::string::npos)
        {
            dir = dir.substr(0, pos + 1);
        }
        return EnsureTrailingSlash(dir);
    }

    static std::string ResolveExternalBasePath()
    {
        const std::string exeDir = GetExecutableDir();
        std::vector<std::string> candidates;
        candidates.reserve(4);

        candidates.push_back(exeDir);

        std::string exeParent = exeDir;
        if (!exeParent.empty() && (exeParent.back() == '\\' || exeParent.back() == '/'))
        {
            exeParent.pop_back();
        }
        const size_t ppos = exeParent.find_last_of("\\/");
        if (ppos != std::string::npos)
        {
            candidates.push_back(EnsureTrailingSlash(exeParent.substr(0, ppos + 1)));
        }

        try
        {
            candidates.push_back(EnsureTrailingSlash(fs::current_path().string()));
        }
        catch (...)
        {
        }

        for (const std::string& base : candidates)
        {
            const std::string cfgPath = base + skCrypt("dataMacro\\Config\\macro_config.json");
            const std::string gunDir = base + skCrypt("dataMacro\\GunData\\");
            if (FileExists(cfgPath) || fs::exists(gunDir))
            {
                return base;
            }
        }

        return exeDir;
    }

    static std::string ReadTextFileSafe(const std::string& path)
    {
        try
        {
            const std::string content = Utils::ReadConfigFile(path);
            if (!content.empty())
            {
                return content;
            }

            std::string embedded;
            if (EmbeddedResources::LoadText(path, embedded))
            {
                return embedded;
            }
            return "";
        }
        catch (...)
        {
            return "";
        }
    }

    static int ReadIntConfig(const json& src, const char* key, const int defaultValue, const int minValue, const int maxValue)
    {
        try
        {
            if (!src.contains(key))
            {
                return defaultValue;
            }

            const int val = src[key].get<int>();
            return std::clamp(val, minValue, maxValue);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    static float ReadFloatConfig(const json& src, const char* key, const float defaultValue, const float minValue, const float maxValue)
    {
        try
        {
            if (!src.contains(key))
            {
                return defaultValue;
            }

            const float val = src[key].get<float>();
            return std::clamp(val, minValue, maxValue);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    static std::string ConfigPath()
    {
        return external_base_path + skCrypt("dataMacro\\Config\\macro_config.json");
    }

    static std::string WeaponPath(const std::string& weapon)
    {
        return external_base_path + skCrypt("dataMacro\\GunData\\") + weapon + skCrypt(".json");
    }

    static void RefreshResolutionScale()
    {
        const int currentX = GetSystemMetrics(SM_CXSCREEN);
        const int currentY = GetSystemMetrics(SM_CYSCREEN);

        int configX = currentX;
        int configY = currentY;

        if (!global_config.is_null() && global_config.contains(skCrypt("resolution")))
        {
            try
            {
                if (global_config[ skCrypt("resolution") ].is_string())
                {
                    std::string resStr = global_config[ skCrypt("resolution") ].get<std::string>();
                    std::transform(resStr.begin(), resStr.end(), resStr.begin(), [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });

                    if (resStr != skCrypt("auto"))
                    {
                        const size_t xPos = resStr.find('x');
                        if (xPos != std::string::npos)
                        {
                            configX = std::stoi(resStr.substr(0, xPos));
                            configY = std::stoi(resStr.substr(xPos + 1));
                        }
                    }
                }
                else if (global_config[ skCrypt("resolution") ].is_object())
                {
                    const auto& r = global_config[ skCrypt("resolution") ];
                    if (r.contains(skCrypt("x")))
                    {
                        configX = r[ skCrypt("x") ].get<int>();
                    }
                    if (r.contains(skCrypt("y")))
                    {
                        configY = r[ skCrypt("y") ].get<int>();
                    }
                }
            }
            catch (...)
            {
                configX = currentX;
                configY = currentY;
            }
        }

        if (configX <= 0 || configY <= 0)
        {
            configX = currentX;
            configY = currentY;
        }

        res_scale_x = static_cast<float>(currentX) / static_cast<float>(configX);
        res_scale_y = static_cast<float>(currentY) / static_cast<float>(configY);
    }

    static void ApplyRuntimeConfig()
    {
        recoil_reset_ms = 300;
        pull_delay_ms = 9;
        pull_delay_jitter_ms = 2;
        x_jitter_range = 1;
        humanize_y_min = 0.95f;
        humanize_y_max = 1.05f;
        angle_to_pixel_x = 20.0f;
        angle_to_pixel_y = 20.0f;
        yaw_gain = 0.40f;
        pitch_gain = 1.00f;
        angle_deadzone = 0.01f;
        macro_mode = 1;
        prefer_external_data = true;

        if (global_config.is_null() || !global_config.contains("runtime") || !global_config["runtime"].is_object())
        {
            return;
        }

        const auto& runtime = global_config[skCrypt("runtime")];
        recoil_reset_ms = ReadIntConfig(runtime, skCrypt("recoil_reset_ms"), recoil_reset_ms, 50, 2000);
        pull_delay_ms = ReadIntConfig(runtime, skCrypt("pull_delay_ms"), pull_delay_ms, 0, 50);
        pull_delay_jitter_ms = ReadIntConfig(runtime, skCrypt("pull_delay_jitter_ms"), pull_delay_jitter_ms, 0, 50);
        x_jitter_range = ReadIntConfig(runtime, skCrypt("x_jitter_range"), x_jitter_range, 0, 12);
        humanize_y_min = ReadFloatConfig(runtime, skCrypt("humanize_y_min"), humanize_y_min, 0.10f, 3.0f);
        humanize_y_max = ReadFloatConfig(runtime, skCrypt("humanize_y_max"), humanize_y_max, 0.10f, 3.0f);
        angle_to_pixel_x = ReadFloatConfig(runtime, skCrypt("angle_to_pixel_x"), angle_to_pixel_x, 0.1f, 200.0f);
        angle_to_pixel_y = ReadFloatConfig(runtime, skCrypt("angle_to_pixel_y"), angle_to_pixel_y, 0.1f, 200.0f);
        yaw_gain = ReadFloatConfig(runtime, skCrypt("yaw_gain"), yaw_gain, 0.01f, 10.0f);
        pitch_gain = ReadFloatConfig(runtime, skCrypt("pitch_gain"), pitch_gain, 0.01f, 10.0f);
        angle_deadzone = ReadFloatConfig(runtime, skCrypt("angle_deadzone"), angle_deadzone, 0.0f, 1.5f);
        ar_max_step_x = ReadFloatConfig(runtime, skCrypt("ar_max_step_x"), ar_max_step_x, 0.1f, 500.0f);
        ar_max_step_y = ReadFloatConfig(runtime, skCrypt("ar_max_step_y"), ar_max_step_y, 0.1f, 500.0f);
        sr_max_step_x = ReadFloatConfig(runtime, skCrypt("sr_max_step_x"), sr_max_step_x, 0.1f, 500.0f);
        sr_max_step_y = ReadFloatConfig(runtime, skCrypt("sr_max_step_y"), sr_max_step_y, 0.1f, 500.0f);
        dmr_max_step_x = ReadFloatConfig(runtime, skCrypt("dmr_max_step_x"), dmr_max_step_x, 0.1f, 500.0f);
        dmr_max_step_y = ReadFloatConfig(runtime, skCrypt("dmr_max_step_y"), dmr_max_step_y, 0.1f, 500.0f);
        
        ar_trigger_fov = ReadFloatConfig(runtime, skCrypt("ar_trigger_fov"), ar_trigger_fov, 0.1f, 50.0f);
        sr_trigger_fov = ReadFloatConfig(runtime, skCrypt("sr_trigger_fov"), sr_trigger_fov, 0.1f, 50.0f);
        dmr_trigger_fov = ReadFloatConfig(runtime, skCrypt("dmr_trigger_fov"), dmr_trigger_fov, 0.1f, 50.0f);
        sg_trigger_fov = ReadFloatConfig(runtime, skCrypt("sg_trigger_fov"), sg_trigger_fov, 0.1f, 50.0f);

        trigger_delay_ms = ReadIntConfig(runtime, skCrypt("trigger_delay_ms"), trigger_delay_ms, 0, 5000);
        sr_autoshot_delay1 = ReadIntConfig(runtime, skCrypt("sr_autoshot_delay1"), sr_autoshot_delay1, 0, 1000);
        sr_autoshot_delay2 = ReadIntConfig(runtime, skCrypt("sr_autoshot_delay2"), sr_autoshot_delay2, 0, 5000);
        
        ar_base_smoothing = ReadFloatConfig(runtime, skCrypt("ar_base_smoothing"), ar_base_smoothing, 1.0f, 50.0f);
        sr_base_smoothing = ReadFloatConfig(runtime, skCrypt("sr_base_smoothing"), sr_base_smoothing, 1.0f, 50.0f);
        dmr_base_smoothing = ReadFloatConfig(runtime, skCrypt("dmr_base_smoothing"), dmr_base_smoothing, 1.0f, 50.0f);
        max_smooth_increase = ReadFloatConfig(runtime, skCrypt("max_smooth_increase"), max_smooth_increase, 0.0f, 20.0f);
        smooth_fov = ReadFloatConfig(runtime, skCrypt("smooth_fov"), smooth_fov, 0.0f, 100.0f);

        if (runtime.contains("ar_trigger_enabled")) try { ar_trigger_enabled = runtime["ar_trigger_enabled"].get<bool>(); } catch (...) {}
        if (runtime.contains("sr_trigger_enabled")) try { sr_trigger_enabled = runtime["sr_trigger_enabled"].get<bool>(); } catch (...) {}
        if (runtime.contains("dmr_trigger_enabled")) try { dmr_trigger_enabled = runtime["dmr_trigger_enabled"].get<bool>(); } catch (...) {}
        if (runtime.contains("sg_trigger_enabled")) try { sg_trigger_enabled = runtime["sg_trigger_enabled"].get<bool>(); } catch (...) {}

        macro_mode = ReadIntConfig(runtime, "macro_mode", macro_mode, 1, 2);

        if (humanize_y_min > humanize_y_max)
        {
            std::swap(humanize_y_min, humanize_y_max);
        }

        if (runtime.contains("prefer_external_data"))
        {
            try
            {
                prefer_external_data = runtime["prefer_external_data"].get<bool>();
            }
            catch (...)
            {
            }
        }
    }

    static void LoadGlobalConfig(const bool forceReload = false)
    {
        const std::string cfgPath = ConfigPath();
        std::string configText = "";
        bool loadedExternal = false;

        if (prefer_external_data)
        {
            configText = ReadTextFileSafe(cfgPath);
            loadedExternal = !configText.empty();
        }

        if (configText.empty() && !forceReload)
        {
            return;
        }

        try
        {
            global_config = configText.empty() ? json::object() : json::parse(configText);
        }
        catch (...)
        {
            global_config = json::object();
        }

        RefreshResolutionScale();
        ApplyRuntimeConfig();

        if (loadedExternal)
        {
            try
            {
                config_write_time = fs::last_write_time(cfgPath);
                has_config_write_time = true;
            }
            catch (...)
            {
                has_config_write_time = false;
            }
        }
        else
        {
            has_config_write_time = false;
        }
    }

    static void TryHotReloadConfig()
    {
        const ULONGLONG now = GetTickCount64();
        if (now - last_config_check_tick < 500)
        {
            return;
        }
        last_config_check_tick = now;

        if (!prefer_external_data)
        {
            return;
        }

        const std::string cfgPath = ConfigPath();
        if (!FileExists(cfgPath))
        {
            return;
        }

        try
        {
            const auto currentWrite = fs::last_write_time(cfgPath);
            if (!has_config_write_time || currentWrite != config_write_time)
            {
                LoadGlobalConfig(true);
            }
        }
        catch (...)
        {
        }
    }

    static void Initialize()
    {
        external_base_path = ResolveExternalBasePath();
        weapon_write_times.clear();
        has_config_write_time = false;
        LoadGlobalConfig(true);
    }

    static std::string NormalizeWeaponName(const std::string& raw)
    {
        std::string name = raw;
        if (name.find(skCrypt("Weap")) == 0) name.erase(0, 4);
        if (name.find(skCrypt("Item_Weapon_")) == 0) name.erase(0, 12);
        if (name.find(skCrypt("_C")) != std::string::npos) name.erase(name.find(skCrypt("_C")));

        if (name == skCrypt("HK416")) return skCrypt("m416");
        if (name == skCrypt("BerylM762")) return skCrypt("m762");
        if (name == skCrypt("AK47")) return skCrypt("akm");
        if (name == skCrypt("SCAR-L")) return skCrypt("scar-l");
        if (name == skCrypt("FNFal")) return skCrypt("slr");
        if (name == skCrypt("G36C")) return skCrypt("g36c");
        if (name == skCrypt("Mk14")) return skCrypt("mk14");
        if (name == skCrypt("M249")) return skCrypt("m249");
        if (name == skCrypt("UMP")) return skCrypt("ump45");
        if (name == skCrypt("Vector")) return skCrypt("vector");
        if (name == skCrypt("Uzi")) return skCrypt("uzi");
        if (name == skCrypt("Thompson")) return skCrypt("tommygun");
        if (name == skCrypt("BizonPP19")) return skCrypt("pp19");
        if (name == skCrypt("Berreta686")) return skCrypt("s686");
        if (name == skCrypt("Saiga12")) return skCrypt("s12k");
        if (name == skCrypt("Winchester")) return skCrypt("s1897");
        if (name == skCrypt("DP12")) return skCrypt("dbs");
        if (name == skCrypt("OriginS12")) return skCrypt("o12");
        if (name == skCrypt("Win1894")) return skCrypt("win94");
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        return name;
    }

    static WeaponCategory GetCategoryByName(const std::string& name)
    {
        if (name == skCrypt("m416") || name == skCrypt("m762") || name == skCrypt("akm") || name == skCrypt("scar-l") || name == skCrypt("g36c") || name == skCrypt("aug") || name == skCrypt("qbz") || name == skCrypt("k2") || name == skCrypt("ace32") || name == skCrypt("famas")) return CAT_AR;
        if (name == skCrypt("slr") || name == skCrypt("sks") || name == skCrypt("mk14") || name == skCrypt("qbu") || name == skCrypt("mini14") || name == skCrypt("vss") || name == skCrypt("mk12") || name == skCrypt("dragunov")) return CAT_DMR;
        if (name == skCrypt("kar98k") || name == skCrypt("m24") || name == skCrypt("awm") || name == skCrypt("mosin") || name == skCrypt("lynx") || name == skCrypt("win94") || name == skCrypt("winchester")) return CAT_SR;
        if (name == skCrypt("ump45") || name == skCrypt("vector") || name == skCrypt("uzi") || name == skCrypt("tommygun") || name == skCrypt("pp19") || name == skCrypt("mp5k") || name == skCrypt("p90") || name == skCrypt("js9")) return CAT_SMG;
        if (name == skCrypt("m249") || name == skCrypt("dp28") || name == skCrypt("mg3")) return CAT_LMG;
        if (name == skCrypt("s12k") || name == skCrypt("s1897") || name == skCrypt("s686") || name == skCrypt("berreta686") || name == skCrypt("dbs") || name == skCrypt("o12")) return CAT_SG;
        if (name == skCrypt("p18c") || name == skCrypt("p1911") || name == skCrypt("p92") || name == skCrypt("r1895") || name == skCrypt("r45") || name == skCrypt("deagle") || name == skCrypt("skorpion")) return CAT_PT;
        if (name.find(skCrypt("panzerfaust")) != std::string::npos || name.find(skCrypt("panzer")) != std::string::npos) return CAT_PANZER;
        return CAT_NONE;
    }

    struct ActiveProfile
    {
        bool enabled = true;
        float multiplier = 1.0f;
        int resetMs = 300;
        int delayMs = 9;
        int delayJitterMs = 2;
        int xJitterRange = 1;
        float humanizeMin = 0.95f;
        float humanizeMax = 1.05f;
        float angleToPixelX = 20.0f;
        float angleToPixelY = 20.0f;
        float yawGain = 0.40f;
        float pitchGain = 1.00f;
        float deadzone = 0.01f;
        float arMaxStepX = 50.0f, arMaxStepY = 50.0f;
        float srMaxStepX = 10.0f, srMaxStepY = 10.0f;
        float dmrMaxStepX = 15.0f, dmrMaxStepY = 15.0f;
        bool arTriggerEnabled = false;
        bool srTriggerEnabled = false;
        bool dmrTriggerEnabled = false;
        bool sgTriggerEnabled = false;
        float arTriggerFov = 2.5f;
        float srTriggerFov = 1.5f;
        float dmrTriggerFov = 2.0f;
        float sgTriggerFov = 4.0f;
        int triggerDelayMs = 120;
        int srDelay1 = 50, srDelay2 = 1200;
        float arBaseSmoothing = 1.0f;
        float srBaseSmoothing = 1.2f;
        float dmrBaseSmoothing = 1.1f;
        float maxSmoothIncrease = 0.25f;
        float smoothFov = 10.0f;
    };

    static std::string CategoryKey(const WeaponCategory category)
    {
        switch (category)
        {
        case CAT_AR: return skCrypt("AR");
        case CAT_SR: return skCrypt("SR");
        case CAT_DMR: return skCrypt("DMR");
        case CAT_SMG: return skCrypt("SMG");
        case CAT_LMG: return skCrypt("LMG");
        case CAT_SG: return skCrypt("SG");
        case CAT_PT: return skCrypt("PT");
        default: return skCrypt("OTHER");
        }
    }

    static void ApplyProfileFields(ActiveProfile& out, const json& src)
    {
        if (!src.is_object())
        {
            return;
        }

        if (src.contains(skCrypt("enabled")))
        {
            try { out.enabled = src[skCrypt("enabled")].get<bool>(); } catch (...) {}
        }

        out.multiplier = ReadFloatConfig(src, skCrypt("multiplier"), out.multiplier, 0.01f, 10.0f);
        out.resetMs = ReadIntConfig(src, skCrypt("reset_ms"), out.resetMs, 50, 2000);
        out.delayMs = ReadIntConfig(src, skCrypt("delay_ms"), out.delayMs, 0, 60);
        out.delayJitterMs = ReadIntConfig(src, skCrypt("delay_jitter_ms"), out.delayJitterMs, 0, 60);
        
        out.arMaxStepX = ReadFloatConfig(src, skCrypt("ar_max_step_x"), out.arMaxStepX, 0.1f, 500.0f);
        out.arMaxStepY = ReadFloatConfig(src, skCrypt("ar_max_step_y"), out.arMaxStepY, 0.1f, 500.0f);
        out.srMaxStepX = ReadFloatConfig(src, skCrypt("sr_max_step_x"), out.srMaxStepX, 0.1f, 500.0f);
        out.srMaxStepY = ReadFloatConfig(src, skCrypt("sr_max_step_y"), out.srMaxStepY, 0.1f, 500.0f);
        out.dmrMaxStepX = ReadFloatConfig(src, skCrypt("dmr_max_step_x"), out.dmrMaxStepX, 0.1f, 500.0f);
        out.dmrMaxStepY = ReadFloatConfig(src, skCrypt("dmr_max_step_y"), out.dmrMaxStepY, 0.1f, 500.0f);

        out.arTriggerFov = ReadFloatConfig(src, skCrypt("ar_trigger_fov"), out.arTriggerFov, 0.1f, 50.0f);
        out.srTriggerFov = ReadFloatConfig(src, skCrypt("sr_trigger_fov"), out.srTriggerFov, 0.1f, 50.0f);
        out.dmrTriggerFov = ReadFloatConfig(src, skCrypt("dmr_trigger_fov"), out.dmrTriggerFov, 0.1f, 50.0f);
        out.sgTriggerFov = ReadFloatConfig(src, skCrypt("sg_trigger_fov"), out.sgTriggerFov, 0.1f, 50.0f);
        out.triggerDelayMs = ReadIntConfig(src, skCrypt("trigger_delay_ms"), out.triggerDelayMs, 0, 5000);
        out.srDelay1 = ReadIntConfig(src, skCrypt("sr_autoshot_delay1"), out.srDelay1, 0, 1000);
        out.srDelay2 = ReadIntConfig(src, skCrypt("sr_autoshot_delay2"), out.srDelay2, 0, 5000);

        out.arBaseSmoothing = ReadFloatConfig(src, skCrypt("ar_base_smoothing"), out.arBaseSmoothing, 1.0f, 50.0f);
        out.srBaseSmoothing = ReadFloatConfig(src, skCrypt("sr_base_smoothing"), out.srBaseSmoothing, 1.0f, 50.0f);
        out.dmrBaseSmoothing = ReadFloatConfig(src, skCrypt("dmr_base_smoothing"), out.dmrBaseSmoothing, 1.0f, 50.0f);
        out.maxSmoothIncrease = ReadFloatConfig(src, skCrypt("max_smooth_increase"), out.maxSmoothIncrease, 0.0f, 20.0f);
        out.smoothFov = ReadFloatConfig(src, skCrypt("smooth_fov"), out.smoothFov, 0.0f, 100.0f);

        if (src.contains(skCrypt("ar_trigger_enabled"))) try { out.arTriggerEnabled = src[skCrypt("ar_trigger_enabled")].get<bool>(); } catch (...) {}
        if (src.contains(skCrypt("sr_trigger_enabled"))) try { out.srTriggerEnabled = src[skCrypt("sr_trigger_enabled")].get<bool>(); } catch (...) {}
        if (src.contains(skCrypt("dmr_trigger_enabled"))) try { out.dmrTriggerEnabled = src[skCrypt("dmr_trigger_enabled")].get<bool>(); } catch (...) {}
        if (src.contains(skCrypt("sg_trigger_enabled"))) try { out.sgTriggerEnabled = src[skCrypt("sg_trigger_enabled")].get<bool>(); } catch (...) {}
        out.xJitterRange = ReadIntConfig(src, skCrypt("x_jitter"), out.xJitterRange, 0, 16);
        out.humanizeMin = ReadFloatConfig(src, skCrypt("humanize_min"), out.humanizeMin, 0.1f, 3.0f);
        out.humanizeMax = ReadFloatConfig(src, skCrypt("humanize_max"), out.humanizeMax, 0.1f, 3.0f);
        out.angleToPixelX = ReadFloatConfig(src, skCrypt("angle_to_pixel_x"), out.angleToPixelX, 0.1f, 200.0f);
        out.angleToPixelY = ReadFloatConfig(src, skCrypt("angle_to_pixel_y"), out.angleToPixelY, 0.1f, 200.0f);
        out.yawGain = ReadFloatConfig(src, skCrypt("yaw_gain"), out.yawGain, 0.01f, 10.0f);
        out.pitchGain = ReadFloatConfig(src, skCrypt("pitch_gain"), out.pitchGain, 0.01f, 10.0f);
        out.deadzone = ReadFloatConfig(src, skCrypt("angle_deadzone"), out.deadzone, 0.0f, 1.5f);

        if (out.humanizeMin > out.humanizeMax)
        {
            std::swap(out.humanizeMin, out.humanizeMax);
        }
    }

    static ActiveProfile GetActiveProfile()
    {
        ActiveProfile profile;
        profile.enabled = true;
        profile.multiplier = global_multiplier;
        profile.resetMs = recoil_reset_ms;
        profile.delayMs = pull_delay_ms;
        profile.delayJitterMs = pull_delay_jitter_ms;
        profile.xJitterRange = x_jitter_range;
        profile.humanizeMin = humanize_y_min;
        profile.humanizeMax = humanize_y_max;
        profile.angleToPixelX = angle_to_pixel_x;
        profile.angleToPixelY = angle_to_pixel_y;
        profile.yawGain = yaw_gain;
        profile.pitchGain = pitch_gain;
        profile.deadzone = angle_deadzone;
        profile.arMaxStepX = ar_max_step_x;
        profile.arMaxStepY = ar_max_step_y;
        profile.srMaxStepX = sr_max_step_x;
        profile.srMaxStepY = sr_max_step_y;
        profile.dmrMaxStepX = dmr_max_step_x;
        profile.dmrMaxStepY = dmr_max_step_y;
        
        profile.arTriggerEnabled = ar_trigger_enabled;
        profile.srTriggerEnabled = sr_trigger_enabled;
        profile.dmrTriggerEnabled = dmr_trigger_enabled;
        profile.sgTriggerEnabled = sg_trigger_enabled;
        profile.arTriggerFov = ar_trigger_fov;
        profile.srTriggerFov = sr_trigger_fov;
        profile.dmrTriggerFov = dmr_trigger_fov;
        profile.sgTriggerFov = sg_trigger_fov;
        profile.triggerDelayMs = trigger_delay_ms;
        profile.srDelay1 = sr_autoshot_delay1;
        profile.srDelay2 = sr_autoshot_delay2;
        
        profile.arBaseSmoothing = ar_base_smoothing;
        profile.srBaseSmoothing = sr_base_smoothing;
        profile.dmrBaseSmoothing = dmr_base_smoothing;
        profile.maxSmoothIncrease = max_smooth_increase;
        profile.smoothFov = smooth_fov;

        if (global_config.is_null() || !global_config.contains(skCrypt("category_profiles")) || !global_config[skCrypt("category_profiles")].is_object())
        {
            return profile;
        }

        const auto& allProfiles = global_config[ skCrypt("category_profiles") ];
        const std::string key = CategoryKey(current_category);

        const json* selected = nullptr;
        if (allProfiles.contains(key) && allProfiles[ key ].is_object())
        {
            selected = &allProfiles[ key ];
        }
        else if (allProfiles.contains(skCrypt("default")) && allProfiles[ skCrypt("default") ].is_object())
        {
            selected = &allProfiles[ skCrypt("default") ];
        }

        if (selected == nullptr)
        {
            return profile;
        }

        const json* modeNode = selected;
        if (selected->contains(skCrypt("mode1")) || selected->contains(skCrypt("mode2")))
        {
            if (macro_mode == 2 && selected->contains(skCrypt("mode2")))
            {
                modeNode = &(*selected)[skCrypt("mode2")];
            }
            else if (selected->contains(skCrypt("mode1")))
            {
                modeNode = &(*selected)[skCrypt("mode1")];
            }
        }

        ApplyProfileFields(profile, *selected);
        if (modeNode != selected)
        {
            ApplyProfileFields(profile, *modeNode);
        }
        return profile;
    }

    static void LoadWeaponData(const std::string& weapon)
    {
        const std::string weaponFile = weapon + ".json";
        const std::string weaponPath = WeaponPath(weapon);
        std::string content = "";

        if (prefer_external_data)
        {
            content = ReadTextFileSafe(weaponPath);
        }

        if (content.empty())
        {
            current_gun_data = json();
            return;
        }

        try
        {
            current_gun_data = json::parse(content);
        }
        catch (...)
        {
            current_gun_data = json();
        }

        if (prefer_external_data && FileExists(weaponPath))
        {
            try
            {
                weapon_write_times[weapon] = fs::last_write_time(weaponPath);
            }
            catch (...)
            {
            }
        }
    }

    static void TryHotReloadCurrentWeapon()
    {
        if (!prefer_external_data || current_weapon_name.empty() || current_weapon_name == skCrypt("None"))
        {
            return;
        }

        const ULONGLONG now = GetTickCount64();
        if (now - last_weapon_check_tick < 350)
        {
            return;
        }
        last_weapon_check_tick = now;

        const std::string weaponPath = WeaponPath(current_weapon_name);
        if (!FileExists(weaponPath))
        {
            return;
        }

        try
        {
            const auto currentWrite = fs::last_write_time(weaponPath);
            const auto it = weapon_write_times.find(current_weapon_name);
            if (it == weapon_write_times.end() || currentWrite != it->second)
            {
                LoadWeaponData(current_weapon_name);
            }
        }
        catch (...)
        {
        }
    }

    static void ClassifyAttachment(const std::string& name)
    {
        if (name.find(skCrypt("Dot")) != std::string::npos || name.find(skCrypt("RedDot")) != std::string::npos || name.find(skCrypt("Aimpoint")) != std::string::npos) current_scope = 1;
        else if (name.find(skCrypt("Holo")) != std::string::npos) current_scope = 2;
        else if (name.find(skCrypt("2x")) != std::string::npos || name.find(skCrypt("Scope2x")) != std::string::npos) current_scope = 3;
        else if (name.find(skCrypt("3x")) != std::string::npos || name.find(skCrypt("Scope3x")) != std::string::npos) current_scope = 4;
        else if (name.find(skCrypt("4x")) != std::string::npos || name.find(skCrypt("ACOG")) != std::string::npos || name.find(skCrypt("Scope4x")) != std::string::npos) current_scope = 5;
        else if (name.find(skCrypt("6x")) != std::string::npos || name.find(skCrypt("Scope6x")) != std::string::npos) current_scope = 6;
        else if (name.find(skCrypt("8x")) != std::string::npos || name.find(skCrypt("Scope8x")) != std::string::npos || name.find(skCrypt("CQBSS")) != std::string::npos) current_scope = 7;
        else if (name.find(skCrypt("15x")) != std::string::npos) current_scope = 8;

        if (name.find(skCrypt("Compensator")) != std::string::npos) current_muzzle = 4;
        else if (name.find(skCrypt("FlashHider")) != std::string::npos) current_muzzle = 1;
        else if (name.find(skCrypt("Suppressor")) != std::string::npos || name.find(skCrypt("Silencer")) != std::string::npos) current_muzzle = 2;

        if (name.find(skCrypt("Angled")) != std::string::npos) current_grip = 2;
        else if (name.find(skCrypt("Half")) != std::string::npos || name.find(skCrypt("HalfGrip")) != std::string::npos) current_grip = 3;
        else if (name.find(skCrypt("Light")) != std::string::npos || name.find(skCrypt("LightGrip")) != std::string::npos) current_grip = 4;
        else if (name.find(skCrypt("Thumb")) != std::string::npos || name.find(skCrypt("ThumbGrip")) != std::string::npos) current_grip = 1;
        else if (name.find(skCrypt("Vertical")) != std::string::npos || name.find(skCrypt("Foregrip")) != std::string::npos) current_grip = 1;
    }

    static void UpdateAttachments(const uint64_t weapon)
    {
        current_scope = 0;
        current_muzzle = 0;
        current_grip = 0;

        if (!weapon) return;

        const uint64_t attachedBase = telemetryMemory::Read<uint64_t>(weapon + telemetry_config::offsets::AttachedItems);
        const int attachedCount = telemetryMemory::Read<int>(weapon + telemetry_config::offsets::AttachedItems + 0x8);
        if (attachedBase > 0x10000 && attachedCount > 0 && attachedCount <= 10)
        {
            for (int i = 0; i < attachedCount; ++i)
            {
                const uint64_t attachItem = telemetryMemory::Read<uint64_t>(attachedBase + (i * 8));
                if (attachItem < 0x10000) continue;

                uint64_t itemTableRow = telemetryMemory::Read<uint64_t>(attachItem + telemetry_config::offsets::WeaponAttachmentData);
                if (itemTableRow < 0x10000)
                {
                    itemTableRow = telemetryMemory::Read<uint64_t>(attachItem + telemetry_config::offsets::ItemTable);
                }
                if (itemTableRow < 0x10000) continue;

                const uint32_t itemID = telemetryMemory::Read<uint32_t>(itemTableRow + telemetry_config::offsets::ItemID);
                if (!itemID) continue;

                const std::string name = FNameUtils::GetNameFast(itemID);
                if (!name.empty() && name.find(skCrypt("Attach")) != std::string::npos)
                {
                    ClassifyAttachment(name);
                }
            }
        }
    }

    static float GetSensMultiplier()
    {
        if (global_config.is_null() || !global_config.contains(skCrypt("sensitivity")))
        {
            return 1.0f;
        }

        auto& sens = global_config[skCrypt("sensitivity")];
        std::string key = skCrypt("none");
        switch (current_scope)
        {
        case 1: key = skCrypt("reddot"); break;
        case 2: key = skCrypt("holosight"); break;
        case 3: key = skCrypt("2x"); break;
        case 4: key = skCrypt("3x"); break;
        case 5: key = skCrypt("4x"); break;
        case 6: key = skCrypt("6x"); break;
        case 7: key = skCrypt("8x"); break;
        case 8: key = skCrypt("15x"); break;
        default: break;
        }

        float baseSens = 1.0f;
        if (sens.contains(key))
        {
            try
            {
                baseSens = sens[key].get<float>();
            }
            catch (...)
            {
            }
        }

        if (telemetryMemory::IsKeyDown(VK_SHIFT) && sens.contains(skCrypt("shift")))
        {
            try
            {
                baseSens *= sens["shift"].get<float>();
            }
            catch (...)
            {
            }
        }
        return baseSens;
    }

    static void ForceScan()
    {
        TryHotReloadConfig();

        if (!G_LocalPawn) return;

        const uint64_t weaponProc = telemetryMemory::Read<uint64_t>(G_LocalPawn + telemetry_config::offsets::WeaponProcessor);
        if (!weaponProc) return;

        const uint8_t currentIdx = telemetryMemory::Read<uint8_t>(weaponProc + telemetry_config::offsets::CurrentWeaponIndex);
        if (currentIdx >= 3)
        {
            current_weapon_name = "None";
            current_gun_data = json();
            return;
        }

        const uint64_t equippedAddr = telemetryMemory::Read<uint64_t>(weaponProc + telemetry_config::offsets::EquippedWeapons);
        const uint64_t weapon = telemetryMemory::Read<uint64_t>(equippedAddr + (currentIdx * 8));
        if (!weapon)
        {
            current_weapon_name = "None";
            current_gun_data = json();
            return;
        }

        const uint32_t objID = telemetry_config::decrypt_cindex(telemetryMemory::Read<uint32_t>(weapon + telemetry_config::offsets::ObjID));
        const std::string rawWeap = FNameUtils::GetNameFast(objID);
        const std::string normalized = NormalizeWeaponName(rawWeap);

        if (normalized != current_weapon_name || current_scope == 0)
        {
            LoadWeaponData(normalized);
            current_weapon_name = normalized;
            current_category = GetCategoryByName(normalized);
            UpdateAttachments(weapon);
            bullet_index = 0;

            if (!current_weapon_name.empty() && current_weapon_name != "None") {
                std::cout << skCrypt("[WEAPON] Now holding: ") << current_weapon_name << std::endl;
            }
        }
    }

    static float NormalizeAngle(float value)
    {
        while (value > 180.0f) value -= 360.0f;
        while (value < -180.0f) value += 360.0f;
        return value;
    }

    static void ResetAngleState()
    {
        angle_anchor_valid = false;
        angle_anchor_yaw = 0.0f;
        angle_anchor_pitch = 0.0f;
        angle_moved_x = 0.0f;
        angle_moved_y = 0.0f;
        pixel_remainder_x = 0.0f;
        pixel_remainder_y = 0.0f;
        last_recoil_pitch = 0.0f;
        last_recoil_yaw = 0.0f;
        last_vec_y = 0.0f;
        bullet_index = 0;
    }

    static void Update()
    {
        const ULONGLONG updateNow = GetTickCount64();
        /*
        if (macro_enabled != last_debug_macro_enabled ||
            (macro_enabled && updateNow - last_debug_state_tick > 2000))
        {
            last_debug_state_tick = updateNow;
            last_debug_macro_enabled = macro_enabled;
            std::cout << "[MACRO][STATE] enabled=" << (macro_enabled ? 1 : 0)
                << " pawn=" << (G_LocalPawn ? 1 : 0)
                << " input=SendInput"
                << " weapon=" << current_weapon_name
                << " category=" << static_cast<int>(current_category)
                << " ads_only=" << (ads_only ? 1 : 0)
                << " humanize=" << (macro_humanize ? 1 : 0)
                << std::endl;
        }
        */

        if (!G_LocalPawn || !macro_enabled)
        {
            if (trigger_firing) { SendInputMacro::Up(); trigger_firing = false; }
            return;
        }

        TryHotReloadConfig();
        TryHotReloadCurrentWeapon();

        const uint64_t weaponProc = telemetryMemory::Read<uint64_t>(G_LocalPawn + telemetry_config::offsets::WeaponProcessor);
        if (!weaponProc) {
            if (trigger_firing) { SendInputMacro::Up(); trigger_firing = false; }
            return;
        }

        const uint8_t currentIdx = telemetryMemory::Read<uint8_t>(weaponProc + telemetry_config::offsets::CurrentWeaponIndex);
        if (currentIdx >= 3) {
            if (trigger_firing) { SendInputMacro::Up(); trigger_firing = false; }
            return;
        }

        const uint64_t equippedAddr = telemetryMemory::Read<uint64_t>(weaponProc + telemetry_config::offsets::EquippedWeapons);
        const uint64_t weapon = telemetryMemory::Read<uint64_t>(equippedAddr + (currentIdx * 8));
        if (!weapon) {
            if (trigger_firing) { SendInputMacro::Up(); trigger_firing = false; }
            return;
        }

        const uint32_t objID = telemetry_config::decrypt_cindex(telemetryMemory::Read<uint32_t>(weapon + telemetry_config::offsets::ObjID));
        const std::string rawWeap = FNameUtils::GetNameFast(objID);
        const std::string normalized = NormalizeWeaponName(rawWeap);

        if (normalized != current_weapon_name)
        {
            LoadWeaponData(normalized);
            current_weapon_name = normalized;
            current_category = GetCategoryByName(normalized);
            bullet_index = 0;
            UpdateAttachments(weapon);
        }

        static ULONGLONG lastAttachmentTick = 0;
        const ULONGLONG now = GetTickCount64();
        if (now - lastAttachmentTick > 250)
        {
            UpdateAttachments(weapon);
            lastAttachmentTick = now;
        }

        CURSORINFO ci = { sizeof(CURSORINFO) };
        bool hasCursor = false;
        if (GetCursorInfo(&ci))
        {
            hasCursor = (ci.flags & CURSOR_SHOWING) != 0;
        }

        const uint64_t mesh = telemetryMemory::Read<uint64_t>(G_LocalPawn + telemetry_config::offsets::Mesh);
        const uint64_t anim = mesh ? telemetryMemory::Read<uint64_t>(mesh + telemetry_config::offsets::AnimScriptInstance) : 0;

        // --- PAOD Context Acquisition ---
        float currentFOV = 80.0f;
        const uint64_t cameraManager = telemetryMemory::Read<uint64_t>(G_PlayerController + telemetry_config::offsets::PlayerCameraManager);
        if (cameraManager) {
            currentFOV = telemetryMemory::Read<float>(cameraManager + telemetry_config::offsets::CameraCacheFOV);
        }
        if (currentFOV < 10.0f || currentFOV > 170.0f) currentFOV = 80.0f;

        bool isInVehicle = false;
        const uint64_t riderComp = telemetryMemory::Read<uint64_t>(G_LocalPawn + telemetry_config::offsets::VehicleRiderComponent);
        if (riderComp) {
            int seatIdx = telemetryMemory::Read<int>(riderComp + telemetry_config::offsets::SeatIndex);
            if (seatIdx != -1) isInVehicle = true;
        }

        bool isReloading = anim ? telemetryMemory::Read<bool>(anim + telemetry_config::offsets::bIsReloading_CP) : false;
        uint8_t characterState = telemetryMemory::Read<uint8_t>(G_LocalPawn + telemetry_config::offsets::CharacterState);
        bool isSwimming = (characterState == 4); // Standard UE4 Swim state

        bool isScoping = true;
        if (ads_only)
        {
            isScoping = anim ? telemetryMemory::Read<bool>(anim + telemetry_config::offsets::bIsScoping_CP) : false;
        }

        const bool manualFire = telemetryMemory::IsKeyDown(VK_LBUTTON);
        const bool fireHeld = (manualFire || trigger_firing) &&
            isScoping &&
            !hasCursor &&
            !isReloading &&
            !telemetryMemory::IsKeyDown(VK_TAB) &&
            !telemetryMemory::IsKeyDown(VK_ESCAPE);

        /*
        if (macro_enabled && now - last_debug_gate_tick > 1000)
        {
            last_debug_gate_tick = now;
            std::cout << "[MACRO][GATE] fireHeld=" << (fireHeld ? 1 : 0)
                << " manualFire=" << (manualFire ? 1 : 0)
                << " triggerHeld=" << (trigger_firing ? 1 : 0)
                << " scoping=" << (isScoping ? 1 : 0)
                << " cursor=" << (hasCursor ? 1 : 0)
                << " reload=" << (isReloading ? 1 : 0)
                << " tab=" << (telemetryMemory::IsKeyDown(VK_TAB) ? 1 : 0)
                << " esc=" << (telemetryMemory::IsKeyDown(VK_ESCAPE) ? 1 : 0)
                << " weapon=" << normalized
                << std::endl;
        }
        */

        const ActiveProfile profile = GetActiveProfile();
        bool isSR = (current_category == CAT_SR);
        bool isDMR = (current_category == CAT_DMR);
        bool isSG = (current_category == CAT_SG);
        bool isAR = (current_category == CAT_AR || current_category == CAT_SMG || current_category == CAT_LMG);

        // --- PAOD Execution Block (Recoil & Aim Move) ---
        if (fireHeld)
        {
            if (!profile.enabled || !anim)
            {
                ResetAngleState();
                return;
            }

            if (now - last_fire_time > static_cast<ULONGLONG>((std::max)(profile.resetMs, 0)))
            {
                ResetAngleState();
            }
            last_fire_time = now;

            const AngleRotation recoilRot = telemetryMemory::Read<AngleRotation>(anim + telemetry_config::offsets::RecoilADSRotation_CP);
            const AngleRotation ctrlRot = telemetryMemory::Read<AngleRotation>(anim + telemetry_config::offsets::ControlRotation_CP);
            const Vector3 recoilVec = telemetryMemory::Read<Vector3>(anim + telemetry_config::offsets::RecoilValueVector);

            /*
            if (now - lastRecoilLog > 100) {
                lastRecoilLog = now;
                std::cout << "[RECOIL][RAW] Pitch=" << recoilRot.Pitch << " CtrlP=" << ctrlRot.Pitch << " VecY=" << recoilVec.y << std::endl;
            }
            */

            if (!std::isfinite(recoilRot.Yaw) || !std::isfinite(recoilRot.Pitch))
            {
                ResetAngleState();
                return;
            }

            if (!angle_anchor_valid)
            {
                angle_anchor_valid = true;
                angle_anchor_yaw = recoilRot.Yaw;
                angle_anchor_pitch = recoilRot.Pitch;
                last_recoil_pitch = 0.0f; // Start from 0 to capture first shot recoil
                last_recoil_yaw = 0.0f;
                angle_moved_x = 0.0f;
                angle_moved_y = 0.0f;
                pixel_remainder_x = 0.0f;
                pixel_remainder_y = 0.0f;
            }

            float pitchDelta = NormalizeAngle(recoilRot.Pitch - last_recoil_pitch);
            float yawDelta = NormalizeAngle(recoilRot.Yaw - last_recoil_yaw);

            // Cap negative delta to prevent "pulling up" during recovery jitter
            // but keep it slightly responsive
            if (pitchDelta < -0.05f) pitchDelta = 0.0f; 

            last_recoil_pitch = recoilRot.Pitch;
            last_recoil_yaw = recoilRot.Yaw;

            float p_m = 1.0f;
            if (characterState == 1 && current_gun_data.contains(skCrypt("c"))) p_m = current_gun_data[skCrypt("c")].get<float>();
            else if (characterState == 2 && current_gun_data.contains(skCrypt("z"))) p_m = current_gun_data[skCrypt("z")].get<float>();

            const float sens = GetSensMultiplier();
            
            // Re-enabled menu settings with 0-100 scale interpretation
            // multiplier and pitchGain now act as percentages (0-100)
            float strengthY = (g_Menu.macro_recoil_strength / 100.0f);
            
            float moveY = pitchDelta * 50.0f * strengthY * sens * res_scale_y * p_m;
            float moveX = 0.0f; 

            // Combined with Vector Recoil
            float vecDelta = recoilVec.y - last_vec_y;
            if (std::abs(vecDelta) < 10.0f && std::abs(vecDelta) > 0.001f) {
                moveY += vecDelta * 1.2f * strengthY * sens; 
            }
            last_vec_y = recoilVec.y;

            if (macro_humanize)
            {
                const float yFactor = 0.98f + (static_cast<float>(rand() % 41) / 1000.0f);
                moveY *= yFactor;
            }

            // --- 1:1 PAOD Sequence (Logic order corrected: Smoothing -> SingleStep Clamp) ---
            
            // Step 1: Smoothing (Visuals.cpp:2927) - Applied BEFORE clamping
            float totalDeltaSize = std::sqrt(moveX * moveX + moveY * moveY);
            float baseSmoothing = isSR ? profile.srBaseSmoothing : (isDMR ? profile.dmrBaseSmoothing : profile.arBaseSmoothing);
            
            baseSmoothing += (static_cast<float>(rand() % 101 - 50) / 1000.0f); // [-0.05, 0.05] randomization
            float baseFOV = profile.smoothFov + (static_cast<float>(rand() % 101 - 50) / 50.0f); 
            float maxIncrease = baseSmoothing * profile.maxSmoothIncrease;

            if (totalDeltaSize <= baseFOV && totalDeltaSize > 0.01f)
            {
                float scale = 1.0f - (totalDeltaSize / baseFOV);
                float dynamicSmoothing = baseSmoothing + (maxIncrease * scale);
                dynamicSmoothing = (std::max)(dynamicSmoothing, baseSmoothing);

                if (dynamicSmoothing > 1.01f)
                {
                    moveX /= dynamicSmoothing;
                    moveY /= dynamicSmoothing;
                }
            }

            // Step 2: SingleStep limit (Visuals.cpp:2981) - Final clamp
            float MovePerTimeY = isSR ? profile.srMaxStepY : (isDMR ? profile.dmrMaxStepY : profile.arMaxStepY);
            
            float fovRatio = 80.0f / (currentFOV > 0.0f ? currentFOV : 80.0f);
            MovePerTimeY *= fovRatio;

            if (isInVehicle) {
                MovePerTimeY *= 2.0f;
            }

            moveY = std::clamp(moveY, -MovePerTimeY, MovePerTimeY);

            const float moveYWithRemainder = moveY + pixel_remainder_y;
            const long outX = static_cast<long>(std::llround(moveX + pixel_remainder_x));
            const long outY = static_cast<long>(std::llround(moveYWithRemainder));
            if (outX != 0 || outY != 0)
            {
                const bool sent = telemetryMemory::MoveMouse(outX, outY);
                if (sent)
                {
                    pixel_remainder_x = (moveX + pixel_remainder_x) - static_cast<float>(outX);
                    pixel_remainder_y = moveYWithRemainder - static_cast<float>(outY);
                    angle_moved_x += static_cast<float>(outX);
                    angle_moved_y += static_cast<float>(outY);
                }
                if (!sent || now - last_debug_sendinput_tick > 500)
                {
                    last_debug_sendinput_tick = now;
                    std::cout << skCrypt("[MACRO][SENDINPUT] move=(") << outX << skCrypt(",") << outY << skCrypt(")")
                        << skCrypt(" ok=") << (sent ? 1 : 0)
                        << skCrypt(" err=") << SendInputMacro::LastErrorCode()
                        << skCrypt(" raw=(") << moveX << skCrypt(",") << moveY << skCrypt(")")
                        << skCrypt(" rem=(") << pixel_remainder_x << skCrypt(",") << pixel_remainder_y << skCrypt(")")
                        << skCrypt(" moved_total=(") << angle_moved_x << skCrypt(",") << angle_moved_y << skCrypt(")")
                        << std::endl;
                }
            }
            else
            {
                pixel_remainder_x = (moveX + pixel_remainder_x) - static_cast<float>(outX);
                pixel_remainder_y = moveYWithRemainder;

                if (std::fabs(moveY) > 0.01f && now - last_debug_sendinput_tick > 500)
                {
                    last_debug_sendinput_tick = now;
                    std::cout << skCrypt("[MACRO][SENDINPUT] pending_vertical raw=(") << moveX << skCrypt(",") << moveY << skCrypt(")")
                        << skCrypt(" rem=(") << pixel_remainder_x << skCrypt(",") << pixel_remainder_y << skCrypt(")")
                        << skCrypt(" rounded=(0,0)")
                        << std::endl;
                }
            }

            int sleepMs = (std::max)(profile.delayMs, 0);
            if (macro_humanize && profile.delayJitterMs > 0) sleepMs += (rand() % (profile.delayJitterMs + 1));
            if (sleepMs > 0) telemetryMemory::StealthSleep(sleepMs);
        }
        else
        {
            ResetAngleState();
        }

        // --- 3. Independent Trigger Pipeline (PAOD Visuals.cpp:7140) ---
        static bool lastTriggerToggleState = false;
        bool currentTriggerToggleState = telemetryMemory::IsKeyDown(VK_F6); // Assuming F6 for toggle for now as placeholder
        if (currentTriggerToggleState && !lastTriggerToggleState) {
            // Logic to toggle auto-shot
        }
        lastTriggerToggleState = currentTriggerToggleState;

        bool weaponTriggerEnabled = isSR ? profile.srTriggerEnabled : (isDMR ? profile.dmrTriggerEnabled : (isSG ? profile.sgTriggerEnabled : profile.arTriggerEnabled));
        bool triggerActive = weaponTriggerEnabled; // Simplification for now
        
        if (triggerActive && isScoping && !isReloading && !isSwimming)
        {
            // Placeholder for actual trigger logic if needed
        } 
        else 
        {
            if (trigger_firing) {
                SendInputMacro::Up();
                trigger_firing = false;
            }
            sr_sequence_start = 0;
        }
    }
};
