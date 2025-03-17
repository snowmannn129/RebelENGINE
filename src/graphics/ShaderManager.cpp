#include "Core/ShaderManager.h"
#include <vector>
#include <iostream>

namespace RebelCAD {
namespace Core {

GLuint ShaderManager::createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return 0;
    }

    // Link program
    GLuint program = linkProgram(vertexShader, fragmentShader);

    // Cleanup shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void ShaderManager::deleteShaderProgram(GLuint programId) {
    if (programId) {
        glDeleteProgram(programId);
    }
}

GLuint ShaderManager::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    if (!checkShaderCompilation(shader)) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint ShaderManager::linkProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    if (!checkProgramLinking(program)) {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

bool ShaderManager::checkShaderCompilation(GLuint shaderId) {
    GLint success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        GLint logLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shaderId, logLength, nullptr, log.data());
        
        std::cerr << "Shader compilation failed:\n" << log.data() << std::endl;
        return false;
    }
    
    return true;
}

bool ShaderManager::checkProgramLinking(GLuint programId) {
    GLint success;
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    
    if (!success) {
        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        
        std::vector<char> log(logLength);
        glGetProgramInfoLog(programId, logLength, nullptr, log.data());
        
        std::cerr << "Program linking failed:\n" << log.data() << std::endl;
        return false;
    }
    
    return true;
}

} // namespace Core
} // namespace RebelCAD
