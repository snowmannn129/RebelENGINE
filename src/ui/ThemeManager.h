#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include "imgui.h"
#include "../core/Log.h"

namespace RebelCAD {

/**
 * @brief Represents a color theme for the application
 */
struct Theme {
    std::string name;
    ImVec4 text;
    ImVec4 textDisabled;
    ImVec4 windowBg;
    ImVec4 childBg;
    ImVec4 popupBg;
    ImVec4 border;
    ImVec4 borderShadow;
    ImVec4 frameBg;
    ImVec4 frameBgHovered;
    ImVec4 frameBgActive;
    ImVec4 titleBg;
    ImVec4 titleBgActive;
    ImVec4 titleBgCollapsed;
    ImVec4 menuBarBg;
    ImVec4 scrollbarBg;
    ImVec4 scrollbarGrab;
    ImVec4 scrollbarGrabHovered;
    ImVec4 scrollbarGrabActive;
    ImVec4 checkMark;
    ImVec4 sliderGrab;
    ImVec4 sliderGrabActive;
    ImVec4 button;
    ImVec4 buttonHovered;
    ImVec4 buttonActive;
    ImVec4 header;
    ImVec4 headerHovered;
    ImVec4 headerActive;
    ImVec4 separator;
    ImVec4 separatorHovered;
    ImVec4 separatorActive;
    ImVec4 resizeGrip;
    ImVec4 resizeGripHovered;
    ImVec4 resizeGripActive;
    ImVec4 plotLines;
    ImVec4 plotLinesHovered;
    ImVec4 plotHistogram;
    ImVec4 plotHistogramHovered;
    ImVec4 tableHeaderBg;
    ImVec4 tableBorderStrong;
    ImVec4 tableBorderLight;
    ImVec4 tableRowBg;
    ImVec4 tableRowBgAlt;
    ImVec4 textSelectedBg;
    ImVec4 dragDropTarget;
    ImVec4 navHighlight;
    ImVec4 navWindowingHighlight;
    ImVec4 navWindowingDimBg;
    ImVec4 modalWindowDimBg;
};

/**
 * @brief Manages application themes and theme switching
 */
class ThemeManager {
public:
    /**
     * @brief Get the singleton instance of ThemeManager
     * @return Reference to the ThemeManager instance
     */
    static ThemeManager& getInstance();

    /**
     * @brief Initialize the theme system with default themes
     */
    void initialize();

    /**
     * @brief Apply a theme by name
     * @param themeName Name of the theme to apply
     * @return true if theme was applied successfully, false otherwise
     */
    bool applyTheme(const std::string& themeName);

    /**
     * @brief Get the current theme name
     * @return Name of the current theme
     */
    std::string getCurrentThemeName() const;

    /**
     * @brief Get all available theme names
     * @return Vector of theme names
     */
    std::vector<std::string> getAvailableThemes() const;

    /**
     * @brief Save current theme preference to disk
     * @return true if saved successfully, false otherwise
     */
    bool saveThemePreference() const;

    /**
     * @brief Load and apply saved theme preference
     * @return true if loaded and applied successfully, false otherwise
     */
    bool loadThemePreference();

protected:
    // Protected for testing purposes
    static constexpr const char* PREFERENCE_FILE = "theme_preference.json";

private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    /**
     * @brief Create and register a new theme
     * @param name Theme name
     * @param theme Theme configuration
     */
    void registerTheme(const std::string& name, const Theme& theme);

    /**
     * @brief Initialize the default light theme
     * @return Theme struct configured for light mode
     */
    Theme createLightTheme();

    /**
     * @brief Initialize the default dark theme
     * @return Theme struct configured for dark mode
     */
    Theme createDarkTheme();

    std::unordered_map<std::string, Theme> themes_;
    std::string currentTheme_;
};

} // namespace RebelCAD
