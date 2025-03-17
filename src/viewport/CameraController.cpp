#include "graphics/CameraController.h"
#include <GLFW/glfw3.h>
#include "graphics/GLMConfig.hpp"
#include <algorithm>
#include <cmath>

namespace RebelCAD {
namespace Graphics {

// Initialize static memory pool
Core::MemoryPool<CameraController> CameraController::s_pool(32);  // Start with space for 32 controllers

// CameraState implementation
bool CameraState::operator==(const CameraState& other) const {
    return position == other.position &&
           target == other.target &&
           upVector == other.upVector &&
           rotation == other.rotation &&
           distance == other.distance &&
           fieldOfView == other.fieldOfView &&
           nearPlane == other.nearPlane &&
           farPlane == other.farPlane;
}

// CameraController implementation
CameraController::CameraController(Viewport* viewport, GLFWwindow* window)
    : m_viewport(viewport)
    , m_window(window)
    , m_transitioning(false)
    , m_transitionTime(0.0f)
    , m_transitionDuration(0.0f)
    , m_updating(false) {
    
    // Initialize current state from viewport
    m_currentState.position = viewport->getEyePosition();
    m_currentState.target = viewport->getTarget();
    m_currentState.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    m_currentState.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    m_currentState.distance = glm::length(m_currentState.position - m_currentState.target);
    m_currentState.fieldOfView = 45.0f;
    m_currentState.nearPlane = 0.1f;
    m_currentState.farPlane = 1000.0f;

    m_targetState = m_currentState;
}

CameraController::~CameraController() {
    // Nothing to clean up
}

bool CameraController::tryLock() {
    bool expected = false;
    return m_updating.compare_exchange_strong(expected, true);
}

void CameraController::unlock() {
    m_updating.store(false);
}

void CameraController::update(float deltaTime) {
    if (!tryLock()) return;

    if (m_transitioning) {
        updateTransition(deltaTime);
    }

    updateViewMatrix();
    unlock();
}

void CameraController::handleMouseButton(int button, int action, int mods) {
    if (!tryLock()) return;

    // Update input state
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_input.leftMouseDown = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        m_input.rightMouseDown = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        m_input.middleMouseDown = (action == GLFW_PRESS);
    }

    // Update modifier states
    m_input.altDown = (mods & GLFW_MOD_ALT) != 0;
    m_input.ctrlDown = (mods & GLFW_MOD_CONTROL) != 0;
    m_input.shiftDown = (mods & GLFW_MOD_SHIFT) != 0;

    // Get current cursor position
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    m_input.lastMouseX = xpos;
    m_input.lastMouseY = ypos;

    unlock();
}

void CameraController::handleMouseMove(double xpos, double ypos) {
    if (!tryLock()) return;

    float deltaX = static_cast<float>(xpos - m_input.lastMouseX);
    float deltaY = static_cast<float>(ypos - m_input.lastMouseY);

    if (m_settings.invertY) {
        deltaY = -deltaY;
    }

    // Handle different mouse modes
    if (m_input.leftMouseDown) {
        if (m_input.altDown) {
            orbit(deltaX, deltaY);
        } else {
            rotate(deltaX, deltaY);
        }
    } else if (m_input.rightMouseDown) {
        pan(deltaX, deltaY);
    } else if (m_input.middleMouseDown) {
        dollyZoom(deltaY);
    }

    m_input.lastMouseX = xpos;
    m_input.lastMouseY = ypos;

    unlock();
}

void CameraController::handleScroll(double xoffset, double yoffset) {
    if (!tryLock()) return;
    zoom(static_cast<float>(yoffset));
    unlock();
}

void CameraController::handleKeyboard(int key, int action, int mods) {
    if (!tryLock()) return;

    // Update modifier states
    m_input.altDown = (mods & GLFW_MOD_ALT) != 0;
    m_input.ctrlDown = (mods & GLFW_MOD_CONTROL) != 0;
    m_input.shiftDown = (mods & GLFW_MOD_SHIFT) != 0;

    // Handle keyboard shortcuts for standard views
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1: setStandardView(ViewportView::Front); break;
            case GLFW_KEY_3: setStandardView(ViewportView::Right); break;
            case GLFW_KEY_7: setStandardView(ViewportView::Top); break;
            case GLFW_KEY_5: setStandardView(ViewportView::Isometric); break;
        }
    }

    unlock();
}

