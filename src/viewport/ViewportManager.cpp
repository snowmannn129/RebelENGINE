#include "graphics/ViewportManager.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace RebelCAD {
namespace Graphics {

ViewportManager::ViewportManager(int width, int height)
    : m_layout(std::make_unique<ViewportLayout>(width, height))
    , m_syncManager(std::make_unique<ViewSyncManager>())
    , m_collisionSystem(std::make_unique<CameraCollision>())
    , m_activeScene(nullptr) {
    
    // Initialize standard camera presets
    m_cameraPresets["Front"] = CameraPreset("Front", ViewportView::Front);
    m_cameraPresets["Back"] = CameraPreset("Back", ViewportView::Back);
    m_cameraPresets["Top"] = CameraPreset("Top", ViewportView::Top);
    m_cameraPresets["Bottom"] = CameraPreset("Bottom", ViewportView::Bottom);
    m_cameraPresets["Left"] = CameraPreset("Left", ViewportView::Left);
    m_cameraPresets["Right"] = CameraPreset("Right", ViewportView::Right);
    m_cameraPresets["Isometric"] = CameraPreset("Isometric", ViewportView::Isometric);
}

ViewportManager::~ViewportManager() = default;

void ViewportManager::setDimensions(int width, int height) {
    m_layout->setDimensions(width, height);
}

void ViewportManager::applyLayoutPreset(const ViewportLayoutPreset& preset) {
    m_layout->applyPreset(preset);
}

ViewportLayoutPreset ViewportManager::saveLayoutPreset(const std::string& name) {
    auto preset = m_layout->saveAsPreset(name);
    m_layoutPresets[name] = preset;
    return preset;
}

Viewport* ViewportManager::getViewport(int row, int col) {
    return m_layout->getViewport(row, col);
}

bool ViewportManager::createSyncGroup(const std::string& name) {
    return m_syncManager->createGroup(name) != nullptr;
}

bool ViewportManager::addToSyncGroup(const std::string& groupName, Viewport* viewport) {
    return m_syncManager->addToGroup(groupName, viewport);
}

void ViewportManager::removeFromSyncGroup(const std::string& groupName, Viewport* viewport) {
    m_syncManager->removeFromGroup(groupName, viewport);
}

void ViewportManager::applyCameraPreset(Viewport* viewport, const CameraPreset& preset,
                                      const ViewTransitionConfig& transition) {
    if (!viewport) return;

    // Calculate target view state based on preset
    ViewState targetState;
    targetState.projection = preset.projection;
    targetState.fieldOfView = preset.fieldOfView;
    targetState.orthoScale = preset.orthoScale;
    targetState.nearPlane = preset.nearPlane;
    targetState.farPlane = preset.farPlane;

    // Calculate camera position based on view orientation
    const float distance = 10.0f; // Base distance from target
    glm::vec3 position(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);

    switch (preset.view) {
        case ViewportView::Front:
            position = glm::vec3(0.0f, 0.0f, distance);
            break;
        case ViewportView::Back:
            position = glm::vec3(0.0f, 0.0f, -distance);
            break;
        case ViewportView::Top:
            position = glm::vec3(0.0f, distance, 0.0f);
            up = glm::vec3(0.0f, 0.0f, -1.0f);
            break;
        case ViewportView::Bottom:
            position = glm::vec3(0.0f, -distance, 0.0f);
            up = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        case ViewportView::Left:
            position = glm::vec3(-distance, 0.0f, 0.0f);
            break;
        case ViewportView::Right:
            position = glm::vec3(distance, 0.0f, 0.0f);
            break;
        case ViewportView::Isometric:
            position = glm::vec3(distance) * glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));
            break;
        default:
            break;
    }

    targetState.cameraPosition = position;
    targetState.cameraTarget = glm::vec3(0.0f);
    targetState.upVector = up;

    startViewTransition(viewport, targetState, transition);
}

CameraPreset ViewportManager::saveCameraPreset(Viewport* viewport, const std::string& name) {
    if (!viewport) return CameraPreset();

    CameraPreset preset;
    preset.name = name;
    preset.view = ViewportView::Custom;
    preset.projection = ViewportProjection::Perspective; // Get from viewport
    preset.fieldOfView = 45.0f; // Get from viewport
    preset.orthoScale = 1.0f; // Get from viewport
    preset.nearPlane = viewport->getNearPlane();
    preset.farPlane = viewport->getFarPlane();

    m_cameraPresets[name] = preset;
    return preset;
}

ViewBookmark ViewportManager::createBookmark(Viewport* viewport, const std::string& name) {
    if (!viewport) return ViewBookmark();

    ViewBookmark bookmark(
        name,
        viewport->getEyePosition(),
        glm::vec3(0.0f), // Get target from viewport
        glm::vec3(0.0f, 1.0f, 0.0f) // Get up vector from viewport
    );

    m_bookmarks[name] = bookmark;
    return bookmark;
}

void ViewportManager::applyBookmark(Viewport* viewport, const ViewBookmark& bookmark,
                                  const ViewTransitionConfig& transition) {
    if (!viewport) return;

    ViewState targetState;
    targetState.cameraPosition = bookmark.cameraPosition;
    targetState.cameraTarget = bookmark.cameraTarget;
    targetState.upVector = bookmark.upVector;
    targetState.projection = bookmark.projection;
    targetState.fieldOfView = bookmark.fieldOfView;
    targetState.orthoScale = bookmark.orthoScale;
    targetState.nearPlane = viewport->getNearPlane();
    targetState.farPlane = viewport->getFarPlane();

    startViewTransition(viewport, targetState, transition);
}

