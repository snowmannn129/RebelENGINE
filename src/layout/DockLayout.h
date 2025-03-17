#pragma once

#include "ui/Widget.h"
#include "imgui.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace RebelCAD {
namespace UI {

/**
 * @brief Represents a docking zone within the layout
 */
enum class DockZone {
    Center,
    Left,
    Right,
    Top,
    Bottom
};

/**
 * @brief Layout manager that implements ImGui-based docking functionality
 * 
 * Handles:
 * - Docking zone management
 * - Widget placement and constraints
 * - Split ratios and resizing
 * - Layout persistence
 */
class DockLayout {
public:
    DockLayout();
    ~DockLayout();

    /**
     * @brief Add a widget to a specific docking zone
     * @param widget Widget to dock
     * @param zone Zone to dock the widget in
     * @param size Initial size (width for Left/Right, height for Top/Bottom)
     * @return ImGui dock ID for the widget
     */
    ImGuiID dockWidget(std::shared_ptr<Widget> widget, DockZone zone, float size = 200.0f);

    /**
     * @brief Remove a widget from the dock layout
     * @param widget Widget to remove
     */
    void undockWidget(std::shared_ptr<Widget> widget);

    /**
     * @brief Check if a widget is currently docked
     * @param widget Widget to check
     * @return True if widget is docked
     */
    bool isWidgetDocked(std::shared_ptr<Widget> widget) const;

    /**
     * @brief Get the current docking zone for a widget
     * @param widget Widget to check
     * @return Zone the widget is docked in, or DockZone::Center if not found
     */
    DockZone getWidgetZone(std::shared_ptr<Widget> widget) const;

    /**
     * @brief Set the split ratio for a docking zone
     * @param zone Zone to modify
     * @param ratio New split ratio (0.0 - 1.0)
     */
    void setSplitRatio(DockZone zone, float ratio);

    /**
     * @brief Get the current split ratio for a zone
     * @param zone Zone to check
     * @return Current split ratio
     */
    float getSplitRatio(DockZone zone) const;

    /**
     * @brief Update and render the dock layout
     * Must be called between ImGui::Begin() and ImGui::End()
     */
    void render();

    /**
     * @brief Save the current dock layout configuration
     * @param filename Path to save configuration
     * @return True if save successful
     */
    bool saveConfiguration(const std::string& filename);

    /**
     * @brief Load a dock layout configuration
     * @param filename Path to load configuration from
     * @return True if load successful
     */
    bool loadConfiguration(const std::string& filename);

    /**
     * @brief Reset the layout to default configuration
     */
    void resetToDefault();

private:
    // Dock node storage
    struct DockNode {
        ImGuiID id;
        DockZone zone;
        float size;
        std::shared_ptr<Widget> widget;
    };

    // Layout state
    ImGuiID m_MainDockID;
    std::unordered_map<DockZone, float> m_SplitRatios;
    std::vector<DockNode> m_DockNodes;
    bool m_Initialized;

    // Internal helpers
    void initialize();
    void setupDockspace();
    ImGuiID createDockNode(DockZone zone, float size);
    void updateSplitRatios();
    
    // Prevent copying
    DockLayout(const DockLayout&) = delete;
    DockLayout& operator=(const DockLayout&) = delete;
};

} // namespace UI
} // namespace RebelCAD
