#include "ui/ContextMenuManager.h"
#include <gtest/gtest.h>
#include <memory>

namespace RebelCAD {
namespace UI {
namespace Tests {

class ContextMenuManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Get instance and clear any existing menus
        auto& manager = ContextMenuManager::getInstance();
        manager.clearContextMenus();
    }

    void TearDown() override {
        auto& manager = ContextMenuManager::getInstance();
        manager.clearContextMenus();
    }
};

TEST_F(ContextMenuManagerTests, RegisterContextMenu) {
    auto& manager = ContextMenuManager::getInstance();
    
    bool callbackCalled = false;
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Test Item", [&callbackCalled]() { callbackCalled = true; })
    };
    
    // Register menu
    manager.registerContextMenu("test", items);
    
    // Verify menu was registered
    EXPECT_TRUE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, ClearContextMenus) {
    auto& manager = ContextMenuManager::getInstance();
    
    // Register a menu
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Test Item", []() {})
    };
    manager.registerContextMenu("test", items);
    
    // Clear menus
    manager.clearContextMenus();
    
    // Verify menu was cleared
    EXPECT_FALSE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, MultipleContextMenus) {
    auto& manager = ContextMenuManager::getInstance();
    
    // Register multiple menus
    std::vector<ContextMenuItem> items1 = {
        ContextMenuItem("Test Item 1", []() {})
    };
    std::vector<ContextMenuItem> items2 = {
        ContextMenuItem("Test Item 2", []() {})
    };
    
    manager.registerContextMenu("test1", items1);
    manager.registerContextMenu("test2", items2);
    
    // Verify both menus were registered
    EXPECT_TRUE(manager.hasContextMenu("test1"));
    EXPECT_TRUE(manager.hasContextMenu("test2"));
}

TEST_F(ContextMenuManagerTests, NonExistentMenu) {
    auto& manager = ContextMenuManager::getInstance();
    
    // Verify non-existent menu returns false
    EXPECT_FALSE(manager.hasContextMenu("nonexistent"));
}

TEST_F(ContextMenuManagerTests, MenuWithSeparator) {
    auto& manager = ContextMenuManager::getInstance();
    
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Item 1", []() {}),
        ContextMenuItem::Separator(),
        ContextMenuItem("Item 2", []() {})
    };
    
    // Register menu with separator
    manager.registerContextMenu("test", items);
    
    // Verify menu was registered
    EXPECT_TRUE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, DisabledMenuItem) {
    auto& manager = ContextMenuManager::getInstance();
    
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Disabled Item", []() {}, false)
    };
    
    // Register menu with disabled item
    manager.registerContextMenu("test", items);
    
    // Verify menu was registered
    EXPECT_TRUE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, SubMenuItems) {
    auto& manager = ContextMenuManager::getInstance();
    
    ContextMenuItem subItem("Sub Item", []() {});
    ContextMenuItem mainItem("Main Item", []() {});
    mainItem.subItems.push_back(subItem);
    
    std::vector<ContextMenuItem> items = { mainItem };
    
    // Register menu with sub-items
    manager.registerContextMenu("test", items);
    
    // Verify menu was registered
    EXPECT_TRUE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, MenuWithShortcuts) {
    auto& manager = ContextMenuManager::getInstance();
    
    bool callbackCalled = false;
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Copy", [&callbackCalled]() { callbackCalled = true; }, "Ctrl+C")
    };
    
    // Register menu with shortcut
    manager.registerContextMenu("test", items);
    
    // Verify menu was registered
    EXPECT_TRUE(manager.hasContextMenu("test"));
}

TEST_F(ContextMenuManagerTests, PositionedMenu) {
    auto& manager = ContextMenuManager::getInstance();
    
    std::vector<ContextMenuItem> items = {
        ContextMenuItem("Test Item", []() {})
    };
    
    // Register menu
    manager.registerContextMenu("test", items);
    
    // Create menu position
    MenuPosition pos(100.0f, 100.0f);
    
    // Verify position is valid
    EXPECT_TRUE(pos.isValid());
    EXPECT_EQ(pos.x, 100.0f);
    EXPECT_EQ(pos.y, 100.0f);
}

TEST_F(ContextMenuManagerTests, InvalidPosition) {
    auto& manager = ContextMenuManager::getInstance();
    
    // Create invalid position
    MenuPosition pos;
    
    // Verify position is invalid
    EXPECT_FALSE(pos.isValid());
    EXPECT_EQ(pos.x, -1.0f);
    EXPECT_EQ(pos.y, -1.0f);
}

} // namespace Tests
} // namespace UI
} // namespace RebelCAD
