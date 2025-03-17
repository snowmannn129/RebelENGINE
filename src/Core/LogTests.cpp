#include <gtest/gtest.h>
#include "core/Log.h"
#include <fstream>
#include <filesystem>
#include <regex>
#include <string>

namespace rebel::tests {

class LogTests : public ::testing::Test {
protected:
    void SetUp() override {
        // First ensure logger is reset from any previous test
        rebel::core::Logger::resetInstance();
        
        // Print current working directory for debugging
        std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
        
        // Ensure logs directory exists and is empty
        if (std::filesystem::exists("logs")) {
            std::cout << "Removing existing logs directory" << std::endl;
            std::filesystem::remove_all("logs");
            // Wait for filesystem operations to complete
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        std::cout << "Creating logs directory" << std::endl;
        std::filesystem::create_directory("logs");
        std::cout << "Logs directory created at: " << std::filesystem::absolute("logs") << std::endl;
        
        // Wait for directory creation to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void TearDown() override {
        // First shutdown logger to close file handles
        rebel::core::Logger::resetInstance();
        
        // Wait for logger shutdown to complete
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Clean up logs after tests
        if (std::filesystem::exists("logs")) {
            std::filesystem::remove_all("logs");
            // Wait for cleanup to complete
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Helper to read log file content
    std::string readLogFile() {
        std::string logPath = "logs/RebelCAD.log";
        if (!std::filesystem::exists(logPath)) {
            throw std::runtime_error("Log file does not exist: " + logPath);
        }
        
        // Wait for any pending file operations
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Open file in binary mode to avoid line ending issues
        std::ifstream file(logPath, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + logPath);
        }
        
        // Set exceptions to catch any read errors
        file.exceptions(std::ios::badbit | std::ios::failbit);
        
        try {
            // Get file size
            file.seekg(0, std::ios::end);
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            // Read the entire file
            std::string buffer(size, '\0');
            file.read(&buffer[0], size);
            
            return buffer;
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to read log file: ") + e.what());
        }
    }

    // Helper to check if log entry matches expected format
    bool isValidLogEntry(const std::string& entry) {
        // Log format: [YYYY-MM-DD HH:MM:SS] [LEVEL] [file:line] message
        std::regex logPattern(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\] \[(TRACE|DEBUG|INFO|WARNING|ERROR|CRITICAL)\] \[.*:\d+\] .*(?:\r\n|\n|\r)?)");
        return std::regex_match(entry, logPattern);
    }
};

TEST_F(LogTests, LoggerSingletonTest) {
    auto& logger1 = rebel::core::Logger::getInstance();
    auto& logger2 = rebel::core::Logger::getInstance();
    EXPECT_EQ(&logger1, &logger2) << "Logger singleton returning different instances";
}

TEST_F(LogTests, BasicLoggingTest) {
    auto& logger = rebel::core::Logger::getInstance();
    
    // Test each log level
    logger.trace("Trace message");
    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    logger.critical("Critical message");

    // Read log file
    std::string logContent = readLogFile();
    
    // Verify all messages were logged
    EXPECT_TRUE(logContent.find("Trace message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Debug message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Info message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Warning message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Error message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Critical message") != std::string::npos);

    // Split log content into lines and verify format of each
    std::stringstream ss(logContent);
    std::string line;
    while (std::getline(ss, line)) {
        EXPECT_TRUE(isValidLogEntry(line)) << "Invalid log entry format: " << line;
    }
}

TEST_F(LogTests, MacroTest) {
    // Test logging macros
    REBEL_LOG_INFO("Test macro message");
    
    std::string logContent = readLogFile();
    EXPECT_TRUE(logContent.find("Test macro message") != std::string::npos);
    
    // Split log content into lines and validate format
    std::stringstream ss(logContent);
    std::string line;
    bool foundValidEntry = false;
    while (std::getline(ss, line)) {
        if (line.find("Test macro message") != std::string::npos) {
            EXPECT_TRUE(isValidLogEntry(line)) << "Invalid log entry format: " << line;
            foundValidEntry = true;
            break;
        }
    }
    EXPECT_TRUE(foundValidEntry) << "Could not find log entry with test message";
}

TEST_F(LogTests, ConcurrentLoggingTest) {
    constexpr int NUM_THREADS = 10;
    constexpr int MESSAGES_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back([i]() {
            auto& logger = rebel::core::Logger::getInstance();
            for (int j = 0; j < MESSAGES_PER_THREAD; ++j) {
                logger.info("Thread " + std::to_string(i) + 
                          " Message " + std::to_string(j));
            }
        });
    }

    // Wait for all threads to complete and messages to be processed
    auto& logger = rebel::core::Logger::getInstance();
    
    // First wait for all threads to finish sending messages
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Then shutdown logger which will wait for all messages to be processed
    logger.shutdown();
    
    // Finally reset the logger instance
    logger.resetInstance();
    
    // Give filesystem a moment to sync
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Now verify log file contains expected number of entries
    std::string logContent = readLogFile();
    int lineCount = 0;
    size_t pos = 0;
    std::string line;
    
    // Count lines while handling different line endings
    while (pos < logContent.size()) {
        // Find next line ending
        size_t endPos = logContent.find("\r\n", pos);
        if (endPos == std::string::npos) {
            endPos = logContent.find('\n', pos);
            if (endPos == std::string::npos) {
                // Last line without line ending
                line = logContent.substr(pos);
                if (!line.empty()) {
                    EXPECT_TRUE(isValidLogEntry(line)) << "Invalid log entry: " << line;
                    lineCount++;
                }
                break;
            }
        }
        
        // Extract line
        line = logContent.substr(pos, endPos - pos);
        EXPECT_TRUE(isValidLogEntry(line)) << "Invalid log entry: " << line;
        lineCount++;
        
        // Move past line ending
        pos = (logContent[endPos] == '\r' && endPos + 1 < logContent.size() && logContent[endPos + 1] == '\n')
            ? endPos + 2  // \r\n
            : endPos + 1; // \n
    }

    EXPECT_EQ(lineCount, NUM_THREADS * MESSAGES_PER_THREAD) 
        << "Expected " << (NUM_THREADS * MESSAGES_PER_THREAD) 
        << " messages but found " << lineCount 
        << "\nLog content:\n" << logContent;
}

} // namespace rebel::tests
