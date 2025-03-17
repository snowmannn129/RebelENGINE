#include <gtest/gtest.h>
#include "ui/widgets/Checkbox.h"
#include "core/EventBus.h"
#include "imgui.h"

using namespace RebelCAD::UI;

class CheckboxTest : public ::testing::Test {
protected:
    void SetUp() override {
        checkbox = std::make_shared<Checkbox>("test_checkbox", "Test Checkbox");
    }

    std::shared_ptr<Checkbox> checkbox;
};

// Constructor tests
TEST_F(CheckboxTest, DefaultConstructor) {
    Checkbox cb("cb1");
    EXPECT_FALSE(cb.isChecked());
    EXPECT_EQ(cb.getLabel(), "");
}

TEST_F(CheckboxTest, ConstructorWithLabel) {
    Checkbox cb("cb2", "Test");
    EXPECT_FALSE(cb.isChecked());
    EXPECT_EQ(cb.getLabel(), "Test");
}

TEST_F(CheckboxTest, ConstructorWithState) {
    Checkbox cb("cb3", "Test", true);
    EXPECT_TRUE(cb.isChecked());
    EXPECT_EQ(cb.getLabel(), "Test");
}

// State management tests
TEST_F(CheckboxTest, SetChecked) {
    checkbox->setChecked(true);
    EXPECT_TRUE(checkbox->isChecked());
    
    checkbox->setChecked(false);
    EXPECT_FALSE(checkbox->isChecked());
}

TEST_F(CheckboxTest, ToggleState) {
    bool initialState = checkbox->isChecked();
    checkbox->onClick(); // Should toggle state
    EXPECT_NE(initialState, checkbox->isChecked());
    
    checkbox->onClick(); // Should toggle back
    EXPECT_EQ(initialState, checkbox->isChecked());
}

// Event handling tests
TEST_F(CheckboxTest, CheckedChangedCallback) {
    bool callbackCalled = false;
    bool newState = false;
    
    checkbox->setOnCheckedChanged([&callbackCalled, &newState](bool state) {
        callbackCalled = true;
        newState = state;
    });
    
    checkbox->setChecked(true);
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(newState);
}

TEST_F(CheckboxTest, KeyboardInteraction) {
    bool initialState = checkbox->isChecked();
    
    // Simulate space key press
    checkbox->onKeyDown(32, 0); // Space key
    EXPECT_NE(initialState, checkbox->isChecked());
    
    // Simulate enter key press
    checkbox->onKeyDown(13, 0); // Enter key
    EXPECT_EQ(initialState, checkbox->isChecked());
}

// Accessibility tests
TEST_F(CheckboxTest, LabelManagement) {
    const std::string newLabel = "New Label";
    checkbox->setLabel(newLabel);
    EXPECT_EQ(checkbox->getLabel(), newLabel);
}

// Layout tests
TEST_F(CheckboxTest, PreferredSize) {
    ImVec2 size = checkbox->calculatePreferredSize();
    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

// Style tests
TEST_F(CheckboxTest, StyleUpdate) {
    nlohmann::json style = {
        {"checkbox", {
            {"size", 20},
            {"padding", 5},
            {"borderWidth", 1}
        }}
    };
    
    checkbox->setStyle(style);
    EXPECT_EQ(checkbox->getStyle(), style);
}

// Focus management tests
TEST_F(CheckboxTest, FocusHandling) {
    EXPECT_TRUE(checkbox->isFocusable());
    
    checkbox->focus();
    EXPECT_TRUE(checkbox->isFocused());
    
    checkbox->blur();
    EXPECT_FALSE(checkbox->isFocused());
}

// Visibility tests
TEST_F(CheckboxTest, VisibilityControl) {
    EXPECT_TRUE(checkbox->isVisible());
    
    checkbox->setVisible(false);
    EXPECT_FALSE(checkbox->isVisible());
    
    checkbox->setVisible(true);
    EXPECT_TRUE(checkbox->isVisible());
}
