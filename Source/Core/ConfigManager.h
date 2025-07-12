#pragma once

#include "XmlManager.h"
#include <unordered_map>
#include <string>

namespace GameEngine {
namespace Core {

struct GraphicsSettings {
    int windowWidth = 1024;
    int windowHeight = 768;
    bool fullscreen = false;
    bool vsync = true;
    int maxFPS = 60;
    bool enableShadows = true;
    bool enableLighting = true;
    float shadowQuality = 1.0f;
    std::string shaderPath = "Shaders";
};

struct AssetSettings {
    std::string assetsDirectory = "Assets";
    std::string modelsDirectory = "Models";
    std::string texturesDirectory = "Textures";
    std::string audioDirectory = "Audio";
    bool enableAssetCache = true;
    int maxCacheSize = 512; // MB
};

struct InputSettings {
    float mouseSensitivity = 1.0f;
    bool invertMouse = false;
    std::unordered_map<std::string, int> keyBindings;
};

struct EngineSettings {
    std::string logLevel = "Info";
    std::string logFile = "engine.log";
    bool enableLogging = true;
    bool enableDebugOutput = false;
    int maxLogFileSize = 10; // MB
};

class ConfigManager {
public:
    static ConfigManager& GetInstance();

    // Configuration loading/saving
    bool LoadConfig(const std::string& configFile = "config.xml");
    bool SaveConfig(const std::string& configFile = "config.xml");
    bool LoadDefaultConfig();

    // Settings access
    const GraphicsSettings& GetGraphicsSettings() const { return m_graphicsSettings; }
    const AssetSettings& GetAssetSettings() const { return m_assetSettings; }
    const InputSettings& GetInputSettings() const { return m_inputSettings; }
    const EngineSettings& GetEngineSettings() const { return m_engineSettings; }

    // Settings modification
    void SetGraphicsSettings(const GraphicsSettings& settings);
    void SetAssetSettings(const AssetSettings& settings);
    void SetInputSettings(const InputSettings& settings);
    void SetEngineSettings(const EngineSettings& settings);

    // Individual setting access
    template<typename T>
    T GetSetting(const std::string& category, const std::string& key, const T& defaultValue = T{});

    template<typename T>
    void SetSetting(const std::string& category, const std::string& key, const T& value);

    // Key bindings
    void SetKeyBinding(const std::string& action, int keyCode);
    int GetKeyBinding(const std::string& action, int defaultKey = 0);

    // Validation
    bool ValidateSettings();
    void ResetToDefaults();

    // File management
    bool CreateConfigBackup();
    bool RestoreConfigBackup();
    std::string GetConfigFilePath() const { return m_configFile; }

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    // Serialization helpers
    void SerializeGraphicsSettings(XmlNode& parentNode);
    void SerializeAssetSettings(XmlNode& parentNode);
    void SerializeInputSettings(XmlNode& parentNode);
    void SerializeEngineSettings(XmlNode& parentNode);

    void DeserializeGraphicsSettings(const XmlNode& parentNode);
    void DeserializeAssetSettings(const XmlNode& parentNode);
    void DeserializeInputSettings(const XmlNode& parentNode);
    void DeserializeEngineSettings(const XmlNode& parentNode);

    // Default initialization
    void InitializeDefaultSettings();

private:
    GraphicsSettings m_graphicsSettings;
    AssetSettings m_assetSettings;
    InputSettings m_inputSettings;
    EngineSettings m_engineSettings;

    std::string m_configFile = "config.xml";
    bool m_settingsLoaded = false;

    // Additional runtime settings
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_customSettings;
};

// Template implementations
template<typename T>
T ConfigManager::GetSetting(const std::string& category, const std::string& key, const T& defaultValue) {
    auto categoryIt = m_customSettings.find(category);
    if (categoryIt != m_customSettings.end()) {
        auto keyIt = categoryIt->second.find(key);
        if (keyIt != categoryIt->second.end()) {
            // Convert string to T - basic implementation
            if constexpr (std::is_same_v<T, std::string>) {
                return keyIt->second;
            } else if constexpr (std::is_same_v<T, int>) {
                try { return std::stoi(keyIt->second); }
                catch (...) { return defaultValue; }
            } else if constexpr (std::is_same_v<T, float>) {
                try { return std::stof(keyIt->second); }
                catch (...) { return defaultValue; }
            } else if constexpr (std::is_same_v<T, bool>) {
                return keyIt->second == "true" || keyIt->second == "1";
            }
        }
    }
    return defaultValue;
}

template<typename T>
void ConfigManager::SetSetting(const std::string& category, const std::string& key, const T& value) {
    std::string stringValue;

    if constexpr (std::is_same_v<T, std::string>) {
        stringValue = value;
    } else if constexpr (std::is_same_v<T, int>) {
        stringValue = std::to_string(value);
    } else if constexpr (std::is_same_v<T, float>) {
        stringValue = std::to_string(value);
    } else if constexpr (std::is_same_v<T, bool>) {
        stringValue = value ? "true" : "false";
    }

    m_customSettings[category][key] = stringValue;
}

// Convenience macro
#define CONFIG_MANAGER GameEngine::Core::ConfigManager::GetInstance()

} // namespace Core
} // namespace GameEngine
