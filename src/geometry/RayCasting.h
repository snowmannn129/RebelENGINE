#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Graphics/Ray.h"
#include "Graphics/SceneNode.h"
#include "Graphics/Frustum.h"

namespace RebelCAD {
namespace Graphics {


/**
 * @brief Result of a ray intersection test
 */
struct RayHitResult {
    SceneNode* node{nullptr};    // Hit object
    float distance{0.0f};        // Distance along ray to hit point
    glm::vec3 hitPoint{0.0f};    // Point of intersection in world space
    glm::vec3 hitNormal{0.0f};   // Surface normal at hit point
};

/**
 * @brief Handles ray casting and intersection testing for object picking
 * 
 * Provides functionality for casting rays into the scene and testing for
 * intersections with objects. Uses spatial partitioning for acceleration.
 */
class RayCasting {
public:
    RayCasting();
    ~RayCasting();

    /**
     * @brief Casts a ray and returns all intersected objects
     * @param rayOrigin Ray starting point in world space
     * @param rayDirection Normalized ray direction
     * @return Vector of hit results, sorted by distance
     */
    std::vector<RayHitResult> castRay(const glm::vec3& rayOrigin,
                                     const glm::vec3& rayDirection) const;

    /**
     * @brief Casts a ray from viewport coordinates
     * @param ray Ray data from viewport
     * @return Vector of hit results, sorted by distance
     */
    std::vector<RayHitResult> castRay(const struct Ray& ray) const;

    /**
     * @brief Tests for objects within a view frustum
     * @param frustum View frustum to test against
     * @return Vector of objects within frustum
     */
    std::vector<SceneNode*> queryFrustum(const Frustum& frustum) const;

private:
    // Implementation details will be added later
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Graphics
} // namespace RebelCAD
