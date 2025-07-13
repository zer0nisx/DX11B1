#include "ConfigManager.h"
#include "Logger.h"
#include "FileSystem.h"
#include <sstream>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef CopyFile

using namespace GameEngine::Core;

ConfigManager& ConfigManager::GetInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::LoadConfig(const std::string& configFile) {
    m_configFile = configFile;

    if (!FILE_SYSTEM.FileExists(configFile)) {
        Logger::GetInstance().LogWarning("Config file not found: " + configFile + " - Loading defaults");
        LoadDefaultConfig();
        SaveConfig(configFile); // Create default config file
        return true;
    }

    auto document = XML_MANAGER.LoadDocument(configFile);
    if (!document) {
        Logger::GetInstance().LogError("Failed to load config file: " + configFile);
        LoadDefaultConfig();
        return false;
    }

    auto root = document->GetRoot();
    if (!root.IsValid() || root.GetName() != "GameEngineConfig") {
        Logger::GetInstance().LogError("Invalid config file format: " + configFile);
        LoadDefaultConfig();
        return false;
    }

    // Load all settings sections
    auto graphicsNode = root.GetFirstChild("Graphics");
    if (graphicsNode.IsValid()) {
        DeserializeGraphicsSettings(graphicsNode);
    }

    auto assetsNode = root.GetFirstChild("Assets");
    if (assetsNode.IsValid()) {
        DeserializeAssetSettings(assetsNode);
    }

    auto inputNode = root.GetFirstChild("Input");
    if (inputNode.IsValid()) {
        DeserializeInputSettings(inputNode);
    }

    auto engineNode = root.GetFirstChild("Engine");
    if (engineNode.IsValid()) {
        DeserializeEngineSettings(engineNode);
    }

    // Load custom settings
    auto customNode = root.GetFirstChild("Custom");
    if (customNode.IsValid()) {
        for (auto categoryNode : customNode.GetChildren("Category")) {
            std::string categoryName = categoryNode.GetAttributeValue("name");
            if (!categoryName.empty()) {
                for (auto settingNode : categoryNode.GetChildren("Setting")) {
                    std::string key = settingNode.GetAttributeValue("key");
                    std::string value = settingNode.GetAttributeValue("value");
                    if (!key.empty()) {
                        m_customSettings[categoryName][key] = value;
                    }
                }
            }
        }
    }

    m_settingsLoaded = true;
    Logger::GetInstance().LogInfo("Configuration loaded successfully from: " + configFile);
    return true;
}

bool ConfigManager::SaveConfig(const std::string& configFile) {
    auto document = XML_MANAGER.CreateDocument();
    document->AddDeclaration("1.0", "UTF-8");

    auto root = document->CreateRoot("GameEngineConfig");
    root.SetAttribute("version", "1.0");

    // Serialize all settings
    SerializeGraphicsSettings(root);
    SerializeAssetSettings(root);
    SerializeInputSettings(root);
    SerializeEngineSettings(root);

    // Serialize custom settings
    if (!m_customSettings.empty()) {
        auto customNode = root.AppendChild("Custom");
        for (const auto& category : m_customSettings) {
            auto categoryNode = customNode.AppendChild("Category");
            categoryNode.SetAttribute("name", category.first);

            for (const auto& setting : category.second) {
                auto settingNode = categoryNode.AppendChild("Setting");
                settingNode.SetAttribute("key", setting.first);
                settingNode.SetAttribute("value", setting.second);
            }
        }
    }

    bool result = XML_MANAGER.SaveDocument(document, configFile);
    if (result) {
        Logger::GetInstance().LogInfo("Configuration saved to: " + configFile);
    } else {
        Logger::GetInstance().LogError("Failed to save configuration to: " + configFile);
    }

    return result;
}

bool ConfigManager::LoadDefaultConfig() {
    InitializeDefaultSettings();
    m_settingsLoaded = true;
    Logger::GetInstance().LogInfo("Default configuration loaded");
    return true;
}

void ConfigManager::SetGraphicsSettings(const GraphicsSettings& settings) {
    m_graphicsSettings = settings;
}

void ConfigManager::SetAssetSettings(const AssetSettings& settings) {
    m_assetSettings = settings;

    // Update FileSystem assets directory
    FILE_SYSTEM.SetAssetsDirectory(settings.assetsDirectory);
}

void ConfigManager::SetInputSettings(const InputSettings& settings) {
    m_inputSettings = settings;
}

void ConfigManager::SetEngineSettings(const EngineSettings& settings) {
    m_engineSettings = settings;
}

void ConfigManager::SetKeyBinding(const std::string& action, int keyCode) {
    m_inputSettings.keyBindings[action] = keyCode;
}

int ConfigManager::GetKeyBinding(const std::string& action, int defaultKey) {
    auto it = m_inputSettings.keyBindings.find(action);
    return (it != m_inputSettings.keyBindings.end()) ? it->second : defaultKey;
}

