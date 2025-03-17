#include "ui/StyleManager.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace RebelCAD::UI;

class StyleManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = &StyleManager::getInstance();
        manager->initialize();

        // Create test themes directory
        std::filesystem::create_directories("themes");
    }

    void TearDown() override {
        manager->shutdown();

        // Clean up test themes
        std::filesystem::remove_all("themes");
    }

    // Helper to create a test theme file
    void createTestTheme(const std::string& name) {
        nlohmann::json theme;
        theme["name"] = name;
        
        // Test colors
        theme["colors"] = {
            {"Text", {1.0f, 1.0f, 1.0f, 1.0f}},
            {"WindowBg", {0.1f, 0.1f, 0.1f, 1.0f}}
        };

        // Test style variables
        theme["styleVars"] = {
            {std::to_string(ImGuiStyleVar_WindowPadding), {8.0f, 8.0f}},
            {std::to_string(ImGuiStyleVar_WindowRounding), 5.0f}
        };

        // Test widget styles
        theme["widgetStyles"] = {
            {"Button", {
                {"padding", {4, 4}},
                {"rounding", 3}
            }}
        };

        // Test font settings
        theme["defaultFont"] = "Default";
        theme["colorScheme"] = "Dark";

        // Write to file
        std::ofstream file("themes/" + name + ".json");
        file << theme.dump(4);
    }

    StyleManager* manager;
};

TEST_F(StyleManagerTest, DefaultTheme) {
    // Default theme should be set after initialization
    EXPECT_EQ(manager->getCurrentTheme(), "Default");
    EXPECT_FALSE(manager->getAvailableThemes().empty());
}

TEST_F(StyleManagerTest, LoadTheme) {
    createTestTheme("TestTheme");
    
    manager->loadTheme("TestTheme");
    EXPECT_EQ(manager->getCurrentTheme(), "TestTheme");
    
    // Verify color loaded correctly
    ImVec4 textColor = manager->getColor("Text");
    EXPECT_FLOAT_EQ(textColor.x, 1.0f);
    EXPECT_FLOAT_EQ(textColor.y, 1.0f);
    EXPECT_FLOAT_EQ(textColor.z, 1.0f);
    EXPECT_FLOAT_EQ(textColor.w, 1.0f);
}

TEST_F(StyleManagerTest, SaveTheme) {
    // Set some test values
    manager->setColor("TestColor", ImVec4(1, 0, 0, 1));
    manager->setStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    
    // Save current theme
    manager->saveTheme("SavedTheme");
    
    // Load it back and verify
    manager->loadTheme("SavedTheme");
    ImVec4 color = manager->getColor("TestColor");
    EXPECT_FLOAT_EQ(color.x, 1.0f);
    EXPECT_FLOAT_EQ(color.y, 0.0f);
    EXPECT_FLOAT_EQ(color.z, 0.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(StyleManagerTest, ColorManagement) {
    ImVec4 testColor(0.5f, 0.6f, 0.7f, 0.8f);
    manager->setColor("TestColor", testColor);
    
    ImVec4 retrievedColor = manager->getColor("TestColor");
    EXPECT_FLOAT_EQ(retrievedColor.x, testColor.x);
    EXPECT_FLOAT_EQ(retrievedColor.y, testColor.y);
    EXPECT_FLOAT_EQ(retrievedColor.z, testColor.z);
    EXPECT_FLOAT_EQ(retrievedColor.w, testColor.w);
}

TEST_F(StyleManagerTest, ColorScheme) {
    manager->setColorScheme("Dark");
    EXPECT_EQ(manager->getCurrentColorScheme(), "Dark");
}

TEST_F(StyleManagerTest, FontManagement) {
    // Default font should be available
    EXPECT_EQ(manager->getCurrentFont(), "Default");
    EXPECT_NE(manager->getFont("Default"), nullptr);

    // Test adding a new font
    manager->addFont("TestFont", "fonts/test.ttf", 16.0f);
    manager->setFont("TestFont");
    EXPECT_EQ(manager->getCurrentFont(), "TestFont");
}

TEST_F(StyleManagerTest, StyleVariables) {
    // Test ImVec2 style var
    ImVec2 padding(10.0f, 10.0f);
    manager->setStyleVar(ImGuiStyleVar_WindowPadding, padding);
    
    // Test float style var
    float rounding = 5.0f;
    manager->setStyleVar(ImGuiStyleVar_WindowRounding, rounding);
    
    // Test style var stack
    manager->pushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
    manager->popStyleVar();
}

TEST_F(StyleManagerTest, WidgetStyling) {
    nlohmann::json buttonStyle = {
        {"padding", {4, 4}},
        {"rounding", 3},
        {"borderSize", 1}
    };
    
    manager->setWidgetStyle("Button", buttonStyle);
    
    nlohmann::json retrievedStyle = manager->getWidgetStyle("Button");
    EXPECT_EQ(retrievedStyle, buttonStyle);
}

TEST_F(StyleManagerTest, ThemeInheritance) {
    createTestTheme("BaseTheme");
    manager->loadTheme("BaseTheme");
    
    // Modify some values
    manager->setColor("CustomColor", ImVec4(1, 0, 0, 1));
    manager->setStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    
    // Save as new theme
    manager->saveTheme("InheritedTheme");
    
    // Load inherited theme and verify values
    manager->loadTheme("InheritedTheme");
    ImVec4 color = manager->getColor("CustomColor");
    EXPECT_FLOAT_EQ(color.x, 1.0f);
    EXPECT_FLOAT_EQ(color.y, 0.0f);
    EXPECT_FLOAT_EQ(color.z, 0.0f);
    EXPECT_FLOAT_EQ(color.w, 1.0f);
}

TEST_F(StyleManagerTest, NonexistentTheme) {
    // Loading non-existent theme should keep current theme
    std::string currentTheme = manager->getCurrentTheme();
    manager->loadTheme("NonexistentTheme");
    EXPECT_EQ(manager->getCurrentTheme(), currentTheme);
}

TEST_F(StyleManagerTest, ResetWidgetStyle) {
    nlohmann::json style = {{"padding", {4, 4}}};
    manager->setWidgetStyle("TestWidget", style);
    manager->resetWidgetStyle("TestWidget");
    
    nlohmann::json emptyStyle = manager->getWidgetStyle("TestWidget");
    EXPECT_TRUE(emptyStyle.empty());
}

TEST_F(StyleManagerTest, MultipleThemes) {
    createTestTheme("Theme1");
    createTestTheme("Theme2");
    
    manager->loadTheme("Theme1");
    EXPECT_EQ(manager->getCurrentTheme(), "Theme1");
    
    manager->loadTheme("Theme2");
    EXPECT_EQ(manager->getCurrentTheme(), "Theme2");
    
    std::vector<std::string> themes = manager->getAvailableThemes();
    EXPECT_GE(themes.size(), 3); // Default + 2 test themes
}
