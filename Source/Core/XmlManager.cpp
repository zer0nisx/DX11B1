#include "XmlManager.h"
#include "Logger.h"
#include "FileSystem.h"
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace GameEngine::Core;

// XmlAttribute implementation
XmlAttribute::XmlAttribute(rapidxml::xml_attribute<>* attr) : m_attribute(attr) {}

std::string XmlAttribute::GetName() const {
	return m_attribute ? std::string(m_attribute->name(), m_attribute->name_size()) : "";
}

std::string XmlAttribute::GetValue() const {
	return m_attribute ? std::string(m_attribute->value(), m_attribute->value_size()) : "";
}

int XmlAttribute::GetValueAsInt(int defaultValue) const {
	try {
		std::string value = GetValue();
		return value.empty() ? defaultValue : std::stoi(value);
	}
	catch (const std::exception&) {
		return defaultValue;
	}
}

float XmlAttribute::GetValueAsFloat(float defaultValue) const {
	try {
		std::string value = GetValue();
		return value.empty() ? defaultValue : std::stof(value);
	}
	catch (const std::exception&) {
		return defaultValue;
	}
}

bool XmlAttribute::GetValueAsBool(bool defaultValue) const {
	std::string value = GetValue();
	if (value.empty()) return defaultValue;

	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	return (value == "true" || value == "1" || value == "yes" || value == "on");
}

// XmlNode implementation
XmlNode::XmlNode(rapidxml::xml_node<>* node) : m_node(node), m_document(nullptr) {
	if (m_node) {
		m_document = m_node->document();
	}
}

std::string XmlNode::GetName() const {
	return m_node ? std::string(m_node->name(), m_node->name_size()) : "";
}

std::string XmlNode::GetValue() const {
	return m_node ? std::string(m_node->value(), m_node->value_size()) : "";
}

void XmlNode::SetValue(const std::string& value) {
	if (!m_node || !m_document) return;

	char* allocatedValue = m_document->allocate_string(value.c_str());
	m_node->value(allocatedValue);
}

int XmlNode::GetValueAsInt(int defaultValue) const {
	try {
		std::string value = GetValue();
		return value.empty() ? defaultValue : std::stoi(value);
	}
	catch (const std::exception&) {
		return defaultValue;
	}
}

float XmlNode::GetValueAsFloat(float defaultValue) const {
	try {
		std::string value = GetValue();
		return value.empty() ? defaultValue : std::stof(value);
	}
	catch (const std::exception&) {
		return defaultValue;
	}
}

bool XmlNode::GetValueAsBool(bool defaultValue) const {
	std::string value = GetValue();
	if (value.empty()) return defaultValue;

	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	return (value == "true" || value == "1" || value == "yes" || value == "on");
}

XmlAttribute XmlNode::GetAttribute(const std::string& name) const {
	if (!m_node) return XmlAttribute(nullptr);

	auto attr = m_node->first_attribute(name.c_str());
	return XmlAttribute(attr);
}

std::string XmlNode::GetAttributeValue(const std::string& name, const std::string& defaultValue) const {
	auto attr = GetAttribute(name);
	return attr.IsValid() ? attr.GetValue() : defaultValue;
}

int XmlNode::GetAttributeValueAsInt(const std::string& name, int defaultValue) const {
	auto attr = GetAttribute(name);
	return attr.IsValid() ? attr.GetValueAsInt(defaultValue) : defaultValue;
}

float XmlNode::GetAttributeValueAsFloat(const std::string& name, float defaultValue) const {
	auto attr = GetAttribute(name);
	return attr.IsValid() ? attr.GetValueAsFloat(defaultValue) : defaultValue;
}

bool XmlNode::GetAttributeValueAsBool(const std::string& name, bool defaultValue) const {
	auto attr = GetAttribute(name);
	return attr.IsValid() ? attr.GetValueAsBool(defaultValue) : defaultValue;
}

void XmlNode::SetAttribute(const std::string& name, const std::string& value) {
	if (!m_node || !m_document) return;

	// Check if attribute already exists
	auto existingAttr = m_node->first_attribute(name.c_str());
	if (existingAttr) {
		char* allocatedValue = m_document->allocate_string(value.c_str());
		existingAttr->value(allocatedValue);
	}
	else {
		char* allocatedName = m_document->allocate_string(name.c_str());
		char* allocatedValue = m_document->allocate_string(value.c_str());
		auto newAttr = m_document->allocate_attribute(allocatedName, allocatedValue);
		m_node->append_attribute(newAttr);
	}
}

void XmlNode::SetAttribute(const std::string& name, int value) {
	std::string stringValue = std::to_string(value);
	SetAttribute(name, stringValue);
}