bool ConfigManager::ValidateSettings() {
    bool valid = true;

    // Validate graphics settings
    if (m_graphicsSettings.windowWidth < 320 || m_graphicsSettings.windowWidth > 7680) {
        Logger::GetInstance().LogWarning("Invalid window width, resetting to 1024");
        m_graphicsSettings.windowWidth = 1024;
        valid = false;
    }

    if (m_graphicsSettings.windowHeight < 240 || m_graphicsSettings.windowHeight > 4320) {
        Logger::GetInstance().LogWarning("Invalid window height, resetting to 768");
        m_graphicsSettings.windowHeight = 768;
        valid = false;
    }

    if (m_graphicsSettings.maxFPS < 30 || m_graphicsSettings.maxFPS > 300) {
        Logger::GetInstance().LogWarning("Invalid max FPS, resetting to 60");
        m_graphicsSettings.maxFPS = 60;
        valid = false;
    }

    // Validate asset settings
    if (!FILE_SYSTEM.DirectoryExists(m_assetSettings.assetsDirectory)) {
        Logger::GetInstance().LogWarning("Assets directory doesn't exist: " + m_assetSettings.assetsDirectory);
        FILE_SYSTEM.CreateDirectories(m_assetSettings.assetsDirectory);
    }

    // Validate input settings
    if (m_inputSettings.mouseSensitivity < 0.1f || m_inputSettings.mouseSensitivity > 10.0f) {
        Logger::GetInstance().LogWarning("Invalid mouse sensitivity, resetting to 1.0");
        m_inputSettings.mouseSensitivity = 1.0f;
        valid = false;
    }

    return valid;
}

void ConfigManager::ResetToDefaults() {
    InitializeDefaultSettings();
    m_customSettings.clear();
    Logger::GetInstance().LogInfo("Configuration reset to defaults");
}

bool ConfigManager::CreateConfigBackup() {
    if (!FILE_SYSTEM.FileExists(m_configFile)) {
        return false;
    }

    std::string backupFile = m_configFile + ".backup";
    bool result = FILE_SYSTEM.CopyFile(m_configFile, backupFile);

    if (result) {
        Logger::GetInstance().LogInfo("Config backup created: " + backupFile);
    }

    return result;
}

bool ConfigManager::RestoreConfigBackup() {
    std::string backupFile = m_configFile + ".backup";

    if (!FILE_SYSTEM.FileExists(backupFile)) {
        Logger::GetInstance().LogError("Config backup not found: " + backupFile);
        return false;
    }

    bool result = FILE_SYSTEM.CopyFile(backupFile, m_configFile);
    if (result) {
        Logger::GetInstance().LogInfo("Config restored from backup");
        LoadConfig(m_configFile);
    }

    return result;
}

void ConfigManager::SerializeGraphicsSettings(XmlNode& parentNode) {
    auto graphicsNode = parentNode.AppendChild("Graphics");

    graphicsNode.SetAttribute("windowWidth", m_graphicsSettings.windowWidth);
    graphicsNode.SetAttribute("windowHeight", m_graphicsSettings.windowHeight);
    graphicsNode.SetAttribute("fullscreen", m_graphicsSettings.fullscreen);
    graphicsNode.SetAttribute("vsync", m_graphicsSettings.vsync);
    graphicsNode.SetAttribute("maxFPS", m_graphicsSettings.maxFPS);
    graphicsNode.SetAttribute("enableShadows", m_graphicsSettings.enableShadows);
    graphicsNode.SetAttribute("enableLighting", m_graphicsSettings.enableLighting);
    graphicsNode.SetAttribute("shadowQuality", m_graphicsSettings.shadowQuality);
    graphicsNode.SetAttribute("shaderPath", m_graphicsSettings.shaderPath);
}

void ConfigManager::SerializeAssetSettings(XmlNode& parentNode) {
    auto assetsNode = parentNode.AppendChild("Assets");

    assetsNode.SetAttribute("assetsDirectory", m_assetSettings.assetsDirectory);
    assetsNode.SetAttribute("modelsDirectory", m_assetSettings.modelsDirectory);
    assetsNode.SetAttribute("texturesDirectory", m_assetSettings.texturesDirectory);
    assetsNode.SetAttribute("audioDirectory", m_assetSettings.audioDirectory);
    assetsNode.SetAttribute("enableAssetCache", m_assetSettings.enableAssetCache);
    assetsNode.SetAttribute("maxCacheSize", m_assetSettings.maxCacheSize);
}

void ConfigManager::SerializeInputSettings(XmlNode& parentNode) {
    auto inputNode = parentNode.AppendChild("Input");

    inputNode.SetAttribute("mouseSensitivity", m_inputSettings.mouseSensitivity);
    inputNode.SetAttribute("invertMouse", m_inputSettings.invertMouse);

    // Serialize key bindings
    if (!m_inputSettings.keyBindings.empty()) {
        auto bindingsNode = inputNode.AppendChild("KeyBindings");
        for (const auto& binding : m_inputSettings.keyBindings) {
            auto bindingNode = bindingsNode.AppendChild("Binding");
            bindingNode.SetAttribute("action", binding.first);
            bindingNode.SetAttribute("key", binding.second);
        }
    }
}

