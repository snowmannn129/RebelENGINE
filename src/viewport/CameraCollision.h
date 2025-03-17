#pragma once

#include <glm/glm.hpp>
#include "Graphics/Viewport.h"
#include "Graphics/SceneGraph.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Handles camera collision detection and response
 * 
 * Prevents camera clipping through objects in the scene by detecting
 * potential collisions and adjusting camera position accordingly.
 */
class CameraCollision {
public:
    /**
     * @brief Configuration for collision detection
     */
    struct Config {
        float minDistance = 0.1f;        ///< Minimum allowed distance to objects
        float probeRadius = 0.5f;        ///< Radius of collision probe sphere
        float smoothingFactor = 0.3f;    ///< Smoothing factor for camera adjustment
        bool enablePrediction = true;    ///< Enable predictive collision detection
        int maxIterations = 5;           ///< Maximum iterations for collision resolution
    };

    /**
     * @brief Collision test result
     */
    struct CollisionResult {
        bool hasCollision = false;       ///< Whether a collision was detected
        glm::vec3 contactPoint;          ///< Point of collision
        glm::vec3 contactNormal;         ///< Surface normal at collision point
        float penetrationDepth = 0.0f;   ///< How far the camera penetrated the object
    };

    CameraCollision() = default;
    ~CameraCollision() = default;

    /**
     * @brief Sets the collision configuration
     * @param config New configuration settings
     */
    void setConfig(const Config& config) { config_ = config; }

    /**
     * @brief Gets the current configuration
     * @return Current configuration settings
     */
    const Config& getConfig() const { return config_; }

    /**
     * @brief Tests for collision between camera and scene
     * @param viewport Current viewport
     * @param scene Scene graph to test against
     * @return Collision test result
     */
    CollisionResult testCollision(const Viewport& viewport, const SceneGraph& scene) const;

    /**
     * @brief Adjusts camera position to resolve collision
     * @param viewport Viewport to adjust
     * @param scene Scene graph to test against
     * @param targetPos Desired camera position
     * @return Adjusted camera position that avoids collision
     */
    glm::vec3 resolveCollision(const Viewport& viewport, const SceneGraph& scene, 
                              const glm::vec3& targetPos) const;

    /**
     * @brief Predicts potential collisions along camera movement path
     * @param viewport Current viewport
     * @param scene Scene graph to test against
     * @param targetPos Target camera position
     * @param steps Number of prediction steps
     * @return True if collision is predicted
     */
    bool predictCollision(const Viewport& viewport, const SceneGraph& scene,
                         const glm::vec3& targetPos, int steps = 10) const;

private:
    Config config_;

    /**
     * @brief Performs sphere cast for collision detection
     * @param origin Ray origin
     * @param direction Ray direction
     * @param radius Sphere radius
     * @param scene Scene to test against
     * @return Collision result
     */
    CollisionResult sphereCast(const glm::vec3& origin, const glm::vec3& direction,
                             float radius, const SceneGraph& scene) const;

    /**
     * @brief Finds closest point on geometry to camera
     * @param cameraPos Camera position
     * @param scene Scene to test against
     * @return Closest point and normal
     */
    std::pair<glm::vec3, glm::vec3> findClosestPoint(const glm::vec3& cameraPos,
                                                     const SceneGraph& scene) const;

    /**
     * @brief Smoothly interpolates camera position for collision response
     * @param currentPos Current camera position
     * @param targetPos Desired camera position
     * @param collisionPoint Point of collision
     * @param normal Surface normal at collision
     * @return Smoothly adjusted camera position
     */
    glm::vec3 smoothAdjustPosition(const glm::vec3& currentPos,
                                  const glm::vec3& targetPos,
                                  const glm::vec3& collisionPoint,
                                  const glm::vec3& normal) const;
};

} // namespace Graphics
} // namespace RebelCAD
