#include <gtest/gtest.h>
#include "ui/widgets/RadioButton.h"
#include "core/EventBus.h"
#include "imgui.h"

using namespace RebelCAD::UI;

class RadioButtonTest : public ::testing::Test {
protected:
    void SetUp() override {
        group = std::make_shared<RadioGroup>("test_group");
        radioButton = std::make_shared<RadioButton>("test_radio", "Test Radio");
        radioButton->setGroup(group);
    }

    std::shared_ptr<RadioGroup> group;
    std::shared_ptr<RadioButton> radioButton;
};

// Constructor tests
TEST_F(RadioButtonTest, DefaultConstructor) {
    RadioButton rb("rb1");
    EXPECT_FALSE(rb.isSelected());
    EXPECT_EQ(rb.getLabel(), "");
    EXPECT_EQ(rb.getGroup(), nullptr);
}

TEST_F(RadioButtonTest, ConstructorWithLabel) {
    RadioButton rb("rb2", "Test");
    EXPECT_FALSE(rb.isSelected());
    EXPECT_EQ(rb.getLabel(), "Test");
}

TEST_F(RadioButtonTest, ConstructorWithState) {
    RadioButton rb("rb3", "Test", true);
    EXPECT_TRUE(rb.isSelected());
    EXPECT_EQ(rb.getLabel(), "Test");
}

// Group management tests
TEST_F(RadioButtonTest, GroupAssignment) {
    RadioButton rb("rb4");
    auto group2 = std::make_shared<RadioGroup>("group2");
    
    rb.setGroup(group2);
    EXPECT_EQ(rb.getGroup(), group2);
    EXPECT_EQ(group2->getButtonCount(), 1);
}

TEST_F(RadioButtonTest, GroupSelection) {
    auto rb1 = std::make_shared<RadioButton>("rb5", "First");
    auto rb2 = std::make_shared<RadioButton>("rb6", "Second");
    
    rb1->setGroup(group);
    rb2->setGroup(group);
    
    rb1->setSelected(true);
    EXPECT_TRUE(rb1->isSelected());
    EXPECT_FALSE(rb2->isSelected());
    
    rb2->setSelected(true);
    EXPECT_FALSE(rb1->isSelected());
    EXPECT_TRUE(rb2->isSelected());
}

TEST_F(RadioButtonTest, GroupDeselection) {
    radioButton->setSelected(true);
    EXPECT_TRUE(radioButton->isSelected());
    
    // Attempting to deselect should not work when it's the only selected button
    radioButton->setSelected(false);
    EXPECT_TRUE(radioButton->isSelected());
}

// Event handling tests
TEST_F(RadioButtonTest, SelectionChangedCallback) {
    bool callbackCalled = false;
    bool newState = false;
    
    radioButton->setOnSelectionChanged([&callbackCalled, &newState](bool state) {
        callbackCalled = true;
        newState = state;
    });
    
    radioButton->setSelected(true);
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(newState);
}

TEST_F(RadioButtonTest, KeyboardInteraction) {
    // Simulate space key press
    radioButton->onKeyDown(32, 0); // Space key
    EXPECT_TRUE(radioButton->isSelected());
    
    // Create another button in the same group
    auto rb2 = std::make_shared<RadioButton>("rb7");
    rb2->setGroup(group);
    
    // Simulate arrow key navigation
    rb2->onKeyDown(39, 0); // Right arrow
    EXPECT_FALSE(radioButton->isSelected());
    EXPECT_TRUE(rb2->isSelected());
}

// Accessibility tests
TEST_F(RadioButtonTest, LabelManagement) {
    const std::string newLabel = "New Label";
    radioButton->setLabel(newLabel);
    EXPECT_EQ(radioButton->getLabel(), newLabel);
}

// Layout tests
TEST_F(RadioButtonTest, PreferredSize) {
    ImVec2 size = radioButton->calculatePreferredSize();
    EXPECT_GT(size.x, 0);
    EXPECT_GT(size.y, 0);
}

// Style tests
TEST_F(RadioButtonTest, StyleUpdate) {
    nlohmann::json style = {
        {"radioButton", {
            {"size", 20},
            {"padding", 5},
            {"borderWidth", 1}
        }}
    };
    
    radioButton->setStyle(style);
    EXPECT_EQ(radioButton->getStyle(), style);
}

// Focus management tests
TEST_F(RadioButtonTest, FocusHandling) {
    EXPECT_TRUE(radioButton->isFocusable());
    
    radioButton->focus();
    EXPECT_TRUE(radioButton->isFocused());
    
    radioButton->blur();
    EXPECT_FALSE(radioButton->isFocused());
}

// Visibility tests
TEST_F(RadioButtonTest, VisibilityControl) {
    EXPECT_TRUE(radioButton->isVisible());
    
    radioButton->setVisible(false);
    EXPECT_FALSE(radioButton->isVisible());
    
    radioButton->setVisible(true);
    EXPECT_TRUE(radioButton->isVisible());
}

// RadioGroup specific tests
class RadioGroupTest : public ::testing::Test {
protected:
    void SetUp() override {
        group = std::make_shared<RadioGroup>("test_group");
        rb1 = std::make_shared<RadioButton>("rb1", "First");
        rb2 = std::make_shared<RadioButton>("rb2", "Second");
        rb3 = std::make_shared<RadioButton>("rb3", "Third");
    }

    std::shared_ptr<RadioGroup> group;
    std::shared_ptr<RadioButton> rb1;
    std::shared_ptr<RadioButton> rb2;
    std::shared_ptr<RadioButton> rb3;
};

TEST_F(RadioGroupTest, ButtonManagement) {
    rb1->setGroup(group);
    EXPECT_EQ(group->getButtonCount(), 1);
    
    rb2->setGroup(group);
    EXPECT_EQ(group->getButtonCount(), 2);
    
    group->removeButton(rb1.get());
    EXPECT_EQ(group->getButtonCount(), 1);
}

TEST_F(RadioGroupTest, SelectionManagement) {
    rb1->setGroup(group);
    rb2->setGroup(group);
    rb3->setGroup(group);
    
    group->selectButton(rb1.get());
    EXPECT_TRUE(rb1->isSelected());
    EXPECT_FALSE(rb2->isSelected());
    EXPECT_FALSE(rb3->isSelected());
    EXPECT_EQ(group->getSelectedButton(), rb1.get());
    
    group->selectButton(rb2.get());
    EXPECT_FALSE(rb1->isSelected());
    EXPECT_TRUE(rb2->isSelected());
    EXPECT_FALSE(rb3->isSelected());
    EXPECT_EQ(group->getSelectedButton(), rb2.get());
}

TEST_F(RadioGroupTest, RemoveSelectedButton) {
    rb1->setGroup(group);
    rb2->setGroup(group);
    
    group->selectButton(rb1.get());
    EXPECT_EQ(group->getSelectedButton(), rb1.get());
    
    group->removeButton(rb1.get());
    EXPECT_EQ(group->getSelectedButton(), nullptr);
    EXPECT_EQ(group->getButtonCount(), 1);
}
