#pragma once

#include <vulkan/vulkan.h>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Manages Vulkan device and instance context
 */
class VulkanContext {
public:
    static VkDevice getDevice();
    static VkInstance getInstance();
    static VkPhysicalDevice getPhysicalDevice();
    static VkQueue getGraphicsQueue();
    static VkQueue getPresentQueue();
    static uint32_t getGraphicsQueueFamily();
    static uint32_t getPresentQueueFamily();

    static void initialize();
    static void cleanup();

private:
    static VulkanContext& getInstance_();

    VulkanContext() = default;
    ~VulkanContext() = default;
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VkInstance instance{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    uint32_t graphicsQueueFamily{0};
    uint32_t presentQueueFamily{0};
};

} // namespace Graphics
} // namespace RebelCAD
