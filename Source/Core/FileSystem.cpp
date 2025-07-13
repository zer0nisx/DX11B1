#include "FileSystem.h"
#include "Logger.h"
#include <algorithm>
#include <iostream>
#include <regex>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#undef CreateDirectory
#undef DeleteFile
#undef CopyFile
#undef MoveFile
#undef GetCurrentDirectory
#endif

using namespace GameEngine::Core;

FileSystem& FileSystem::GetInstance() {
    static FileSystem instance;
    return instance;
}

bool FileSystem::FileExists(const std::string& path) const {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::FileExists - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::DirectoryExists(const std::string& path) const {
    try {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::DirectoryExists - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::CreateDirectory(const std::string& path) const {
    try {
        if (DirectoryExists(path)) {
            return true;
        }

        bool result = std::filesystem::create_directory(path);
        if (result) {
            Logger::GetInstance().LogInfo("Created directory: " + path);
        }
        return result;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::CreateDirectory - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::CreateDirectories(const std::string& path) const {
    try {
        if (DirectoryExists(path)) {
            return true;
        }

        bool result = std::filesystem::create_directories(path);
        if (result) {
            Logger::GetInstance().LogInfo("Created directories: " + path);
        }
        return result;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::CreateDirectories - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::DeleteFile(const std::string& path) const {
    try {
        if (!FileExists(path)) {
            return true;
        }

        bool result = std::filesystem::remove(path);
        if (result) {
            Logger::GetInstance().LogInfo("Deleted file: " + path);
        }
        return result;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::DeleteFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::DeleteDirectory(const std::string& path, bool recursive) const {
    try {
        if (!DirectoryExists(path)) {
            return true;
        }

        uintmax_t result;
        if (recursive) {
            result = std::filesystem::remove_all(path);
        } else {
            result = std::filesystem::remove(path) ? 1 : 0;
        }

        if (result > 0) {
            Logger::GetInstance().LogInfo("Deleted directory: " + path + " (" + std::to_string(result) + " items)");
        }
        return result > 0;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::DeleteDirectory - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::CopyFile(const std::string& source, const std::string& destination) const {
    try {
        if (!FileExists(source)) {
            Logger::GetInstance().LogError("FileSystem::CopyFile - Source file does not exist: " + source);
            return false;
        }

        // Create destination directory if it doesn't exist
        std::string destDir = GetDirectoryPath(destination);
        if (!destDir.empty() && !DirectoryExists(destDir)) {
            CreateDirectories(destDir);
        }

        bool result = std::filesystem::copy_file(source, destination,
            std::filesystem::copy_options::overwrite_existing);

        if (result) {
            Logger::GetInstance().LogInfo("Copied file: " + source + " -> " + destination);
        }
        return result;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::CopyFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::MoveFile(const std::string& source, const std::string& destination) const {
    try {
        if (!FileExists(source)) {
            Logger::GetInstance().LogError("FileSystem::MoveFile - Source file does not exist: " + source);
            return false;
        }

        // Create destination directory if it doesn't exist
        std::string destDir = GetDirectoryPath(destination);
        if (!destDir.empty() && !DirectoryExists(destDir)) {
            CreateDirectories(destDir);
        }

        std::filesystem::rename(source, destination);
        Logger::GetInstance().LogInfo("Moved file: " + source + " -> " + destination);
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::MoveFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::ReadTextFile(const std::string& path, std::string& outContent) const {
    try {
        auto file = OpenFileForReading(path, FileMode::Read);
        if (!file || !file->is_open()) {
            return false;
        }

        // Get file size for efficient reading
        file->seekg(0, std::ios::end);
        std::streamsize size = file->tellg();
        file->seekg(0, std::ios::beg);

        outContent.resize(static_cast<size_t>(size));
        file->read(outContent.data(), size);

        return file->good() || file->eof();
    } catch (const std::exception& e) {
        Logger::GetInstance().LogError("FileSystem::ReadTextFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::WriteTextFile(const std::string& path, const std::string& content) const {
    try {
        // Create directory if it doesn't exist
        std::string dir = GetDirectoryPath(path);
        if (!dir.empty() && !DirectoryExists(dir)) {
            CreateDirectories(dir);
        }

        auto file = OpenFileForWriting(path, FileMode::Write);
        if (!file || !file->is_open()) {
            return false;
        }

        *file << content;
        return file->good();
    } catch (const std::exception& e) {
        Logger::GetInstance().LogError("FileSystem::WriteTextFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::ReadBinaryFile(const std::string& path, std::vector<uint8_t>& outData) const {
    try {
        auto file = OpenFileForReading(path, FileMode::ReadBinary);
        if (!file || !file->is_open()) {
            return false;
        }

        // Get file size
        file->seekg(0, std::ios::end);
        std::streamsize size = file->tellg();
        file->seekg(0, std::ios::beg);

        outData.resize(static_cast<size_t>(size));
        file->read(reinterpret_cast<char*>(outData.data()), size);

        return file->good() || file->eof();
    } catch (const std::exception& e) {
        Logger::GetInstance().LogError("FileSystem::ReadBinaryFile - " + std::string(e.what()));
        return false;
    }
}

bool FileSystem::WriteBinaryFile(const std::string& path, const std::vector<uint8_t>& data) const {
    try {
        // Create directory if it doesn't exist
        std::string dir = GetDirectoryPath(path);
        if (!dir.empty() && !DirectoryExists(dir)) {
            CreateDirectories(dir);
        }

        auto file = OpenFileForWriting(path, FileMode::WriteBinary);
        if (!file || !file->is_open()) {
            return false;
        }

        file->write(reinterpret_cast<const char*>(data.data()), data.size());
        return file->good();
    } catch (const std::exception& e) {
        Logger::GetInstance().LogError("FileSystem::WriteBinaryFile - " + std::string(e.what()));
        return false;
    }
}

std::unique_ptr<std::ifstream> FileSystem::OpenFileForReading(const std::string& path, FileMode mode) const {
    auto file = std::make_unique<std::ifstream>(path, GetOpenMode(mode));
    if (!file->is_open()) {
        Logger::GetInstance().LogError("FileSystem::OpenFileForReading - Cannot open file: " + path);
        return nullptr;
    }
    return file;
}

std::unique_ptr<std::ofstream> FileSystem::OpenFileForWriting(const std::string& path, FileMode mode) const {
    auto file = std::make_unique<std::ofstream>(path, GetOpenMode(mode));
    if (!file->is_open()) {
        Logger::GetInstance().LogError("FileSystem::OpenFileForWriting - Cannot open file: " + path);
        return nullptr;
    }
    return file;
}

FileInfo FileSystem::GetFileInfo(const std::string& path) const {
    FileInfo info;
    info.filename = GetFileName(path);
    info.fullPath = GetAbsolutePath(path);
    info.extension = GetFileExtension(path);
    info.exists = FileExists(path) || DirectoryExists(path);
    info.isDirectory = DirectoryExists(path);
    info.size = info.exists && !info.isDirectory ? GetFileSize(path) : 0;
    info.lastWriteTime = info.exists ? GetLastWriteTime(path) : std::filesystem::file_time_type{};

    return info;
}

size_t FileSystem::GetFileSize(const std::string& path) const {
    try {
        if (!FileExists(path)) {
            return 0;
        }
        return static_cast<size_t>(std::filesystem::file_size(path));
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetFileSize - " + std::string(e.what()));
        return 0;
    }
}

std::filesystem::file_time_type FileSystem::GetLastWriteTime(const std::string& path) const {
    try {
        if (!FileExists(path) && !DirectoryExists(path)) {
            return std::filesystem::file_time_type{};
        }
        return std::filesystem::last_write_time(path);
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetLastWriteTime - " + std::string(e.what()));
        return std::filesystem::file_time_type{};
    }
}

std::vector<std::string> FileSystem::GetFilesInDirectory(const std::string& path,
    const std::string& extension, bool recursive) const {
    std::vector<std::string> files;

    try {
        if (!DirectoryExists(path)) {
            return files;
        }

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();

                    if (extension.empty() || GetFileExtension(filePath) == extension) {
                        files.push_back(filePath);
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_regular_file()) {
                    std::string filePath = entry.path().string();

                    if (extension.empty() || GetFileExtension(filePath) == extension) {
                        files.push_back(filePath);
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetFilesInDirectory - " + std::string(e.what()));
    }

    return files;
}

std::vector<std::string> FileSystem::GetDirectoriesInDirectory(const std::string& path, bool recursive) const {
    std::vector<std::string> directories;

    try {
        if (!DirectoryExists(path)) {
            return directories;
        }

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path().string());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory()) {
                    directories.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetDirectoriesInDirectory - " + std::string(e.what()));
    }

    return directories;
}

std::string FileSystem::GetCurrentDirectory() const {
    if (m_currentDirectory.empty()) {
        try {
            m_currentDirectory = std::filesystem::current_path().string();
        } catch (const std::filesystem::filesystem_error& e) {
            Logger::GetInstance().LogError("FileSystem::GetCurrentDirectory - " + std::string(e.what()));
            m_currentDirectory = ".";
        }
    }
    return m_currentDirectory;
}

std::string FileSystem::GetExecutableDirectory() const {
    if (m_executableDirectory.empty()) {
#ifdef _WIN32
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        m_executableDirectory = GetDirectoryPath(std::string(buffer));
#else
        // Linux/Unix implementation
        char buffer[1024];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            m_executableDirectory = GetDirectoryPath(std::string(buffer));
        } else {
            m_executableDirectory = GetCurrentDirectory();
        }
#endif
    }
    return m_executableDirectory;
}

std::string FileSystem::GetAbsolutePath(const std::string& path) const {
    try {
        return std::filesystem::absolute(path).string();
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetAbsolutePath - " + std::string(e.what()));
        return path;
    }
}

std::string FileSystem::GetRelativePath(const std::string& path, const std::string& base) const {
    try {
        std::string basePath = base.empty() ? GetCurrentDirectory() : base;
        return std::filesystem::relative(path, basePath).string();
    } catch (const std::filesystem::filesystem_error& e) {
        Logger::GetInstance().LogError("FileSystem::GetRelativePath - " + std::string(e.what()));
        return path;
    }
}

std::string FileSystem::GetFileName(const std::string& path) const {
    return std::filesystem::path(path).filename().string();
}

std::string FileSystem::GetFileNameWithoutExtension(const std::string& path) const {
    return std::filesystem::path(path).stem().string();
}

std::string FileSystem::GetFileExtension(const std::string& path) const {
    std::string ext = std::filesystem::path(path).extension().string();
    if (!ext.empty() && ext[0] == '.') {
        ext = ext.substr(1);
    }
    return ext;
}

std::string FileSystem::GetDirectoryPath(const std::string& path) const {
    return std::filesystem::path(path).parent_path().string();
}

std::string FileSystem::CombinePaths(const std::string& path1, const std::string& path2) const {
    return (std::filesystem::path(path1) / path2).string();
}

void FileSystem::SetAssetsDirectory(const std::string& path) {
    m_assetsDirectory = path;

    // Create assets directory structure if it doesn't exist
    CreateDirectories(m_assetsDirectory);
    CreateDirectories(CombinePaths(m_assetsDirectory, "Models"));
    CreateDirectories(CombinePaths(m_assetsDirectory, "Textures"));
    CreateDirectories(CombinePaths(m_assetsDirectory, "Shaders"));
    CreateDirectories(CombinePaths(m_assetsDirectory, "Audio"));
    CreateDirectories(CombinePaths(m_assetsDirectory, "Config"));
}

std::string FileSystem::GetAssetPath(const std::string& relativePath) const {
    return CombinePaths(m_assetsDirectory, relativePath);
}

std::string FileSystem::GetModelPath(const std::string& filename) const {
    return CombinePaths(CombinePaths(m_assetsDirectory, "Models"), filename);
}

std::string FileSystem::GetTexturePath(const std::string& filename) const {
    return CombinePaths(CombinePaths(m_assetsDirectory, "Textures"), filename);
}

std::string FileSystem::GetShaderPath(const std::string& filename) const {
    return CombinePaths(CombinePaths(m_assetsDirectory, "Shaders"), filename);
}

std::vector<std::string> FileSystem::FindFiles(const std::string& pattern,
    const std::string& searchPath, bool recursive) const {
    std::vector<std::string> foundFiles;
    std::string actualSearchPath = searchPath.empty() ? GetCurrentDirectory() : searchPath;

    try {
        std::regex regexPattern(pattern, std::regex_constants::icase);

        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(actualSearchPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, regexPattern)) {
                        foundFiles.push_back(entry.path().string());
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(actualSearchPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (std::regex_match(filename, regexPattern)) {
                        foundFiles.push_back(entry.path().string());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        Logger::GetInstance().LogError("FileSystem::FindFiles - " + std::string(e.what()));
    }

    return foundFiles;
}

std::string FileSystem::FindFile(const std::string& filename, const std::vector<std::string>& searchPaths) const {
    // First check current directory
    if (FileExists(filename)) {
        return GetAbsolutePath(filename);
    }

    // Then check each search path
    for (const auto& searchPath : searchPaths) {
        std::string fullPath = CombinePaths(searchPath, filename);
        if (FileExists(fullPath)) {
            return GetAbsolutePath(fullPath);
        }
    }

    // Check assets directory
    std::string assetPath = GetAssetPath(filename);
    if (FileExists(assetPath)) {
        return GetAbsolutePath(assetPath);
    }

    return "";
}

bool FileSystem::IsFileNewer(const std::string& file1, const std::string& file2) const {
    auto time1 = GetLastWriteTime(file1);
    auto time2 = GetLastWriteTime(file2);
    return time1 > time2;
}

bool FileSystem::HasFileChanged(const std::string& path, std::filesystem::file_time_type& lastKnownTime) const {
    auto currentTime = GetLastWriteTime(path);
    if (currentTime > lastKnownTime) {
        lastKnownTime = currentTime;
        return true;
    }
    return false;
}

std::ios_base::openmode FileSystem::GetOpenMode(FileMode mode) const {
    switch (mode) {
        case FileMode::Read:
            return std::ios::in;
        case FileMode::Write:
            return std::ios::out | std::ios::trunc;
        case FileMode::Append:
            return std::ios::out | std::ios::app;
        case FileMode::ReadBinary:
            return std::ios::in | std::ios::binary;
        case FileMode::WriteBinary:
            return std::ios::out | std::ios::binary | std::ios::trunc;
        default:
            return std::ios::in;
    }
}

bool FileSystem::IsValidPath(const std::string& path) const {
    return !path.empty() && path.find_first_of("<>:\"|?*") == std::string::npos;
}
