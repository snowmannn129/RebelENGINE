#include "graphics/Shader.h"
#include "graphics/GraphicsSystem.h"
#include <fstream>
#include <sstream>

namespace RebelCAD {
namespace Graphics {

std::shared_ptr<Shader> Shader::Create(const std::string& name,
                                     ShaderType type,
                                     const std::string& source) {
    auto& graphics = GraphicsSystem::getInstance();
    std::shared_ptr<Shader> shader;

    // Create appropriate shader based on current graphics API
    if (graphics.getAPI() == GraphicsAPI::OpenGL) {
        // OpenGL implementation will be in OpenGLShader
        REBEL_LOG_INFO("Creating OpenGL shader: " + name);
        // shader = std::make_shared<OpenGLShader>(name, type);
    } else {
        // Vulkan implementation will be in VulkanShader
        REBEL_LOG_INFO("Creating Vulkan shader: " + name);
        // shader = std::make_shared<VulkanShader>(name, type);
    }

    if (!shader) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType,
                                 "Failed to create shader of type " + std::to_string(static_cast<int>(type)));
        return nullptr;
    }

    if (!shader->compile(source)) {
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed,
                                 "Failed to compile shader: " + name);
        return nullptr;
    }

    return shader;
}

std::shared_ptr<Shader> Shader::CreateFromFile(const std::string& name,
                                             ShaderType type,
                                             const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            REBEL_THROW_GRAPHICS_ERROR(FileIOError,
                                     "Failed to open shader file: " + filepath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return Create(name, type, buffer.str());
    }
    catch (const GraphicsError& e) {
        throw; // Re-throw graphics errors
    }
    catch (const std::exception& e) {
        REBEL_THROW_GRAPHICS_ERROR(FileIOError,
                                 "Failed to read shader file: " + filepath + 
                                 "\nError: " + e.what());
    }
    return nullptr;
}

std::shared_ptr<Shader> Shader::CreateFromSPIRV(const std::string& name,
                                               ShaderType type,
                                               const std::vector<uint32_t>& spirv) {
    auto& graphics = GraphicsSystem::getInstance();
    std::shared_ptr<Shader> shader;

    if (graphics.getAPI() == GraphicsAPI::OpenGL) {
        REBEL_LOG_INFO("Creating OpenGL shader from SPIR-V: " + name);
        // shader = std::make_shared<OpenGLShader>(name, type);
    } else {
        REBEL_LOG_INFO("Creating Vulkan shader from SPIR-V: " + name);
        // shader = std::make_shared<VulkanShader>(name, type);
    }

    if (!shader) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType,
                                 "Failed to create shader of type " + std::to_string(static_cast<int>(type)));
        return nullptr;
    }

    if (!shader->compileSpirv(spirv)) {
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed,
                                 "Failed to compile SPIR-V shader: " + name);
        return nullptr;
    }

    return shader;
}

std::shared_ptr<ShaderProgram> ShaderProgram::Create(const std::string& name) {
    auto& graphics = GraphicsSystem::getInstance();
    std::shared_ptr<ShaderProgram> program;

    if (graphics.getAPI() == GraphicsAPI::OpenGL) {
        REBEL_LOG_INFO("Creating OpenGL shader program: " + name);
        // program = std::make_shared<OpenGLShaderProgram>(name);
    } else {
        REBEL_LOG_INFO("Creating Vulkan shader program: " + name);
        // program = std::make_shared<VulkanShaderProgram>(name);
    }

    if (!program) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidProgram,
                                 "Failed to create shader program: " + name);
    }

    return program;
}

} // namespace Graphics
} // namespace RebelCAD
