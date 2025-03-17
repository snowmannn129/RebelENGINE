#include "ui/StyleManager.h"
#include "core/Log.h"
#include <fstream>
#include <filesystem>

namespace RebelCAD {
namespace UI {

StyleManager& StyleManager::getInstance() {
    static StyleManager instance;
    return instance;
}

StyleManager::StyleManager()
    : m_Initialized(false)
{
    setupEventHandlers();
}

StyleManager::~StyleManager() {
    if (m_Initialized) {
        shutdown();
    }
}

void StyleManager::initialize() {
    if (m_Initialized) {
        REBEL_LOG_WARNING("StyleManager already initialized");
        return;
    }

    // Set up default theme and fonts
    setupDefaultTheme();
    setupDefaultFonts();

    m_Initialized = true;
    REBEL_LOG_INFO("StyleManager initialized");
}

void StyleManager::shutdown() {
    if (!m_Initialized) return;

    // Clear fonts
    for (auto& [name, fontData] : m_Fonts) {
        fontData.font = nullptr; // ImGui owns the font memory
    }
    m_Fonts.clear();

    // Clear themes
    m_Themes.clear();

    m_Initialized = false;
    REBEL_LOG_INFO("StyleManager shut down");
}

void StyleManager::setupEventHandlers() {
    auto& eventBus = Core::EventBus::getInstance();
    
    m_ThemeChangedSubscription = eventBus.subscribe<StyleChangedEvent>(
        [this](const StyleChangedEvent& event, const Core::EventMetadata&) {
            if (event.type == StyleChangeType::ThemeChanged) {
                REBEL_LOG_INFO("Theme changed to {}", event.target);
                applyTheme();
            }
        }
    );
}

void StyleManager::setupDefaultTheme() {
    Theme defaultTheme;
    defaultTheme.name = "Default";
    
    // Default colors
    defaultTheme.colors = {
        {"Text", ImVec4(1.00f, 1.00f, 1.00f, 1.00f)},
        {"TextDisabled", ImVec4(0.50f, 0.50f, 0.50f, 1.00f)},
        {"WindowBg", ImVec4(0.06f, 0.06f, 0.06f, 0.94f)},
        {"ChildBg", ImVec4(0.00f, 0.00f, 0.00f, 0.00f)},
        {"PopupBg", ImVec4(0.08f, 0.08f, 0.08f, 0.94f)},
        {"Border", ImVec4(0.43f, 0.43f, 0.50f, 0.50f)},
        {"BorderShadow", ImVec4(0.00f, 0.00f, 0.00f, 0.00f)},
        {"FrameBg", ImVec4(0.16f, 0.29f, 0.48f, 0.54f)},
        {"FrameBgHovered", ImVec4(0.26f, 0.59f, 0.98f, 0.40f)},
        {"FrameBgActive", ImVec4(0.26f, 0.59f, 0.98f, 0.67f)},
        {"TitleBg", ImVec4(0.04f, 0.04f, 0.04f, 1.00f)},
        {"TitleBgActive", ImVec4(0.16f, 0.29f, 0.48f, 1.00f)},
        {"TitleBgCollapsed", ImVec4(0.00f, 0.00f, 0.00f, 0.51f)},
        {"MenuBarBg", ImVec4(0.14f, 0.14f, 0.14f, 1.00f)},
        {"ScrollbarBg", ImVec4(0.02f, 0.02f, 0.02f, 0.53f)},
        {"ScrollbarGrab", ImVec4(0.31f, 0.31f, 0.31f, 1.00f)},
        {"ScrollbarGrabHovered", ImVec4(0.41f, 0.41f, 0.41f, 1.00f)},
        {"ScrollbarGrabActive", ImVec4(0.51f, 0.51f, 0.51f, 1.00f)},
        {"CheckMark", ImVec4(0.26f, 0.59f, 0.98f, 1.00f)},
        {"SliderGrab", ImVec4(0.24f, 0.52f, 0.88f, 1.00f)},
        {"SliderGrabActive", ImVec4(0.26f, 0.59f, 0.98f, 1.00f)},
        {"Button", ImVec4(0.26f, 0.59f, 0.98f, 0.40f)},
        {"ButtonHovered", ImVec4(0.26f, 0.59f, 0.98f, 1.00f)},
        {"ButtonActive", ImVec4(0.06f, 0.53f, 0.98f, 1.00f)},
        {"Header", ImVec4(0.26f, 0.59f, 0.98f, 0.31f)},
        {"HeaderHovered", ImVec4(0.26f, 0.59f, 0.98f, 0.80f)},
        {"HeaderActive", ImVec4(0.26f, 0.59f, 0.98f, 1.00f)},
        {"Separator", ImVec4(0.43f, 0.43f, 0.50f, 0.50f)},
        {"SeparatorHovered", ImVec4(0.10f, 0.40f, 0.75f, 0.78f)},
        {"SeparatorActive", ImVec4(0.10f, 0.40f, 0.75f, 1.00f)},
        {"ResizeGrip", ImVec4(0.26f, 0.59f, 0.98f, 0.20f)},
        {"ResizeGripHovered", ImVec4(0.26f, 0.59f, 0.98f, 0.67f)},
        {"ResizeGripActive", ImVec4(0.26f, 0.59f, 0.98f, 0.95f)},
        {"Tab", ImVec4(0.18f, 0.35f, 0.58f, 0.86f)},
        {"TabHovered", ImVec4(0.26f, 0.59f, 0.98f, 0.80f)},
        {"TabActive", ImVec4(0.20f, 0.41f, 0.68f, 1.00f)},
        {"TabUnfocused", ImVec4(0.07f, 0.10f, 0.15f, 0.97f)},
        {"TabUnfocusedActive", ImVec4(0.14f, 0.26f, 0.42f, 1.00f)},
        {"DockingPreview", ImVec4(0.26f, 0.59f, 0.98f, 0.70f)},
        {"DockingEmptyBg", ImVec4(0.20f, 0.20f, 0.20f, 1.00f)},
        {"PlotLines", ImVec4(0.61f, 0.61f, 0.61f, 1.00f)},
        {"PlotLinesHovered", ImVec4(1.00f, 0.43f, 0.35f, 1.00f)},
        {"PlotHistogram", ImVec4(0.90f, 0.70f, 0.00f, 1.00f)},
        {"PlotHistogramHovered", ImVec4(1.00f, 0.60f, 0.00f, 1.00f)},
        {"TextSelectedBg", ImVec4(0.26f, 0.59f, 0.98f, 0.35f)},
        {"DragDropTarget", ImVec4(1.00f, 1.00f, 0.00f, 0.90f)},
        {"NavHighlight", ImVec4(0.26f, 0.59f, 0.98f, 1.00f)},
        {"NavWindowingHighlight", ImVec4(1.00f, 1.00f, 1.00f, 0.70f)},
        {"NavWindowingDimBg", ImVec4(0.80f, 0.80f, 0.80f, 0.20f)},
        {"ModalWindowDimBg", ImVec4(0.80f, 0.80f, 0.80f, 0.35f)}
    };

    // Default style variables
    defaultTheme.styleVars2 = {
        {ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f)},
        {ImGuiStyleVar_FramePadding, ImVec2(4.0f, 3.0f)},
        {ImGuiStyleVar_CellPadding, ImVec2(4.0f, 2.0f)},
        {ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 4.0f)},
        {ImGuiStyleVar_ItemInnerSpacing, ImVec2(4.0f, 4.0f)},
        {ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f)},
        {ImGuiStyleVar_SelectableTextAlign, ImVec2(0.0f, 0.0f)}
    };

    defaultTheme.styleVars1 = {
        {ImGuiStyleVar_WindowRounding, 0.0f},
        {ImGuiStyleVar_WindowBorderSize, 1.0f},
        {ImGuiStyleVar_ChildRounding, 0.0f},
        {ImGuiStyleVar_ChildBorderSize, 1.0f},
        {ImGuiStyleVar_PopupRounding, 0.0f},
        {ImGuiStyleVar_PopupBorderSize, 1.0f},
        {ImGuiStyleVar_FrameRounding, 0.0f},
        {ImGuiStyleVar_FrameBorderSize, 0.0f},
        {ImGuiStyleVar_IndentSpacing, 21.0f},
        {ImGuiStyleVar_ScrollbarSize, 14.0f},
        {ImGuiStyleVar_ScrollbarRounding, 9.0f},
        {ImGuiStyleVar_GrabMinSize, 10.0f},
        {ImGuiStyleVar_GrabRounding, 0.0f},
        {ImGuiStyleVar_TabRounding, 4.0f},
        {ImGuiStyleVar_TabBorderSize, 0.0f},
        {ImGuiStyleVar_DisabledAlpha, 0.6f}
    };

    // Default widget styles
    defaultTheme.widgetStyles = {
        {"Button", {
            {"padding", {4, 4}},
            {"rounding", 3},
            {"borderSize", 1}
        }},
        {"Input", {
            {"padding", {4, 2}},
            {"rounding", 2},
            {"borderSize", 1}
        }},
        {"Panel", {
            {"padding", {8, 8}},
            {"rounding", 0},
            {"borderSize", 1}
        }}
    };

    m_Themes["Default"] = defaultTheme;
    m_CurrentTheme = "Default";
}

