#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core/Log.h"
#include "graphics/GraphicsError.h"

namespace RebelCAD {
namespace Graphics {

enum class ShaderType {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessControl,
    TessEvaluation
};

class Shader {
public:
    static std::shared_ptr<Shader> Create(const std::string& name,
                                        ShaderType type,
                                        const std::string& source);
    
    static std::shared_ptr<Shader> CreateFromFile(const std::string& name,
                                                 ShaderType type,
                                                 const std::string& filepath);
    
    static std::shared_ptr<Shader> CreateFromSPIRV(const std::string& name,
                                                  ShaderType type,
                                                  const std::vector<uint32_t>& spirv);

    virtual ~Shader() = default;

    // Getters
    const std::string& getName() const { return name; }
    ShaderType getType() const { return type; }
    bool isCompiled() const { return compiled; }

    // Delete copy and move constructors/assignments
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = delete;
    Shader& operator=(Shader&&) = delete;

protected:
    Shader(const std::string& name, ShaderType type)
        : name(name)
        , type(type)
        , compiled(false) {}

    virtual bool compile(const std::string& source) = 0;
    virtual bool compileSpirv(const std::vector<uint32_t>& spirv) = 0;

    std::string name;
    ShaderType type;
    bool compiled;

    friend class ShaderProgram;
};

class ShaderProgram {
public:
    static std::shared_ptr<ShaderProgram> Create(const std::string& name);

    virtual ~ShaderProgram() = default;

    virtual void attach(const std::shared_ptr<Shader>& shader) = 0;
    virtual bool link() = 0;
    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    // Uniform setters
    virtual void setInt(const std::string& name, int value) = 0;
    virtual void setFloat(const std::string& name, float value) = 0;
    virtual void setVec2(const std::string& name, const float* value) = 0;
    virtual void setVec3(const std::string& name, const float* value) = 0;
    virtual void setVec4(const std::string& name, const float* value) = 0;
    virtual void setMat3(const std::string& name, const float* value) = 0;
    virtual void setMat4(const std::string& name, const float* value) = 0;

    // Getters
    const std::string& getName() const { return name; }
    bool isLinked() const { return linked; }

    // Delete copy and move constructors/assignments
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&&) = delete;
    ShaderProgram& operator=(ShaderProgram&&) = delete;

protected:
    explicit ShaderProgram(const std::string& name)
        : name(name)
        , linked(false) {}

    std::string name;
    bool linked;
};

} // namespace Graphics
} // namespace RebelCAD
