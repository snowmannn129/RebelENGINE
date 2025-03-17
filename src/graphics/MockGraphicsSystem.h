#pragma once

#include "graphics/GraphicsSystem.h"
#include <vector>
#include <memory>

namespace RebelCAD {
namespace Testing {

class MockGraphicsSystem : public Graphics::GraphicsSystem {
public:
    struct DrawCall {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> uvs;
        std::vector<unsigned int> indices;
        bool is_preview = false;
    };

    std::vector<DrawCall> draw_calls;

    void Clear() {
        draw_calls.clear();
    }

    // Mock implementations of IPreviewRenderer methods
    void beginPreview() override {}
    void endPreview() override {}

    void renderTriangleMesh(const float* vertices, size_t vertexCount) override {
        DrawCall call;
        call.vertices.assign(vertices, vertices + vertexCount * 3);
        call.is_preview = true;
        draw_calls.push_back(std::move(call));
    }

    void renderTriangleMeshWithAttributes(
        const float* vertices,
        const float* normals,
        const float* uvs,
        const unsigned int* indices,
        size_t vertexCount,
        size_t indexCount) override {
        
        DrawCall call;
        call.vertices.assign(vertices, vertices + vertexCount * 3);
        if (normals) {
            call.normals.assign(normals, normals + vertexCount * 3);
        }
        if (uvs) {
            call.uvs.assign(uvs, uvs + vertexCount * 2);
        }
        if (indices) {
            call.indices.assign(indices, indices + indexCount);
        }
        call.is_preview = true;
        draw_calls.push_back(std::move(call));
    }

    // Mock implementations of GraphicsSystem methods
    void initialize(Graphics::GraphicsAPI api = Graphics::GraphicsAPI::Vulkan,
                   const Graphics::WindowProperties& props = Graphics::WindowProperties()) override {}
    void shutdown() override {}
    void beginFrame() override {}
    void endFrame() override {}
    void present() override {}
    void setWindowTitle(const std::string& title) override {}
    void setWindowSize(uint32_t width, uint32_t height) override {}
    void setWindowPos(int x, int y) override {}
    void setVSync(bool enabled) override {}
    void setFullscreen(bool enabled) override {}
    void* getWindow() const override { return nullptr; }
    bool shouldClose() const override { return false; }
    int getWindowPosX() const override { return 0; }
    int getWindowPosY() const override { return 0; }
    void drawLine(float startX, float startY, float endX, float endY) override {}
    void setLineColor(const Graphics::Color& color) override {}
    void setLineThickness(float thickness) override {}
    void setLineDashPattern(const std::vector<float>& pattern) override {}
    void setRenderMode(Graphics::RenderMode mode) override {}
    void setColor(const Graphics::Color& color) override {}

    // Helper for verification
    bool VerifyDrawCall(
        size_t index,
        size_t expected_vertices,
        size_t expected_indices = 0,
        bool expect_normals = false,
        bool expect_uvs = false) const {
        
        if (index >= draw_calls.size()) {
            return false;
        }

        const auto& call = draw_calls[index];
        if (call.vertices.size() != expected_vertices * 3) {
            return false;
        }
        if (expected_indices > 0 && call.indices.size() != expected_indices) {
            return false;
        }
        if (expect_normals && call.normals.empty()) {
            return false;
        }
        if (expect_uvs && call.uvs.empty()) {
            return false;
        }

        return true;
    }

protected:
    void initializeOpenGL() override {}
    void initializeVulkan() override {}
};

} // namespace Testing
} // namespace RebelCAD
