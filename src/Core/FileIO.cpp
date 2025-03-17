#include "core/FileIO.h"
#include "core/Log.h"
#include <fstream>
#include <sstream>

namespace RebelCAD {
namespace Core {

std::vector<uint8_t> FileIO::readBinaryFile(const std::string& path) {
    checkFileExists(path);
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        REBEL_THROW_ERROR(FileIOError, "Failed to open file for reading: " + path);
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        REBEL_THROW_ERROR(FileIOError, "Failed to read file: " + path);
    }
    
    return buffer;
}

std::string FileIO::readTextFile(const std::string& path) {
    checkFileExists(path);
    
    std::ifstream file(path);
    if (!file) {
        REBEL_THROW_ERROR(FileIOError, "Failed to open file for reading: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (file.fail()) {
        REBEL_THROW_ERROR(FileIOError, "Failed to read file: " + path);
    }
    
    return buffer.str();
}

void FileIO::writeBinaryFile(const std::string& path, const std::vector<uint8_t>& data) {
    validatePath(path);
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        REBEL_THROW_ERROR(FileIOError, "Failed to open file for writing: " + path);
    }
    
    if (!file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
        REBEL_THROW_ERROR(FileIOError, "Failed to write file: " + path);
    }
}

void FileIO::writeTextFile(const std::string& path, const std::string& content) {
    validatePath(path);
    
    std::ofstream file(path);
    if (!file) {
        REBEL_THROW_ERROR(FileIOError, "Failed to open file for writing: " + path);
    }
    
    if (!(file << content)) {
        REBEL_THROW_ERROR(FileIOError, "Failed to write file: " + path);
    }
}

bool FileIO::createDirectory(const std::string& path) {
    validatePath(path);
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to create directory: " + path + " (" + e.what() + ")");
        return false; // Never reached due to throw, but needed for compiler
    }
}

bool FileIO::removeDirectory(const std::string& path, bool recursive) {
    checkDirectoryExists(path);
    try {
        if (recursive) {
            return std::filesystem::remove_all(path) > 0;
        } else {
            return std::filesystem::remove(path);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to remove directory: " + path + " (" + e.what() + ")");
        return false; // Never reached due to throw, but needed for compiler
    }
}

std::vector<std::string> FileIO::listDirectory(const std::string& path) {
    checkDirectoryExists(path);
    
    std::vector<std::string> entries;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            entries.push_back(entry.path().string());
        }
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to list directory: " + path + " (" + e.what() + ")");
    }
    return entries;
}

std::string FileIO::getAbsolutePath(const std::string& path) {
    try {
        return std::filesystem::absolute(path).string();
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get absolute path: " + path + " (" + e.what() + ")");
        return ""; // Never reached due to throw, but needed for compiler
    }
}

std::string FileIO::getRelativePath(const std::string& path, const std::string& base) {
    try {
        return std::filesystem::relative(path, base).string();
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get relative path: " + path + " (" + e.what() + ")");
        return ""; // Never reached due to throw, but needed for compiler
    }
}

std::string FileIO::getParentPath(const std::string& path) {
    try {
        return std::filesystem::path(path).parent_path().string();
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get parent path: " + path + " (" + e.what() + ")");
        return ""; // Never reached due to throw, but needed for compiler
    }
}

std::string FileIO::getFileName(const std::string& path) {
    try {
        return std::filesystem::path(path).filename().string();
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get file name: " + path + " (" + e.what() + ")");
        return ""; // Never reached due to throw, but needed for compiler
    }
}

std::string FileIO::getFileExtension(const std::string& path) {
    try {
        return std::filesystem::path(path).extension().string();
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get file extension: " + path + " (" + e.what() + ")");
        return ""; // Never reached due to throw, but needed for compiler
    }
}

bool FileIO::exists(const std::string& path) {
    try {
        return std::filesystem::exists(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to check if path exists: " + path + " (" + e.what() + ")");
        return false; // Never reached due to throw, but needed for compiler
    }
}

bool FileIO::isDirectory(const std::string& path) {
    try {
        return std::filesystem::is_directory(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to check if path is directory: " + path + " (" + e.what() + ")");
        return false; // Never reached due to throw, but needed for compiler
    }
}

bool FileIO::isFile(const std::string& path) {
    try {
        return std::filesystem::is_regular_file(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to check if path is file: " + path + " (" + e.what() + ")");
        return false; // Never reached due to throw, but needed for compiler
    }
}

size_t FileIO::getFileSize(const std::string& path) {
    try {
        return std::filesystem::file_size(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get file size: " + path + " (" + e.what() + ")");
        return 0; // Never reached due to throw, but needed for compiler
    }
}

std::filesystem::file_time_type FileIO::getLastModified(const std::string& path) {
    try {
        return std::filesystem::last_write_time(path);
    } catch (const std::filesystem::filesystem_error& e) {
        REBEL_THROW_ERROR(FileIOError, "Failed to get last modified time: " + path + " (" + e.what() + ")");
        return std::filesystem::file_time_type{}; // Never reached due to throw, but needed for compiler
    }
}

void FileIO::validatePath(const std::string& path) {
    if (path.empty()) {
        REBEL_THROW_ERROR(InvalidArgument, "Path cannot be empty");
    }
}

void FileIO::checkFileExists(const std::string& path) {
    if (!exists(path)) {
        REBEL_THROW_ERROR(FileNotFound, "File not found: " + path);
    }
    if (!isFile(path)) {
        REBEL_THROW_ERROR(InvalidArgument, "Path is not a file: " + path);
    }
}

void FileIO::checkDirectoryExists(const std::string& path) {
    if (!exists(path)) {
        REBEL_THROW_ERROR(FileNotFound, "Directory not found: " + path);
    }
    if (!isDirectory(path)) {
        REBEL_THROW_ERROR(InvalidArgument, "Path is not a directory: " + path);
    }
}

} // namespace Core
} // namespace RebelCAD
