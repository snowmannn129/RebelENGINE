#pragma once

#include "ui/Panel.h"
#include "core/EventBus.h"
#include "imgui.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace RebelCAD {
namespace UI {

/**
 * @brief Manages dockable panels and layout configuration
 * 
 * Handles:
 * - Panel registration and management
 * - Docking space configuration
 * - Layout persistence
 * - Panel visibility
 */
class DockingManager {
public:
    static DockingManager& getInstance();

    // Delete copy constructor and assignment
    DockingManager(const DockingManager&) = delete;
    DockingManager& operator=(const DockingManager&) = delete;

    /**
     * @brief Register a panel with the docking system
     * @param name Unique identifier for the panel
     * @param panel Panel instance to register
     */
    void registerPanel(const std::string& name, std::shared_ptr<Panel> panel);

    /**
     * @brief Unregister a panel
     * @param name Name of panel to unregister
     */
    void unregisterPanel(const std::string& name);

    /**
     * @brief Get names of all registered panels
     * @return Vector of panel names
     */
    std::vector<std::string> getPanelNames() const;

    /**
     * @brief Check if a panel is currently visible
     * @param name Name of panel to check
     * @return True if panel is visible
     */
    bool isPanelVisible(const std::string& name) const;

    /**
     * @brief Set panel visibility
     * @param name Name of panel to modify
     * @param visible New visibility state
     */
    void showPanel(const std::string& name, bool visible);

    /**
     * @brief Render all visible panels in the docking space
     */
    void renderPanels();

    /**
     * @brief Reset panel layout to default configuration
     */
    void resetToDefaultLayout();

    /**
     * @brief Save current layout to file
     * @param filename Path to save layout
     */
    void saveLayout(const std::string& filename);

    /**
     * @brief Load layout from file
     * @param filename Path to load layout from
     */
    void loadLayout(const std::string& filename);

private:
    DockingManager();
    ~DockingManager();

    // Panel storage
    std::unordered_map<std::string, std::shared_ptr<Panel>> m_Panels;
    
    // Docking configuration
    ImGuiID m_DockspaceID;
    bool m_DockspaceInitialized;

    // Layout management
    void initializeDockspace();
    void setupDefaultLayout();
    
    // Event handling
    void setupEventHandlers();
    size_t m_LayoutChangedSubscription;

    // Event definitions
    struct LayoutChangedEvent {
        std::string source;
    };
};

} // namespace UI
} // namespace RebelCAD
