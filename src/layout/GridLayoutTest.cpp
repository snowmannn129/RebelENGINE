#include <gtest/gtest.h>
#include <memory>
#include "../../../include/ui/layouts/GridLayout.h"
#include "../../../include/ui/widgets/Button.h"
#include "../../../include/ui/widgets/TextInput.h"

using namespace RebelCAD::UI;

class GridLayoutTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a 3x3 grid for testing
        grid = std::make_unique<GridLayout>(3, 3);
    }

    std::unique_ptr<GridLayout> grid;
};

TEST_F(GridLayoutTest, ConstructorInitializesCorrectly) {
    GridLayout testGrid(2, 4);
    EXPECT_NO_THROW(testGrid.update(800, 600));
}

TEST_F(GridLayoutTest, AddWidgetBasicPlacement) {
    auto button = std::make_shared<Button>("Test Button");
    EXPECT_TRUE(grid->addWidget(button, 0, 0));
    
    // Verify invalid positions are rejected
    EXPECT_FALSE(grid->addWidget(button, 3, 3)); // Out of bounds
    EXPECT_FALSE(grid->addWidget(button, -1, 0)); // Negative index
}

TEST_F(GridLayoutTest, AddWidgetWithSpanning) {
    auto textInput = std::make_shared<TextInput>();
    
    // Add widget spanning 2x2
    EXPECT_TRUE(grid->addWidget(textInput, 0, 0, 2, 2));
    
    // Attempt to add widget in occupied space
    auto button = std::make_shared<Button>("Test");
    EXPECT_FALSE(grid->addWidget(button, 0, 1)); // Should fail - space occupied
    EXPECT_FALSE(grid->addWidget(button, 1, 0)); // Should fail - space occupied
    EXPECT_TRUE(grid->addWidget(button, 2, 2));  // Should succeed - space available
}

TEST_F(GridLayoutTest, RemoveWidget) {
    auto button = std::make_shared<Button>("Test");
    grid->addWidget(button, 1, 1);
    
    EXPECT_TRUE(grid->removeWidget(button));
    // Space should be available again
    EXPECT_TRUE(grid->addWidget(std::make_shared<Button>("New"), 1, 1));
}

TEST_F(GridLayoutTest, SpacingAndPadding) {
    grid->setSpacing(10.0f);
    grid->setPadding(5.0f);
    
    // Add widgets to test layout calculations
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    
    grid->addWidget(button1, 0, 0);
    grid->addWidget(button2, 0, 1);
    
    // Force layout update
    grid->update(300, 200);
}

TEST_F(GridLayoutTest, ComplexLayout) {
    // Create a complex layout with multiple widgets and spans
    auto button1 = std::make_shared<Button>("B1");
    auto button2 = std::make_shared<Button>("B2");
    auto textInput = std::make_shared<TextInput>();
    
    EXPECT_TRUE(grid->addWidget(button1, 0, 0));
    EXPECT_TRUE(grid->addWidget(button2, 0, 2));
    EXPECT_TRUE(grid->addWidget(textInput, 1, 0, 2, 3)); // Span bottom two rows
    
    grid->update(600, 400);
}

TEST_F(GridLayoutTest, RenderIntegration) {
    auto button = std::make_shared<Button>("Test");
    grid->addWidget(button, 1, 1);
    
    // Verify render doesn't throw
    EXPECT_NO_THROW(grid->render());
}