std::vector<std::string> ViewportManager::getBookmarkNames() const {
    std::vector<std::string> names;
    names.reserve(m_bookmarks.size());
    for (const auto& [name, _] : m_bookmarks) {
        names.push_back(name);
    }
    return names;
}

ViewBookmark ViewportManager::getBookmark(const std::string& name) const {
    auto it = m_bookmarks.find(name);
    if (it != m_bookmarks.end()) {
        return it->second;
    }
    return ViewBookmark(); // Return empty bookmark if not found
}

void ViewportManager::deleteBookmark(const std::string& name) {
    m_bookmarks.erase(name);
}

std::vector<std::string> ViewportManager::getPresetNames() const {
    std::vector<std::string> names;
    names.reserve(m_cameraPresets.size());
    for (const auto& [name, _] : m_cameraPresets) {
        names.push_back(name);
    }
    return names;
}

CameraPreset ViewportManager::getCameraPreset(const std::string& name) const {
    auto it = m_cameraPresets.find(name);
    if (it != m_cameraPresets.end()) {
        return it->second;
    }
    return CameraPreset(); // Return empty preset if not found
}

std::vector<std::string> ViewportManager::getLayoutPresetNames() const {
    std::vector<std::string> names;
    names.reserve(m_layoutPresets.size());
    for (const auto& [name, _] : m_layoutPresets) {
        names.push_back(name);
    }
    return names;
}

ViewportLayoutPreset ViewportManager::getLayoutPreset(const std::string& name) const {
    auto it = m_layoutPresets.find(name);
    if (it != m_layoutPresets.end()) {
        return it->second;
    }
    return ViewportLayoutPreset(); // Return empty preset if not found
}

void ViewportManager::startViewTransition(Viewport* viewport, const ViewState& targetState,
                                        const ViewTransitionConfig& config) {
    if (!viewport) return;

    // Create transition state
    TransitionState state;
    state.startState.cameraPosition = viewport->getEyePosition();
    state.startState.cameraTarget = glm::vec3(0.0f); // Get from viewport
    state.startState.upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Get from viewport
    state.targetState = targetState;
    state.config = config;
    state.startTime = std::chrono::steady_clock::now();
    state.progress = 0.0f;

    m_activeTransitions[viewport] = state;
}

void ViewportManager::cancelTransition(Viewport* viewport) {
    if (!viewport) return;
    m_activeTransitions.erase(viewport);
}

bool ViewportManager::isTransitioning(Viewport* viewport) const {
    return m_activeTransitions.contains(viewport);
}

void ViewportManager::update() {
    updateTransitions();
    m_layout->updateAll();
}

glm::vec3 ViewportManager::resolveCollision(Viewport* viewport, const glm::vec3& targetPos) {
    if (!m_activeScene || !viewport) {
        return targetPos;
    }
    return m_collisionSystem->resolveCollision(*viewport, *m_activeScene, targetPos);
}

void ViewportManager::updateTransitions() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = m_activeTransitions.begin(); it != m_activeTransitions.end();) {
        auto& [viewport, state] = *it;
        
        // Calculate progress
        float elapsed = std::chrono::duration<float>(now - state.startTime).count();
        state.progress = elapsed / state.config.duration;
        
        if (state.progress >= 1.0f) {
            // Transition complete
            viewport->setView(
                state.targetState.cameraPosition,
                state.targetState.cameraTarget,
                state.targetState.upVector
            );
            it = m_activeTransitions.erase(it);
        } else {
            // Interpolate state
            float t = calculateEasing(state.progress, 
                                   state.config.easeInFactor,
                                   state.config.easeOutFactor);
            
            ViewState currentState = interpolateViewStates(state.startState,
                                                         state.targetState,
                                                         t);
            
            // Resolve any collisions during transition
            glm::vec3 resolvedPos = resolveCollision(viewport, currentState.cameraPosition);
            viewport->setView(
                resolvedPos,
                currentState.cameraTarget,
                currentState.upVector
            );
            
            ++it;
        }
    }
}

ViewState ViewportManager::interpolateViewStates(const ViewState& start,
                                               const ViewState& end,
                                               float t) {
    ViewState result;
    
    // Linear interpolation for position
    result.cameraPosition = glm::mix(start.cameraPosition, end.cameraPosition, t);
    result.cameraTarget = glm::mix(start.cameraTarget, end.cameraTarget, t);
    
    // Spherical interpolation for rotation
    glm::quat startRot = glm::quatLookAt(
        glm::normalize(start.cameraPosition - start.cameraTarget),
        start.upVector
    );
    glm::quat endRot = glm::quatLookAt(
        glm::normalize(end.cameraPosition - end.cameraTarget),
        end.upVector
    );
    glm::quat currentRot = glm::slerp(startRot, endRot, t);
    
    // Extract up vector from interpolated rotation
    result.upVector = currentRot * glm::vec3(0.0f, 1.0f, 0.0f);
    
    return result;
}

float ViewportManager::calculateEasing(float t, float easeIn, float easeOut) {
    // Cubic Bezier easing
    float t2 = t * t;
    float t3 = t2 * t;
    return (3.0f * t2 - 2.0f * t3) * (1.0f + (easeIn + easeOut) * 0.5f);
}

void ViewportManager::finalizeTransition(Viewport* viewport) {
    auto it = m_activeTransitions.find(viewport);
    if (it != m_activeTransitions.end()) {
        auto& [_, state] = *it;
        // Ensure final position respects collisions
        glm::vec3 resolvedPos = resolveCollision(viewport, state.targetState.cameraPosition);
        viewport->setView(
            resolvedPos,
            state.targetState.cameraTarget,
            state.targetState.upVector
        );
        m_activeTransitions.erase(it);
    }
}

} // namespace Graphics
} // namespace RebelCAD
