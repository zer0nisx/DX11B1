#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

namespace GameEngine {
namespace Core {

// Forward declarations
class XmlNode;
class XmlDocument;

// XML attribute wrapper
class XmlAttribute {
public:
    XmlAttribute(rapidxml::xml_attribute<>* attr);

    std::string GetName() const;
    std::string GetValue() const;

    // Type conversions
    int GetValueAsInt(int defaultValue = 0) const;
    float GetValueAsFloat(float defaultValue = 0.0f) const;
    bool GetValueAsBool(bool defaultValue = false) const;

    // Validation
    bool IsValid() const { return m_attribute != nullptr; }

private:
    rapidxml::xml_attribute<>* m_attribute;
};

// XML node wrapper
class XmlNode {
public:
    XmlNode(rapidxml::xml_node<>* node = nullptr);

    // Node information
    std::string GetName() const;
    std::string GetValue() const;
    void SetValue(const std::string& value);

    // Type conversions for value
    int GetValueAsInt(int defaultValue = 0) const;
    float GetValueAsFloat(float defaultValue = 0.0f) const;
    bool GetValueAsBool(bool defaultValue = false) const;

    // Attributes
    XmlAttribute GetAttribute(const std::string& name) const;
    std::string GetAttributeValue(const std::string& name, const std::string& defaultValue = "") const;
    int GetAttributeValueAsInt(const std::string& name, int defaultValue = 0) const;
    float GetAttributeValueAsFloat(const std::string& name, float defaultValue = 0.0f) const;
    bool GetAttributeValueAsBool(const std::string& name, bool defaultValue = false) const;

    void SetAttribute(const std::string& name, const std::string& value);
    void SetAttribute(const std::string& name, int value);
    void SetAttribute(const std::string& name, float value);
    void SetAttribute(const std::string& name, bool value);

    bool HasAttribute(const std::string& name) const;
    void RemoveAttribute(const std::string& name);
    std::vector<XmlAttribute> GetAllAttributes() const;

    // Children navigation
    XmlNode GetFirstChild() const;
    XmlNode GetFirstChild(const std::string& name) const;
    XmlNode GetNextSibling() const;
    XmlNode GetNextSibling(const std::string& name) const;
    XmlNode GetParent() const;

    std::vector<XmlNode> GetChildren() const;
    std::vector<XmlNode> GetChildren(const std::string& name) const;

    // Children manipulation
    XmlNode AppendChild(const std::string& name);
    XmlNode PrependChild(const std::string& name);
    XmlNode InsertChild(const std::string& name, const XmlNode& beforeThis);
    void RemoveChild(const XmlNode& child);
    void RemoveAllChildren();

    // Content helpers
    std::string GetText() const;
    void SetText(const std::string& text);

    // Validation
    bool IsValid() const { return m_node != nullptr; }
    bool IsEmpty() const;

    // Iteration support
    class Iterator {
    public:
        Iterator(rapidxml::xml_node<>* node) : m_current(node) {}

        XmlNode operator*() const { return XmlNode(m_current); }
        Iterator& operator++() {
            m_current = m_current ? m_current->next_sibling() : nullptr;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return m_current != other.m_current;
        }

    private:
        rapidxml::xml_node<>* m_current;
    };

    Iterator begin() const;
    Iterator end() const;

private:
    friend class XmlDocument;
    rapidxml::xml_node<>* m_node;
    rapidxml::xml_document<>* m_document;
};

// XML document wrapper
class XmlDocument {
public:
    XmlDocument();
    ~XmlDocument();

    // Loading and saving
    bool LoadFromFile(const std::string& filename);
    bool LoadFromString(const std::string& xmlContent);
    bool SaveToFile(const std::string& filename, bool formatted = true) const;
    std::string SaveToString(bool formatted = true) const;

    // Document manipulation
    void Clear();
    bool IsLoaded() const { return m_loaded; }

    // Root node access
    XmlNode GetRoot() const;
    XmlNode CreateRoot(const std::string& name);

    // Node creation (for building XML)
    XmlNode CreateNode(const std::string& name);
    XmlAttribute CreateAttribute(const std::string& name, const std::string& value);

    // Document information
    std::string GetFilename() const { return m_filename; }
    std::string GetEncoding() const;
    void SetEncoding(const std::string& encoding);

    // XML declaration
    void AddDeclaration(const std::string& version = "1.0",
                       const std::string& encoding = "UTF-8",
                       const std::string& standalone = "");

private:
    rapidxml::xml_document<> m_document;
    std::vector<char> m_xmlContent;
    std::string m_filename;
    bool m_loaded;

    // Memory management for RapidXML
    std::vector<std::unique_ptr<char[]>> m_allocatedStrings;

    char* AllocateString(const std::string& str);
    void ClearAllocatedStrings();
    void PrintNode(const rapidxml::xml_node<>* node, std::string& result, int indent, bool formatted) const;
};

// XML Manager for high-level operations
class XmlManager {
public:
    static XmlManager& GetInstance();

    // Document management
    std::shared_ptr<XmlDocument> LoadDocument(const std::string& filename);
    std::shared_ptr<XmlDocument> CreateDocument();
    bool SaveDocument(std::shared_ptr<XmlDocument> document, const std::string& filename);

    // Cache management
    void ClearCache();
    bool IsDocumentCached(const std::string& filename) const;
    void RemoveFromCache(const std::string& filename);

    // Validation
    bool ValidateXmlFile(const std::string& filename) const;
    bool ValidateXmlString(const std::string& xmlContent) const;

    // Utility functions
    std::string EscapeXmlString(const std::string& str) const;
    std::string UnescapeXmlString(const std::string& str) const;

    // Configuration file helpers
    bool LoadConfig(const std::string& filename, std::unordered_map<std::string, std::string>& config);
    bool SaveConfig(const std::string& filename, const std::unordered_map<std::string, std::string>& config);

    // Serialization helpers
    template<typename T>
    bool SerializeObject(const T& object, XmlNode& node,
                        std::function<void(const T&, XmlNode&)> serializer);

    template<typename T>
    bool DeserializeObject(T& object, const XmlNode& node,
                          std::function<void(T&, const XmlNode&)> deserializer);

private:
    XmlManager() = default;
    ~XmlManager() = default;
    XmlManager(const XmlManager&) = delete;
    XmlManager& operator=(const XmlManager&) = delete;

    std::unordered_map<std::string, std::weak_ptr<XmlDocument>> m_documentCache;
};

// Convenience macros
#define XML_MANAGER GameEngine::Core::XmlManager::GetInstance()

// Template implementations
template<typename T>
bool XmlManager::SerializeObject(const T& object, XmlNode& node,
                                std::function<void(const T&, XmlNode&)> serializer) {
    try {
        if (!node.IsValid()) {
            return false;
        }

        serializer(object, node);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

template<typename T>
bool XmlManager::DeserializeObject(T& object, const XmlNode& node,
                                  std::function<void(T&, const XmlNode&)> deserializer) {
    try {
        if (!node.IsValid()) {
            return false;
        }

        deserializer(object, node);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

} // namespace Core
} // namespace GameEngine