void ConfigManager::SerializeEngineSettings(XmlNode& parentNode) {
    auto engineNode = parentNode.AppendChild("Engine");

    engineNode.SetAttribute("logLevel", m_engineSettings.logLevel);
    engineNode.SetAttribute("logFile", m_engineSettings.logFile);
    engineNode.SetAttribute("enableLogging", m_engineSettings.enableLogging);
    engineNode.SetAttribute("enableDebugOutput", m_engineSettings.enableDebugOutput);
    engineNode.SetAttribute("maxLogFileSize", m_engineSettings.maxLogFileSize);
}

void ConfigManager::DeserializeGraphicsSettings(const XmlNode& parentNode) {
    m_graphicsSettings.windowWidth = parentNode.GetAttributeValueAsInt("windowWidth", 1024);
    m_graphicsSettings.windowHeight = parentNode.GetAttributeValueAsInt("windowHeight", 768);
    m_graphicsSettings.fullscreen = parentNode.GetAttributeValueAsBool("fullscreen", false);
    m_graphicsSettings.vsync = parentNode.GetAttributeValueAsBool("vsync", true);
    m_graphicsSettings.maxFPS = parentNode.GetAttributeValueAsInt("maxFPS", 60);
    m_graphicsSettings.enableShadows = parentNode.GetAttributeValueAsBool("enableShadows", true);
    m_graphicsSettings.enableLighting = parentNode.GetAttributeValueAsBool("enableLighting", true);
    m_graphicsSettings.shadowQuality = parentNode.GetAttributeValueAsFloat("shadowQuality", 1.0f);
    m_graphicsSettings.shaderPath = parentNode.GetAttributeValue("shaderPath", "Shaders");
}

void ConfigManager::DeserializeAssetSettings(const XmlNode& parentNode) {
    m_assetSettings.assetsDirectory = parentNode.GetAttributeValue("assetsDirectory", "Assets");
    m_assetSettings.modelsDirectory = parentNode.GetAttributeValue("modelsDirectory", "Models");
    m_assetSettings.texturesDirectory = parentNode.GetAttributeValue("texturesDirectory", "Textures");
    m_assetSettings.audioDirectory = parentNode.GetAttributeValue("audioDirectory", "Audio");
    m_assetSettings.enableAssetCache = parentNode.GetAttributeValueAsBool("enableAssetCache", true);
    m_assetSettings.maxCacheSize = parentNode.GetAttributeValueAsInt("maxCacheSize", 512);
}

void ConfigManager::DeserializeInputSettings(const XmlNode& parentNode) {
    m_inputSettings.mouseSensitivity = parentNode.GetAttributeValueAsFloat("mouseSensitivity", 1.0f);
    m_inputSettings.invertMouse = parentNode.GetAttributeValueAsBool("invertMouse", false);

    // Deserialize key bindings
    auto bindingsNode = parentNode.GetFirstChild("KeyBindings");
    if (bindingsNode.IsValid()) {
        for (auto bindingNode : bindingsNode.GetChildren("Binding")) {
            std::string action = bindingNode.GetAttributeValue("action");
            int key = bindingNode.GetAttributeValueAsInt("key", 0);

            if (!action.empty() && key != 0) {
                m_inputSettings.keyBindings[action] = key;
            }
        }
    }
}

void ConfigManager::DeserializeEngineSettings(const XmlNode& parentNode) {
    m_engineSettings.logLevel = parentNode.GetAttributeValue("logLevel", "Info");
    m_engineSettings.logFile = parentNode.GetAttributeValue("logFile", "engine.log");
    m_engineSettings.enableLogging = parentNode.GetAttributeValueAsBool("enableLogging", true);
    m_engineSettings.enableDebugOutput = parentNode.GetAttributeValueAsBool("enableDebugOutput", false);
    m_engineSettings.maxLogFileSize = parentNode.GetAttributeValueAsInt("maxLogFileSize", 10);
}

void ConfigManager::InitializeDefaultSettings() {
    // Graphics defaults
    m_graphicsSettings = GraphicsSettings{};

    // Asset defaults
    m_assetSettings = AssetSettings{};

    // Input defaults
    m_inputSettings = InputSettings{};

    // Initialize default key bindings
    m_inputSettings.keyBindings["Forward"] = 'W';
    m_inputSettings.keyBindings["Backward"] = 'S';
    m_inputSettings.keyBindings["Left"] = 'A';
    m_inputSettings.keyBindings["Right"] = 'D';
    m_inputSettings.keyBindings["Jump"] = ' ';
    m_inputSettings.keyBindings["Run"] = VK_SHIFT;
    m_inputSettings.keyBindings["Exit"] = VK_ESCAPE;
    m_inputSettings.keyBindings["ToggleLighting"] = 'L';
    m_inputSettings.keyBindings["ToggleShadows"] = 'S';
    m_inputSettings.keyBindings["ChangeTexture"] = 'T';
    m_inputSettings.keyBindings["ResetRotation"] = 'R';
    m_inputSettings.keyBindings["CreateCube"] = 'C';
    m_inputSettings.keyBindings["ShowInfo"] = VK_SPACE;

    // Engine defaults
    m_engineSettings = EngineSettings{};
}
