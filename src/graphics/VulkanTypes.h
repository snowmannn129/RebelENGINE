#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// Forward declarations for Vulkan types
struct VkInstance_T;
typedef struct VkInstance_T* VkInstance;
struct VkDevice_T;
typedef struct VkDevice_T* VkDevice;
struct VkPhysicalDevice_T;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
struct VkQueue_T;
typedef struct VkQueue_T* VkQueue;
struct VkCommandPool_T;
typedef struct VkCommandPool_T* VkCommandPool;
struct VkCommandBuffer_T;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
struct VkSurfaceKHR_T;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
struct VkSwapchainKHR_T;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;

namespace RebelCAD {
namespace Graphics {

// Vulkan-specific types can be forward declared here
class VulkanContext;
class VulkanDevice;
class VulkanSwapchain;

} // namespace Graphics
} // namespace RebelCAD
