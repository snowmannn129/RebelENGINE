#include "ui/ThemeManager.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace RebelCAD {

ThemeManager& ThemeManager::getInstance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::initialize() {
    // Register default themes
    registerTheme("Light", createLightTheme());
    registerTheme("Dark", createDarkTheme());
    
    // Try to load saved preference, fallback to light theme
    if (!loadThemePreference()) {
        applyTheme("Light");
    }
}

bool ThemeManager::applyTheme(const std::string& themeName) {
    auto it = themes_.find(themeName);
    if (it == themes_.end()) {
        Log::error("Theme '{}' not found", themeName);
        return false;
    }

    const auto& theme = it->second;
    auto& style = ImGui::GetStyle();
    
    // Apply colors
    style.Colors[ImGuiCol_Text] = theme.text;
    style.Colors[ImGuiCol_TextDisabled] = theme.textDisabled;
    style.Colors[ImGuiCol_WindowBg] = theme.windowBg;
    style.Colors[ImGuiCol_ChildBg] = theme.childBg;
    style.Colors[ImGuiCol_PopupBg] = theme.popupBg;
    style.Colors[ImGuiCol_Border] = theme.border;
    style.Colors[ImGuiCol_BorderShadow] = theme.borderShadow;
    style.Colors[ImGuiCol_FrameBg] = theme.frameBg;
    style.Colors[ImGuiCol_FrameBgHovered] = theme.frameBgHovered;
    style.Colors[ImGuiCol_FrameBgActive] = theme.frameBgActive;
    style.Colors[ImGuiCol_TitleBg] = theme.titleBg;
    style.Colors[ImGuiCol_TitleBgActive] = theme.titleBgActive;
    style.Colors[ImGuiCol_TitleBgCollapsed] = theme.titleBgCollapsed;
    style.Colors[ImGuiCol_MenuBarBg] = theme.menuBarBg;
    style.Colors[ImGuiCol_ScrollbarBg] = theme.scrollbarBg;
    style.Colors[ImGuiCol_ScrollbarGrab] = theme.scrollbarGrab;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = theme.scrollbarGrabHovered;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = theme.scrollbarGrabActive;
    style.Colors[ImGuiCol_CheckMark] = theme.checkMark;
    style.Colors[ImGuiCol_SliderGrab] = theme.sliderGrab;
    style.Colors[ImGuiCol_SliderGrabActive] = theme.sliderGrabActive;
    style.Colors[ImGuiCol_Button] = theme.button;
    style.Colors[ImGuiCol_ButtonHovered] = theme.buttonHovered;
    style.Colors[ImGuiCol_ButtonActive] = theme.buttonActive;
    style.Colors[ImGuiCol_Header] = theme.header;
    style.Colors[ImGuiCol_HeaderHovered] = theme.headerHovered;
    style.Colors[ImGuiCol_HeaderActive] = theme.headerActive;
    style.Colors[ImGuiCol_Separator] = theme.separator;
    style.Colors[ImGuiCol_SeparatorHovered] = theme.separatorHovered;
    style.Colors[ImGuiCol_SeparatorActive] = theme.separatorActive;
    style.Colors[ImGuiCol_ResizeGrip] = theme.resizeGrip;
    style.Colors[ImGuiCol_ResizeGripHovered] = theme.resizeGripHovered;
    style.Colors[ImGuiCol_ResizeGripActive] = theme.resizeGripActive;
    style.Colors[ImGuiCol_PlotLines] = theme.plotLines;
    style.Colors[ImGuiCol_PlotLinesHovered] = theme.plotLinesHovered;
    style.Colors[ImGuiCol_PlotHistogram] = theme.plotHistogram;
    style.Colors[ImGuiCol_PlotHistogramHovered] = theme.plotHistogramHovered;
    style.Colors[ImGuiCol_TableHeaderBg] = theme.tableHeaderBg;
    style.Colors[ImGuiCol_TableBorderStrong] = theme.tableBorderStrong;
    style.Colors[ImGuiCol_TableBorderLight] = theme.tableBorderLight;
    style.Colors[ImGuiCol_TableRowBg] = theme.tableRowBg;
    style.Colors[ImGuiCol_TableRowBgAlt] = theme.tableRowBgAlt;
    style.Colors[ImGuiCol_TextSelectedBg] = theme.textSelectedBg;
    style.Colors[ImGuiCol_DragDropTarget] = theme.dragDropTarget;
    style.Colors[ImGuiCol_NavHighlight] = theme.navHighlight;
    style.Colors[ImGuiCol_NavWindowingHighlight] = theme.navWindowingHighlight;
    style.Colors[ImGuiCol_NavWindowingDimBg] = theme.navWindowingDimBg;
    style.Colors[ImGuiCol_ModalWindowDimBg] = theme.modalWindowDimBg;

    currentTheme_ = themeName;
    return true;
}