void StyleManager::setupDefaultFonts() {
    // Default font is handled by ImGui
    m_Fonts["Default"] = FontData{"", 13.0f, nullptr};
    m_CurrentFont = "Default";
}

void StyleManager::loadTheme(const std::string& themeName) {
    if (!m_Initialized) {
        REBEL_LOG_ERROR("StyleManager not initialized");
        return;
    }

    std::string oldTheme = m_CurrentTheme;
    
    try {
        std::string filename = "themes/" + themeName + ".json";
        Theme theme;
        loadThemeFromFile(filename, theme);
        m_Themes[themeName] = theme;
        m_CurrentTheme = themeName;

        // Apply the theme
        applyTheme();

        // Notify theme change
        auto& eventBus = Core::EventBus::getInstance();
        StyleChangedEvent event{
            StyleChangeType::ThemeChanged,
            themeName,
            {{"oldTheme", oldTheme}}
        };
        eventBus.publish(event);
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to load theme {}: {}", themeName, e.what());
    }
}

void StyleManager::saveTheme(const std::string& themeName) {
    if (!m_Initialized) {
        REBEL_LOG_ERROR("StyleManager not initialized");
        return;
    }

    auto it = m_Themes.find(themeName);
    if (it == m_Themes.end()) {
        REBEL_LOG_ERROR("Theme {} not found", themeName);
        return;
    }

    try {
        std::string filename = "themes/" + themeName + ".json";
        saveThemeToFile(filename, it->second);
        REBEL_LOG_INFO("Theme {} saved to {}", themeName, filename);
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to save theme {}: {}", themeName, e.what());
    }
}

