#include <gtest/gtest.h>
#include "core/FileIO.h"
#include "core/Error.h"
#include <filesystem>
#include <fstream>

using namespace RebelCAD::Core;

class FileIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory and files
        testDir = "test_files";
        FileIO::createDirectory(testDir);
        
        // Create a test text file
        textFilePath = testDir + "/test.txt";
        std::ofstream textFile(textFilePath);
        textFile << "Test content";
        textFile.close();
        
        // Create a test binary file
        binaryFilePath = testDir + "/test.bin";
        std::vector<uint8_t> binaryData = {0x00, 0x01, 0x02, 0x03};
        std::ofstream binFile(binaryFilePath, std::ios::binary);
        binFile.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
        binFile.close();
    }

    void TearDown() override {
        // Clean up test files
        if (std::filesystem::exists(testDir)) {
            std::filesystem::remove_all(testDir);
        }
    }

    std::string testDir;
    std::string textFilePath;
    std::string binaryFilePath;
};

// File Operations Tests
TEST_F(FileIOTest, ReadTextFile) {
    std::string content = FileIO::readTextFile(textFilePath);
    EXPECT_EQ(content, "Test content");
}

TEST_F(FileIOTest, ReadBinaryFile) {
    std::vector<uint8_t> expected = {0x00, 0x01, 0x02, 0x03};
    std::vector<uint8_t> content = FileIO::readBinaryFile(binaryFilePath);
    EXPECT_EQ(content, expected);
}

TEST_F(FileIOTest, WriteTextFile) {
    std::string newPath = testDir + "/write_test.txt";
    std::string content = "New content";
    FileIO::writeTextFile(newPath, content);
    EXPECT_EQ(FileIO::readTextFile(newPath), content);
}

TEST_F(FileIOTest, WriteBinaryFile) {
    std::string newPath = testDir + "/write_test.bin";
    std::vector<uint8_t> data = {0x04, 0x05, 0x06};
    FileIO::writeBinaryFile(newPath, data);
    EXPECT_EQ(FileIO::readBinaryFile(newPath), data);
}

// Directory Operations Tests
TEST_F(FileIOTest, CreateAndRemoveDirectory) {
    std::string newDir = testDir + "/new_dir";
    EXPECT_TRUE(FileIO::createDirectory(newDir));
    EXPECT_TRUE(FileIO::exists(newDir));
    EXPECT_TRUE(FileIO::isDirectory(newDir));
    EXPECT_TRUE(FileIO::removeDirectory(newDir));
    EXPECT_FALSE(FileIO::exists(newDir));
}

TEST_F(FileIOTest, ListDirectory) {
    auto entries = FileIO::listDirectory(testDir);
    EXPECT_EQ(entries.size(), 2);  // test.txt and test.bin
    
    // Convert to set for easier comparison
    std::set<std::string> entrySet;
    for (const auto& entry : entries) {
        entrySet.insert(FileIO::getFileName(entry));
    }
    
    EXPECT_TRUE(entrySet.find("test.txt") != entrySet.end());
    EXPECT_TRUE(entrySet.find("test.bin") != entrySet.end());
}

// Path Operations Tests
TEST_F(FileIOTest, PathOperations) {
    EXPECT_EQ(FileIO::getFileName(textFilePath), "test.txt");
    EXPECT_EQ(FileIO::getFileExtension(textFilePath), ".txt");
    EXPECT_EQ(FileIO::getParentPath(textFilePath), testDir);
}

// Error Handling Tests
TEST_F(FileIOTest, FileNotFoundError) {
    EXPECT_THROW({
        FileIO::readTextFile("nonexistent.txt");
    }, Error);
    
    try {
        FileIO::readTextFile("nonexistent.txt");
    } catch (const Error& e) {
        EXPECT_EQ(e.getCode(), ErrorCode::FileNotFound);
    }
}

TEST_F(FileIOTest, InvalidPathError) {
    EXPECT_THROW({
        FileIO::writeTextFile("", "content");
    }, Error);
    
    try {
        FileIO::writeTextFile("", "content");
    } catch (const Error& e) {
        EXPECT_EQ(e.getCode(), ErrorCode::InvalidArgument);
    }
}

TEST_F(FileIOTest, DirectoryOperationErrors) {
    // Try to create directory with invalid path
    EXPECT_THROW({
        FileIO::createDirectory("");
    }, Error);
    
    // Try to remove non-existent directory
    EXPECT_THROW({
        FileIO::removeDirectory("nonexistent");
    }, Error);
    
    // Try to list non-existent directory
    EXPECT_THROW({
        FileIO::listDirectory("nonexistent");
    }, Error);
}

TEST_F(FileIOTest, FileInformationQueries) {
    EXPECT_TRUE(FileIO::exists(textFilePath));
    EXPECT_TRUE(FileIO::isFile(textFilePath));
    EXPECT_FALSE(FileIO::isDirectory(textFilePath));
    EXPECT_EQ(FileIO::getFileSize(textFilePath), 12); // "Test content" length
}

TEST_F(FileIOTest, RecursiveDirectoryOperations) {
    // Create nested directories
    std::string nestedDir = testDir + "/nested/deep";
    EXPECT_TRUE(FileIO::createDirectory(nestedDir));
    
    // Create a file in the nested directory
    std::string nestedFile = nestedDir + "/nested.txt";
    FileIO::writeTextFile(nestedFile, "Nested content");
    
    // Remove parent directory recursively
    EXPECT_TRUE(FileIO::removeDirectory(testDir + "/nested", true));
    EXPECT_FALSE(FileIO::exists(nestedDir));
    EXPECT_FALSE(FileIO::exists(nestedFile));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
