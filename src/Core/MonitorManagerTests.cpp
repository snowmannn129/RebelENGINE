#include <gtest/gtest.h>
#define TESTING
#include "ui/MonitorManager.h"
#include <GLFW/glfw3.h>

namespace RebelCAD {
namespace UI {
namespace Tests {

class MonitorManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize GLFW
        if (!glfwInit()) {
            FAIL() << "Failed to initialize GLFW";
        }
    }

    void TearDown() override {
        glfwTerminate();
    }
};

TEST_F(MonitorManagerTest, Initialization) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    // Should have at least one monitor (the primary)
    auto monitors = manager.getMonitors();
    ASSERT_GE(monitors.size(), 1);
}

TEST_F(MonitorManagerTest, PrimaryMonitorDetection) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    auto* primary = manager.getPrimaryMonitor();
    ASSERT_NE(primary, nullptr);
    EXPECT_TRUE(primary->isPrimary);
}

TEST_F(MonitorManagerTest, MonitorPositionDetection) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    // Get primary monitor info
    auto* primary = manager.getPrimaryMonitor();
    ASSERT_NE(primary, nullptr);
    
    // Test position detection at monitor center
    int centerX = primary->workAreaX + primary->workAreaWidth / 2;
    int centerY = primary->workAreaY + primary->workAreaHeight / 2;
    
    auto* detectedMonitor = manager.getMonitorAtPosition(centerX, centerY);
    ASSERT_NE(detectedMonitor, nullptr);
    EXPECT_EQ(detectedMonitor->handle, primary->handle);
}

TEST_F(MonitorManagerTest, MonitorProperties) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    auto monitors = manager.getMonitors();
    ASSERT_GE(monitors.size(), 1);
    
    const auto& monitor = monitors[0];
    
    // Basic property checks
    EXPECT_GT(monitor.width, 0);
    EXPECT_GT(monitor.height, 0);
    EXPECT_GT(monitor.refreshRate, 0);
    EXPECT_GT(monitor.dpiScale, 0.0f);
    EXPECT_FALSE(monitor.name.empty());
    
    // Work area checks
    EXPECT_GT(monitor.workAreaWidth, 0);
    EXPECT_GT(monitor.workAreaHeight, 0);
    EXPECT_LE(monitor.workAreaWidth, monitor.width);
    EXPECT_LE(monitor.workAreaHeight, monitor.height);
}

TEST_F(MonitorManagerTest, MonitorCallbacks) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    bool callbackCalled = false;
    manager.setMonitorChangeCallback([&callbackCalled]() {
        callbackCalled = true;
    });
    
    // Simulate monitor change using the test helper method
    // (since we can't physically connect/disconnect monitors in tests)
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    manager.simulateMonitorEvent(primary, GLFW_CONNECTED);
    
    EXPECT_TRUE(callbackCalled);
}

TEST_F(MonitorManagerTest, WindowPositioning) {
    auto& manager = MonitorManager::getInstance();
    manager.initialize();
    
    // Create a test window
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Make window invisible for testing
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
    ASSERT_NE(window, nullptr);
    
    auto* primary = manager.getPrimaryMonitor();
    ASSERT_NE(primary, nullptr);
    
    // Test centered positioning
    manager.moveWindowToMonitor(window, *primary, true);
    
    int posX, posY;
    glfwGetWindowPos(window, &posX, &posY);
    
    // Window should be somewhere within the monitor's work area
    EXPECT_GE(posX, primary->workAreaX);
    EXPECT_LE(posX, primary->workAreaX + primary->workAreaWidth);
    EXPECT_GE(posY, primary->workAreaY);
    EXPECT_LE(posY, primary->workAreaY + primary->workAreaHeight);
    
    glfwDestroyWindow(window);
}

} // namespace Tests
} // namespace UI
} // namespace RebelCAD