void StyleManager::loadThemeFromFile(const std::string& filename, Theme& theme) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open theme file: " + filename);
    }

    nlohmann::json json;
    file >> json;

    theme.name = json["name"];
    
    // Load colors
    auto colors = json["colors"];
    for (auto it = colors.begin(); it != colors.end(); ++it) {
        auto color = it.value();
        theme.colors[it.key()] = ImVec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }

    // Load style variables
    auto styleVars = json["styleVars"];
    for (auto it = styleVars.begin(); it != styleVars.end(); ++it) {
        auto var = ImGuiStyleVar_(std::stoi(it.key()));
        auto value = it.value();
        
        if (value.is_array()) {
            theme.styleVars2[var] = ImVec2(
                value[0].get<float>(),
                value[1].get<float>()
            );
        } else {
            theme.styleVars1[var] = value.get<float>();
        }
    }

    // Load widget styles
    theme.widgetStyles = json["widgetStyles"];
    
    // Load font settings
    theme.defaultFont = json["defaultFont"];
    theme.colorScheme = json["colorScheme"];
}

void StyleManager::saveThemeToFile(const std::string& filename, const Theme& theme) {
    nlohmann::json json;
    
    json["name"] = theme.name;

    // Save colors
    nlohmann::json colors;
    for (const auto& [name, color] : theme.colors) {
        colors[name] = {color.x, color.y, color.z, color.w};
    }
    json["colors"] = colors;

    // Save style variables
    nlohmann::json styleVars;
    for (const auto& [var, value] : theme.styleVars2) {
        styleVars[std::to_string(var)] = {value.x, value.y};
    }
    for (const auto& [var, value] : theme.styleVars1) {
        styleVars[std::to_string(var)] = value;
    }
    json["styleVars"] = styleVars;

    // Save widget styles
    json["widgetStyles"] = theme.widgetStyles;
    
    // Save font settings
    json["defaultFont"] = theme.defaultFont;
    json["colorScheme"] = theme.colorScheme;

    // Create themes directory if it doesn't exist
    std::filesystem::create_directories(std::filesystem::path(filename).parent_path());

    // Write to file
    std::ofstream file(filename);
    file << json.dump(4);
}

