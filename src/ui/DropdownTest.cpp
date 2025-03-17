#include "ui/widgets/Dropdown.h"
#include <gtest/gtest.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <vector>

namespace RebelCAD::UI {

class DropdownTest : public ::testing::Test {
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

        // Create dropdown
        dropdown = std::make_unique<Dropdown>("test_dropdown");
    }

    void TearDown() override {
        dropdown.reset();

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
    void simulateFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        dropdown->render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GLFWwindow* window;
    std::unique_ptr<Dropdown> dropdown;
};

TEST_F(DropdownTest, Construction) {
    EXPECT_EQ(dropdown->getId(), "test_dropdown");
    EXPECT_EQ(dropdown->getItems().size(), 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), -1);
    EXPECT_TRUE(dropdown->getSelectedItem().empty());
    EXPECT_TRUE(dropdown->isFocusable());
    EXPECT_TRUE(dropdown->isEnabled());
    EXPECT_TRUE(dropdown->isVisible());
}

TEST_F(DropdownTest, AddsItems) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    EXPECT_EQ(dropdown->getItems().size(), 3);
    EXPECT_EQ(dropdown->getItems(), items);
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, SelectsItemByIndex) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    dropdown->setSelectedIndex(1);
    EXPECT_EQ(dropdown->getSelectedIndex(), 1);
    EXPECT_EQ(dropdown->getSelectedItem(), "Item 2");
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, SelectsItemByValue) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    dropdown->setSelectedItem("Item 3");
    EXPECT_EQ(dropdown->getSelectedIndex(), 2);
    EXPECT_EQ(dropdown->getSelectedItem(), "Item 3");
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, HandlesInvalidIndex) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    dropdown->setSelectedIndex(5);
    EXPECT_EQ(dropdown->getSelectedIndex(), -1);
    EXPECT_TRUE(dropdown->getSelectedItem().empty());
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, HandlesInvalidItem) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    dropdown->setSelectedItem("Invalid Item");
    EXPECT_EQ(dropdown->getSelectedIndex(), -1);
    EXPECT_TRUE(dropdown->getSelectedItem().empty());
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, ClearsItems) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    dropdown->setSelectedIndex(1);
    
    dropdown->clearItems();
    EXPECT_EQ(dropdown->getItems().size(), 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), -1);
    EXPECT_TRUE(dropdown->getSelectedItem().empty());
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, SelectionChangedEvent) {
    bool eventReceived = false;
    
    dropdown->addEventListener("selectionChanged", [&](const Widget& widget) {
        eventReceived = true;
        const auto& dd = static_cast<const Dropdown&>(widget);
        EXPECT_EQ(dd.getSelectedItem(), "Item 2");
        EXPECT_EQ(dd.getSelectedIndex(), 1);
    });

    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    dropdown->setSelectedIndex(1);

    EXPECT_TRUE(eventReceived);
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, StyleApplication) {
    nlohmann::json style = {
        {"backgroundColor", {1.0f, 0.0f, 0.0f, 1.0f}},
        {"textColor", {0.0f, 1.0f, 0.0f, 1.0f}},
        {"borderColor", {0.0f, 0.0f, 1.0f, 1.0f}},
        {"rounding", 5.0f},
        {"padding", {10.0f, 5.0f}},
        {"borderSize", 2.0f}
    };
    
    dropdown->setStyle(style);
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, ExpandedState) {
    EXPECT_FALSE(dropdown->isExpanded());
    
    dropdown->setExpanded(true);
    EXPECT_TRUE(dropdown->isExpanded());
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
    
    dropdown->setExpanded(false);
    EXPECT_FALSE(dropdown->isExpanded());
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, KeyboardNavigation) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    // Simulate arrow down
    dropdown->onKeyDown(GLFW_KEY_DOWN, 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), 0);
    
    dropdown->onKeyDown(GLFW_KEY_DOWN, 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), 1);
    
    // Simulate arrow up
    dropdown->onKeyDown(GLFW_KEY_UP, 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), 0);
    
    // Simulate Home key
    dropdown->onKeyDown(GLFW_KEY_HOME, 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), 0);
    
    // Simulate End key
    dropdown->onKeyDown(GLFW_KEY_END, 0);
    EXPECT_EQ(dropdown->getSelectedIndex(), 2);
    
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, DisabledState) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    dropdown->setEnabled(false);
    EXPECT_FALSE(dropdown->isEnabled());
    
    // Selection should not change when disabled
    dropdown->setSelectedIndex(1);
    EXPECT_EQ(dropdown->getSelectedIndex(), -1);
    
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

TEST_F(DropdownTest, PreferredSizeCalculation) {
    std::vector<std::string> items = {"Item 1", "Item 2", "Item 3"};
    dropdown->setItems(items);
    
    // Set constraints
    dropdown->setMinSize(ImVec2(50, 30));
    dropdown->setMaxSize(ImVec2(200, 100));

    ImVec2 size = dropdown->getPreferredSize();
    EXPECT_GE(size.x, dropdown->getMinSize().x);
    EXPECT_LE(size.x, dropdown->getMaxSize().x);
    EXPECT_GE(size.y, dropdown->getMinSize().y);
    EXPECT_LE(size.y, dropdown->getMaxSize().y);
    
    EXPECT_NO_FATAL_FAILURE(simulateFrame());
}

} // namespace RebelCAD::UI
