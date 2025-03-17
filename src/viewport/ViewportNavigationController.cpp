#include "graphics/ViewportNavigationController.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

ViewportNavigationController::ViewportNavigationController(
    std::shared_ptr<Viewport> viewport
) : m_viewport(viewport)
  , m_currentMode(NavigationMode::Orbit)
  , m_config()
{
    updateProjectionBasedMode();
}

void ViewportNavigationController::setMode(NavigationMode mode) {
    if (!isValidModeForProjection(mode)) {
        mode = getDefaultModeForProjection();
    }

    if (m_currentMode != mode) {
        m_currentMode = mode;
        if (m_modeChangeCallback) {
            m_modeChangeCallback(mode);
        }
    }
}

void ViewportNavigationController::update(
    float deltaX,
    float deltaY,
    float deltaTime
) {
    switch (m_currentMode) {
        case NavigationMode::Orbit:
            handleOrbit(deltaX, deltaY);
            break;
        case NavigationMode::Pan:
            handlePan(deltaX, deltaY);
            break;
        case NavigationMode::PanZoom:
            handlePanZoom(deltaX, deltaY, 0.0f);
            break;
        case NavigationMode::Scroll:
            handleScroll(deltaX, deltaY);
            break;
        case NavigationMode::Walk:
        case NavigationMode::Fly:
            // Movement handled by handleKeyMovement
            break;
        default:
            break;
    }

    // Update movement physics
    if (m_movementState.moving) {
        if (m_currentMode == NavigationMode::Walk || m_currentMode == NavigationMode::Fly) {
            glm::vec3 position = m_viewport->getEyePosition();
            position += m_movementState.velocity * deltaTime;
            m_viewport->setView(position, position + glm::normalize(m_viewport->getTarget() - position), glm::vec3(0, 1, 0));
        }
    }
}

void ViewportNavigationController::handleMouseWheel(float delta) {
    switch (m_currentMode) {
        case NavigationMode::Zoom:
            handleZoom(delta);
            break;
        case NavigationMode::PanZoom:
            handlePanZoom(0.0f, 0.0f, delta);
            break;
        default:
            break;
    }
}

void ViewportNavigationController::handleKeyMovement(
    float forward,
    float right,
    float up,
    float deltaTime
) {
    if (m_currentMode == NavigationMode::Walk) {
        handleWalk(forward, right, up, deltaTime);
    } else if (m_currentMode == NavigationMode::Fly) {
        handleFly(forward, right, up, deltaTime);
    }
}

void ViewportNavigationController::startBoxZoom(float startX, float startY) {
    if (m_currentMode == NavigationMode::BoxZoom) {
        m_boxZoomState.active = true;
        m_boxZoomState.startX = startX;
        m_boxZoomState.startY = startY;
        m_boxZoomState.currentX = startX;
        m_boxZoomState.currentY = startY;
    }
}

void ViewportNavigationController::updateBoxZoom(float currentX, float currentY) {
    if (m_boxZoomState.active) {
        m_boxZoomState.currentX = currentX;
        m_boxZoomState.currentY = currentY;
    }
}

void ViewportNavigationController::finishBoxZoom(float endX, float endY) {
    if (m_boxZoomState.active) {
        calculateBoxZoom(m_boxZoomState.startX, m_boxZoomState.startY, endX, endY);
        m_boxZoomState.active = false;
    }
}

void ViewportNavigationController::setConfig(const NavigationConfig& config) {
    m_config = config;
}

void ViewportNavigationController::setModeChangeCallback(
    std::function<void(NavigationMode)> callback
) {
    m_modeChangeCallback = std::move(callback);
}

// Private methods

void ViewportNavigationController::handleOrbit(float deltaX, float deltaY) {
    if (m_viewport->getProjectionType() != ViewportProjection::Perspective) {
        return;
    }

    glm::vec3 position = m_viewport->getEyePosition();
    glm::vec3 target = m_viewport->getTarget();
    glm::vec3 up(0, 1, 0);

    // Calculate orbit angles
    float sensitivity = m_config.orbitSensitivity;
    float yaw = deltaX * sensitivity;
    float pitch = deltaY * (m_config.invertOrbitY ? -1.0f : 1.0f) * sensitivity;

    // Create rotation quaternions
    glm::quat yawQuat = glm::angleAxis(yaw, up);
    glm::vec3 right = glm::normalize(glm::cross(m_viewport->getTarget() - position, up));
    glm::quat pitchQuat = glm::angleAxis(pitch, right);

    // Apply rotations
    glm::vec3 toTarget = position - target;
    toTarget = glm::rotate(yawQuat * pitchQuat, toTarget);
    position = target + toTarget;

    m_viewport->setView(position, target, up);
}

