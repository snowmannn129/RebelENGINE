#pragma once

#include <memory>
#include <atomic>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "graphics/Viewport.h"
#include "core/MemoryPool.h"

struct GLFWwindow;  // Forward declaration

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents the state of a camera
 */
struct CameraState {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 upVector;
    glm::quat rotation;
    float distance;
    float fieldOfView;
    float nearPlane;
    float farPlane;

    bool operator==(const CameraState& other) const;
    bool operator!=(const CameraState& other) const { return !(*this == other); }
};

/**
 * @brief Represents camera movement constraints
 */
struct CameraConstraints {
    float minDistance = 0.1f;
    float maxDistance = 1000.0f;
    float minFieldOfView = 10.0f;
    float maxFieldOfView = 120.0f;
    float minPitch = -89.0f;
    float maxPitch = 89.0f;
    bool lockRoll = true;
    bool boundingBoxConstraint = false;
    glm::vec3 boundingBoxMin = glm::vec3(-1000.0f);
    glm::vec3 boundingBoxMax = glm::vec3(1000.0f);
};

/**
 * @brief Represents camera movement settings
 */
struct CameraSettings {
    float orbitSpeed = 0.005f;
    float panSpeed = 0.002f;
    float zoomSpeed = 0.1f;
    float rotateSpeed = 0.005f;
    float smoothFactor = 0.15f;
    bool invertY = false;
    bool smoothTransitions = true;
    float transitionDuration = 0.5f;
};

/**
 * @brief Manages camera control and input handling for a viewport
 * 
 * The CameraController class provides a comprehensive system for manipulating
 * the camera view through user input. It supports:
 * - Orbit, pan, zoom, and free rotation controls
 * - Smooth transitions and interpolation
 * - Input customization and constraints
 * - Thread-safe state management
 */
class CameraController {
public:
    /**
     * @brief Create a new camera controller
     * @param viewport The viewport to control
     * @param window GLFW window for input handling
     */
    CameraController(Viewport* viewport, GLFWwindow* window);
    ~CameraController();

    /**
     * @brief Update camera state based on input and time
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Handle mouse button input
     * @param button Mouse button that was pressed/released
     * @param action GLFW action (press/release)
     * @param mods Modifier keys held
     */
    void handleMouseButton(int button, int action, int mods);

    /**
     * @brief Handle mouse movement
     * @param xpos New cursor X position
     * @param ypos New cursor Y position
     */
    void handleMouseMove(double xpos, double ypos);

    /**
     * @brief Handle mouse wheel scroll
     * @param xoffset Horizontal scroll amount
     * @param yoffset Vertical scroll amount
     */
    void handleScroll(double xoffset, double yoffset);

    /**
     * @brief Handle keyboard input
     * @param key Key that was pressed/released
     * @param action GLFW action (press/release)
     * @param mods Modifier keys held
     */
    void handleKeyboard(int key, int action, int mods);

    /**
     * @brief Set camera view to a standard orientation
     * @param view Standard view to set
     * @param animate Whether to animate the transition
     */
    void setStandardView(ViewportView view, bool animate = true);

    /**
     * @brief Set camera state directly
     * @param state New camera state
     * @param animate Whether to animate the transition
     */
    void setState(const CameraState& state, bool animate = true);

    /**
     * @brief Get current camera state
     * @return Current camera state
     */
    CameraState getState() const;

    /**
     * @brief Set camera constraints
     * @param constraints New constraints
     */
    void setConstraints(const CameraConstraints& constraints);

    /**
     * @brief Get current camera constraints
     * @return Current constraints
     */
    CameraConstraints getConstraints() const;

    /**
     * @brief Set camera settings
     * @param settings New settings
     */
    void setSettings(const CameraSettings& settings);

    /**
     * @brief Get current camera settings
     * @return Current settings
     */
    CameraSettings getSettings() const;

    /**
     * @brief Focus camera on a point
     * @param point Point to focus on
     * @param distance Desired distance from point
     * @param animate Whether to animate the transition
     */
    void focusOn(const glm::vec3& point, float distance, bool animate = true);

    /**
     * @brief Frame a bounding box in the view
     * @param min Minimum point of bounding box
     * @param max Maximum point of bounding box
     * @param animate Whether to animate the transition
     */
    void frameRegion(const glm::vec3& min, const glm::vec3& max, bool animate = true);

private:
    // Input state tracking
    struct InputState {
        bool leftMouseDown = false;
        bool rightMouseDown = false;
        bool middleMouseDown = false;
        bool altDown = false;
        bool ctrlDown = false;
        bool shiftDown = false;
        double lastMouseX = 0.0;
        double lastMouseY = 0.0;
    };

    // Camera operations
    void orbit(float deltaX, float deltaY);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);
    void rotate(float deltaX, float deltaY);
    void dollyZoom(float delta);

    // State management
    void updateViewMatrix();
    void applyConstraints(CameraState& state) const;
    void startTransition(const CameraState& target, float duration);
    void updateTransition(float deltaTime);
    glm::vec3 projectToViewPlane(const glm::vec3& worldPoint) const;
    float calculateOptimalDistance(const glm::vec3& min, const glm::vec3& max) const;

    // Member variables
    Viewport* m_viewport;
    GLFWwindow* m_window;
    InputState m_input;
    CameraState m_currentState;
    CameraState m_targetState;
    CameraConstraints m_constraints;
    CameraSettings m_settings;
    
    // Transition state
    std::atomic<bool> m_transitioning;
    float m_transitionTime;
    float m_transitionDuration;

    // Thread safety
    mutable std::atomic<bool> m_updating;
    bool tryLock();
    void unlock();

    // Memory management
    static Core::MemoryPool<CameraController> s_pool;
};

} // namespace Graphics
} // namespace RebelCAD
