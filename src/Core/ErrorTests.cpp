#include <gtest/gtest.h>
#include "core/Error.h"
#include <string>

namespace rebel::tests {

class ErrorTests : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ErrorTests, BasicErrorTest) {
    rebel::core::Error error(rebel::core::ErrorCode::SystemError, 
                           "Test error message", 
                           "TestFile.cpp", 
                           42);
    
    EXPECT_EQ(error.code(), rebel::core::ErrorCode::SystemError);
    EXPECT_EQ(error.message(), "Test error message");
    EXPECT_EQ(error.file(), "TestFile.cpp");
    EXPECT_EQ(error.line(), 42);
    EXPECT_STREQ(error.what(), "Test error message");
}

TEST_F(ErrorTests, ThrowErrorTest) {
    try {
        REBEL_THROW_ERROR(rebel::core::ErrorCode::FileNotFound, "File not found error");
        FAIL() << "Expected error to be thrown";
    }
    catch (const rebel::core::Error& e) {
        EXPECT_EQ(e.code(), rebel::core::ErrorCode::FileNotFound);
        EXPECT_TRUE(std::string(e.what()).find("File not found error") != std::string::npos);
    }
}

TEST_F(ErrorTests, AssertMacroTest) {
    bool condition = false;
    try {
        REBEL_ASSERT(condition, rebel::core::ErrorCode::InvalidMesh, "Invalid mesh error");
        FAIL() << "Expected assertion to throw";
    }
    catch (const rebel::core::Error& e) {
        EXPECT_EQ(e.code(), rebel::core::ErrorCode::InvalidMesh);
        EXPECT_TRUE(std::string(e.what()).find("Invalid mesh error") != std::string::npos);
    }
}

TEST_F(ErrorTests, ErrorCodeTest) {
    // Test various error codes
    std::vector<rebel::core::ErrorCode> codes = {
        rebel::core::ErrorCode::None,
        rebel::core::ErrorCode::SystemError,
        rebel::core::ErrorCode::FileNotFound,
        rebel::core::ErrorCode::GraphicsError,
        rebel::core::ErrorCode::GeometryError,
        rebel::core::ErrorCode::CADError,
        rebel::core::ErrorCode::ConstraintError,
        rebel::core::ErrorCode::AssemblyError
    };

    for (const auto& code : codes) {
        rebel::core::Error error(code, "Test message");
        EXPECT_EQ(error.code(), code);
    }
}

TEST_F(ErrorTests, ErrorPropagationTest) {
    try {
        try {
            REBEL_THROW_ERROR(rebel::core::ErrorCode::InvalidVertex, "Inner error");
        }
        catch (const rebel::core::Error& e) {
            // Verify inner error
            EXPECT_EQ(e.code(), rebel::core::ErrorCode::InvalidVertex);
            // Rethrow with new context
            REBEL_THROW_ERROR(rebel::core::ErrorCode::InvalidMesh, 
                            std::string("Outer error: ") + e.what());
        }
        FAIL() << "Expected error to be rethrown";
    }
    catch (const rebel::core::Error& e) {
        EXPECT_EQ(e.code(), rebel::core::ErrorCode::InvalidMesh);
        EXPECT_TRUE(std::string(e.what()).find("Outer error") != std::string::npos);
        EXPECT_TRUE(std::string(e.what()).find("Inner error") != std::string::npos);
    }
}

} // namespace rebel::tests
