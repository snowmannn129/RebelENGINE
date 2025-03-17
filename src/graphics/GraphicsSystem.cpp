#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "graphics/GraphicsSystem.h"
#include "graphics/GLCommon.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "core/Log.h"
#include "core/Error.h"

namespace RebelCAD {
namespace Graphics {

// Implementation details
struct GraphicsSystem::GraphicsSystemImpl {
    GraphicsSystemImpl() 
        : window(nullptr)
        , vulkanInstance(VK_NULL_HANDLE)
        , vulkanDevice(VK_NULL_HANDLE)
        , vulkanPhysicalDevice(VK_NULL_HANDLE)
        , vulkanSurface(VK_NULL_HANDLE)
        , vulkanSwapchain(VK_NULL_HANDLE)
        , currentLineColor(1.0f, 1.0f, 1.0f, 1.0f)
        , currentLineThickness(1.0f)
        , currentRenderMode(RenderMode::Normal)
        , currentColor(1.0f, 1.0f, 1.0f, 1.0f) {}
    
    ~GraphicsSystemImpl() {
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    // Window handle
    GLFWwindow* window;
    
    // Vulkan specific members
    VkInstance vulkanInstance;
    VkDevice vulkanDevice;
    VkPhysicalDevice vulkanPhysicalDevice;
    VkSurfaceKHR vulkanSurface;
    VkSwapchainKHR vulkanSwapchain;

    // Rendering state
    Color currentLineColor;
    float currentLineThickness;
    std::vector<float> currentLineDashPattern;
    RenderMode currentRenderMode;
    Color currentColor;
};

GraphicsSystem& GraphicsSystem::getInstance() {
    static GraphicsSystem instance;
    return instance;
}

GraphicsSystem::GraphicsSystem()
    : impl(std::make_unique<GraphicsSystemImpl>())
    , currentAPI(GraphicsAPI::Vulkan)
    , initialized(false) {
        Core::Logger::getInstance().info("Creating GraphicsSystem");
}

GraphicsSystem::~GraphicsSystem() {
    if (initialized) {
        shutdown();
    }
}

void GraphicsSystem::initialize(GraphicsAPI api, const WindowProperties& props) {
    if (initialized) {
        REBEL_THROW_ERROR(GraphicsError, "Graphics system already initialized");
    }

    REBEL_LOG_INFO("Initializing graphics system with " + 
                   std::string(api == GraphicsAPI::OpenGL ? "OpenGL" : "Vulkan"));

    // Initialize GLFW
    if (!glfwInit()) {
        REBEL_THROW_ERROR(GraphicsError, "Failed to initialize GLFW");
    }

    currentAPI = api;
    windowProps = props;

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, api == GraphicsAPI::OpenGL ? GLFW_OPENGL_API : GLFW_NO_API);
    impl->window = glfwCreateWindow(props.width, 
                                  props.height,
                                  props.title.c_str(),
                                  props.fullscreen ? glfwGetPrimaryMonitor() : nullptr,
                                  nullptr);

    if (!impl->window) {
        glfwTerminate();
        REBEL_THROW_ERROR(GraphicsError, "Failed to create window");
    }

    // Initialize graphics API
    try {
        if (api == GraphicsAPI::OpenGL) {
            initializeOpenGL();
        } else {
            initializeVulkan();
        }
    }
    catch (...) {
        glfwDestroyWindow(impl->window);
        glfwTerminate();
        throw;
    }

    initialized = true;
    Core::Logger::getInstance().info("Graphics system initialized successfully");
}

void GraphicsSystem::shutdown() {
    if (!initialized) {
        return;
    }

    Core::Logger::getInstance().info("Shutting down graphics system");

    if (currentAPI == GraphicsAPI::Vulkan) {
        // Cleanup Vulkan resources
        if (impl->vulkanSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(impl->vulkanDevice, impl->vulkanSwapchain, nullptr);
            impl->vulkanSwapchain = VK_NULL_HANDLE;
        }
        if (impl->vulkanSurface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(impl->vulkanInstance, impl->vulkanSurface, nullptr);
            impl->vulkanSurface = VK_NULL_HANDLE;
        }
        if (impl->vulkanDevice != VK_NULL_HANDLE) {
            vkDestroyDevice(impl->vulkanDevice, nullptr);
            impl->vulkanDevice = VK_NULL_HANDLE;
        }
        if (impl->vulkanInstance != VK_NULL_HANDLE) {
            vkDestroyInstance(impl->vulkanInstance, nullptr);
            impl->vulkanInstance = VK_NULL_HANDLE;
        }
    }

    initialized = false;
}

void GraphicsSystem::beginFrame() {
    if (!initialized) {
        REBEL_THROW_ERROR(GraphicsError, "Graphics system not initialized");
    }

    // Process window events
    glfwPollEvents();

    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL frame begin
    } else {
        // Vulkan frame begin
    }
}

void GraphicsSystem::endFrame() {
    if (!initialized) {
        return;
    }

    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL frame end
    } else {
        // Vulkan frame end
    }
}

void GraphicsSystem::present() {
    if (!initialized) {
        return;
    }

    if (currentAPI == GraphicsAPI::OpenGL) {
        glfwSwapBuffers(impl->window);
    } else {
        // Vulkan present
    }
}

void GraphicsSystem::setWindowTitle(const std::string& title) {
    if (initialized && impl->window) {
        glfwSetWindowTitle(impl->window, title.c_str());
        windowProps.title = title;
    }
}

