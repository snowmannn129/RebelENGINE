#pragma once

#include "graphics/GLCommon.hpp"
#include "core/Config.hpp"
#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <numbers>
#include "core/Log.h"
#include "core/Error.h"
#include "graphics/Color.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/IPreviewRenderer.h"
#include "graphics/VulkanTypes.h"

namespace RebelCAD {
namespace Graphics {

enum class GraphicsAPI {
    OpenGL,
    Vulkan
};

struct WindowProperties {
    std::string title;
    uint32_t width;
    uint32_t height;
    bool vsync;
    bool fullscreen;

    WindowProperties(const std::string& title = "RebelCAD",
                    uint32_t width = 1280,
                    uint32_t height = 720,
                    bool vsync = true,
                    bool fullscreen = false)
        : title(title)
        , width(width)
        , height(height)
        , vsync(vsync)
        , fullscreen(fullscreen) {}
};

class GraphicsSystem : public IPreviewRenderer {
public:
    static GraphicsSystem& getInstance();

    virtual void initialize(GraphicsAPI api = GraphicsAPI::Vulkan,
                   const WindowProperties& props = WindowProperties());
    virtual void shutdown();

    virtual void beginFrame();
    virtual void endFrame();
    virtual void present();

    // Window management
    virtual void setWindowTitle(const std::string& title);
    virtual void setWindowSize(uint32_t width, uint32_t height);
    virtual void setWindowPos(int x, int y);
    virtual void setVSync(bool enabled);
    virtual void setFullscreen(bool enabled);
    virtual void* getWindow() const;
    virtual bool shouldClose() const;
    virtual int getWindowPosX() const;
    virtual int getWindowPosY() const;

    // Getters
    GraphicsAPI getAPI() const { return currentAPI; }
    bool isInitialized() const { return initialized; }
    const WindowProperties& getWindowProperties() const { return windowProps; }

    // Delete copy and move constructors/assignments
    GraphicsSystem(const GraphicsSystem&) = delete;
    GraphicsSystem& operator=(const GraphicsSystem&) = delete;
    GraphicsSystem(GraphicsSystem&&) = delete;
    GraphicsSystem& operator=(GraphicsSystem&&) = delete;

    // Line rendering methods
    virtual void drawLine(float startX, float startY, float endX, float endY);
    virtual void setLineColor(const Color& color);
    virtual void setLineThickness(float thickness);
    virtual void setLineDashPattern(const std::vector<float>& pattern);

    // Preview rendering methods from IPreviewRenderer
    virtual void beginPreview() override;
    virtual void endPreview() override;
    virtual void renderTriangleMesh(const float* vertices, size_t vertexCount) override;
    virtual void renderTriangleMeshWithAttributes(
        const float* vertices,
        const float* normals,
        const float* uvs,
        const unsigned int* indices,
        size_t vertexCount,
        size_t indexCount) override;

    // Additional rendering methods
    virtual void setRenderMode(RenderMode mode);
    virtual void setColor(const Color& color);

    virtual ~GraphicsSystem();

protected:
    GraphicsSystem(); // Protected constructor for singleton

private:
    struct GraphicsSystemImpl;
    std::unique_ptr<GraphicsSystemImpl> impl;

    GraphicsAPI currentAPI;
    WindowProperties windowProps;
    bool initialized;

    // Platform-specific initialization
    virtual void initializeOpenGL();
    virtual void initializeVulkan();
};

} // namespace Graphics
} // namespace RebelCAD