void XmlNode::SetAttribute(const std::string& name, float value) {
	std::string stringValue = std::to_string(value);
	SetAttribute(name, stringValue);
}

void XmlNode::SetAttribute(const std::string& name, bool value) {
	std::string stringValue = value ? "true" : "false";
	SetAttribute(name, stringValue);
}

bool XmlNode::HasAttribute(const std::string& name) const {
	return m_node && m_node->first_attribute(name.c_str()) != nullptr;
}

void XmlNode::RemoveAttribute(const std::string& name) {
	if (!m_node) return;

	auto attr = m_node->first_attribute(name.c_str());
	if (attr) {
		m_node->remove_attribute(attr);
	}
}

std::vector<XmlAttribute> XmlNode::GetAllAttributes() const {
	std::vector<XmlAttribute> attributes;

	if (!m_node) return attributes;

	for (auto attr = m_node->first_attribute(); attr; attr = attr->next_attribute()) {
		attributes.emplace_back(attr);
	}

	return attributes;
}

XmlNode XmlNode::GetFirstChild() const {
	return m_node ? XmlNode(m_node->first_node()) : XmlNode();
}

XmlNode XmlNode::GetFirstChild(const std::string& name) const {
	return m_node ? XmlNode(m_node->first_node(name.c_str())) : XmlNode();
}

XmlNode XmlNode::GetNextSibling() const {
	return m_node ? XmlNode(m_node->next_sibling()) : XmlNode();
}

XmlNode XmlNode::GetNextSibling(const std::string& name) const {
	return m_node ? XmlNode(m_node->next_sibling(name.c_str())) : XmlNode();
}

XmlNode XmlNode::GetParent() const {
	return m_node ? XmlNode(m_node->parent()) : XmlNode();
}

std::vector<XmlNode> XmlNode::GetChildren() const {
	std::vector<XmlNode> children;

	if (!m_node) return children;

	for (auto child = m_node->first_node(); child; child = child->next_sibling()) {
		children.emplace_back(child);
	}

	return children;
}

std::vector<XmlNode> XmlNode::GetChildren(const std::string& name) const {
	std::vector<XmlNode> children;

	if (!m_node) return children;

	for (auto child = m_node->first_node(name.c_str()); child; child = child->next_sibling(name.c_str())) {
		children.emplace_back(child);
	}

	return children;
}

XmlNode XmlNode::AppendChild(const std::string& name) {
	if (!m_node || !m_document) return XmlNode();

	char* allocatedName = m_document->allocate_string(name.c_str());
	auto newNode = m_document->allocate_node(rapidxml::node_element, allocatedName);
	m_node->append_node(newNode);

	return XmlNode(newNode);
}

XmlNode XmlNode::PrependChild(const std::string& name) {
	if (!m_node || !m_document) return XmlNode();

	char* allocatedName = m_document->allocate_string(name.c_str());
	auto newNode = m_document->allocate_node(rapidxml::node_element, allocatedName);
	m_node->prepend_node(newNode);

	return XmlNode(newNode);
}

XmlNode XmlNode::InsertChild(const std::string& name, const XmlNode& beforeThis) {
	if (!m_node || !m_document || !beforeThis.m_node) return XmlNode();

	char* allocatedName = m_document->allocate_string(name.c_str());
	auto newNode = m_document->allocate_node(rapidxml::node_element, allocatedName);
	m_node->insert_node(beforeThis.m_node, newNode);

	return XmlNode(newNode);
}

void XmlNode::RemoveChild(const XmlNode& child) {
	if (m_node && child.m_node) {
		m_node->remove_node(child.m_node);
	}
}

void XmlNode::RemoveAllChildren() {
	if (!m_node) return;

	m_node->remove_all_nodes();
}

std::string XmlNode::GetText() const {
	if (!m_node) return "";

	// Look for text node child
	for (auto child = m_node->first_node(); child; child = child->next_sibling()) {
		if (child->type() == rapidxml::node_data || child->type() == rapidxml::node_cdata) {
			return std::string(child->value(), child->value_size());
		}
	}

	return GetValue();
}

void XmlNode::SetText(const std::string& text) {
	if (!m_node || !m_document) return;

	// Remove existing text nodes
	auto child = m_node->first_node();
	while (child) {
		auto next = child->next_sibling();
		if (child->type() == rapidxml::node_data || child->type() == rapidxml::node_cdata) {
			m_node->remove_node(child);
		}
		child = next;
	}

	// Add new text node
	if (!text.empty()) {
		char* allocatedText = m_document->allocate_string(text.c_str());
		auto textNode = m_document->allocate_node(rapidxml::node_data, nullptr, allocatedText);
		m_node->append_node(textNode);
	}
}

