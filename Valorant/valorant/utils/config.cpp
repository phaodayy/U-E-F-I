#include "config.h"
#include <fstream>
#include <sstream>

namespace Config {
ConfigManager &ConfigManager::Get() {
  static ConfigManager instance;
  return instance;
}

void ConfigManager::Load(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open())
    return;

  std::string line;
  while (std::getline(file, line)) {
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      m_Data[key] = value;
    }
  }

  file.close();
}

void ConfigManager::Save(const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open())
    return;

  for (const auto &pair : m_Data) {
    file << pair.first << "=" << pair.second << "\n";
  }

  file.close();
}

void ConfigManager::SetBool(const std::string &key, bool value) {
  m_Data[key] = value ? "1" : "0";
}

void ConfigManager::SetFloat(const std::string &key, float value) {
  m_Data[key] = std::to_string(value);
}

void ConfigManager::SetInt(const std::string &key, int value) {
  m_Data[key] = std::to_string(value);
}

bool ConfigManager::GetBool(const std::string &key, bool defaultValue) {
  auto it = m_Data.find(key);
  if (it == m_Data.end())
    return defaultValue;
  return it->second == "1";
}

float ConfigManager::GetFloat(const std::string &key, float defaultValue) {
  auto it = m_Data.find(key);
  if (it == m_Data.end())
    return defaultValue;
  return std::stof(it->second);
}

int ConfigManager::GetInt(const std::string &key, int defaultValue) {
  auto it = m_Data.find(key);
  if (it == m_Data.end())
    return defaultValue;
  return std::stoi(it->second);
}
} // namespace Config
