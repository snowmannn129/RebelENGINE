#include "ui/widgets/TextInput.h"
#include <gtest/gtest.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace RebelCAD::UI;

class TextInputTest : public ::testing::Test {
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
    void simulateFrame(TextInput& input) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        input.render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GLFWwindow* window;
};

TEST_F(TextInputTest, Construction) {
    TextInput input("test_input", "Enter text...");
    EXPECT_EQ(input.getId(), "test_input");
    EXPECT_EQ(input.getPlaceholder(), "Enter text...");
    EXPECT_TRUE(input.getText().empty());
    EXPECT_TRUE(input.isFocusable());
    EXPECT_TRUE(input.isEnabled());
    EXPECT_TRUE(input.isVisible());
    EXPECT_FALSE(input.isPasswordMode());
}

TEST_F(TextInputTest, TextManagement) {
    TextInput input("test_input");
    
    input.setText("Hello World");
    EXPECT_EQ(input.getText(), "Hello World");
    EXPECT_EQ(input.getCursorPosition(), 11);

    input.setText("");
    EXPECT_TRUE(input.getText().empty());
    EXPECT_EQ(input.getCursorPosition(), 0);
}

TEST_F(TextInputTest, PlaceholderText) {
    TextInput input("test_input", "Enter text...");
    EXPECT_EQ(input.getPlaceholder(), "Enter text...");

    input.setPlaceholder("New placeholder");
    EXPECT_EQ(input.getPlaceholder(), "New placeholder");

    // Verify display text shows placeholder when empty and not focused
    EXPECT_EQ(input.getDisplayText(), "New placeholder");

    // Set some text
    input.setText("Hello");
    EXPECT_EQ(input.getDisplayText(), "Hello");
}

TEST_F(TextInputTest, PasswordMode) {
    TextInput input("test_input");
    input.setText("secret");
    
    EXPECT_FALSE(input.isPasswordMode());
    EXPECT_EQ(input.getDisplayText(), "secret");

    input.setPasswordMode(true);
    EXPECT_TRUE(input.isPasswordMode());
    EXPECT_EQ(input.getDisplayText(), "******");
}

TEST_F(TextInputTest, SelectionManagement) {
    TextInput input("test_input");
    input.setText("Hello World");

    // Test selectAll
    input.selectAll();
    size_t start, end;
    input.getSelection(start, end);
    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(input.hasSelection());

    // Test setSelection
    input.setSelection(0, 5);
    input.getSelection(start, end);
    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 5);
    EXPECT_TRUE(input.hasSelection());
    EXPECT_EQ(input.getSelectedText(), "Hello");

    // Test clearSelection
    input.clearSelection();
    input.getSelection(start, end);
    EXPECT_EQ(start, end);
    EXPECT_FALSE(input.hasSelection());
}

TEST_F(TextInputTest, CursorManagement) {
    TextInput input("test_input");
    input.setText("Hello");

    input.setCursorPosition(2);
    EXPECT_EQ(input.getCursorPosition(), 2);

    // Test bounds
    input.setCursorPosition(10);
    EXPECT_EQ(input.getCursorPosition(), 5); // Should clamp to text length
}

TEST_F(TextInputTest, ValidationCallback) {
    TextInput input("test_input");
    bool validatorCalled = false;

    input.setValidator([&validatorCalled](const std::string& text) {
        validatorCalled = true;
        return text.length() <= 5;
    });

    // Test valid input
    input.setText("Hello");
    EXPECT_TRUE(validatorCalled);
    EXPECT_EQ(input.getText(), "Hello");

    validatorCalled = false;

    // Test invalid input
    input.setText("Too long text");
    EXPECT_TRUE(validatorCalled);
    EXPECT_EQ(input.getText(), "Hello"); // Should keep old text
}

TEST_F(TextInputTest, TextChangedCallback) {
    TextInput input("test_input");
    std::string lastText;
    bool callbackCalled = false;

    input.setOnTextChanged([&](const std::string& text) {
        lastText = text;
        callbackCalled = true;
    });

    input.setText("Hello");
    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(lastText, "Hello");
}

TEST_F(TextInputTest, SubmitCallback) {
    TextInput input("test_input");
    bool submitCalled = false;
    std::string submittedText;

    input.setOnSubmit([&](const std::string& text) {
        submitCalled = true;
        submittedText = text;
    });

    input.setText("Hello");
    input.onKeyDown(GLFW_KEY_ENTER, 0);
    EXPECT_TRUE(submitCalled);
    EXPECT_EQ(submittedText, "Hello");
}

TEST_F(TextInputTest, KeyboardNavigation) {
    TextInput input("test_input");
    input.setText("Hello World");

    // Test left/right movement
    input.setCursorPosition(5);
    input.onKeyDown(GLFW_KEY_LEFT, 0);
    EXPECT_EQ(input.getCursorPosition(), 4);

    input.onKeyDown(GLFW_KEY_RIGHT, 0);
    EXPECT_EQ(input.getCursorPosition(), 5);

    // Test word movement
    input.setCursorPosition(5);
    input.onKeyDown(GLFW_KEY_LEFT, GLFW_MOD_CONTROL);
    EXPECT_EQ(input.getCursorPosition(), 0);

    input.onKeyDown(GLFW_KEY_RIGHT, GLFW_MOD_CONTROL);
    EXPECT_EQ(input.getCursorPosition(), 5);

    // Test home/end
    input.onKeyDown(GLFW_KEY_HOME, 0);
    EXPECT_EQ(input.getCursorPosition(), 0);

    input.onKeyDown(GLFW_KEY_END, 0);
    EXPECT_EQ(input.getCursorPosition(), 11);
}

TEST_F(TextInputTest, StyleApplication) {
    TextInput input("test_input");

    nlohmann::json style = {
        {"textColor", {0.0f, 0.0f, 0.0f, 1.0f}},
        {"placeholderColor", {0.5f, 0.5f, 0.5f, 1.0f}},
        {"backgroundColor", {1.0f, 1.0f, 1.0f, 1.0f}},
        {"borderColor", {0.7f, 0.7f, 0.7f, 1.0f}},
        {"rounding", 5.0f},
        {"padding", {10.0f, 5.0f}},
        {"borderSize", 2.0f}
    };

    input.setStyle(style);
    simulateFrame(input);

    // Style verification would require checking ImGui state
    // which is not directly accessible in tests
    EXPECT_NO_FATAL_FAILURE(input.render());
}

TEST_F(TextInputTest, FocusManagement) {
    TextInput input("test_input");
    EXPECT_TRUE(input.isFocusable());
    EXPECT_FALSE(input.isFocused());

    input.focus();
    EXPECT_TRUE(input.isFocused());

    input.blur();
    EXPECT_FALSE(input.isFocused());
}

TEST_F(TextInputTest, VisibilityControl) {
    TextInput input("test_input");
    EXPECT_TRUE(input.isVisible());

    input.setVisible(false);
    EXPECT_FALSE(input.isVisible());
    EXPECT_NO_FATAL_FAILURE(simulateFrame(input));

    input.setVisible(true);
    EXPECT_TRUE(input.isVisible());
    EXPECT_NO_FATAL_FAILURE(simulateFrame(input));
}

TEST_F(TextInputTest, EnabledState) {
    TextInput input("test_input");
    EXPECT_TRUE(input.isEnabled());

    input.setEnabled(false);
    EXPECT_FALSE(input.isEnabled());

    // Test that disabled input doesn't accept text
    input.setText("Initial");
    input.handleTextInput("Test");
    EXPECT_EQ(input.getText(), "Initial");
}
