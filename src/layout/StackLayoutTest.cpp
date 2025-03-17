#include <gtest/gtest.h>
#include <memory>
#include "../../../include/ui/layouts/StackLayout.h"
#include "../../../include/ui/widgets/Button.h"
#include "../../../include/ui/widgets/TextInput.h"

using namespace RebelCAD::UI;

class StackLayoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a default vertical stack layout
        stack = std::make_unique<StackLayout>();
    }

    std::unique_ptr<StackLayout> stack;
};

TEST_F(StackLayoutTest, ConstructorInitializesCorrectly) {
    // Test default constructor (vertical)
    StackLayout defaultStack;
    EXPECT_EQ(defaultStack.orientation(), StackLayout::Orientation::Vertical);
    EXPECT_EQ(defaultStack.spacing(), 0.0f);
    EXPECT_TRUE(defaultStack.widgets().empty());

    // Test custom orientation constructor
    StackLayout horizontalStack(StackLayout::Orientation::Horizontal);
    EXPECT_EQ(horizontalStack.orientation(), StackLayout::Orientation::Horizontal);
}

TEST_F(StackLayoutTest, AddAndRemoveWidgets) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    stack->addWidget(button1);
    EXPECT_EQ(stack->widgets().size(), 1);
    
    stack->addWidget(button2);
    EXPECT_EQ(stack->widgets().size(), 2);
    
    EXPECT_TRUE(stack->removeWidget(button1));
    EXPECT_EQ(stack->widgets().size(), 1);
    
    // Try removing non-existent widget
    EXPECT_FALSE(stack->removeWidget(button1));
}

TEST_F(StackLayoutTest, OrientationChange) {
    auto button = std::make_shared<Button>("Test");
    stack->addWidget(button);
    
    stack->setOrientation(StackLayout::Orientation::Horizontal);
    EXPECT_EQ(stack->orientation(), StackLayout::Orientation::Horizontal);
    
    // Verify layout updates with orientation change
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 300, 200));
}

TEST_F(StackLayoutTest, SpacingAdjustment) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    stack->addWidget(button1);
    stack->addWidget(button2);
    
    stack->setSpacing(15.0f);
    EXPECT_EQ(stack->spacing(), 15.0f);
    
    // Verify layout updates with spacing change
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 300, 200));
}

TEST_F(StackLayoutTest, VerticalStackLayout) {
    // Add multiple buttons to test vertical stacking
    for (int i = 0; i < 3; i++) {
        stack->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    stack->setSpacing(10.0f);
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 200, 300));
}

TEST_F(StackLayoutTest, HorizontalStackLayout) {
    stack->setOrientation(StackLayout::Orientation::Horizontal);
    
    // Add multiple buttons to test horizontal stacking
    for (int i = 0; i < 3; i++) {
        stack->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    stack->setSpacing(10.0f);
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 300, 200));
}

TEST_F(StackLayoutTest, MixedWidgetTypes) {
    // Test layout with different widget types
    auto button = std::make_shared<Button>("Button");
    auto textInput = std::make_shared<TextInput>();
    
    stack->addWidget(button);
    stack->addWidget(textInput);
    
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 400, 300));
}

TEST_F(StackLayoutTest, EmptyLayout) {
    // Verify empty layout handles updates correctly
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 300, 200));
}

TEST_F(StackLayoutTest, LayoutBoundaries) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    stack->addWidget(button1);
    stack->addWidget(button2);
    
    // Test with zero dimensions
    EXPECT_NO_THROW(stack->updateLayout(0, 0, 0, 0));
    
    // Test with negative dimensions (should handle gracefully)
    EXPECT_NO_THROW(stack->updateLayout(0, 0, -100, -100));
}

TEST_F(StackLayoutTest, WidgetOrder) {
    auto button1 = std::make_shared<Button>("First");
    auto button2 = std::make_shared<Button>("Second");
    auto button3 = std::make_shared<Button>("Third");
    
    stack->addWidget(button1);
    stack->addWidget(button2);
    stack->addWidget(button3);
    
    const auto& widgets = stack->widgets();
    EXPECT_EQ(widgets[0], button1);
    EXPECT_EQ(widgets[1], button2);
    EXPECT_EQ(widgets[2], button3);
}
