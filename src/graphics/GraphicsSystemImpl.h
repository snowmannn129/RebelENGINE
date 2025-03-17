#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"

namespace RebelCAD {
namespace Graphics {

// Forward declare GraphicsSystem to avoid circular dependency
class GraphicsSystem;

// Implementation details for GraphicsSystem
struct GraphicsSystemImpl {
    GraphicsSystemImpl();
    ~GraphicsSystemImpl();

    GLFWwindow* window{nullptr};
    VkInstance vulkanInstance{VK_NULL_HANDLE};
    VkDevice vulkanDevice{VK_NULL_HANDLE};
    VkPhysicalDevice vulkanPhysicalDevice{VK_NULL_HANDLE};
    VkSurfaceKHR vulkanSurface{VK_NULL_HANDLE};
    VkSwapchainKHR vulkanSwapchain{VK_NULL_HANDLE};
    
    // Rendering state
    Color currentLineColor{1.0f, 1.0f, 1.0f, 1.0f};
    float currentLineThickness{1.0f};
    std::vector<float> currentLineDashPattern;
    RenderMode currentRenderMode{RenderMode::Normal};
    Color currentColor{1.0f, 1.0f, 1.0f, 1.0f};
};

} // namespace Graphics
} // namespace RebelCAD