void ViewportNavigationController::handlePan(float deltaX, float deltaY) {
    glm::vec3 position = m_viewport->getEyePosition();
    glm::vec3 target = m_viewport->getTarget();
    glm::vec3 up(0, 1, 0);

    // Calculate pan vectors
    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    glm::vec3 upDir = glm::cross(right, forward);

    float sensitivity = m_config.panSensitivity;
    float distance = glm::length(target - position);
    
    // Scale movement by distance for consistent feel
    glm::vec3 movement = right * (-deltaX * sensitivity * distance) +
                        upDir * (deltaY * (m_config.invertPanY ? -1.0f : 1.0f) * sensitivity * distance);

    position += movement;
    target += movement;

    m_viewport->setView(position, target, up);
}

void ViewportNavigationController::handleZoom(float delta) {
    if (m_viewport->getProjectionType() == ViewportProjection::Perspective) {
        // Perspective zoom - move camera closer/further
        glm::vec3 position = m_viewport->getEyePosition();
        glm::vec3 target = m_viewport->getTarget();
        glm::vec3 direction = glm::normalize(target - position);
        
        float distance = glm::length(target - position);
        float zoomFactor = 1.0f + (delta * m_config.zoomSensitivity);
        
        if (m_config.smoothZoom) {
            zoomFactor = glm::mix(1.0f, zoomFactor, 0.5f);
        }
        
        position = target - direction * (distance * zoomFactor);
        m_viewport->setView(position, target, glm::vec3(0, 1, 0));
    } else {
        // Orthographic zoom - adjust scale
        float currentScale = m_viewport->getOrthoScale();
        float newScale = currentScale * (1.0f - delta * m_config.zoomSensitivity);
        newScale = glm::clamp(newScale, 0.1f, 1000.0f);
        
        float aspect = m_viewport->getAspectRatio();
        m_viewport->setOrthographic(
            -newScale * aspect,
            newScale * aspect,
            -newScale,
            newScale,
            m_viewport->getNearPlane(),
            m_viewport->getFarPlane()
        );
    }
}

void ViewportNavigationController::handleWalk(
    float forward,
    float right,
    float up,
    float deltaTime
) {
    if (m_viewport->getProjectionType() != ViewportProjection::Perspective) {
        return;
    }

    glm::vec3 position = m_viewport->getEyePosition();
    glm::vec3 target = m_viewport->getTarget();
    glm::vec3 direction = glm::normalize(target - position);
    glm::vec3 rightDir = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
    glm::vec3 upDir(0, 1, 0);

    // Calculate movement velocity
    m_movementState.velocity = direction * forward * m_config.walkSpeed +
                             rightDir * right * m_config.walkSpeed +
                             upDir * up * m_config.walkSpeed;
    
    m_movementState.moving = glm::length(m_movementState.velocity) > 0.0f;
}

void ViewportNavigationController::handleFly(
    float forward,
    float right,
    float up,
    float deltaTime
) {
    if (m_viewport->getProjectionType() != ViewportProjection::Perspective) {
        return;
    }

    glm::vec3 position = m_viewport->getEyePosition();
    glm::vec3 target = m_viewport->getTarget();
    glm::vec3 direction = glm::normalize(target - position);
    glm::vec3 rightDir = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
    glm::vec3 upDir = glm::normalize(glm::cross(rightDir, direction));

    // Calculate movement velocity with acceleration
    float targetSpeed = m_config.flySpeed;
    float acceleration = 2.0f; // Units per second squared
    
    if (glm::length(glm::vec3(forward, right, up)) > 0.0f) {
        m_movementState.acceleration = std::min(
            m_movementState.acceleration + acceleration * deltaTime,
            targetSpeed
        );
    } else {
        m_movementState.acceleration = std::max(
            m_movementState.acceleration - acceleration * deltaTime,
            0.0f
        );
    }

    m_movementState.velocity = (direction * forward +
                              rightDir * right +
                              upDir * up) * m_movementState.acceleration;
    
    m_movementState.moving = m_movementState.acceleration > 0.0f;
}

