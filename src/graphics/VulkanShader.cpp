#include "graphics/VulkanShader.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/VulkanContext.h"
#include "core/Error.h"
#include <shaderc/shaderc.hpp>
#include <fstream>
#include <sstream>

namespace RebelCAD {
namespace Graphics {

VulkanShader::~VulkanShader() {
    if (handle != VK_NULL_HANDLE) {
        vkDestroyShaderModule(VulkanContext::getDevice(), handle, nullptr);
    }
}

VkShaderStageFlagBits VulkanShader::shaderTypeToVulkan(ShaderType type) {
    switch (type) {
        case ShaderType::Vertex:         return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderType::Fragment:       return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderType::Compute:        return VK_SHADER_STAGE_COMPUTE_BIT;
        case ShaderType::Geometry:       return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderType::TessControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderType::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        default:
            REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Unknown shader type");
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
}

bool VulkanShader::compile(const std::string& source) {
    // Compile GLSL to SPIR-V using shaderc
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Set compilation options
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

    // Determine shader kind
    shaderc_shader_kind kind;
    switch (type) {
        case ShaderType::Vertex:         kind = shaderc_vertex_shader; break;
        case ShaderType::Fragment:       kind = shaderc_fragment_shader; break;
        case ShaderType::Compute:        kind = shaderc_compute_shader; break;
        case ShaderType::Geometry:       kind = shaderc_geometry_shader; break;
        case ShaderType::TessControl:    kind = shaderc_tess_control_shader; break;
        case ShaderType::TessEvaluation: kind = shaderc_tess_evaluation_shader; break;
        default:
            REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Unknown shader type");
            return false;
    }

    // Compile to SPIR-V
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
        source, kind, name.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed, result.GetErrorMessage());
        return false;
    }

    // Convert to vector and create shader module
    std::vector<uint32_t> spirv(result.cbegin(), result.cend());
    return createShaderModule(spirv);
}

bool VulkanShader::compileSpirv(const std::vector<uint32_t>& spirv) {
    return createShaderModule(spirv);
}

bool VulkanShader::createShaderModule(const std::vector<uint32_t>& spirv) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkResult result = vkCreateShaderModule(VulkanContext::getDevice(), &createInfo, nullptr, &handle);
    if (result != VK_SUCCESS) {
        REBEL_THROW_GRAPHICS_ERROR(ShaderCompilationFailed, "Failed to create shader module");
        return false;
    }

    compiled = true;
    return true;
}

// VulkanShaderProgram implementation
VulkanShaderProgram::~VulkanShaderProgram() {
    cleanupUniformBuffers();
    
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(VulkanContext::getDevice(), pipelineLayout, nullptr);
    }
    if (descriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(VulkanContext::getDevice(), descriptorSetLayout, nullptr);
    }
}

void VulkanShaderProgram::attach(const std::shared_ptr<Shader>& shader) {
    auto vulkanShader = std::dynamic_pointer_cast<VulkanShader>(shader);
    if (!vulkanShader) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidShaderType, "Invalid shader type for Vulkan shader program");
    }

    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.stage = vulkanShader->getStage();
    stageInfo.module = vulkanShader->getHandle();
    stageInfo.pName = "main";

    shaderStages.push_back(stageInfo);
    attachedShaders.push_back(vulkanShader);
}

bool VulkanShaderProgram::link() {
    try {
        createDescriptorSetLayout();
        createPipelineLayout();
        linked = true;
        return true;
    }
    catch (const GraphicsError& e) {
        throw; // Re-throw graphics errors
    }
}

void VulkanShaderProgram::bind() const {
    // Binding is handled by the pipeline in Vulkan
}

void VulkanShaderProgram::unbind() const {
    // No-op in Vulkan
}

void VulkanShaderProgram::createDescriptorSetLayout() {
    // TODO: Implement descriptor set layout creation
    // This will be based on reflection of shader uniforms
}

void VulkanShaderProgram::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkResult result = vkCreatePipelineLayout(VulkanContext::getDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS) {
        REBEL_THROW_GRAPHICS_ERROR(InvalidProgram, "Failed to create pipeline layout");
    }
}

void VulkanShaderProgram::updateUniformBuffer(const std::string& name, const void* data, size_t size) {
    auto it = uniformBuffers.find(name);
    if (it == uniformBuffers.end()) {
        // Create new uniform buffer
        UniformBuffer ub{};
        ub.size = size;
        // TODO: Create buffer and allocate memory
        uniformBuffers[name] = ub;
        it = uniformBuffers.find(name);
    }

    // Copy data to buffer
    memcpy(it->second.mapped, data, size);
}

void VulkanShaderProgram::cleanupUniformBuffers() {
    for (auto& [name, ub] : uniformBuffers) {
        vkDestroyBuffer(VulkanContext::getDevice(), ub.buffer, nullptr);
        vkFreeMemory(VulkanContext::getDevice(), ub.memory, nullptr);
    }
    uniformBuffers.clear();
}

// Uniform setters
void VulkanShaderProgram::setInt(const std::string& name, int value) {
    updateUniformBuffer(name, &value, sizeof(int));
}

void VulkanShaderProgram::setFloat(const std::string& name, float value) {
    updateUniformBuffer(name, &value, sizeof(float));
}

void VulkanShaderProgram::setVec2(const std::string& name, const float* value) {
    updateUniformBuffer(name, value, sizeof(float) * 2);
}

void VulkanShaderProgram::setVec3(const std::string& name, const float* value) {
    updateUniformBuffer(name, value, sizeof(float) * 3);
}

void VulkanShaderProgram::setVec4(const std::string& name, const float* value) {
    updateUniformBuffer(name, value, sizeof(float) * 4);
}

void VulkanShaderProgram::setMat3(const std::string& name, const float* value) {
    updateUniformBuffer(name, value, sizeof(float) * 9);
}

void VulkanShaderProgram::setMat4(const std::string& name, const float* value) {
    updateUniformBuffer(name, value, sizeof(float) * 16);
}

} // namespace Graphics
} // namespace RebelCAD
