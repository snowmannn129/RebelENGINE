#pragma once

#include "core/EventBus.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <memory>
#include <functional>

namespace RebelCAD {
namespace UI {

/**
 * @brief Base class for all dockable UI panels in RebelCAD
 * 
 * Provides core functionality for ImGui-based panels including:
 * - Docking support
 * - Event system integration
 * - Basic window management
 * - State persistence
 */
class Panel {
public:
    /**
     * @brief Construct a new Panel
     * @param title The panel's display title
     */
    explicit Panel(const std::string& title)
        : m_Title(title)
        , m_IsVisible(true)
        , m_IsDocked(true)
        , m_WindowFlags(ImGuiWindowFlags_None)
    {
        // Subscribe to relevant events
        auto& eventBus = Core::EventBus::getInstance();
        m_VisibilityChangedSubscription = eventBus.subscribe<PanelVisibilityChangedEvent>(
            [this](const PanelVisibilityChangedEvent& event, const Core::EventMetadata&) {
                if (event.panelTitle == m_Title) {
                    setVisible(event.isVisible);
                }
            }
        );
    }

    virtual ~Panel() {
        // Unsubscribe from events
        auto& eventBus = Core::EventBus::getInstance();
        eventBus.unsubscribe(m_VisibilityChangedSubscription);
    }

    // Delete copy constructor and assignment
    Panel(const Panel&) = delete;
    Panel& operator=(const Panel&) = delete;

    /**
     * @brief Render the panel's contents
     * 
     * Derived classes must implement this to define their specific UI
     */
    virtual void render() = 0;

    /**
     * @brief Render the panel's window and contents
     * @param dockspaceId ID of the dockspace to render in
     */
    void renderWindow(ImGuiID dockspaceId = 0) {
        if (!m_IsVisible) return;

        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        
        if (dockspaceId != 0 && m_IsDocked) {
            ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
        }

        if (ImGui::Begin(m_Title.c_str(), &m_IsVisible, m_WindowFlags)) {
            render();
        }
        ImGui::End();

        // If window was closed, notify through event system
        if (!m_IsVisible) {
            auto& eventBus = Core::EventBus::getInstance();
            eventBus.publish(PanelVisibilityChangedEvent{m_Title, false});
        }
    }

    /**
     * @brief Set the panel's visibility
     * @param visible New visibility state
     */
    virtual void setVisible(bool visible) {
        m_IsVisible = visible;
    }

    /**
     * @brief Set whether the panel should be docked
     * @param docked True to dock, false to float
     */
    virtual void setDocked(bool docked) {
        m_IsDocked = docked;
    }

    /**
     * @brief Set the panel's window flags
     * @param flags ImGui window flags
     */
    void setWindowFlags(ImGuiWindowFlags flags) {
        m_WindowFlags = flags;
    }

    /**
     * @brief Add window flags to the current set
     * @param flags ImGui window flags to add
     */
    void addWindowFlags(ImGuiWindowFlags flags) {
        m_WindowFlags |= flags;
    }

    /**
     * @brief Remove window flags from the current set
     * @param flags ImGui window flags to remove
     */
    void removeWindowFlags(ImGuiWindowFlags flags) {
        m_WindowFlags &= ~flags;
    }

    // Accessors
    const std::string& getTitle() const { return m_Title; }
    bool isVisible() const { return m_IsVisible; }
    bool isDocked() const { return m_IsDocked; }
    ImGuiWindowFlags getWindowFlags() const { return m_WindowFlags; }

protected:
    std::string m_Title;
    bool m_IsVisible;
    bool m_IsDocked;
    ImGuiWindowFlags m_WindowFlags;
    size_t m_VisibilityChangedSubscription;

    // Event definitions
    struct PanelVisibilityChangedEvent {
        std::string panelTitle;
        bool isVisible;
    };
};

} // namespace UI
} // namespace RebelCAD