bool XmlNode::IsEmpty() const {
	return !m_node || (m_node->first_node() == nullptr &&
		m_node->first_attribute() == nullptr &&
		std::string(m_node->value(), m_node->value_size()).empty());
}

XmlNode::Iterator XmlNode::begin() const {
	return Iterator(m_node ? m_node->first_node() : nullptr);
}

XmlNode::Iterator XmlNode::end() const {
	return Iterator(nullptr);
}

// XmlDocument implementation
XmlDocument::XmlDocument() : m_loaded(false) {
	m_document.clear();
}

XmlDocument::~XmlDocument() {
	Clear();
}

bool XmlDocument::LoadFromFile(const std::string& filename) {
	Clear();

	try {
		if (!FILE_SYSTEM.FileExists(filename)) {
			Logger::GetInstance().LogError("XmlDocument::LoadFromFile - File not found: " + filename);
			return false;
		}

		std::string content;
		if (!FILE_SYSTEM.ReadTextFile(filename, content)) {
			Logger::GetInstance().LogError("XmlDocument::LoadFromFile - Failed to read file: " + filename);
			return false;
		}

		if (LoadFromString(content)) {
			m_filename = filename;
			return true;
		}

		return false;
	}
	catch (const std::exception& e) {
		Logger::GetInstance().LogError("XmlDocument::LoadFromFile - Exception: " + std::string(e.what()));
		return false;
	}
}

bool XmlDocument::LoadFromString(const std::string& xmlContent) {
	Clear();

	try {
		if (xmlContent.empty()) {
			Logger::GetInstance().LogError("XmlDocument::LoadFromString - Empty XML content");
			return false;
		}

		m_xmlContent.resize(xmlContent.size() + 1);
		std::copy(xmlContent.begin(), xmlContent.end(), m_xmlContent.begin());
		m_xmlContent[xmlContent.size()] = '\0';

		m_document.parse<0>(m_xmlContent.data());
		m_loaded = true;

		Logger::GetInstance().LogInfo("XML document loaded successfully");
		return true;
	}
	catch (const rapidxml::parse_error& e) {
		Logger::GetInstance().LogError("XmlDocument::LoadFromString - Parse error: " + std::string(e.what()));
		Clear();
		return false;
	}
	catch (const std::exception& e) {
		Logger::GetInstance().LogError("XmlDocument::LoadFromString - Exception: " + std::string(e.what()));
		Clear();
		return false;
	}
}

bool XmlDocument::SaveToFile(const std::string& filename, bool formatted) const {
	if (!m_loaded) {
		Logger::GetInstance().LogError("XmlDocument::SaveToFile - No document loaded");
		return false;
	}

	try {
		std::string content = SaveToString(formatted);
		if (content.empty()) {
			return false;
		}

		bool result = FILE_SYSTEM.WriteTextFile(filename, content);
		if (result) {
			Logger::GetInstance().LogInfo("XML document saved to: " + filename);
		}
		return result;
	}
	catch (const std::exception& e) {
		Logger::GetInstance().LogError("XmlDocument::SaveToFile - Exception: " + std::string(e.what()));
		return false;
	}
}

std::string XmlDocument::SaveToString(bool formatted) const {
	if (!m_loaded) {
		Logger::GetInstance().LogError("XmlDocument::SaveToString - No document loaded");
		return "";
	}

	try {
		std::string result;
		PrintNode(&m_document, result, 0, formatted);
		return result;
	}
	catch (const std::exception& e) {
		Logger::GetInstance().LogError("XmlDocument::SaveToString - Exception: " + std::string(e.what()));
		return "";
	}
}

void XmlDocument::PrintNode(const rapidxml::xml_node<>* node, std::string& result, int indent, bool formatted) const {
	if (!node) return;

	std::string indentStr = formatted ? std::string(indent * 2, ' ') : "";

	if (node->type() == rapidxml::node_element) {
		result += indentStr + "<" + std::string(node->name(), node->name_size());

		// Add attributes
		for (auto attr = node->first_attribute(); attr; attr = attr->next_attribute()) {
			result += " " + std::string(attr->name(), attr->name_size()) +
				"=\"" + std::string(attr->value(), attr->value_size()) + "\"";
		}

		if (node->first_node() || node->value_size() > 0) {
			result += ">";

			if (node->value_size() > 0) {
				result += std::string(node->value(), node->value_size());
			}

			if (formatted && node->first_node()) result += "\n";

			// Print child nodes
			for (auto child = node->first_node(); child; child = child->next_sibling()) {
				PrintNode(child, result, indent + 1, formatted);
			}

			if (formatted && node->first_node()) result += indentStr;
			result += "</" + std::string(node->name(), node->name_size()) + ">";
		}
		else {
			result += "/>";
		}

		if (formatted) result += "\n";
	}
	else if (node->type() == rapidxml::node_data) {
		result += std::string(node->value(), node->value_size());
	}

	// Process sibling nodes at document level
	if (indent == 0) {
		for (auto sibling = node->next_sibling(); sibling; sibling = sibling->next_sibling()) {
			PrintNode(sibling, result, indent, formatted);
		}
	}
}