void ViewportNavigationController::handlePanZoom(
    float deltaX,
    float deltaY,
    float zoomDelta
) {
    if (m_viewport->getProjectionType() != ViewportProjection::Orthographic) {
        return;
    }

    // Handle pan
    handlePan(deltaX, deltaY);

    // Handle zoom
    if (zoomDelta != 0.0f) {
        handleZoom(zoomDelta);
    }
}

void ViewportNavigationController::handleScroll(float deltaX, float deltaY) {
    if (m_viewport->getProjectionType() != ViewportProjection::Orthographic) {
        return;
    }

    glm::vec3 position = m_viewport->getEyePosition();
    glm::vec3 target = m_viewport->getTarget();
    
    float scale = m_viewport->getOrthoScale();
    glm::vec3 movement = glm::vec3(
        deltaX * m_config.scrollSensitivity * scale,
        deltaY * m_config.scrollSensitivity * scale,
        0.0f
    );

    position += movement;
    target += movement;

    m_viewport->setView(position, target, glm::vec3(0, 1, 0));
}

void ViewportNavigationController::calculateBoxZoom(
    float startX,
    float startY,
    float endX,
    float endY
) {
    if (m_viewport->getProjectionType() != ViewportProjection::Orthographic) {
        return;
    }

    // Convert screen coordinates to world space
    glm::vec2 min = glm::vec2(
        std::min(startX, endX),
        std::min(startY, endY)
    );
    glm::vec2 max = glm::vec2(
        std::max(startX, endX),
        std::max(startY, endY)
    );

    // Calculate center and size of selection
    glm::vec2 center = (min + max) * 0.5f;
    glm::vec2 size = max - min;

    // Convert to world space
    glm::vec3 worldMin = m_viewport->screenToRay(min).origin;
    glm::vec3 worldMax = m_viewport->screenToRay(max).origin;
    
    // Calculate new orthographic scale
    float currentScale = m_viewport->getOrthoScale();
    float aspect = m_viewport->getAspectRatio();
    float scaleX = glm::abs(worldMax.x - worldMin.x) / (2.0f * aspect);
    float scaleY = glm::abs(worldMax.y - worldMin.y) / 2.0f;
    float newScale = std::max(scaleX, scaleY);

    // Apply new view
    m_viewport->setOrthographic(
        -newScale * aspect,
        newScale * aspect,
        -newScale,
        newScale,
        m_viewport->getNearPlane(),
        m_viewport->getFarPlane()
    );

    // Center view on selection
    glm::vec3 worldCenter = m_viewport->screenToRay(center).origin;
    glm::vec3 currentPos = m_viewport->getEyePosition();
    glm::vec3 newPos = glm::vec3(worldCenter.x, worldCenter.y, currentPos.z);
    m_viewport->setView(
        newPos,
        glm::vec3(worldCenter.x, worldCenter.y, 0.0f),
        glm::vec3(0, 1, 0)
    );
}

void ViewportNavigationController::updateProjectionBasedMode() {
    if (!isValidModeForProjection(m_currentMode)) {
        setMode(getDefaultModeForProjection());
    }
}

bool ViewportNavigationController::isValidModeForProjection(
    NavigationMode mode
) const {
    ViewportProjection proj = m_viewport->getProjectionType();

    switch (mode) {
        case NavigationMode::Orbit:
        case NavigationMode::Walk:
        case NavigationMode::Fly:
            return proj == ViewportProjection::Perspective;

        case NavigationMode::PanZoom:
        case NavigationMode::Scroll:
        case NavigationMode::BoxZoom:
            return proj == ViewportProjection::Orthographic;

        case NavigationMode::Pan:
        case NavigationMode::Zoom:
            return true;  // Valid for both projections

        default:
            return false;
    }
}

NavigationMode ViewportNavigationController::getDefaultModeForProjection() const {
    return m_viewport->getProjectionType() == ViewportProjection::Perspective
        ? NavigationMode::Orbit
        : NavigationMode::PanZoom;
}

} // namespace Graphics
} // namespace RebelCAD
