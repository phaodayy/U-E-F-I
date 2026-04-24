#pragma once
#include <string>
#include <map>

namespace Config
{
    // Configuration manager
    class ConfigManager
    {
    public:
        static ConfigManager& Get();
        
        void Load(const std::string& filename);
        void Save(const std::string& filename);
        
        void SetBool(const std::string& key, bool value);
        void SetFloat(const std::string& key, float value);
        void SetInt(const std::string& key, int value);
        
        bool GetBool(const std::string& key, bool defaultValue = false);
        float GetFloat(const std::string& key, float defaultValue = 0.0f);
        int GetInt(const std::string& key, int defaultValue = 0);
        
    private:
        ConfigManager() = default;
        std::map<std::string, std::string> m_Data;
    };
}