void StyleManager::applyTheme() {
    if (!m_Initialized) {
        REBEL_LOG_ERROR("StyleManager not initialized");
        return;
    }

    auto it = m_Themes.find(m_CurrentTheme);
    if (it == m_Themes.end()) {
        REBEL_LOG_ERROR("Current theme {} not found", m_CurrentTheme);
        return;
    }

    const auto& theme = it->second;

    // Apply colors
    applyThemeColors(theme);

    // Apply styles
    applyThemeStyles(theme);

    // Apply font if specified
    if (!theme.defaultFont.empty()) {
        setFont(theme.defaultFont);
    }

    REBEL_LOG_INFO("Applied theme: {}", m_CurrentTheme);
}

void StyleManager::applyThemeColors(const Theme& theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Map theme colors to ImGui colors
    for (const auto& [name, color] : theme.colors) {
        if (name == "Text") style.Colors[ImGuiCol_Text] = color;
        else if (name == "TextDisabled") style.Colors[ImGuiCol_TextDisabled] = color;
        else if (name == "WindowBg") style.Colors[ImGuiCol_WindowBg] = color;
        // ... map all other colors
    }
}

void StyleManager::applyThemeStyles(const Theme& theme) {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Apply style variables
    for (const auto& [var, value] : theme.styleVars2) {
        switch (var) {
            case ImGuiStyleVar_WindowPadding: style.WindowPadding = value; break;
            case ImGuiStyleVar_FramePadding: style.FramePadding = value; break;
            case ImGuiStyleVar_ItemSpacing: style.ItemSpacing = value; break;
            // ... handle all ImVec2 style vars
            default: break;
        }
    }

    for (const auto& [var, value] : theme.styleVars1) {
        switch (var) {
            case ImGuiStyleVar_WindowRounding: style.WindowRounding = value; break;
            case ImGuiStyleVar_FrameRounding: style.FrameRounding = value; break;
            case ImGuiStyleVar_ScrollbarSize: style.ScrollbarSize = value; break;
            // ... handle all float style vars
            default: break;
        }
    }
}

void StyleManager::setDefaultTheme(const std::string& themeName) {
    if (m_Themes.find(themeName) == m_Themes.end()) {
        REBEL_LOG_ERROR("Theme {} not found", themeName);
        return;
    }

    std::string oldTheme = m_CurrentTheme;
    m_CurrentTheme = themeName;
    applyTheme();

        // Notify theme change
        auto& eventBus = Core::EventBus::getInstance();
        StyleChangedEvent event{
            StyleChangeType::ThemeChanged,
            themeName,
            {{"oldTheme", oldTheme}}
        };
        eventBus.publish(event);
}

std::string StyleManager::getCurrentTheme() const {
    return m_CurrentTheme;
}

std::vector<std::string> StyleManager::getAvailableThemes() const {
    std::vector<std::string> themes;
    themes.reserve(m_Themes.size());
    for (const auto& [name, _] : m_Themes) {
        themes.push_back(name);
    }
    return themes;
}

