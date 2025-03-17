#include <gtest/gtest.h>
#include "graphics/GraphicsError.h"

using namespace RebelCAD::Graphics;

TEST(GraphicsErrorTest, ThrowsAndCatchesGraphicsError) {
    try {
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed, "Failed to compile shader");
        FAIL() << "Expected GraphicsError";
    }
    catch (const GraphicsError& e) {
        EXPECT_EQ(e.getGraphicsCode(), GraphicsErrorCode::ShaderCompilationFailed);
        EXPECT_EQ(e.getCode(), RebelCAD::Core::ErrorCode::GraphicsError);
        EXPECT_STREQ(e.what(), "Failed to compile shader");
    }
}

TEST(GraphicsErrorTest, FormatsOpenGLErrors) {
    std::string message = GraphicsError::formatAPIError(0x0500, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code 1280: GL_INVALID_ENUM");

    message = GraphicsError::formatAPIError(0x0501, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code 1281: GL_INVALID_VALUE");

    message = GraphicsError::formatAPIError(0x0502, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code 1282: GL_INVALID_OPERATION");
}

TEST(GraphicsErrorTest, FormatsVulkanErrors) {
    std::string message = GraphicsError::formatAPIError(-1, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code -1: VK_ERROR_OUT_OF_HOST_MEMORY");

    message = GraphicsError::formatAPIError(-2, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code -2: VK_ERROR_OUT_OF_DEVICE_MEMORY");

    message = GraphicsError::formatAPIError(-9, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code -9: VK_ERROR_INCOMPATIBLE_DRIVER");
}

TEST(GraphicsErrorTest, HandlesUnknownErrorCodes) {
    std::string message = GraphicsError::formatAPIError(99999, "Test operation");
    EXPECT_EQ(message, "Test operation failed with error code 99999: Unknown error code");
}

TEST(GraphicsErrorTest, ErrorCodeEnumeration) {
    // Test that all error codes are properly defined
    GraphicsError error(GraphicsErrorCode::None, "Test");
    EXPECT_EQ(error.getGraphicsCode(), GraphicsErrorCode::None);

    error = GraphicsError(GraphicsErrorCode::ShaderCompilationFailed, "Test");
    EXPECT_EQ(error.getGraphicsCode(), GraphicsErrorCode::ShaderCompilationFailed);

    error = GraphicsError(GraphicsErrorCode::InvalidShaderType, "Test");
    EXPECT_EQ(error.getGraphicsCode(), GraphicsErrorCode::InvalidShaderType);

    error = GraphicsError(GraphicsErrorCode::InvalidOperation, "Test");
    EXPECT_EQ(error.getGraphicsCode(), GraphicsErrorCode::InvalidOperation);
}

TEST(GraphicsErrorTest, InheritanceFromCoreError) {
    GraphicsError error(GraphicsErrorCode::ShaderCompilationFailed, "Test message");
    
    // Should be catchable as a Core::Error
    try {
        throw error;
        FAIL() << "Expected Core::Error";
    }
    catch (const RebelCAD::Core::Error& e) {
        EXPECT_EQ(e.getCode(), RebelCAD::Core::ErrorCode::GraphicsError);
        EXPECT_STREQ(e.what(), "Test message");
    }
}
