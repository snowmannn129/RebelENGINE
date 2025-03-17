#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "graphics/OpenGLShader.h"
#include "core/Log.h"
#include "core/Error.h"
#include <vector>
#include <sstream>
#include <cstring>

// OpenGL function pointer types
typedef void (APIENTRY* PFNGLSHADERBINARYPROC)(GLsizei count, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length);
typedef void (APIENTRY* PFNGLSPECIALIZESHADERPROC)(GLuint shader, const GLchar* pEntryPoint, GLuint numSpecializationConstants, const GLuint* pConstantIndex, const GLuint* pConstantValue);

// OpenGL constants if not defined by GLAD
#ifndef GL_MAJOR_VERSION
#define GL_MAJOR_VERSION 0x821B
#endif

#ifndef GL_MINOR_VERSION
#define GL_MINOR_VERSION 0x821C
#endif

#ifndef GL_ARB_gl_spirv
#define GL_ARB_gl_spirv 1
#define GL_SHADER_BINARY_FORMAT_SPIR_V 0x9551
#endif

// Function pointers for OpenGL 4.6 SPIR-V support
static PFNGLSHADERBINARYPROC glShaderBinaryProc = nullptr;
static PFNGLSPECIALIZESHADERPROC glSpecializeShaderProc = nullptr;

// Helper function to load SPIR-V extension functions
static bool loadSpirVFunctions() {
    if (glShaderBinaryProc && glSpecializeShaderProc) {
        return true;
    }

    glShaderBinaryProc = reinterpret_cast<PFNGLSHADERBINARYPROC>(
        glfwGetProcAddress("glShaderBinary"));
    glSpecializeShaderProc = reinterpret_cast<PFNGLSPECIALIZESHADERPROC>(
        glfwGetProcAddress("glSpecializeShader"));

    return glShaderBinaryProc && glSpecializeShaderProc;
}

namespace RebelCAD {
namespace Graphics {

OpenGLShader::OpenGLShader(const std::string& name, ShaderType type)
    : Shader(name, type), handle(0) {
}

OpenGLShader::~OpenGLShader() {
    if (handle != 0) {
        glDeleteShader(handle);
    }
}

GLenum OpenGLShader::shaderTypeToGL(ShaderType type) {
    switch (type) {
        case ShaderType::Vertex:         return GL_VERTEX_SHADER;
        case ShaderType::Fragment:       return GL_FRAGMENT_SHADER;
        case ShaderType::Compute:        return GL_COMPUTE_SHADER;
        case ShaderType::Geometry:       return GL_GEOMETRY_SHADER;
        case ShaderType::TessControl:    return GL_TESS_CONTROL_SHADER;
        case ShaderType::TessEvaluation: return GL_TESS_EVALUATION_SHADER;
        default:
            throw Core::Error(Core::ErrorCode::GraphicsError, "Unknown shader type");
    }
}

bool OpenGLShader::compile(const std::string& source) {
    if (handle != 0) {
        glDeleteShader(handle);
    }

    handle = glCreateShader(shaderTypeToGL(type));
    if (handle == 0) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Failed to create shader object");
        return false;
    }

    const char* sourceCStr = source.c_str();
    glShaderSource(handle, 1, &sourceCStr, nullptr);
    glCompileShader(handle);

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint maxLength = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(handle, maxLength, &maxLength, infoLog.data());

        std::string error = std::string(infoLog.data());
        glDeleteShader(handle);
        handle = 0;
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed, error);
        return false;
    }

    compiled = true;
    return true;
}

