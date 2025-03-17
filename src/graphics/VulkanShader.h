#pragma once

#include "graphics/Shader.h"
#include <vulkan/vulkan.h>
#include <unordered_map>

namespace RebelCAD {
namespace Graphics {

class VulkanShader : public Shader {
public:
    VulkanShader(const std::string& name, ShaderType type)
        : Shader(name, type)
        , handle(VK_NULL_HANDLE)
        , stage(shaderTypeToVulkan(type)) {}
        
    ~VulkanShader() override;

    VkShaderModule getHandle() const { return handle; }
    VkShaderStageFlagBits getStage() const { return stage; }

protected:
    bool compile(const std::string& source) override;
    bool compileSpirv(const std::vector<uint32_t>& spirv) override;

private:
    VkShaderModule handle{VK_NULL_HANDLE};
    VkShaderStageFlagBits stage{VK_SHADER_STAGE_VERTEX_BIT};  // Default to vertex, will be set in constructor
    
    static VkShaderStageFlagBits shaderTypeToVulkan(ShaderType type);
    bool createShaderModule(const std::vector<uint32_t>& spirv);
};

class VulkanShaderProgram : public ShaderProgram {
public:
    explicit VulkanShaderProgram(const std::string& name)
        : ShaderProgram(name)
        , pipelineLayout(VK_NULL_HANDLE)
        , descriptorSetLayout(VK_NULL_HANDLE) {}
        
    ~VulkanShaderProgram() override;

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

    // Vulkan-specific getters
    const std::vector<VkPipelineShaderStageCreateInfo>& getShaderStages() const { return shaderStages; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }
    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    std::vector<std::shared_ptr<VulkanShader>> attachedShaders;

    // Uniform buffer management
    struct UniformBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
        void* mapped;
        size_t size;
    };
    std::unordered_map<std::string, UniformBuffer> uniformBuffers;

    void createDescriptorSetLayout();
    void createPipelineLayout();
    void updateUniformBuffer(const std::string& name, const void* data, size_t size);
    void cleanupUniformBuffers();
};

} // namespace Graphics
} // namespace RebelCAD
