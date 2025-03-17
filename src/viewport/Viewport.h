#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Graphics/Frustum.h"
#include "Graphics/ViewportTypes.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a viewport for rendering and interaction
 * 
 * Handles coordinate transformations between screen space and world space,
 * manages camera matrices, and provides utilities for picking operations.
 */
class Viewport {
public:
    Viewport();
    ~Viewport() = default;

    /**
     * @brief Sets the viewport dimensions
     * @param x Left coordinate
     * @param y Bottom coordinate
     * @param width Viewport width
     * @param height Viewport height
     */
    void setViewport(int x, int y, int width, int height);

    /**
     * @brief Sets perspective projection matrix
     * @param fov Field of view in radians
     * @param aspect Aspect ratio (width/height)
     * @param nearPlane Near clip plane distance
     * @param farPlane Far clip plane distance
     */
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);

    /**
     * @brief Sets orthographic projection matrix
     * @param left Left plane distance
     * @param right Right plane distance
     * @param bottom Bottom plane distance
     * @param top Top plane distance
     * @param nearPlane Near clip plane distance
     * @param farPlane Far clip plane distance
     */
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

    /**
     * @brief Sets the view matrix
     * @param eye Camera position
     * @param target Look-at target
     * @param up Up vector
     */
    void setView(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);

    /**
     * @brief Converts screen coordinates to a world-space ray
     * @param screenPos Screen coordinates (x,y)
     * @return Ray direction in world space
     */
    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };
    Ray screenToRay(const glm::vec2& screenPos) const;

    /**
     * @brief Creates a frustum from a screen-space rectangle
     * @param rectMin Minimum screen coordinates
     * @param rectMax Maximum screen coordinates
     * @return Frustum in world space
     */
    Frustum screenRectToFrustum(const glm::vec2& rectMin, const glm::vec2& rectMax) const;

    /**
     * @brief Converts world coordinates to screen coordinates
     * @param worldPos World position
     * @return Screen coordinates (x,y)
     */
    glm::vec2 worldToScreen(const glm::vec3& worldPos) const;

    // Getters
    const glm::mat4& getViewMatrix() const { return viewMatrix_; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix_; }
    const glm::mat4& getViewProjectionMatrix() const { return viewProjectionMatrix_; }
    glm::vec3 getEyePosition() const { return eyePosition_; }
    glm::vec3 getTarget() const { return target_; }
    glm::vec3 getViewDirection() const { return glm::normalize(target_ - eyePosition_); }
    ViewportProjection getProjectionType() const { return projectionType_; }
    float getOrthoScale() const { return orthoScale_; }
    
    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    float getAspectRatio() const { return aspectRatio_; }
    float getNearPlane() const { return nearPlane_; }
    float getFarPlane() const { return farPlane_; }

private:
    // Viewport dimensions
    int x_;
    int y_;
    int width_;
    int height_;

    // Camera parameters
    float fov_;
    float aspectRatio_;
    float nearPlane_;
    float farPlane_;
    float orthoScale_;
    glm::vec3 eyePosition_;
    glm::vec3 target_;
    glm::vec3 up_;
    ViewportProjection projectionType_;

    // Matrices
    glm::mat4 viewMatrix_;
    glm::mat4 projectionMatrix_;
    glm::mat4 viewProjectionMatrix_;

    // Updates the combined view-projection matrix
    void updateViewProjectionMatrix();
};

} // namespace Graphics
} // namespace RebelCAD
