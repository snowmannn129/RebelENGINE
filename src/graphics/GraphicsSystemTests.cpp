#include <gtest/gtest.h>
#include "graphics/GraphicsSystem.h"
#include <thread>
#include <chrono>

namespace rebel::tests {

class GraphicsSystemTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure graphics system is not initialized
        if (graphics::GraphicsSystem::getInstance().isInitialized()) {
            graphics::GraphicsSystem::getInstance().shutdown();
        }
    }

    void TearDown() override {
        // Clean up graphics system
        if (graphics::GraphicsSystem::getInstance().isInitialized()) {
            graphics::GraphicsSystem::getInstance().shutdown();
        }
    }
};

TEST_F(GraphicsSystemTests, InitializationTest) {
    auto& graphics = graphics::GraphicsSystem::getInstance();
    
    // Test OpenGL initialization
    graphics::WindowProperties openglProps("OpenGL Test", 800, 600);
    EXPECT_NO_THROW(graphics.initialize(graphics::GraphicsAPI::OpenGL, openglProps));
    EXPECT_TRUE(graphics.isInitialized());
    EXPECT_EQ(graphics.getAPI(), graphics::GraphicsAPI::OpenGL);
    graphics.shutdown();
    
    // Test Vulkan initialization
    graphics::WindowProperties vulkanProps("Vulkan Test", 800, 600);
    EXPECT_NO_THROW(graphics.initialize(graphics::GraphicsAPI::Vulkan, vulkanProps));
    EXPECT_TRUE(graphics.isInitialized());
    EXPECT_EQ(graphics.getAPI(), graphics::GraphicsAPI::Vulkan);
    graphics.shutdown();
}

TEST_F(GraphicsSystemTests, WindowPropertiesTest) {
    auto& graphics = graphics::GraphicsSystem::getInstance();
    
    graphics::WindowProperties props("Window Test", 1024, 768, true, false);
    graphics.initialize(graphics::GraphicsAPI::Vulkan, props);
    
    // Test window properties
    const auto& currentProps = graphics.getWindowProperties();
    EXPECT_EQ(currentProps.title, "Window Test");
    EXPECT_EQ(currentProps.width, 1024);
    EXPECT_EQ(currentProps.height, 768);
    EXPECT_TRUE(currentProps.vsync);
    EXPECT_FALSE(currentProps.fullscreen);
    
    // Test property modifications
    graphics.setWindowTitle("Updated Title");
    graphics.setWindowSize(800, 600);
    graphics.setVSync(false);
    
    const auto& updatedProps = graphics.getWindowProperties();
    EXPECT_EQ(updatedProps.title, "Updated Title");
    EXPECT_EQ(updatedProps.width, 800);
    EXPECT_EQ(updatedProps.height, 600);
    EXPECT_FALSE(updatedProps.vsync);
}

TEST_F(GraphicsSystemTests, RenderingLoopTest) {
    auto& graphics = graphics::GraphicsSystem::getInstance();
    graphics.initialize(graphics::GraphicsAPI::Vulkan);
    
    // Test basic rendering loop
    EXPECT_NO_THROW({
        graphics.beginFrame();
        // Simulate some rendering work
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        graphics.endFrame();
        graphics.present();
    });
}

TEST_F(GraphicsSystemTests, ErrorHandlingTest) {
    auto& graphics = graphics::GraphicsSystem::getInstance();
    
    // Test double initialization
    graphics.initialize(graphics::GraphicsAPI::Vulkan);
    EXPECT_THROW(graphics.initialize(graphics::GraphicsAPI::Vulkan), rebel::core::Error);
    
    // Test operations without initialization
    graphics.shutdown();
    EXPECT_THROW(graphics.beginFrame(), rebel::core::Error);
}

} // namespace rebel::tests
