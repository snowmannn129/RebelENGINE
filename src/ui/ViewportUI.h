#pragma once

#include <memory>
#include <functional>
#include "graphics/ViewportManager.h"
#include "graphics/ViewportTypes.h"

namespace RebelCAD {
namespace UI {

/**
 * @brief Widget for viewport manipulation controls
 * 
 * Provides UI controls for:
 * - View rotation (orbit, pan, zoom)
 * - Standard view selection
 * - Projection type switching
 */
class ViewManipulationWidget {
public:
    ViewManipulationWidget(Graphics::ViewportManager* manager);
    
    /**
     * @brief Render the widget for a specific viewport
     * @param viewport Target viewport
     */
    void render(Graphics::Viewport* viewport);

    /**
     * @brief Set widget visibility
     * @param visible Show/hide widget
     */
    void setVisible(bool visible);

    /**
     * @brief Check if widget is visible
     */
    bool isVisible() const;

private:
    Graphics::ViewportManager* m_manager;
    bool m_visible;
    
    // UI state
    bool m_isDragging;
    float m_orbitSpeed;
    float m_panSpeed;
    float m_zoomSpeed;

    // Helper methods
    void renderOrbitControls();
    void renderViewButtons();
    void renderProjectionToggle();
    void handleOrbitDrag();
    void handlePanDrag();
    void handleZoom();
};

/**
 * @brief Panel for configuring viewport layouts
 * 
 * Provides UI for:
 * - Layout preset selection
 * - Custom layout creation
 * - Grid configuration
 * - Layout saving
 */
class LayoutConfigPanel {
public:
    LayoutConfigPanel(Graphics::ViewportManager* manager);
    
    /**
     * @brief Render the configuration panel
     */
    void render();

    /**
     * @brief Set panel visibility
     */
    void setVisible(bool visible);

    /**
     * @brief Check if panel is visible
     */
    bool isVisible() const;

private:
    Graphics::ViewportManager* m_manager;
    bool m_visible;
    
    // UI state
    int m_gridRows;
    int m_gridCols;
    std::string m_newPresetName;

    // Helper methods
    void renderPresetSelector();
    void renderGridConfig();
    void renderSavePreset();
    void applyGridConfig();
};

/**
 * @brief Widget for selecting and managing camera presets
 */
class CameraPresetSelector {
public:
    CameraPresetSelector(Graphics::ViewportManager* manager);
    
    /**
     * @brief Render the preset selector for a viewport
     */
    void render(Graphics::Viewport* viewport);

    /**
     * @brief Set selector visibility
     */
    void setVisible(bool visible);

    /**
     * @brief Check if selector is visible
     */
    bool isVisible() const;

private:
    Graphics::ViewportManager* m_manager;
    bool m_visible;
    
    // UI state
    std::string m_newPresetName;
    Graphics::ViewTransitionConfig m_transitionConfig;

    // Helper methods
    void renderPresetList();
    void renderPresetCreation();
    void renderTransitionConfig();
};

/**
 * @brief Widget for managing view bookmarks
 */
class ViewBookmarkManager {
public:
    ViewBookmarkManager(Graphics::ViewportManager* manager);
    
    /**
     * @brief Render the bookmark manager for a viewport
     */
    void render(Graphics::Viewport* viewport);

    /**
     * @brief Set manager visibility
     */
    void setVisible(bool visible);

    /**
     * @brief Check if manager is visible
     */
    bool isVisible() const;

private:
    Graphics::ViewportManager* m_manager;
    bool m_visible;
    
    // UI state
    std::string m_newBookmarkName;
    Graphics::ViewTransitionConfig m_transitionConfig;

    // Helper methods
    void renderBookmarkList();
    void renderBookmarkCreation();
    void renderTransitionConfig();
};

/**
 * @brief System for providing visual feedback during viewport operations
 */
class ViewFeedbackSystem {
public:
    ViewFeedbackSystem(Graphics::ViewportManager* manager);
    
    /**
     * @brief Update and render feedback elements
     */
    void update();

    /**
     * @brief Show transition progress feedback
     */
    void showTransitionFeedback(Graphics::Viewport* viewport, float progress);

    /**
     * @brief Show view change feedback
     */
    void showViewChangeFeedback(Graphics::Viewport* viewport, Graphics::ViewportView view);

    /**
     * @brief Show projection change feedback
     */
    void showProjectionChangeFeedback(Graphics::Viewport* viewport, Graphics::ViewportProjection proj);

private:
    Graphics::ViewportManager* m_manager;
    
    struct FeedbackState {
        bool active;
        float duration;
        float elapsed;
        std::string message;
    };
    std::unordered_map<Graphics::Viewport*, FeedbackState> m_feedback;

    // Helper methods
    void renderFeedback(Graphics::Viewport* viewport, const FeedbackState& state);
    void updateFeedbackTimers();
};

} // namespace UI
} // namespace RebelCAD
