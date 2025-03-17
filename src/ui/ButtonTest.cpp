#include "ui/widgets/Button.h"
#include <gtest/gtest.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace RebelCAD::UI;

class ButtonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize GLFW
        if (!glfwInit()) {
            FAIL() << "Failed to initialize GLFW";
        }

        // Create test window
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for testing
        window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            FAIL() << "Failed to create GLFW window";
        }

        // Make context current
        glfwMakeContextCurrent(window);

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    void TearDown() override {
        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup GLFW
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    // Helper to simulate a frame
    void simulateFrame(Button& button) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        button.render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GLFWwindow* window;
};

TEST_F(ButtonTest, Construction) {
    Button button("test_button", "Click Me");
    EXPECT_EQ(button.getId(), "test_button");
    EXPECT_EQ(button.getLabel(), "Click Me");
    EXPECT_TRUE(button.isFocusable());
    EXPECT_TRUE(button.isEnabled());
    EXPECT_TRUE(button.isVisible());
}

TEST_F(ButtonTest, LabelManagement) {
    Button button("test_button", "Initial Label");
    EXPECT_EQ(button.getLabel(), "Initial Label");

    button.setLabel("New Label");
    EXPECT_EQ(button.getLabel(), "New Label");
}

TEST_F(ButtonTest, ClickHandling) {
    Button button("test_button", "Click Me");
    bool clicked = false;
    button.setOnClick([&clicked]() { clicked = true; });

    // Simulate click
    button.onMouseDown(0); // Left button
    button.onMouseUp(0);
    button.onClick();

    EXPECT_TRUE(clicked);
}

TEST_F(ButtonTest, DisabledState) {
    Button button("test_button", "Click Me");
    bool clicked = false;
    button.setOnClick([&clicked]() { clicked = true; });

    button.setEnabled(false);
    EXPECT_FALSE(button.isEnabled());

    // Click should not trigger when disabled
    button.onMouseDown(0);
    button.onMouseUp(0);
    button.onClick();

    EXPECT_FALSE(clicked);
}

TEST_F(ButtonTest, StyleApplication) {
    Button button("test_button", "Styled Button");

    nlohmann::json style = {
        {"normalColor", {0.5f, 0.5f, 0.5f, 1.0f}},
        {"hoverColor", {0.6f, 0.6f, 0.6f, 1.0f}},
        {"pressedColor", {0.4f, 0.4f, 0.4f, 1.0f}},
        {"textColor", {1.0f, 1.0f, 1.0f, 1.0f}},
        {"rounding", 5.0f},
        {"padding", {10.0f, 5.0f}},
        {"borderSize", 2.0f}
    };

    button.setStyle(style);
    simulateFrame(button);

    // Style verification would require checking ImGui state
    // which is not directly accessible in tests
    EXPECT_NO_FATAL_FAILURE(button.render());
}

TEST_F(ButtonTest, PreferredSizeCalculation) {
    Button button("test_button", "Test");
    
    // Set constraints
    button.setMinSize(ImVec2(50, 30));
    button.setMaxSize(ImVec2(200, 100));

    ImVec2 size = button.calculatePreferredSize();
    EXPECT_GE(size.x, button.getMinSize().x);
    EXPECT_LE(size.x, button.getMaxSize().x);
    EXPECT_GE(size.y, button.getMinSize().y);
    EXPECT_LE(size.y, button.getMaxSize().y);
}

TEST_F(ButtonTest, StateManagement) {
    Button button("test_button", "Test");

    // Test hover state
    button.onMouseEnter();
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));

    // Test pressed state
    button.onMouseDown(0);
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));

    // Test release state
    button.onMouseUp(0);
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));

    // Test leave state
    button.onMouseLeave();
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));
}

TEST_F(ButtonTest, VisibilityControl) {
    Button button("test_button", "Test");
    EXPECT_TRUE(button.isVisible());

    button.setVisible(false);
    EXPECT_FALSE(button.isVisible());
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));

    button.setVisible(true);
    EXPECT_TRUE(button.isVisible());
    EXPECT_NO_FATAL_FAILURE(simulateFrame(button));
}

TEST_F(ButtonTest, FocusManagement) {
    Button button("test_button", "Test");
    EXPECT_TRUE(button.isFocusable());
    EXPECT_FALSE(button.isFocused());

    button.focus();
    EXPECT_TRUE(button.isFocused());

    button.blur();
    EXPECT_FALSE(button.isFocused());
}

TEST_F(ButtonTest, PositionAndSize) {
    Button button("test_button", "Test");
    
    ImVec2 position(100, 100);
    ImVec2 size(200, 50);

    button.setPosition(position);
    button.setSize(size);

    EXPECT_EQ(button.getPosition().x, position.x);
    EXPECT_EQ(button.getPosition().y, position.y);
    EXPECT_EQ(button.getSize().x, size.x);
    EXPECT_EQ(button.getSize().y, size.y);
}