void XmlDocument::Clear() {
	m_document.clear();
	m_xmlContent.clear();
	ClearAllocatedStrings();
	m_filename.clear();
	m_loaded = false;
}

XmlNode XmlDocument::GetRoot() const {
	return m_loaded ? XmlNode(m_document.first_node()) : XmlNode();
}

XmlNode XmlDocument::CreateRoot(const std::string& name) {
	if (!name.empty()) {
		Clear();
		m_loaded = true;
	}

	char* allocatedName = AllocateString(name);
	auto rootNode = m_document.allocate_node(rapidxml::node_element, allocatedName);
	m_document.append_node(rootNode);

	return XmlNode(rootNode);
}

XmlNode XmlDocument::CreateNode(const std::string& name) {
	char* allocatedName = AllocateString(name);
	auto node = m_document.allocate_node(rapidxml::node_element, allocatedName);
	return XmlNode(node);
}

XmlAttribute XmlDocument::CreateAttribute(const std::string& name, const std::string& value) {
	char* allocatedName = AllocateString(name);
	char* allocatedValue = AllocateString(value);
	auto attr = m_document.allocate_attribute(allocatedName, allocatedValue);
	return XmlAttribute(attr);
}

std::string XmlDocument::GetEncoding() const {
	auto declaration = m_document.first_node("", 0, true);
	if (declaration) {
		auto encodingAttr = declaration->first_attribute("encoding");
		if (encodingAttr) {
			return std::string(encodingAttr->value(), encodingAttr->value_size());
		}
	}
	return "UTF-8";
}

void XmlDocument::SetEncoding(const std::string& encoding) {
	auto declaration = m_document.first_node("", 0, true);
	if (!declaration) {
		AddDeclaration("1.0", encoding);
	}
	else {
		auto encodingAttr = declaration->first_attribute("encoding");
		if (encodingAttr) {
			char* allocatedValue = AllocateString(encoding);
			encodingAttr->value(allocatedValue);
		}
		else {
			char* allocatedName = AllocateString("encoding");
			char* allocatedValue = AllocateString(encoding);
			auto newAttr = m_document.allocate_attribute(allocatedName, allocatedValue);
			declaration->append_attribute(newAttr);
		}
	}
}

void XmlDocument::AddDeclaration(const std::string& version,
	const std::string& encoding,
	const std::string& standalone) {
	auto declaration = m_document.allocate_node(rapidxml::node_declaration);

	// Version attribute
	if (!version.empty()) {
		char* versionName = AllocateString("version");
		char* versionValue = AllocateString(version);
		auto versionAttr = m_document.allocate_attribute(versionName, versionValue);
		declaration->append_attribute(versionAttr);
	}

	// Encoding attribute
	if (!encoding.empty()) {
		char* encodingName = AllocateString("encoding");
		char* encodingValue = AllocateString(encoding);
		auto encodingAttr = m_document.allocate_attribute(encodingName, encodingValue);
		declaration->append_attribute(encodingAttr);
	}

	// Standalone attribute
	if (!standalone.empty()) {
		char* standaloneName = AllocateString("standalone");
		char* standaloneValue = AllocateString(standalone);
		auto standaloneAttr = m_document.allocate_attribute(standaloneName, standaloneValue);
		declaration->append_attribute(standaloneAttr);
	}

	m_document.prepend_node(declaration);
}

char* XmlDocument::AllocateString(const std::string& str) {
	auto buffer = std::make_unique<char[]>(str.length() + 1);
	std::strcpy(buffer.get(), str.c_str());
	char* result = buffer.get();
	m_allocatedStrings.push_back(std::move(buffer));
	return result;
}

void XmlDocument::ClearAllocatedStrings() {
	m_allocatedStrings.clear();
}

// XmlManager implementation
XmlManager& XmlManager::GetInstance() {
	static XmlManager instance;
	return instance;
}

