#pragma once

#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a ray in 3D space
 * 
 * Used for ray casting and intersection testing. A ray is defined by
 * an origin point and a direction vector.
 */
struct Ray {
    glm::vec3 origin{0.0f};      // Starting point of the ray
    glm::vec3 direction{0.0f};    // Normalized direction vector
    float tMin{0.0f};            // Minimum distance along ray
    float tMax{std::numeric_limits<float>::infinity()}; // Maximum distance along ray

    /**
     * @brief Constructs a ray with given origin and direction
     * @param o Ray origin point
     * @param d Ray direction (will be normalized)
     */
    Ray(const glm::vec3& o = glm::vec3(0.0f), const glm::vec3& d = glm::vec3(0.0f, 0.0f, -1.0f))
        : origin(o)
        , direction(glm::normalize(d))
    {}

    /**
     * @brief Gets point along ray at given distance
     * @param t Distance along ray
     * @return Point in 3D space
     */
    glm::vec3 pointAt(float t) const {
        return origin + direction * t;
    }

    /**
     * @brief Checks if a distance is within ray bounds
     * @param t Distance to check
     * @return True if distance is within bounds
     */
    bool isValidT(float t) const {
        return t >= tMin && t <= tMax;
    }
};

} // namespace Graphics
} // namespace RebelCAD
