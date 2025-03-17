#pragma once

#include "core/Error.h"
#include <string>

namespace RebelCAD {
namespace Graphics {

// Graphics-specific error codes
enum class GraphicsErrorCode {
    None = 0,
    ShaderCompilationFailed,
    ShaderLinkingFailed,
    TextureLoadFailed,
    InvalidShaderType,
    InvalidUniform,
    InvalidAttribute,
    InvalidFramebuffer,
    InvalidVertexArray,
    InvalidBuffer,
    InvalidTexture,
    InvalidSampler,
    InvalidProgram,
    InvalidOperation,
    OutOfMemory,
    APIError,
    DriverError,
    UnsupportedFeature,
    InvalidGeometry,
    InvalidTransform,
    InvalidState,
    FileIOError
};

class GraphicsError : public Core::Error {
public:
    GraphicsError(GraphicsErrorCode graphicsCode, const std::string& message)
        : Core::Error(Core::ErrorCode::GraphicsError, message)
        , graphicsCode_(graphicsCode) {}

    GraphicsErrorCode getGraphicsCode() const { return graphicsCode_; }

    static void throwError(GraphicsErrorCode code, const std::string& message) {
        throw GraphicsError(code, message);
    }

    // Helper method to format OpenGL/Vulkan error codes into messages
    static std::string formatAPIError(uint32_t errorCode, const std::string& context) {
        std::string message = context + " failed with error code " + std::to_string(errorCode) + ": ";
        switch (errorCode) {
            // OpenGL error codes
            case 0x0500: message += "GL_INVALID_ENUM"; break;
            case 0x0501: message += "GL_INVALID_VALUE"; break;
            case 0x0502: message += "GL_INVALID_OPERATION"; break;
            case 0x0503: message += "GL_STACK_OVERFLOW"; break;
            case 0x0504: message += "GL_STACK_UNDERFLOW"; break;
            case 0x0505: message += "GL_OUT_OF_MEMORY"; break;
            case 0x0506: message += "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case 0x0507: message += "GL_CONTEXT_LOST"; break;
            // Vulkan error codes
            case -1: message += "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
            case -2: message += "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
            case -3: message += "VK_ERROR_INITIALIZATION_FAILED"; break;
            case -4: message += "VK_ERROR_DEVICE_LOST"; break;
            case -5: message += "VK_ERROR_MEMORY_MAP_FAILED"; break;
            case -6: message += "VK_ERROR_LAYER_NOT_PRESENT"; break;
            case -7: message += "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
            case -8: message += "VK_ERROR_FEATURE_NOT_PRESENT"; break;
            case -9: message += "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
            case -10: message += "VK_ERROR_TOO_MANY_OBJECTS"; break;
            case -11: message += "VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
            default: message += "Unknown error code";
        }
        return message;
    }

private:
    GraphicsErrorCode graphicsCode_;
};

// Convenience macro for throwing graphics errors
#define REBEL_THROW_GRAPHICS_ERROR(code, message) \
    RebelCAD::Graphics::GraphicsError::throwError(RebelCAD::Graphics::GraphicsErrorCode::code, message)

// Convenience macro for checking OpenGL errors
#define REBEL_CHECK_GL_ERROR(context) \
    do { \
        GLenum err = glGetError(); \
        if (err != GL_NO_ERROR) { \
            REBEL_THROW_GRAPHICS_ERROR(APIError, \
                RebelCAD::Graphics::GraphicsError::formatAPIError(err, context)); \
        } \
    } while (0)

// Convenience macro for checking Vulkan results
#define REBEL_CHECK_VK_RESULT(result, context) \
    do { \
        if (result != VK_SUCCESS) { \
            REBEL_THROW_GRAPHICS_ERROR(APIError, \
                RebelCAD::Graphics::GraphicsError::formatAPIError(result, context)); \
        } \
    } while (0)

} // namespace Graphics
} // namespace RebelCAD