void CameraController::setStandardView(ViewportView view, bool animate) {
    if (!tryLock()) return;

    CameraState targetState = m_currentState;
    float distance = m_currentState.distance;

    switch (view) {
        case ViewportView::Front:
            targetState.position = glm::vec3(0.0f, 0.0f, distance);
            targetState.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            break;
        case ViewportView::Back:
            targetState.position = glm::vec3(0.0f, 0.0f, -distance);
            targetState.rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case ViewportView::Right:
            targetState.position = glm::vec3(distance, 0.0f, 0.0f);
            targetState.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case ViewportView::Left:
            targetState.position = glm::vec3(-distance, 0.0f, 0.0f);
            targetState.rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case ViewportView::Top:
            targetState.position = glm::vec3(0.0f, distance, 0.0f);
            targetState.rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case ViewportView::Bottom:
            targetState.position = glm::vec3(0.0f, -distance, 0.0f);
            targetState.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case ViewportView::Isometric:
            targetState.position = glm::vec3(distance) * 0.577f;  // 1/√3
            targetState.rotation = glm::quat(glm::vec3(glm::radians(-35.264f), glm::radians(45.0f), 0.0f));
            break;
    }

    setState(targetState, animate);
    unlock();
}

void CameraController::setState(const CameraState& state, bool animate) {
    if (!tryLock()) return;

    if (animate && m_settings.smoothTransitions) {
        startTransition(state, m_settings.transitionDuration);
    } else {
        m_currentState = state;
        m_targetState = state;
        applyConstraints(m_currentState);
        updateViewMatrix();
    }

    unlock();
}

CameraState CameraController::getState() const {
    return m_currentState;
}

void CameraController::setConstraints(const CameraConstraints& constraints) {
    if (!tryLock()) return;
    m_constraints = constraints;
    applyConstraints(m_currentState);
    updateViewMatrix();
    unlock();
}

CameraConstraints CameraController::getConstraints() const {
    return m_constraints;
}

void CameraController::setSettings(const CameraSettings& settings) {
    if (!tryLock()) return;
    m_settings = settings;
    unlock();
}

CameraSettings CameraController::getSettings() const {
    return m_settings;
}

void CameraController::focusOn(const glm::vec3& point, float distance, bool animate) {
    if (!tryLock()) return;

    CameraState targetState = m_currentState;
    targetState.target = point;
    targetState.distance = distance;
    
    // Maintain current view direction
    glm::vec3 direction = glm::normalize(m_currentState.position - m_currentState.target);
    targetState.position = point + direction * distance;

    setState(targetState, animate);
    unlock();
}

void CameraController::frameRegion(const glm::vec3& min, const glm::vec3& max, bool animate) {
    if (!tryLock()) return;

    glm::vec3 center = (min + max) * 0.5f;
    float distance = calculateOptimalDistance(min, max);

    CameraState targetState = m_currentState;
    targetState.target = center;
    targetState.distance = distance;
    
    // Maintain current view direction
    glm::vec3 direction = glm::normalize(m_currentState.position - m_currentState.target);
    targetState.position = center + direction * distance;

    setState(targetState, animate);
    unlock();
}

void CameraController::orbit(float deltaX, float deltaY) {
    float pitch = deltaY * m_settings.orbitSpeed;
    float yaw = deltaX * m_settings.orbitSpeed;

    // Create rotation quaternions
    glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    // Combine rotations
    glm::quat rotation = yawQuat * pitchQuat;

    // Apply rotation to position relative to target
    glm::vec3 toCamera = m_currentState.position - m_currentState.target;
    toCamera = rotation * toCamera;

    // Update position and rotation
    m_currentState.position = m_currentState.target + toCamera;
    m_currentState.rotation = rotation * m_currentState.rotation;

    applyConstraints(m_currentState);
    updateViewMatrix();
}

void CameraController::pan(float deltaX, float deltaY) {
    // Calculate pan vectors in view space
    glm::vec3 right = glm::normalize(glm::cross(m_currentState.target - m_currentState.position, m_currentState.upVector));
    glm::vec3 up = glm::normalize(glm::cross(right, m_currentState.target - m_currentState.position));

    // Scale movement by distance to target for consistent speed
    float scale = m_currentState.distance * m_settings.panSpeed;
    glm::vec3 offset = right * (-deltaX * scale) + up * (deltaY * scale);

    // Update position and target
    m_currentState.position += offset;
    m_currentState.target += offset;

    applyConstraints(m_currentState);
    updateViewMatrix();
}

