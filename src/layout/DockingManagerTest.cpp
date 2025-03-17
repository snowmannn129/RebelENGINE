#include "ui/DockingManager.h"
#include "ui/Panel.h"
#include <gtest/gtest.h>
#include <memory>

using namespace RebelCAD::UI;

// Test panel implementation
class TestPanel : public Panel {
public:
    explicit TestPanel(const std::string& title) : Panel(title) {}
    void render() override {
        // Simple test content
        ImGui::Text("Test Panel Content");
    }
};

class DockingManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &DockingManager::getInstance();
    }

    void TearDown() override {
        // Clean up any registered panels
        for (const auto& name : manager->getPanelNames()) {
            manager->unregisterPanel(name);
        }
    }

    DockingManager* manager;
};

TEST_F(DockingManagerTest, RegisterPanel) {
    auto panel = std::make_shared<TestPanel>("Test Panel");
    manager->registerPanel("test", panel);
    
    auto names = manager->getPanelNames();
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "test");
}

TEST_F(DockingManagerTest, UnregisterPanel) {
    auto panel = std::make_shared<TestPanel>("Test Panel");
    manager->registerPanel("test", panel);
    manager->unregisterPanel("test");
    
    auto names = manager->getPanelNames();
    EXPECT_TRUE(names.empty());
}

TEST_F(DockingManagerTest, PanelVisibility) {
    auto panel = std::make_shared<TestPanel>("Test Panel");
    manager->registerPanel("test", panel);
    
    EXPECT_TRUE(manager->isPanelVisible("test"));
    manager->showPanel("test", false);
    EXPECT_FALSE(manager->isPanelVisible("test"));
}

TEST_F(DockingManagerTest, MultipleRegistrations) {
    auto panel1 = std::make_shared<TestPanel>("Panel 1");
    auto panel2 = std::make_shared<TestPanel>("Panel 2");
    
    manager->registerPanel("panel1", panel1);
    manager->registerPanel("panel2", panel2);
    
    auto names = manager->getPanelNames();
    EXPECT_EQ(names.size(), 2);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "panel1") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "panel2") != names.end());
}

TEST_F(DockingManagerTest, RegisterDuplicatePanel) {
    auto panel1 = std::make_shared<TestPanel>("Test Panel");
    auto panel2 = std::make_shared<TestPanel>("Test Panel");
    
    manager->registerPanel("test", panel1);
    manager->registerPanel("test", panel2); // Should be ignored
    
    auto names = manager->getPanelNames();
    EXPECT_EQ(names.size(), 1);
}

TEST_F(DockingManagerTest, UnregisterNonexistentPanel) {
    auto names = manager->getPanelNames();
    size_t initialCount = names.size();
    
    manager->unregisterPanel("nonexistent");
    
    names = manager->getPanelNames();
    EXPECT_EQ(names.size(), initialCount);
}

TEST_F(DockingManagerTest, LayoutSaveLoad) {
    auto panel = std::make_shared<TestPanel>("Test Panel");
    manager->registerPanel("test", panel);
    
    // Set initial state
    manager->showPanel("test", true);
    
    // Save layout
    manager->saveLayout("test_layout.json");
    
    // Change state
    manager->showPanel("test", false);
    EXPECT_FALSE(manager->isPanelVisible("test"));
    
    // Load layout
    manager->loadLayout("test_layout.json");
    EXPECT_TRUE(manager->isPanelVisible("test"));
}

TEST_F(DockingManagerTest, DefaultLayout) {
    // Register standard panels
    manager->registerPanel("Feature Tree", std::make_shared<TestPanel>("Feature Tree"));
    manager->registerPanel("Properties", std::make_shared<TestPanel>("Properties"));
    manager->registerPanel("Console", std::make_shared<TestPanel>("Console"));
    manager->registerPanel("Layer Manager", std::make_shared<TestPanel>("Layer Manager"));
    manager->registerPanel("Toolbox", std::make_shared<TestPanel>("Toolbox"));
    
    // Reset to default layout
    manager->resetToDefaultLayout();
    
    // Verify all panels are visible
    EXPECT_TRUE(manager->isPanelVisible("Feature Tree"));
    EXPECT_TRUE(manager->isPanelVisible("Properties"));
    EXPECT_TRUE(manager->isPanelVisible("Console"));
    EXPECT_TRUE(manager->isPanelVisible("Layer Manager"));
    EXPECT_TRUE(manager->isPanelVisible("Toolbox"));
}