void GraphicsSystem::setWindowSize(uint32_t width, uint32_t height) {
    if (initialized && impl->window) {
        glfwSetWindowSize(impl->window, width, height);
        windowProps.width = width;
        windowProps.height = height;
    }
}

void GraphicsSystem::setWindowPos(int x, int y) {
    if (initialized && impl->window) {
        glfwSetWindowPos(impl->window, x, y);
    }
}

int GraphicsSystem::getWindowPosX() const {
    if (!initialized || !impl->window) {
        return 0;
    }
    int x, y;
    glfwGetWindowPos(impl->window, &x, &y);
    return x;
}

int GraphicsSystem::getWindowPosY() const {
    if (!initialized || !impl->window) {
        return 0;
    }
    int x, y;
    glfwGetWindowPos(impl->window, &x, &y);
    return y;
}

void* GraphicsSystem::getWindow() const {
    return static_cast<void*>(impl->window);
}

bool GraphicsSystem::shouldClose() const {
    return initialized && impl->window && glfwWindowShouldClose(impl->window);
}

void GraphicsSystem::setVSync(bool enabled) {
    if (initialized) {
        windowProps.vsync = enabled;
        if (currentAPI == GraphicsAPI::OpenGL) {
            glfwSwapInterval(enabled ? 1 : 0);
        }
        // Vulkan vsync is handled through present mode
    }
}

void GraphicsSystem::setFullscreen(bool enabled) {
    if (initialized && impl->window) {
        GLFWmonitor* monitor = enabled ? glfwGetPrimaryMonitor() : nullptr;
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        
        glfwSetWindowMonitor(impl->window,
                            monitor,
                            0, 0,
                            enabled ? mode->width : windowProps.width,
                            enabled ? mode->height : windowProps.height,
                            GLFW_DONT_CARE);
        
        windowProps.fullscreen = enabled;
    }
}

void GraphicsSystem::drawLine(float startX, float startY, float endX, float endY) {
    if (!initialized) return;
    
    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL line drawing implementation
    } else {
        // Vulkan line drawing implementation
    }
}

void GraphicsSystem::setLineColor(const Color& color) {
    impl->currentLineColor = color;
}

void GraphicsSystem::setLineThickness(float thickness) {
    impl->currentLineThickness = thickness;
}

void GraphicsSystem::setLineDashPattern(const std::vector<float>& pattern) {
    impl->currentLineDashPattern = pattern;
}

void GraphicsSystem::setRenderMode(RenderMode mode) {
    impl->currentRenderMode = mode;
}

void GraphicsSystem::setColor(const Color& color) {
    impl->currentColor = color;
}

void GraphicsSystem::beginPreview() {
    if (!initialized) return;
    
    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL preview begin implementation
    } else {
        // Vulkan preview begin implementation
    }
}

void GraphicsSystem::endPreview() {
    if (!initialized) return;
    
    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL preview end implementation
    } else {
        // Vulkan preview end implementation
    }
}

void GraphicsSystem::renderTriangleMesh(const float* vertices, size_t vertexCount) {
    if (!initialized || !vertices || vertexCount == 0) return;
    
    if (currentAPI == GraphicsAPI::OpenGL) {
        // OpenGL mesh rendering implementation
    } else {
        // Vulkan mesh rendering implementation
    }
}

void GraphicsSystem::renderTriangleMeshWithAttributes(
    const float* vertices,
    const float* normals,
    const float* uvs,
    const unsigned int* indices,
    size_t vertexCount,
    size_t indexCount) {
    // Implementation
}

void GraphicsSystem::initializeOpenGL() {
    glfwMakeContextCurrent(impl->window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        REBEL_THROW_ERROR(GraphicsError, "Failed to initialize GLAD");
    }

    // Log OpenGL info
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    REBEL_LOG_INFO("OpenGL Info:");
    REBEL_LOG_INFO("  Vendor: " + std::string((const char*)vendor));
    REBEL_LOG_INFO("  Renderer: " + std::string((const char*)renderer));
    REBEL_LOG_INFO("  Version: " + std::string((const char*)version));

    // Enable basic OpenGL features
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LINE_SMOOTH);
    
    // Set default viewport
    int width, height;
    glfwGetFramebufferSize(impl->window, &width, &height);
    glViewport(0, 0, width, height);

    // Set up error callback
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
                            GLsizei length, const GLchar* message, const void* userParam) {
        std::string severityStr;
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: severityStr = "HIGH"; break;
            case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_LOW: severityStr = "LOW"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: return; // Ignore notifications
            default: severityStr = "UNKNOWN"; break;
        }
        
        std::string msg = "OpenGL Debug [" + severityStr + "]: " + message;
        if (severity == GL_DEBUG_SEVERITY_HIGH) {
            REBEL_LOG_ERROR(msg);
        } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
            REBEL_LOG_WARNING(msg);
        } else {
            REBEL_LOG_INFO(msg);
        }
    }, nullptr);

    REBEL_LOG_INFO("OpenGL initialized successfully");
}

void GraphicsSystem::initializeVulkan() {
    // Create Vulkan instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = windowProps.title.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "RebelCAD Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // TODO: Add validation layers and instance extensions

    if (vkCreateInstance(&createInfo, nullptr, &impl->vulkanInstance) != VK_SUCCESS) {
        REBEL_THROW_ERROR(GraphicsError, "Failed to create Vulkan instance");
    }

    // TODO: Implement full Vulkan initialization
    // - Select physical device
    // - Create logical device
    // - Create surface
    // - Create swapchain
    
    REBEL_LOG_INFO("Vulkan initialization placeholder");
}

} // namespace Graphics
} // namespace RebelCAD