std::shared_ptr<XmlDocument> XmlManager::LoadDocument(const std::string& filename) {
	// Check cache first
	auto it = m_documentCache.find(filename);
	if (it != m_documentCache.end()) {
		if (auto existing = it->second.lock()) {
			return existing;
		}
		else {
			m_documentCache.erase(it);
		}
	}

	// Load new document
	auto document = std::make_shared<XmlDocument>();
	if (document->LoadFromFile(filename)) {
		m_documentCache[filename] = document;
		return document;
	}

	return nullptr;
}

std::shared_ptr<XmlDocument> XmlManager::CreateDocument() {
	return std::make_shared<XmlDocument>();
}

bool XmlManager::SaveDocument(std::shared_ptr<XmlDocument> document, const std::string& filename) {
	if (!document) {
		Logger::GetInstance().LogError("XmlManager::SaveDocument - Invalid document");
		return false;
	}

	return document->SaveToFile(filename);
}

void XmlManager::ClearCache() {
	m_documentCache.clear();
}

bool XmlManager::IsDocumentCached(const std::string& filename) const {
	auto it = m_documentCache.find(filename);
	return it != m_documentCache.end() && !it->second.expired();
}

void XmlManager::RemoveFromCache(const std::string& filename) {
	m_documentCache.erase(filename);
}

bool XmlManager::ValidateXmlFile(const std::string& filename) const {
	try {
		auto testDoc = std::make_unique<XmlDocument>();
		return testDoc->LoadFromFile(filename);
	}
	catch (const std::exception&) {
		return false;
	}
}

bool XmlManager::ValidateXmlString(const std::string& xmlContent) const {
	try {
		auto testDoc = std::make_unique<XmlDocument>();
		return testDoc->LoadFromString(xmlContent);
	}
	catch (const std::exception&) {
		return false;
	}
}

std::string XmlManager::EscapeXmlString(const std::string& str) const {
	std::string result = str;

	// Replace special characters
	size_t pos = 0;
	while ((pos = result.find('&', pos)) != std::string::npos) {
		result.replace(pos, 1, "&amp;");
		pos += 5;
	}
	pos = 0;
	while ((pos = result.find('<', pos)) != std::string::npos) {
		result.replace(pos, 1, "&lt;");
		pos += 4;
	}
	pos = 0;
	while ((pos = result.find('>', pos)) != std::string::npos) {
		result.replace(pos, 1, "&gt;");
		pos += 4;
	}
	pos = 0;
	while ((pos = result.find('"', pos)) != std::string::npos) {
		result.replace(pos, 1, "&quot;");
		pos += 6;
	}
	pos = 0;
	while ((pos = result.find('\'', pos)) != std::string::npos) {
		result.replace(pos, 1, "&apos;");
		pos += 6;
	}

	return result;
}

std::string XmlManager::UnescapeXmlString(const std::string& str) const {
	std::string result = str;

	// Replace escaped characters
	size_t pos = 0;
	while ((pos = result.find("&amp;", pos)) != std::string::npos) {
		result.replace(pos, 5, "&");
		pos += 1;
	}
	pos = 0;
	while ((pos = result.find("&lt;", pos)) != std::string::npos) {
		result.replace(pos, 4, "<");
		pos += 1;
	}
	pos = 0;
	while ((pos = result.find("&gt;", pos)) != std::string::npos) {
		result.replace(pos, 4, ">");
		pos += 1;
	}
	pos = 0;
	while ((pos = result.find("&quot;", pos)) != std::string::npos) {
		result.replace(pos, 6, "\"");
		pos += 1;
	}
	pos = 0;
	while ((pos = result.find("&apos;", pos)) != std::string::npos) {
		result.replace(pos, 6, "'");
		pos += 1;
	}

	return result;
}

bool XmlManager::LoadConfig(const std::string& filename, std::unordered_map<std::string, std::string>& config) {
	auto document = LoadDocument(filename);
	if (!document) {
		return false;
	}

	auto root = document->GetRoot();
	if (!root.IsValid()) {
		return false;
	}

	// Load configuration from XML
	for (auto configNode : root.GetChildren("setting")) {
		std::string key = configNode.GetAttributeValue("key");
		std::string value = configNode.GetAttributeValue("value");

		if (!key.empty()) {
			config[key] = value;
		}
	}

	return true;
}

bool XmlManager::SaveConfig(const std::string& filename, const std::unordered_map<std::string, std::string>& config) {
	auto document = CreateDocument();
	auto root = document->CreateRoot("configuration");

	for (const auto& pair : config) {
		auto settingNode = root.AppendChild("setting");
		settingNode.SetAttribute("key", pair.first);
		settingNode.SetAttribute("value", pair.second);
	}

	return SaveDocument(document, filename);
}