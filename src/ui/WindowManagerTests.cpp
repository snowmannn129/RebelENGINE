#include <gtest/gtest.h>
#include "ui/WindowManager.h"

using namespace RebelCAD::UI;

class WindowManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Get instance of WindowManager and ensure it starts clean
        WindowManager::getInstance().destroyAllWindows();
    }

    void TearDown() override {
        WindowManager::getInstance().destroyAllWindows();
    }
};

TEST_F(WindowManagerTest, CreateWindow) {
    auto& manager = WindowManager::getInstance();
    auto window = manager.createWindow("Test Window");
    
    ASSERT_NE(window, nullptr);
    EXPECT_EQ(manager.getWindowCount(), 1);
    EXPECT_EQ(window->getTitle(), "Test Window");
    EXPECT_EQ(manager.getActiveWindow(), window);
}

TEST_F(WindowManagerTest, DestroyWindow) {
    auto& manager = WindowManager::getInstance();
    auto window = manager.createWindow();
    ASSERT_NE(window, nullptr);
    
    manager.destroyWindow(window);
    EXPECT_EQ(manager.getWindowCount(), 0);
    EXPECT_EQ(manager.getActiveWindow(), nullptr);
}

TEST_F(WindowManagerTest, MultipleWindows) {
    auto& manager = WindowManager::getInstance();
    
    auto window1 = manager.createWindow("Window 1");
    auto window2 = manager.createWindow("Window 2");
    auto window3 = manager.createWindow("Window 3");
    
    ASSERT_NE(window1, nullptr);
    ASSERT_NE(window2, nullptr);
    ASSERT_NE(window3, nullptr);
    
    EXPECT_EQ(manager.getWindowCount(), 3);
    
    // First window should be active by default
    EXPECT_EQ(manager.getActiveWindow(), window1);
    
    // Test window list
    auto windows = manager.getWindows();
    EXPECT_EQ(windows.size(), 3);
    EXPECT_TRUE(std::find(windows.begin(), windows.end(), window1) != windows.end());
    EXPECT_TRUE(std::find(windows.begin(), windows.end(), window2) != windows.end());
    EXPECT_TRUE(std::find(windows.begin(), windows.end(), window3) != windows.end());
}

TEST_F(WindowManagerTest, DestroyActiveWindow) {
    auto& manager = WindowManager::getInstance();
    
    auto window1 = manager.createWindow("Window 1");
    auto window2 = manager.createWindow("Window 2");
    
    ASSERT_NE(window1, nullptr);
    ASSERT_NE(window2, nullptr);
    EXPECT_EQ(manager.getActiveWindow(), window1);
    
    // Destroying active window should make another window active
    manager.destroyWindow(window1);
    EXPECT_EQ(manager.getWindowCount(), 1);
    EXPECT_EQ(manager.getActiveWindow(), window2);
}

TEST_F(WindowManagerTest, DestroyAllWindows) {
    auto& manager = WindowManager::getInstance();
    
    manager.createWindow("Window 1");
    manager.createWindow("Window 2");
    manager.createWindow("Window 3");
    
    EXPECT_EQ(manager.getWindowCount(), 3);
    
    manager.destroyAllWindows();
    EXPECT_EQ(manager.getWindowCount(), 0);
    EXPECT_EQ(manager.getActiveWindow(), nullptr);
}

TEST_F(WindowManagerTest, WindowClosing) {
    auto& manager = WindowManager::getInstance();
    
    auto window = manager.createWindow();
    ASSERT_NE(window, nullptr);
    
    // Simulate window closing
    window->shutdown();
    
    // Update should remove closed windows
    manager.updateAll();
    EXPECT_EQ(manager.getWindowCount(), 0);
    EXPECT_EQ(manager.getActiveWindow(), nullptr);
}
