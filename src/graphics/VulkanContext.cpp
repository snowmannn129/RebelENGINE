#include "graphics/VulkanContext.h"

namespace RebelCAD {
namespace Graphics {

VulkanContext& VulkanContext::getInstance_() {
    static VulkanContext instance;
    return instance;
}

VkDevice VulkanContext::getDevice() {
    return getInstance_().device;
}

VkInstance VulkanContext::getInstance() {
    return getInstance_().instance;
}

VkPhysicalDevice VulkanContext::getPhysicalDevice() {
    return getInstance_().physicalDevice;
}

VkQueue VulkanContext::getGraphicsQueue() {
    return getInstance_().graphicsQueue;
}

VkQueue VulkanContext::getPresentQueue() {
    return getInstance_().presentQueue;
}

uint32_t VulkanContext::getGraphicsQueueFamily() {
    return getInstance_().graphicsQueueFamily;
}

uint32_t VulkanContext::getPresentQueueFamily() {
    return getInstance_().presentQueueFamily;
}

void VulkanContext::initialize() {
    // TODO: Implement Vulkan initialization
    // This will include:
    // - Create instance
    // - Select physical device
    // - Create logical device
    // - Get queue handles
}

void VulkanContext::cleanup() {
    auto& instance = getInstance_();
    
    if (instance.device != VK_NULL_HANDLE) {
        vkDestroyDevice(instance.device, nullptr);
        instance.device = VK_NULL_HANDLE;
    }
    
    if (instance.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance.instance, nullptr);
        instance.instance = VK_NULL_HANDLE;
    }
}

} // namespace Graphics
} // namespace RebelCAD
