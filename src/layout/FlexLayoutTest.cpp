#include <gtest/gtest.h>
#include <memory>
#include "../../../include/ui/layouts/FlexLayout.h"
#include "../../../include/ui/widgets/Button.h"
#include "../../../include/ui/widgets/TextInput.h"

using namespace RebelCAD::UI;

class FlexLayoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        flex = std::make_unique<FlexLayout>();
    }

    std::unique_ptr<FlexLayout> flex;
};

TEST_F(FlexLayoutTest, ConstructorInitializesCorrectly) {
    FlexLayout testFlex;
    EXPECT_NO_THROW(testFlex.update(800, 600));
}

TEST_F(FlexLayoutTest, AddAndRemoveWidgets) {
    auto button = std::make_shared<Button>("Test Button");
    auto textInput = std::make_shared<TextInput>();
    
    // Test adding widgets
    EXPECT_TRUE(flex->addWidget(button));
    EXPECT_TRUE(flex->addWidget(textInput));
    
    // Test removing widgets
    EXPECT_TRUE(flex->removeWidget(button));
    EXPECT_TRUE(flex->removeWidget(textInput));
    
    // Test removing non-existent widget
    EXPECT_FALSE(flex->removeWidget(button));
}

TEST_F(FlexLayoutTest, FlexDirection) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    flex->addWidget(button1);
    flex->addWidget(button2);
    
    // Test different directions
    flex->setDirection(FlexLayout::Direction::Row);
    flex->update(800, 600);
    
    flex->setDirection(FlexLayout::Direction::Column);
    flex->update(800, 600);
    
    flex->setDirection(FlexLayout::Direction::RowReverse);
    flex->update(800, 600);
    
    flex->setDirection(FlexLayout::Direction::ColumnReverse);
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, FlexWrap) {
    // Add multiple widgets to test wrapping
    for (int i = 0; i < 5; ++i) {
        flex->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    // Test different wrap modes
    flex->setWrap(FlexLayout::Wrap::NoWrap);
    flex->update(300, 600);
    
    flex->setWrap(FlexLayout::Wrap::Wrap);
    flex->update(300, 600);
    
    flex->setWrap(FlexLayout::Wrap::WrapReverse);
    flex->update(300, 600);
}

TEST_F(FlexLayoutTest, JustifyContent) {
    for (int i = 0; i < 3; ++i) {
        flex->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    // Test all justify content modes
    flex->setJustifyContent(FlexLayout::JustifyContent::Start);
    flex->update(800, 600);
    
    flex->setJustifyContent(FlexLayout::JustifyContent::End);
    flex->update(800, 600);
    
    flex->setJustifyContent(FlexLayout::JustifyContent::Center);
    flex->update(800, 600);
    
    flex->setJustifyContent(FlexLayout::JustifyContent::SpaceBetween);
    flex->update(800, 600);
    
    flex->setJustifyContent(FlexLayout::JustifyContent::SpaceAround);
    flex->update(800, 600);
    
    flex->setJustifyContent(FlexLayout::JustifyContent::SpaceEvenly);
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, AlignItems) {
    for (int i = 0; i < 3; ++i) {
        flex->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    // Test all align items modes
    flex->setAlignItems(FlexLayout::AlignItems::Start);
    flex->update(800, 600);
    
    flex->setAlignItems(FlexLayout::AlignItems::End);
    flex->update(800, 600);
    
    flex->setAlignItems(FlexLayout::AlignItems::Center);
    flex->update(800, 600);
    
    flex->setAlignItems(FlexLayout::AlignItems::Stretch);
    flex->update(800, 600);
    
    flex->setAlignItems(FlexLayout::AlignItems::Baseline);
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, AlignContent) {
    // Add enough widgets to cause wrapping
    for (int i = 0; i < 6; ++i) {
        flex->addWidget(std::make_shared<Button>("Button " + std::to_string(i)));
    }
    
    flex->setWrap(FlexLayout::Wrap::Wrap);
    
    // Test all align content modes
    flex->setAlignContent(FlexLayout::AlignContent::Start);
    flex->update(300, 600);
    
    flex->setAlignContent(FlexLayout::AlignContent::End);
    flex->update(300, 600);
    
    flex->setAlignContent(FlexLayout::AlignContent::Center);
    flex->update(300, 600);
    
    flex->setAlignContent(FlexLayout::AlignContent::Stretch);
    flex->update(300, 600);
    
    flex->setAlignContent(FlexLayout::AlignContent::SpaceBetween);
    flex->update(300, 600);
    
    flex->setAlignContent(FlexLayout::AlignContent::SpaceAround);
    flex->update(300, 600);
}

TEST_F(FlexLayoutTest, FlexItemProperties) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    auto button3 = std::make_shared<Button>("B3");
    
    // Test different flex properties
    FlexLayout::FlexItemProperties props1;
    props1.grow = 1;
    props1.shrink = 1;
    props1.basis = 100;
    
    FlexLayout::FlexItemProperties props2;
    props2.grow = 2;
    props2.shrink = 1;
    props2.basis = 100;
    
    FlexLayout::FlexItemProperties props3;
    props3.grow = 1;
    props3.shrink = 2;
    props3.basis = 100;
    props3.alignSelf = FlexLayout::AlignItems::Center;
    
    flex->addWidget(button1, props1);
    flex->addWidget(button2, props2);
    flex->addWidget(button3, props3);
    
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, GapBetweenItems) {
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    flex->addWidget(button1);
    flex->addWidget(button2);
    
    // Test gap between items
    flex->setGap(20.0f);
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, ComplexLayout) {
    // Create a complex layout with multiple widgets and different properties
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    auto textInput = std::make_shared<TextInput>();
    
    FlexLayout::FlexItemProperties growProps;
    growProps.grow = 2;
    
    FlexLayout::FlexItemProperties centerProps;
    centerProps.alignSelf = FlexLayout::AlignItems::Center;
    
    flex->setDirection(FlexLayout::Direction::Row);
    flex->setWrap(FlexLayout::Wrap::Wrap);
    flex->setJustifyContent(FlexLayout::JustifyContent::SpaceBetween);
    flex->setAlignItems(FlexLayout::AlignItems::Stretch);
    flex->setGap(10.0f);
    
    flex->addWidget(button1, growProps);
    flex->addWidget(button2);
    flex->addWidget(textInput, centerProps);
    
    flex->update(800, 600);
}

TEST_F(FlexLayoutTest, RenderIntegration) {
    auto button = std::make_shared<Button>("Test");
    flex->addWidget(button);
    
    // Verify render doesn't throw
    EXPECT_NO_THROW(flex->render());
}
