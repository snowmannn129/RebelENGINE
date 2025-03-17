#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include "graphics/ViewportTypes.h"
#include "graphics/ViewportLayout.h"
#include "graphics/ViewSyncManager.h"
#include "graphics/CameraCollision.h"
#include "graphics/SceneGraph.h"
#include "core/EventSystem.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Central manager for viewport system functionality
 * 
 * The ViewportManager class orchestrates all viewport-related operations including:
 * - Layout management
 * - View synchronization
 * - Camera presets and transitions
 * - View bookmarks
 * - Event handling
 */
class ViewportManager {
public:
    /**
     * @brief Create a new viewport manager
     * @param width Initial window width
     * @param height Initial window height
     */
    ViewportManager(int width, int height);
    ~ViewportManager();

    // Layout Management
    //------------------

    /**
     * @brief Set the window dimensions
     * @param width New window width
     * @param height New window height
     */
    void setDimensions(int width, int height);

    /**
     * @brief Apply a layout preset
     * @param preset Layout configuration to apply
     */
    void applyLayoutPreset(const ViewportLayoutPreset& preset);

    /**
     * @brief Save current layout as preset
     * @param name Preset identifier
     * @return Created preset configuration
     */
    ViewportLayoutPreset saveLayoutPreset(const std::string& name);

    /**
     * @brief Get viewport at grid position
     * @param row Grid row
     * @param col Grid column
     * @return Pointer to viewport if found
     */
    Viewport* getViewport(int row, int col);

    // View Synchronization
    //--------------------

    /**
     * @brief Create a new sync group
     * @param name Group identifier
     * @return true if created successfully
     */
    bool createSyncGroup(const std::string& name);

    /**
     * @brief Add viewport to sync group
     * @param groupName Group identifier
     * @param viewport Viewport to add
     * @return true if added successfully
     */
    bool addToSyncGroup(const std::string& groupName, Viewport* viewport);

    /**
     * @brief Remove viewport from sync group
     * @param groupName Group identifier
     * @param viewport Viewport to remove
     */
    void removeFromSyncGroup(const std::string& groupName, Viewport* viewport);

    // Camera & View Management
    //------------------------

    /**
     * @brief Apply camera preset to viewport
     * @param viewport Target viewport
     * @param preset Camera configuration to apply
     * @param transition Optional transition configuration
     */
    void applyCameraPreset(Viewport* viewport, const CameraPreset& preset,
                          const ViewTransitionConfig& transition = ViewTransitionConfig());

    /**
     * @brief Save current viewport state as preset
     * @param viewport Source viewport
     * @param name Preset identifier
     * @return Created camera preset
     */
    CameraPreset saveCameraPreset(Viewport* viewport, const std::string& name);

    /**
     * @brief Create view bookmark
     * @param viewport Source viewport
     * @param name Bookmark identifier
     * @return Created bookmark
     */
    ViewBookmark createBookmark(Viewport* viewport, const std::string& name);

    /**
     * @brief Apply view bookmark to viewport
     * @param viewport Target viewport
     * @param bookmark Bookmark to apply
     * @param transition Optional transition configuration
     */
    void applyBookmark(Viewport* viewport, const ViewBookmark& bookmark,
                      const ViewTransitionConfig& transition = ViewTransitionConfig());

    /**
     * @brief Get list of saved bookmarks
     * @return Vector of bookmark names
     */
    std::vector<std::string> getBookmarkNames() const;

    /**
     * @brief Get a specific view bookmark
     * @param name Bookmark identifier
     * @return View bookmark configuration
     */
    ViewBookmark getBookmark(const std::string& name) const;

    /**
     * @brief Delete a view bookmark
     * @param name Bookmark identifier
     */
    void deleteBookmark(const std::string& name);

    /**
     * @brief Get list of camera presets
     * @return Vector of preset names
     */
    std::vector<std::string> getPresetNames() const;

    /**
     * @brief Get a specific camera preset
     * @param name Preset identifier
     * @return Camera preset configuration
     */
    CameraPreset getCameraPreset(const std::string& name) const;

    /**
     * @brief Get list of layout presets
     * @return Vector of preset names
     */
    std::vector<std::string> getLayoutPresetNames() const;

    /**
     * @brief Get a specific layout preset
     * @param name Preset identifier
     * @return Layout preset configuration
     */
    ViewportLayoutPreset getLayoutPreset(const std::string& name) const;

    // View Transitions
    //----------------

    /**
     * @brief Start view transition
     * @param viewport Target viewport
     * @param targetState Desired view state
     * @param config Transition configuration
     */
    void startViewTransition(Viewport* viewport, const ViewState& targetState,
                           const ViewTransitionConfig& config = ViewTransitionConfig());

    /**
     * @brief Cancel active transition
     * @param viewport Target viewport
     */
    void cancelTransition(Viewport* viewport);

    /**
     * @brief Check if viewport is transitioning
     * @param viewport Target viewport
     * @return true if transition is active
     */
    bool isTransitioning(Viewport* viewport) const;

    // Event Handling
    //--------------

    /**
     * @brief Update all viewports
     * Processes transitions and triggers redraws
     */
    void update();

    /**
     * @brief Set the active scene for collision detection
     * @param scene Scene graph to use for collision testing
     */
    void setActiveScene(const SceneGraph* scene) { m_activeScene = scene; }

    /**
     * @brief Get the current collision configuration
     * @return Current collision settings
     */
    CameraCollision::Config getCollisionConfig() const { return m_collisionSystem->getConfig(); }

    /**
     * @brief Set collision detection configuration
     * @param config New collision settings
     */
    void setCollisionConfig(const CameraCollision::Config& config) { m_collisionSystem->setConfig(config); }

private:
    // Core components
    std::unique_ptr<ViewportLayout> m_layout;
    std::unique_ptr<ViewSyncManager> m_syncManager;
    std::unique_ptr<CameraCollision> m_collisionSystem;
    const SceneGraph* m_activeScene;

    // Saved configurations
    std::unordered_map<std::string, CameraPreset> m_cameraPresets;
    std::unordered_map<std::string, ViewBookmark> m_bookmarks;
    std::unordered_map<std::string, ViewportLayoutPreset> m_layoutPresets;

    // Transition management
    struct TransitionState {
        ViewState startState;
        ViewState targetState;
        ViewTransitionConfig config;
        std::chrono::steady_clock::time_point startTime;
        float progress;
    };
    std::unordered_map<Viewport*, TransitionState> m_activeTransitions;

    // Helper methods
    void updateTransitions();
    glm::vec3 resolveCollision(Viewport* viewport, const glm::vec3& targetPos);
    ViewState interpolateViewStates(const ViewState& start, const ViewState& end, float t);
    float calculateEasing(float t, float easeIn, float easeOut);
    void finalizeTransition(Viewport* viewport);
};

} // namespace Graphics
} // namespace RebelCAD