std::string ThemeManager::getCurrentThemeName() const {
    return currentTheme_;
}

std::vector<std::string> ThemeManager::getAvailableThemes() const {
    std::vector<std::string> themeNames;
    themeNames.reserve(themes_.size());
    for (const auto& [name, _] : themes_) {
        themeNames.push_back(name);
    }
    return themeNames;
}

bool ThemeManager::saveThemePreference() const {
    try {
        nlohmann::json j;
        j["theme"] = currentTheme_;
        
        std::ofstream file(PREFERENCE_FILE);
        if (!file.is_open()) {
            Log::error("Failed to open theme preference file for writing");
            return false;
        }
        
        file << j.dump(4);
        return true;
    }
    catch (const std::exception& e) {
        Log::error("Failed to save theme preference: {}", e.what());
        return false;
    }
}

bool ThemeManager::loadThemePreference() {
    try {
        std::ifstream file(PREFERENCE_FILE);
        if (!file.is_open()) {
            Log::info("No theme preference file found");
            return false;
        }
        
        nlohmann::json j;
        file >> j;
        
        std::string themeName = j["theme"];
        return applyTheme(themeName);
    }
    catch (const std::exception& e) {
        Log::error("Failed to load theme preference: {}", e.what());
        return false;
    }
}

void ThemeManager::registerTheme(const std::string& name, const Theme& theme) {
    themes_[name] = theme;
}

Theme ThemeManager::createLightTheme() {
    Theme theme;
    theme.name = "Light";
    
    // Light theme colors
    theme.text = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    theme.textDisabled = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    theme.windowBg = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    theme.childBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.popupBg = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    theme.border = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    theme.borderShadow = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.frameBg = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    theme.frameBgHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    theme.frameBgActive = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    theme.titleBg = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    theme.titleBgActive = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    theme.titleBgCollapsed = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    theme.menuBarBg = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    theme.scrollbarBg = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    theme.scrollbarGrab = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    theme.scrollbarGrabHovered = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    theme.scrollbarGrabActive = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    theme.checkMark = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.sliderGrab = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    theme.sliderGrabActive = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    theme.button = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    theme.buttonHovered = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.buttonActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    theme.header = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    theme.headerHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    theme.headerActive = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.separator = ImVec4(0.39f, 0.39f, 0.39f, 0.62f);
    theme.separatorHovered = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    theme.separatorActive = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    theme.resizeGrip = ImVec4(0.35f, 0.35f, 0.35f, 0.17f);
    theme.resizeGripHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    theme.resizeGripActive = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    theme.plotLines = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    theme.plotLinesHovered = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    theme.plotHistogram = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    theme.plotHistogramHovered = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    theme.tableHeaderBg = ImVec4(0.78f, 0.87f, 0.98f, 1.00f);
    theme.tableBorderStrong = ImVec4(0.57f, 0.57f, 0.64f, 1.00f);
    theme.tableBorderLight = ImVec4(0.68f, 0.68f, 0.74f, 1.00f);
    theme.tableRowBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.tableRowBgAlt = ImVec4(0.30f, 0.30f, 0.30f, 0.09f);
    theme.textSelectedBg = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    theme.dragDropTarget = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    theme.navHighlight = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    theme.navWindowingHighlight = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
    theme.navWindowingDimBg = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    theme.modalWindowDimBg = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    
    return theme;
}

Theme ThemeManager::createDarkTheme() {
    Theme theme;
    theme.name = "Dark";
    
    // Dark theme colors
    theme.text = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    theme.textDisabled = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    theme.windowBg = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    theme.childBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.popupBg = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    theme.border = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    theme.borderShadow = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.frameBg = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    theme.frameBgHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    theme.frameBgActive = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    theme.titleBg = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    theme.titleBgActive = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    theme.titleBgCollapsed = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    theme.menuBarBg = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    theme.scrollbarBg = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    theme.scrollbarGrab = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    theme.scrollbarGrabHovered = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    theme.scrollbarGrabActive = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    theme.checkMark = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.sliderGrab = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    theme.sliderGrabActive = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.button = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    theme.buttonHovered = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.buttonActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    theme.header = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    theme.headerHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    theme.headerActive = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.separator = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    theme.separatorHovered = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    theme.separatorActive = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    theme.resizeGrip = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    theme.resizeGripHovered = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    theme.resizeGripActive = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    theme.plotLines = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    theme.plotLinesHovered = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    theme.plotHistogram = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    theme.plotHistogramHovered = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    theme.tableHeaderBg = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    theme.tableBorderStrong = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    theme.tableBorderLight = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    theme.tableRowBg = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    theme.tableRowBgAlt = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    theme.textSelectedBg = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    theme.dragDropTarget = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    theme.navHighlight = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    theme.navWindowingHighlight = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    theme.navWindowingDimBg = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    theme.modalWindowDimBg = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    
    return theme;
}

} // namespace RebelCAD
