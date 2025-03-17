#include "gtest/gtest.h"
#include "ui/layouts/DockLayout.h"
#include "ui/widgets/Button.h"
#include "ui/ImGuiWrapper.h"
#include <memory>

using namespace RebelCAD::UI;

class DockLayoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize ImGui context for testing
        ImGuiWrapper::getInstance().initialize();
        layout = std::make_unique<DockLayout>();
        
        // Create test widgets
        button1 = std::make_shared<Button>("Button1");
        button2 = std::make_shared<Button>("Button2");
        button3 = std::make_shared<Button>("Button3");
    }

    void TearDown() override {
        button1.reset();
        button2.reset();
        button3.reset();
        layout.reset();
        ImGuiWrapper::getInstance().shutdown();
    }

    std::unique_ptr<DockLayout> layout;
    std::shared_ptr<Button> button1;
    std::shared_ptr<Button> button2;
    std::shared_ptr<Button> button3;
};

TEST_F(DockLayoutTest, DockWidgetTest) {
    // Test docking a widget
    ImGuiID id = layout->dockWidget(button1, DockZone::Left);
    EXPECT_NE(id, 0);
    EXPECT_TRUE(layout->isWidgetDocked(button1));
    EXPECT_EQ(layout->getWidgetZone(button1), DockZone::Left);
}

TEST_F(DockLayoutTest, UndockWidgetTest) {
    // Dock and then undock a widget
    layout->dockWidget(button1, DockZone::Right);
    EXPECT_TRUE(layout->isWidgetDocked(button1));
    
    layout->undockWidget(button1);
    EXPECT_FALSE(layout->isWidgetDocked(button1));
    EXPECT_EQ(layout->getWidgetZone(button1), DockZone::Center); // Default zone
}

TEST_F(DockLayoutTest, MultipleWidgetDockingTest) {
    // Test docking multiple widgets in different zones
    layout->dockWidget(button1, DockZone::Left);
    layout->dockWidget(button2, DockZone::Right);
    layout->dockWidget(button3, DockZone::Bottom);

    EXPECT_TRUE(layout->isWidgetDocked(button1));
    EXPECT_TRUE(layout->isWidgetDocked(button2));
    EXPECT_TRUE(layout->isWidgetDocked(button3));

    EXPECT_EQ(layout->getWidgetZone(button1), DockZone::Left);
    EXPECT_EQ(layout->getWidgetZone(button2), DockZone::Right);
    EXPECT_EQ(layout->getWidgetZone(button3), DockZone::Bottom);
}

TEST_F(DockLayoutTest, SplitRatioTest) {
    // Test split ratio management
    float testRatio = 0.3f;
    layout->setSplitRatio(DockZone::Left, testRatio);
    EXPECT_FLOAT_EQ(layout->getSplitRatio(DockZone::Left), testRatio);

    // Test invalid ratio clamping
    layout->setSplitRatio(DockZone::Right, -0.5f);
    EXPECT_FLOAT_EQ(layout->getSplitRatio(DockZone::Right), 0.0f);

    layout->setSplitRatio(DockZone::Top, 1.5f);
    EXPECT_FLOAT_EQ(layout->getSplitRatio(DockZone::Top), 1.0f);
}

TEST_F(DockLayoutTest, ConfigurationPersistenceTest) {
    // Setup initial layout
    layout->dockWidget(button1, DockZone::Left);
    layout->dockWidget(button2, DockZone::Right);
    layout->setSplitRatio(DockZone::Left, 0.3f);
    layout->setSplitRatio(DockZone::Right, 0.7f);

    // Save configuration
    EXPECT_TRUE(layout->saveConfiguration("test_layout.json"));

    // Create new layout and load configuration
    auto newLayout = std::make_unique<DockLayout>();
    EXPECT_TRUE(newLayout->loadConfiguration("test_layout.json"));

    // Verify loaded configuration matches
    EXPECT_FLOAT_EQ(newLayout->getSplitRatio(DockZone::Left), 0.3f);
    EXPECT_FLOAT_EQ(newLayout->getSplitRatio(DockZone::Right), 0.7f);
}

TEST_F(DockLayoutTest, ResetToDefaultTest) {
    // Setup custom layout
    layout->dockWidget(button1, DockZone::Left);
    layout->dockWidget(button2, DockZone::Right);
    layout->setSplitRatio(DockZone::Left, 0.3f);

    // Reset to default
    layout->resetToDefault();

    // Verify default state
    EXPECT_FALSE(layout->isWidgetDocked(button1));
    EXPECT_FALSE(layout->isWidgetDocked(button2));
    EXPECT_FLOAT_EQ(layout->getSplitRatio(DockZone::Left), 0.2f); // Default ratio
}

TEST_F(DockLayoutTest, InvalidOperationsTest) {
    // Test docking null widget
    ImGuiID id = layout->dockWidget(nullptr, DockZone::Left);
    EXPECT_EQ(id, 0);

    // Test undocking null widget (should not crash)
    layout->undockWidget(nullptr);

    // Test querying invalid widget
    EXPECT_FALSE(layout->isWidgetDocked(nullptr));
    EXPECT_EQ(layout->getWidgetZone(nullptr), DockZone::Center);
}

TEST_F(DockLayoutTest, RedockingTest) {
    // Test redocking same widget in different zones
    ImGuiID id1 = layout->dockWidget(button1, DockZone::Left);
    EXPECT_NE(id1, 0);
    EXPECT_EQ(layout->getWidgetZone(button1), DockZone::Left);

    ImGuiID id2 = layout->dockWidget(button1, DockZone::Right);
    EXPECT_NE(id2, 0);
    EXPECT_NE(id1, id2);
    EXPECT_EQ(layout->getWidgetZone(button1), DockZone::Right);
}

TEST_F(DockLayoutTest, CustomSizeTest) {
    // Test docking with custom sizes
    layout->dockWidget(button1, DockZone::Left, 300.0f);
    layout->dockWidget(button2, DockZone::Right, 400.0f);

    // Note: Can't directly test sizes as they depend on ImGui rendering
    // Instead verify widgets are docked correctly
    EXPECT_TRUE(layout->isWidgetDocked(button1));
    EXPECT_TRUE(layout->isWidgetDocked(button2));
}
