#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Camera class for 3D viewport navigation and view management
 * 
 * Provides functionality for:
 * - Camera positioning and orientation
 * - View/projection matrix management
 * - View frustum calculations
 * - Distance calculations for LOD
 */
class ViewportCamera {
public:
    ViewportCamera();
    ~ViewportCamera() = default;

    /**
     * @brief Sets the camera position
     * @param position New camera position
     */
    void setPosition(const glm::vec3& position);

    /**
     * @brief Sets the camera target (look-at point)
     * @param target Point the camera looks at
     */
    void setTarget(const glm::vec3& target);

    /**
     * @brief Sets the camera up vector
     * @param up New up vector
     */
    void setUp(const glm::vec3& up);

    /**
     * @brief Sets the perspective projection parameters
     * @param fov Field of view in radians
     * @param aspect Aspect ratio (width/height)
     * @param nearPlane Near clipping plane distance
     * @param farPlane Far clipping plane distance
     */
    void setPerspective(float fov, float aspect, float nearPlane, float farPlane);

    /**
     * @brief Gets the view matrix
     * @return Current view matrix
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Gets the projection matrix
     * @return Current projection matrix
     */
    glm::mat4 getProjectionMatrix() const;

    /**
     * @brief Gets the view-projection matrix
     * @return Combined view-projection matrix
     */
    glm::mat4 getViewProjectionMatrix() const;

    /**
     * @brief Gets the camera position
     * @return Current camera position
     */
    glm::vec3 getPosition() const { return position_; }

    /**
     * @brief Gets the camera target
     * @return Current look-at point
     */
    glm::vec3 getTarget() const { return target_; }

    /**
     * @brief Gets the camera up vector
     * @return Current up vector
     */
    glm::vec3 getUp() const { return up_; }

    /**
     * @brief Gets the camera forward vector
     * @return Normalized direction the camera is looking
     */
    glm::vec3 getForward() const;

    /**
     * @brief Gets the camera right vector
     * @return Normalized right vector
     */
    glm::vec3 getRight() const;

    /**
     * @brief Calculates distance to a point in world space
     * @param point Point to calculate distance to
     * @return Distance from camera to point
     */
    float distanceTo(const glm::vec3& point) const;

    /**
     * @brief Calculates distance to a bounding sphere
     * @param center Sphere center
     * @param radius Sphere radius
     * @return Distance from camera to nearest point on sphere
     */
    float distanceToSphere(const glm::vec3& center, float radius) const;

    /**
     * @brief Updates the view frustum planes
     * Called internally when view or projection changes
     */
    void updateFrustum();

    /**
     * @brief Checks if a point is visible in the view frustum
     * @param point Point to test
     * @return true if point is visible
     */
    bool isPointVisible(const glm::vec3& point) const;

    /**
     * @brief Checks if a sphere is visible in the view frustum
     * @param center Sphere center
     * @param radius Sphere radius
     * @return true if any part of sphere is visible
     */
    bool isSphereVisible(const glm::vec3& center, float radius) const;

private:
    // Camera parameters
    glm::vec3 position_;
    glm::vec3 target_;
    glm::vec3 up_;

    // Projection parameters
    float fov_;
    float aspect_;
    float nearPlane_;
    float farPlane_;

    // Cached matrices
    mutable bool viewDirty_;
    mutable bool projDirty_;
    mutable glm::mat4 viewMatrix_;
    mutable glm::mat4 projMatrix_;
    mutable glm::mat4 viewProjMatrix_;

    // Frustum planes
    struct FrustumPlane {
        glm::vec4 equation;  // Plane equation (ax + by + cz + d = 0)
    };
    FrustumPlane frustumPlanes_[6];  // left, right, bottom, top, near, far

    /**
     * @brief Updates the view matrix if needed
     */
    void updateViewMatrix() const;

    /**
     * @brief Updates the projection matrix if needed
     */
    void updateProjMatrix() const;
};

} // namespace Graphics
} // namespace RebelCAD