bool OpenGLShader::compileSpirv(const std::vector<uint32_t>& spirv) {
    // Check for SPIR-V support through OpenGL version
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    
    // OpenGL 4.6 is required for SPIR-V
    if (major < 4 || (major == 4 && minor < 6)) {
        REBEL_THROW_GRAPHICS_ERROR(UnsupportedFeature, "SPIR-V not supported by this OpenGL implementation");
        return false;
    }

    if (handle != 0) {
        glDeleteShader(handle);
    }

    handle = glCreateShader(shaderTypeToGL(type));
    if (handle == 0) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Failed to create shader object");
        return false;
    }

    if (!loadSpirVFunctions()) {
        REBEL_THROW_GRAPHICS_ERROR(UnsupportedFeature, "Failed to load SPIR-V extension functions");
        return false;
    }

    glShaderBinaryProc(1, &handle, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
    glSpecializeShaderProc(handle, "main", 0, nullptr, nullptr);

    // Check compilation status
    GLint success = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint maxLength = 0;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(handle, maxLength, &maxLength, infoLog.data());

        std::string error = std::string(infoLog.data());
        glDeleteShader(handle);
        handle = 0;
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed, error);
        return false;
    }

    compiled = true;
    return true;
}

// OpenGLShaderProgram implementation

OpenGLShaderProgram::OpenGLShaderProgram(const std::string& name)
    : ShaderProgram(name), handle(0) {
    handle = glCreateProgram();
    if (handle == 0) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidProgram, "Failed to create shader program");
    }
}

OpenGLShaderProgram::~OpenGLShaderProgram() {
    if (handle != 0) {
        glDeleteProgram(handle);
    }
}

void OpenGLShaderProgram::attach(const std::shared_ptr<Shader>& shader) {
    auto glShader = std::dynamic_pointer_cast<OpenGLShader>(shader);
    if (!glShader) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Invalid shader type - expected OpenGLShader");
    }

    if (!glShader->isCompiled()) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Cannot attach uncompiled shader");
    }

    glAttachShader(handle, glShader->getHandle());
}

bool OpenGLShaderProgram::link() {
    glLinkProgram(handle);

    GLint success = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success) {
        GLint maxLength = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(handle, maxLength, &maxLength, infoLog.data());

        std::string error = std::string(infoLog.data());
        REBEL_THROW_GRAPHICS_ERROR(ShaderLinkingFailed, error);
        return false;
    }

    // Cache uniform locations after successful linking
    cacheUniformLocations();
    linked = true;
    return true;
}

void OpenGLShaderProgram::bind() const {
    glUseProgram(handle);
}

void OpenGLShaderProgram::unbind() const {
    glUseProgram(0);
}

GLint OpenGLShaderProgram::getUniformLocation(const std::string& name) {
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(handle, name.c_str());
    if (location == -1) {
        std::stringstream ss;
        ss << "Uniform '" << name << "' not found in shader program '" << getName() << "'";
        Core::Logger::getInstance().warning(ss.str());
    }
    uniformLocations[name] = location;
    return location;
}

void OpenGLShaderProgram::cacheUniformLocations() {
    uniformLocations.clear();

    GLint numUniforms = 0;
    glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniforms);

    GLint maxNameLength = 0;
    glGetProgramiv(handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

    std::vector<GLchar> nameData(maxNameLength);
    for (GLint i = 0; i < numUniforms; i++) {
        GLsizei length;
        GLint size;
        GLenum type;
        glGetActiveUniform(handle, i, maxNameLength, &length, &size, &type, nameData.data());
        std::string name(nameData.data(), length);
        
        GLint location = glGetUniformLocation(handle, name.c_str());
        if (location != -1) {
            uniformLocations[name] = location;
        }
    }
}

void OpenGLShaderProgram::setInt(const std::string& name, int value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void OpenGLShaderProgram::setFloat(const std::string& name, float value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void OpenGLShaderProgram::setVec2(const std::string& name, const float* value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform2fv(location, 1, value);
    }
}

void OpenGLShaderProgram::setVec3(const std::string& name, const float* value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform3fv(location, 1, value);
    }
}

void OpenGLShaderProgram::setVec4(const std::string& name, const float* value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniform4fv(location, 1, value);
    }
}

void OpenGLShaderProgram::setMat3(const std::string& name, const float* value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix3fv(location, 1, GL_FALSE, value);
    }
}

void OpenGLShaderProgram::setMat4(const std::string& name, const float* value) {
    GLint location = getUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}

} // namespace Graphics
} // namespace RebelCAD
