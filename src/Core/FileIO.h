#pragma once

#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "core/Error.h"

namespace RebelCAD {
namespace Core {

class FileIO {
public:
    // File operations
    static std::vector<uint8_t> readBinaryFile(const std::string& path);
    static std::string readTextFile(const std::string& path);
    static void writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data);
    static void writeTextFile(const std::string& path, const std::string& content);
    
    // Directory operations
    static bool createDirectory(const std::string& path);
    static bool removeDirectory(const std::string& path, bool recursive = false);
    static std::vector<std::string> listDirectory(const std::string& path);
    
    // Path operations
    static std::string getAbsolutePath(const std::string& path);
    static std::string getRelativePath(const std::string& path, const std::string& base);
    static std::string getParentPath(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string getFileExtension(const std::string& path);
    
    // File information
    static bool exists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isFile(const std::string& path);
    static size_t getFileSize(const std::string& path);
    static std::filesystem::file_time_type getLastModified(const std::string& path);

private:
    // Prevent instantiation
    FileIO() = delete;
    ~FileIO() = delete;
    FileIO(const FileIO&) = delete;
    FileIO& operator=(const FileIO&) = delete;

    // Helper functions
    static void validatePath(const std::string& path);
    static void checkFileExists(const std::string& path);
    static void checkDirectoryExists(const std::string& path);
};

} // namespace Core
} // namespace RebelCAD