void CameraController::zoom(float delta) {
    float factor = 1.0f - (delta * m_settings.zoomSpeed);
    m_currentState.distance *= factor;

    // Update position while maintaining view direction
    glm::vec3 direction = glm::normalize(m_currentState.position - m_currentState.target);
    m_currentState.position = m_currentState.target + direction * m_currentState.distance;

    applyConstraints(m_currentState);
    updateViewMatrix();
}

void CameraController::rotate(float deltaX, float deltaY) {
    float pitch = deltaY * m_settings.rotateSpeed;
    float yaw = deltaX * m_settings.rotateSpeed;

    // Create rotation quaternions
    glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    // Update rotation
    m_currentState.rotation = yawQuat * pitchQuat * m_currentState.rotation;

    applyConstraints(m_currentState);
    updateViewMatrix();
}

void CameraController::dollyZoom(float delta) {
    // Adjust field of view
    float factor = 1.0f - (delta * m_settings.zoomSpeed * 0.1f);
    m_currentState.fieldOfView *= factor;

    applyConstraints(m_currentState);
    updateViewMatrix();
}

void CameraController::updateViewMatrix() {
    m_viewport->setView(m_currentState.position, m_currentState.target, m_currentState.upVector);
}

void CameraController::applyConstraints(CameraState& state) const {
    // Clamp distance
    state.distance = glm::clamp(state.distance, m_constraints.minDistance, m_constraints.maxDistance);

    // Clamp field of view
    state.fieldOfView = glm::clamp(state.fieldOfView, m_constraints.minFieldOfView, m_constraints.maxFieldOfView);

    if (m_constraints.boundingBoxConstraint) {
        // Clamp position within bounding box
        state.position = glm::clamp(state.position, m_constraints.boundingBoxMin, m_constraints.boundingBoxMax);
        state.target = glm::clamp(state.target, m_constraints.boundingBoxMin, m_constraints.boundingBoxMax);
    }

    if (m_constraints.lockRoll) {
        // Extract euler angles
        glm::vec3 euler = glm::eulerAngles(state.rotation);
        
        // Clamp pitch
        euler.x = glm::clamp(euler.x, glm::radians(m_constraints.minPitch), glm::radians(m_constraints.maxPitch));
        
        // Zero out roll
        euler.z = 0.0f;
        
        // Reconstruct quaternion
        state.rotation = glm::quat(euler);
    }
}

void CameraController::startTransition(const CameraState& target, float duration) {
    m_targetState = target;
    applyConstraints(m_targetState);
    m_transitionTime = 0.0f;
    m_transitionDuration = duration;
    m_transitioning = true;
}

void CameraController::updateTransition(float deltaTime) {
    m_transitionTime += deltaTime;
    float t = m_transitionTime / m_transitionDuration;

    if (t >= 1.0f) {
        m_currentState = m_targetState;
        m_transitioning = false;
        return;
    }

    // Smooth step interpolation
    t = t * t * (3.0f - 2.0f * t);

    // Interpolate state
    m_currentState.position = glm::mix(m_currentState.position, m_targetState.position, t);
    m_currentState.target = glm::mix(m_currentState.target, m_targetState.target, t);
    m_currentState.rotation = glm::slerp(m_currentState.rotation, m_targetState.rotation, t);
    m_currentState.distance = glm::mix(m_currentState.distance, m_targetState.distance, t);
    m_currentState.fieldOfView = glm::mix(m_currentState.fieldOfView, m_targetState.fieldOfView, t);

    applyConstraints(m_currentState);
}

glm::vec3 CameraController::projectToViewPlane(const glm::vec3& worldPoint) const {
    // Create view matrix
    glm::mat4 view = glm::lookAt(
        m_currentState.position,
        m_currentState.target,
        m_currentState.upVector
    );

    // Transform point to view space
    glm::vec4 viewPoint = view * glm::vec4(worldPoint, 1.0f);

    // Project onto view plane (z = 0)
    return glm::vec3(viewPoint) / viewPoint.w;
}

float CameraController::calculateOptimalDistance(const glm::vec3& min, const glm::vec3& max) const {
    // Calculate bounding box dimensions
    glm::vec3 dimensions = max - min;
    float maxDimension = glm::max(dimensions.x, glm::max(dimensions.y, dimensions.z));

    // Calculate distance based on field of view
    float halfFov = glm::radians(m_currentState.fieldOfView * 0.5f);
    return (maxDimension * 0.5f) / glm::tan(halfFov);
}

} // namespace Graphics
} // namespace RebelCAD
