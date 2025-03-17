#pragma once

#include <string>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Defines the projection type for a viewport
 */
enum class ViewportProjection {
    Perspective,    ///< 3D perspective projection
    Orthographic,   ///< Orthographic (2D) projection
};

/**
 * @brief Defines standard view orientations
 */
enum class ViewportView {
    Isometric,      ///< Default isometric view
    Front,          ///< Front view (+Z)
    Back,           ///< Back view (-Z)
    Top,            ///< Top view (+Y)
    Bottom,         ///< Bottom view (-Y)
    Left,           ///< Left view (-X)
    Right,          ///< Right view (+X)
    Custom          ///< Custom view orientation
};

/**
 * @brief Represents a saved view bookmark
 */
struct ViewBookmark {
    std::string name;                ///< Bookmark identifier
    glm::vec3 cameraPosition;        ///< Camera position
    glm::vec3 cameraTarget;          ///< Look-at target
    glm::vec3 upVector;              ///< Up vector
    ViewportProjection projection;    ///< Projection type
    float fieldOfView;               ///< FOV for perspective projection
    float orthoScale;                ///< Scale for orthographic projection
    
    ViewBookmark() = default;
    ViewBookmark(const std::string& n, const glm::vec3& pos, const glm::vec3& target, 
                const glm::vec3& up, ViewportProjection proj = ViewportProjection::Perspective,
                float fov = 45.0f, float scale = 1.0f)
        : name(n), cameraPosition(pos), cameraTarget(target), upVector(up),
          projection(proj), fieldOfView(fov), orthoScale(scale) {}
};

/**
 * @brief Configuration for view transitions
 */
struct ViewTransitionConfig {
    float duration = 0.5f;           ///< Transition duration in seconds
    bool smoothRotation = true;      ///< Use smooth rotation interpolation
    bool smoothZoom = true;          ///< Use smooth zoom interpolation
    float easeInFactor = 0.3f;       ///< Easing factor for transition start
    float easeOutFactor = 0.3f;      ///< Easing factor for transition end
};

/**
 * @brief Camera preset configuration
 */
struct CameraPreset {
    std::string name;                ///< Preset identifier
    ViewportView view;               ///< Standard view orientation
    ViewportProjection projection;    ///< Projection type
    float fieldOfView;               ///< FOV for perspective projection
    float orthoScale;                ///< Scale for orthographic projection
    float nearPlane;                 ///< Near clip plane distance
    float farPlane;                  ///< Far clip plane distance
    
    CameraPreset() = default;
    CameraPreset(const std::string& n, ViewportView v, 
                ViewportProjection proj = ViewportProjection::Perspective,
                float fov = 45.0f, float scale = 1.0f,
                float near = 0.1f, float far = 1000.0f)
        : name(n), view(v), projection(proj), fieldOfView(fov),
          orthoScale(scale), nearPlane(near), farPlane(far) {}
};

/**
 * @brief Defines standard layout configurations
 */
enum class StandardLayoutType {
    Single,         ///< Single viewport
    Double,         ///< Two viewports side by side
    Triple,         ///< Three viewports (main + two smaller)
    Quad,           ///< Four equal viewports
    ThreeByTwo,     ///< Six viewports (3x2 grid)
    TwoByThree      ///< Six viewports (2x3 grid)
};

/**
 * @brief Complete view state for a viewport
 */
struct ViewState {
    glm::vec3 position;              ///< Camera position
    glm::vec3 target;                ///< Look-at target
    glm::vec3 upVector;              ///< Up vector
    ViewportProjection projection;    ///< Projection type
    float fieldOfView;               ///< FOV for perspective projection
    float orthoScale;                ///< Scale for orthographic projection
    float nearPlane;                 ///< Near clip plane distance
    float farPlane;                  ///< Far clip plane distance
    
    ViewState()
        : position(0.0f, 0.0f, 10.0f)
        , target(0.0f, 0.0f, 0.0f)
        , upVector(0.0f, 1.0f, 0.0f)
        , projection(ViewportProjection::Perspective)
        , fieldOfView(45.0f)
        , orthoScale(5.0f)
        , nearPlane(0.1f)
        , farPlane(1000.0f)
    {}
    
    ViewState(
        const glm::vec3& pos,
        const glm::vec3& tgt,
        const glm::vec3& up,
        ViewportProjection proj = ViewportProjection::Perspective,
        float fov = 45.0f,
        float scale = 5.0f,
        float near = 0.1f,
        float far = 1000.0f
    )
        : position(pos)
        , target(tgt)
        , upVector(up)
        , projection(proj)
        , fieldOfView(fov)
        , orthoScale(scale)
        , nearPlane(near)
        , farPlane(far)
    {}
};

} // namespace Graphics
} // namespace RebelCAD
