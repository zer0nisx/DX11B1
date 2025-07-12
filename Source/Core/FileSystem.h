#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>

namespace GameEngine {
namespace Core {

enum class FileMode {
    Read,
    Write,
    Append,
    ReadBinary,
    WriteBinary
};

struct FileInfo {
    std::string filename;
    std::string fullPath;
    std::string extension;
    size_t size;
    bool exists;
    bool isDirectory;
    std::filesystem::file_time_type lastWriteTime;
};

class FileSystem {
public:
    static FileSystem& GetInstance();

    // File operations
    bool FileExists(const std::string& path) const;
    bool DirectoryExists(const std::string& path) const;
    bool CreateDirectory(const std::string& path) const;
    bool CreateDirectories(const std::string& path) const;
    bool DeleteFile(const std::string& path) const;
    bool DeleteDirectory(const std::string& path, bool recursive = false) const;
    bool CopyFile(const std::string& source, const std::string& destination) const;
    bool MoveFile(const std::string& source, const std::string& destination) const;

    // File reading/writing
    bool ReadTextFile(const std::string& path, std::string& outContent) const;
    bool WriteTextFile(const std::string& path, const std::string& content) const;
    bool ReadBinaryFile(const std::string& path, std::vector<uint8_t>& outData) const;
    bool WriteBinaryFile(const std::string& path, const std::vector<uint8_t>& data) const;

    // File streaming
    std::unique_ptr<std::ifstream> OpenFileForReading(const std::string& path, FileMode mode = FileMode::Read) const;
    std::unique_ptr<std::ofstream> OpenFileForWriting(const std::string& path, FileMode mode = FileMode::Write) const;

    // File information
    FileInfo GetFileInfo(const std::string& path) const;
    size_t GetFileSize(const std::string& path) const;
    std::filesystem::file_time_type GetLastWriteTime(const std::string& path) const;

    // Directory operations
    std::vector<std::string> GetFilesInDirectory(const std::string& path,
        const std::string& extension = "", bool recursive = false) const;
    std::vector<std::string> GetDirectoriesInDirectory(const std::string& path,
        bool recursive = false) const;

    // Path utilities
    std::string GetCurrentDirectory() const;
    std::string GetExecutableDirectory() const;
    std::string GetAbsolutePath(const std::string& path) const;
    std::string GetRelativePath(const std::string& path, const std::string& base = "") const;
    std::string GetFileName(const std::string& path) const;
    std::string GetFileNameWithoutExtension(const std::string& path) const;
    std::string GetFileExtension(const std::string& path) const;
    std::string GetDirectoryPath(const std::string& path) const;
    std::string CombinePaths(const std::string& path1, const std::string& path2) const;

    // Asset paths
    void SetAssetsDirectory(const std::string& path);
    std::string GetAssetsDirectory() const { return m_assetsDirectory; }
    std::string GetAssetPath(const std::string& relativePath) const;
    std::string GetModelPath(const std::string& filename) const;
    std::string GetTexturePath(const std::string& filename) const;
    std::string GetShaderPath(const std::string& filename) const;

    // Search functionality
    std::vector<std::string> FindFiles(const std::string& pattern,
        const std::string& searchPath = "", bool recursive = true) const;
    std::string FindFile(const std::string& filename,
        const std::vector<std::string>& searchPaths) const;

    // Watch functionality
    bool IsFileNewer(const std::string& file1, const std::string& file2) const;
    bool HasFileChanged(const std::string& path, std::filesystem::file_time_type& lastKnownTime) const;

private:
    FileSystem() = default;
    ~FileSystem() = default;
    FileSystem(const FileSystem&) = delete;
    FileSystem& operator=(const FileSystem&) = delete;

    std::ios_base::openmode GetOpenMode(FileMode mode) const;
    bool IsValidPath(const std::string& path) const;

private:
    std::string m_assetsDirectory = "Assets";
    mutable std::string m_currentDirectory;
    mutable std::string m_executableDirectory;
};

// Convenience macros
#define FILE_SYSTEM GameEngine::Core::FileSystem::GetInstance()

} // namespace Core
} // namespace GameEngine
