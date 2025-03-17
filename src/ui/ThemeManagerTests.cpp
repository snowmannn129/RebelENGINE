#include <gtest/gtest.h>
#include "ui/ThemeManager.h"
#include <filesystem>
#include <fstream>

namespace RebelCAD {
namespace Tests {

class ThemeManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing preference file
        if (std::filesystem::exists(ThemeManager::PREFERENCE_FILE)) {
            std::filesystem::remove(ThemeManager::PREFERENCE_FILE);
        }
        
        themeManager = &ThemeManager::getInstance();
        themeManager->initialize();
    }

    void TearDown() override {
        // Clean up preference file after tests
        if (std::filesystem::exists(ThemeManager::PREFERENCE_FILE)) {
            std::filesystem::remove(ThemeManager::PREFERENCE_FILE);
        }
    }

    ThemeManager* themeManager;
};

TEST_F(ThemeManagerTest, InitializationCreatesDefaultThemes) {
    auto themes = themeManager->getAvailableThemes();
    ASSERT_EQ(themes.size(), 2);
    EXPECT_TRUE(std::find(themes.begin(), themes.end(), "Light") != themes.end());
    EXPECT_TRUE(std::find(themes.begin(), themes.end(), "Dark") != themes.end());
}

TEST_F(ThemeManagerTest, ThemeSwitching) {
    EXPECT_TRUE(themeManager->applyTheme("Dark"));
    EXPECT_EQ(themeManager->getCurrentThemeName(), "Dark");
    
    EXPECT_TRUE(themeManager->applyTheme("Light"));
    EXPECT_EQ(themeManager->getCurrentThemeName(), "Light");
    
    EXPECT_FALSE(themeManager->applyTheme("NonexistentTheme"));
}

TEST_F(ThemeManagerTest, ThemePreferencePersistence) {
    // Set theme and save preference
    EXPECT_TRUE(themeManager->applyTheme("Dark"));
    EXPECT_TRUE(themeManager->saveThemePreference());
    
    // Create new instance and load preference
    ThemeManager& newInstance = ThemeManager::getInstance();
    EXPECT_TRUE(newInstance.loadThemePreference());
    EXPECT_EQ(newInstance.getCurrentThemeName(), "Dark");
}

TEST_F(ThemeManagerTest, ColorApplication) {
    // Apply dark theme and verify some key colors
    EXPECT_TRUE(themeManager->applyTheme("Dark"));
    auto& style = ImGui::GetStyle();
    
    // Check a few representative colors
    auto darkThemeText = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    auto darkThemeWindowBg = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_Text].x, darkThemeText.x);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_Text].y, darkThemeText.y);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_Text].z, darkThemeText.z);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_Text].w, darkThemeText.w);
    
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_WindowBg].x, darkThemeWindowBg.x);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_WindowBg].y, darkThemeWindowBg.y);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_WindowBg].z, darkThemeWindowBg.z);
    EXPECT_FLOAT_EQ(style.Colors[ImGuiCol_WindowBg].w, darkThemeWindowBg.w);
}

} // namespace Tests
} // namespace RebelCAD
