#pragma once

#include <string>
#include <glad/glad.h>
#include <unordered_map>
#include <memory>

namespace RebelCAD {
namespace Core {

/**
 * @brief Manages OpenGL shader programs and their lifecycle
 * 
 * Provides utilities for creating, compiling, and managing shader programs.
 * Handles shader compilation errors and program linking.
 */
class ShaderManager {
public:
    /**
     * @brief Creates a shader program from vertex and fragment shader source
     * @param vertexSource Source code for vertex shader
     * @param fragmentSource Source code for fragment shader
     * @return OpenGL program ID or 0 if compilation failed
     */
    static GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);

    /**
     * @brief Deletes a shader program
     * @param programId OpenGL program ID to delete
     */
    static void deleteShaderProgram(GLuint programId);

private:
    /**
     * @brief Compiles a shader from source
     * @param type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @param source Shader source code
     * @return OpenGL shader ID or 0 if compilation failed
     */
    static GLuint compileShader(GLenum type, const char* source);

    /**
     * @brief Links compiled shaders into a program
     * @param vertexShader Vertex shader ID
     * @param fragmentShader Fragment shader ID
     * @return OpenGL program ID or 0 if linking failed
     */
    static GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);

    /**
     * @brief Checks for shader compilation errors
     * @param shaderId Shader ID to check
     * @return true if compilation succeeded
     */
    static bool checkShaderCompilation(GLuint shaderId);

    /**
     * @brief Checks for program linking errors
     * @param programId Program ID to check
     * @return true if linking succeeded
     */
    static bool checkProgramLinking(GLuint programId);
};

} // namespace Core
} // namespace RebelCAD