void StyleManager::setColor(const std::string& name, const ImVec4& color) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.colors[name] = color;
        applyTheme();
    }
}

ImVec4 StyleManager::getColor(const std::string& name) const {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        auto colorIt = it->second.colors.find(name);
        if (colorIt != it->second.colors.end()) {
            return colorIt->second;
        }
    }
    return ImVec4(1, 1, 1, 1);
}

void StyleManager::setColorScheme(const std::string& schemeName) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.colorScheme = schemeName;
        applyTheme();
    }
}

std::string StyleManager::getCurrentColorScheme() const {
    auto it = m_Themes.find(m_CurrentTheme);
    return it != m_Themes.end() ? it->second.colorScheme : "Default";
}

void StyleManager::addFont(const std::string& name, const std::string& path, float size) {
    if (!m_Initialized) {
        REBEL_LOG_ERROR("StyleManager not initialized");
        return;
    }

    try {
        ImFont* font = loadFontFromFile(path, size);
        if (font) {
            m_Fonts[name] = FontData{path, size, font};
            REBEL_LOG_INFO("Added font: {}", name);
        }
    }
    catch (const std::exception& e) {
        REBEL_LOG_ERROR("Failed to add font {}: {}", name, e.what());
    }
}

ImFont* StyleManager::loadFontFromFile(const std::string& path, float size) {
    ImGuiIO& io = ImGui::GetIO();
    return io.Fonts->AddFontFromFileTTF(path.c_str(), size);
}

void StyleManager::setDefaultFont(const std::string& name) {
    auto it = m_Fonts.find(name);
    if (it != m_Fonts.end()) {
        auto themeIt = m_Themes.find(m_CurrentTheme);
        if (themeIt != m_Themes.end()) {
            themeIt->second.defaultFont = name;
            applyTheme();
        }
    }
}

void StyleManager::setFont(const std::string& name) {
    auto it = m_Fonts.find(name);
    if (it != m_Fonts.end()) {
        m_CurrentFont = name;
        if (it->second.font) {
            ImGui::GetIO().FontDefault = it->second.font;
        }
    }
}

ImFont* StyleManager::getFont(const std::string& name) const {
    auto it = m_Fonts.find(name);
    return it != m_Fonts.end() ? it->second.font : nullptr;
}

std::string StyleManager::getCurrentFont() const {
    return m_CurrentFont;
}

void StyleManager::setStyleVar(ImGuiStyleVar var, const ImVec2& value) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.styleVars2[var] = value;
        applyTheme();
    }
}

void StyleManager::setStyleVar(ImGuiStyleVar var, float value) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.styleVars1[var] = value;
        applyTheme();
    }
}

void StyleManager::resetStyleVar(ImGuiStyleVar var) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.styleVars2.erase(var);
        it->second.styleVars1.erase(var);
        applyTheme();
    }
}

void StyleManager::pushStyleVar(ImGuiStyleVar var, const ImVec2& value) {
    ImGui::PushStyleVar(var, value);
}

void StyleManager::pushStyleVar(ImGuiStyleVar var, float value) {
    ImGui::PushStyleVar(var, value);
}

void StyleManager::popStyleVar(int count) {
    ImGui::PopStyleVar(count);
}

void StyleManager::setWidgetStyle(const std::string& widgetType, const nlohmann::json& style) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.widgetStyles[widgetType] = style;
        applyTheme();
    }
}

nlohmann::json StyleManager::getWidgetStyle(const std::string& widgetType) const {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        auto styleIt = it->second.widgetStyles.find(widgetType);
        if (styleIt != it->second.widgetStyles.end()) {
            return styleIt->second;
        }
    }
    return nlohmann::json::object();
}

void StyleManager::resetWidgetStyle(const std::string& widgetType) {
    auto it = m_Themes.find(m_CurrentTheme);
    if (it != m_Themes.end()) {
        it->second.widgetStyles.erase(widgetType);
        applyTheme();
    }
}

} // namespace UI
} // namespace RebelCAD
