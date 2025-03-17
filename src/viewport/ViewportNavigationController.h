#pragma once

#include "Viewport.h"
#include <memory>
#include <functional>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Navigation modes for different projection types
 */
enum class NavigationMode {
    Orbit,          ///< Orbit around target point (perspective)
    Pan,            ///< Pan view (both)
    Zoom,           ///< Zoom in/out (both)
    Walk,           ///< Walk through scene (perspective)
    Fly,            ///< Fly through scene (perspective)
    PanZoom,        ///< Combined pan and zoom (orthographic)
    Scroll,         ///< Scroll view (orthographic)
    BoxZoom         ///< Box zoom selection (orthographic)
};

/**
 * @brief Configuration for navigation behavior
 */
struct NavigationConfig {
    float orbitSensitivity = 0.01f;
    float panSensitivity = 0.01f;
    float zoomSensitivity = 0.1f;
    float walkSpeed = 5.0f;
    float flySpeed = 10.0f;
    float scrollSensitivity = 0.05f;
    bool invertOrbitY = false;
    bool invertPanY = false;
    bool smoothZoom = true;
};

/**
 * @brief Manages viewport navigation based on projection type
 */
class ViewportNavigationController {
public:
    /**
     * @brief Constructor
     * @param viewport Viewport to control
     */
    explicit ViewportNavigationController(std::shared_ptr<Viewport> viewport);

    /**
     * @brief Sets the navigation mode
     * @param mode New navigation mode
     */
    void setMode(NavigationMode mode);

    /**
     * @brief Gets the current navigation mode
     * @return Current mode
     */
    NavigationMode getMode() const { return m_currentMode; }

    /**
     * @brief Updates navigation based on input
     * @param deltaX Mouse X movement
     * @param deltaY Mouse Y movement
     * @param deltaTime Time since last update
     */
    void update(float deltaX, float deltaY, float deltaTime);

    /**
     * @brief Handles mouse wheel input
     * @param delta Wheel delta
     */
    void handleMouseWheel(float delta);

    /**
     * @brief Handles key input for walk/fly modes
     * @param forward Forward movement
     * @param right Right movement
     * @param up Up movement
     * @param deltaTime Time since last update
     */
    void handleKeyMovement(
        float forward,
        float right,
        float up,
        float deltaTime
    );

    /**
     * @brief Starts box zoom selection
     * @param startX Starting X coordinate
     * @param startY Starting Y coordinate
     */
    void startBoxZoom(float startX, float startY);

    /**
     * @brief Updates box zoom selection
     * @param currentX Current X coordinate
     * @param currentY Current Y coordinate
     */
    void updateBoxZoom(float currentX, float currentY);

    /**
     * @brief Finishes box zoom selection
     * @param endX Ending X coordinate
     * @param endY Ending Y coordinate
     */
    void finishBoxZoom(float endX, float endY);

    /**
     * @brief Sets navigation configuration
     * @param config New configuration
     */
    void setConfig(const NavigationConfig& config);

    /**
     * @brief Gets current navigation configuration
     * @return Current configuration
     */
    const NavigationConfig& getConfig() const { return m_config; }

    /**
     * @brief Sets callback for mode changes
     * @param callback Function to call when mode changes
     */
    void setModeChangeCallback(std::function<void(NavigationMode)> callback);

private:
    // Navigation implementations
    void handleOrbit(float deltaX, float deltaY);
    void handlePan(float deltaX, float deltaY);
    void handleZoom(float delta);
    void handleWalk(float forward, float right, float up, float deltaTime);
    void handleFly(float forward, float right, float up, float deltaTime);
    void handlePanZoom(float deltaX, float deltaY, float zoomDelta);
    void handleScroll(float deltaX, float deltaY);
    void calculateBoxZoom(float startX, float startY, float endX, float endY);

    // Helper methods
    void updateProjectionBasedMode();
    bool isValidModeForProjection(NavigationMode mode) const;
    NavigationMode getDefaultModeForProjection() const;

    std::shared_ptr<Viewport> m_viewport;
    NavigationMode m_currentMode;
    NavigationConfig m_config;
    std::function<void(NavigationMode)> m_modeChangeCallback;

    // Box zoom state
    struct {
        bool active = false;
        float startX = 0.0f;
        float startY = 0.0f;
        float currentX = 0.0f;
        float currentY = 0.0f;
    } m_boxZoomState;

    // Movement state
    struct {
        glm::vec3 velocity = glm::vec3(0.0f);
        float acceleration = 0.0f;
        bool moving = false;
    } m_movementState;
};

} // namespace Graphics
} // namespace RebelCAD
