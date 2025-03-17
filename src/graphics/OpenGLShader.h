#pragma once

#include "graphics/Shader.h"
#include <glad/glad.h>
#include <unordered_map>

namespace RebelCAD {
namespace Graphics {

class OpenGLShader : public Shader {
public:
    OpenGLShader(const std::string& name, ShaderType type);
    ~OpenGLShader() override;

    GLuint getHandle() const { return handle; }

protected:
    bool compile(const std::string& source) override;
    bool compileSpirv(const std::vector<uint32_t>& spirv) override;

private:
    GLuint handle;
    static GLenum shaderTypeToGL(ShaderType type);
};

class OpenGLShaderProgram : public ShaderProgram {
public:
    explicit OpenGLShaderProgram(const std::string& name);
    ~OpenGLShaderProgram() override;

    void attach(const std::shared_ptr<Shader>& shader) override;
    bool link() override;
    void bind() const override;
    void unbind() const override;

    // Uniform setters
    void setInt(const std::string& name, int value) override;
    void setFloat(const std::string& name, float value) override;
    void setVec2(const std::string& name, const float* value) override;
    void setVec3(const std::string& name, const float* value) override;
    void setVec4(const std::string& name, const float* value) override;
    void setMat3(const std::string& name, const float* value) override;
    void setMat4(const std::string& name, const float* value) override;

private:
    GLuint handle;
    std::unordered_map<std::string, GLint> uniformLocations;
    
    GLint getUniformLocation(const std::string& name);
    void cacheUniformLocations();
};

} // namespace Graphics
} // namespace RebelCAD
