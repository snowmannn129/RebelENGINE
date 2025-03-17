#pragma once

#include "core/EventBus.h"
#include "imgui.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace UI {

/**
 * @brief Manages UI styling and theming for RebelCAD
 * 
 * Handles:
 * - Theme management
 * - Color schemes
 * - Font management
 * - Style inheritance
 * - Dynamic styling
 */
class StyleManager {
public:
    // Style change event types
    enum class StyleChangeType {
        ThemeChanged,
        ColorChanged,
        FontChanged,
        StyleVarChanged,
        WidgetStyleChanged
    };

    // Style change event data
    struct StyleChangedEvent {
        StyleChangeType type;
        std::string target;  // Theme name, color name, font name, etc.
        nlohmann::json data; // Additional event data
    };

    static StyleManager& getInstance();

    // Delete copy constructor and assignment
    StyleManager(const StyleManager&) = delete;
    StyleManager& operator=(const StyleManager&) = delete;

    /**
     * @brief Initialize the style manager
     */
    void initialize();

    /**
     * @brief Clean up style manager resources
     */
    void shutdown();

    /**
     * @brief Apply the current theme
     */
    void applyTheme();

    // Theme management
    void loadTheme(const std::string& themeName);
    void saveTheme(const std::string& themeName);
    void setDefaultTheme(const std::string& themeName);
    std::string getCurrentTheme() const;
    std::vector<std::string> getAvailableThemes() const;

    // Color management
    void setColor(const std::string& name, const ImVec4& color);
    ImVec4 getColor(const std::string& name) const;
    void setColorScheme(const std::string& schemeName);
    std::string getCurrentColorScheme() const;

    // Font management
    void addFont(const std::string& name, const std::string& path, float size);
    void setDefaultFont(const std::string& name);
    void setFont(const std::string& name);
    ImFont* getFont(const std::string& name) const;
    std::string getCurrentFont() const;

    // Style overrides
    void setStyleVar(ImGuiStyleVar var, const ImVec2& value);
    void setStyleVar(ImGuiStyleVar var, float value);
    void resetStyleVar(ImGuiStyleVar var);
    void pushStyleVar(ImGuiStyleVar var, const ImVec2& value);
    void pushStyleVar(ImGuiStyleVar var, float value);
    void popStyleVar(int count = 1);

    // Widget-specific styling
    void setWidgetStyle(const std::string& widgetType, const nlohmann::json& style);
    nlohmann::json getWidgetStyle(const std::string& widgetType) const;
    void resetWidgetStyle(const std::string& widgetType);

private:
    StyleManager();
    ~StyleManager();

    // Theme data
    struct Theme {
        std::string name;
        std::unordered_map<std::string, ImVec4> colors;
        std::unordered_map<ImGuiStyleVar, ImVec2> styleVars2;
        std::unordered_map<ImGuiStyleVar, float> styleVars1;
        std::unordered_map<std::string, nlohmann::json> widgetStyles;
        std::string defaultFont;
        std::string colorScheme;
    };

    // Font data
    struct FontData {
        std::string path;
        float size;
        ImFont* font;
    };

    std::string m_CurrentTheme;
    std::unordered_map<std::string, Theme> m_Themes;
    std::unordered_map<std::string, FontData> m_Fonts;
    std::string m_CurrentFont;
    bool m_Initialized;

    // Event handling
    void setupEventHandlers();
    size_t m_ThemeChangedSubscription;

    // Theme loading/saving
    void loadThemeFromFile(const std::string& filename, Theme& theme);
    void saveThemeToFile(const std::string& filename, const Theme& theme);
    void applyThemeColors(const Theme& theme);
    void applyThemeStyles(const Theme& theme);

    // Font loading
    ImFont* loadFontFromFile(const std::string& path, float size);

    // Default theme setup
    void setupDefaultTheme();
    void setupDefaultFonts();
};

} // namespace UI
} // namespace RebelCAD
