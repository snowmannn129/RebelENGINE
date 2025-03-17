#pragma once

#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

enum class RenderMode {
    Normal,
    Preview,
    Selected,
    Highlighted
};

/**
 * @brief Axis-Aligned Bounding Box structure
 * 
 * Represents a box in 3D space whose faces are aligned with the coordinate axes.
 * Used for collision detection, picking, and visibility testing.
 */
struct AABB {
    glm::vec3 min;  ///< Minimum point (smallest x, y, z coordinates)
    glm::vec3 max;  ///< Maximum point (largest x, y, z coordinates)

    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

    /**
     * @brief Transforms the AABB by a matrix
     * @param transform The transformation matrix
     * @return New AABB containing the transformed box
     */
    AABB transform(const glm::mat4& transform) const {
        // Get the matrix components for faster access
        glm::vec3 translation = glm::vec3(transform[3]);
        glm::vec3 scale = glm::vec3(
            glm::length(glm::vec3(transform[0])),
            glm::length(glm::vec3(transform[1])),
            glm::length(glm::vec3(transform[2]))
        );

        // Transform min and max points
        glm::vec3 newMin = min * scale + translation;
        glm::vec3 newMax = max * scale + translation;

        // Ensure min is actually minimum and max is maximum after transformation
        return AABB(
            glm::min(newMin, newMax),
            glm::max(newMin, newMax)
        );
    }

    /**
     * @brief Expands the AABB to include another AABB
     * @param other The AABB to include
     */
    void merge(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    /**
     * @brief Tests if a point is inside the AABB
     * @param point The point to test
     * @return true if the point is inside the AABB
     */
    bool contains(const glm::vec3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    /**
     * @brief Tests if this AABB intersects another AABB
     * @param other The AABB to test against
     * @return true if the AABBs intersect
     */
    bool intersects(const AABB& other) const {
        return min.x <= other.max.x && max.x >= other.min.x &&
               min.y <= other.max.y && max.y >= other.min.y &&
               min.z <= other.max.z && max.z >= other.min.z;
    }
};

} // namespace Graphics
} // namespace RebelCAD
