#pragma once

#include <array>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a view frustum for culling and selection
 * 
 * Defines a frustum using six planes (near, far, left, right, top, bottom)
 * Used for both view frustum culling and volume selection operations.
 */
class Frustum {
public:
    // Plane indices
    enum PlaneID {
        LEFT = 0,
        RIGHT,
        BOTTOM,
        TOP,
        NEAR,
        FAR,
        COUNT
    };

    /**
     * @brief Represents a plane using normal and distance
     */
    struct Plane {
        glm::vec3 normal;   // Plane normal (normalized)
        float distance;     // Distance from origin to plane along normal

        Plane() : normal(0.0f), distance(0.0f) {}
        Plane(const glm::vec3& n, float d) : normal(n), distance(d) {}

        /**
         * @brief Tests if a point is on the positive side of the plane
         * @param point Point to test
         * @return True if point is on positive side (in front) of plane
         */
        bool isOnPositiveSide(const glm::vec3& point) const {
            return glm::dot(normal, point) + distance > 0.0f;
        }
    };

    Frustum() = default;

    /**
     * @brief Constructs a frustum from view and projection matrices
     * @param viewProj Combined view-projection matrix
     */
    explicit Frustum(const glm::mat4& viewProj);

    /**
     * @brief Updates the frustum planes from a view-projection matrix
     * @param viewProj Combined view-projection matrix
     */
    void update(const glm::mat4& viewProj);

    /**
     * @brief Tests if a point is inside the frustum
     * @param point Point to test
     * @return True if point is inside frustum
     */
    bool containsPoint(const glm::vec3& point) const;

    /**
     * @brief Tests if a sphere intersects the frustum
     * @param center Sphere center
     * @param radius Sphere radius
     * @return True if sphere intersects frustum
     */
    bool intersectsSphere(const glm::vec3& center, float radius) const;

    /**
     * @brief Tests if an axis-aligned bounding box intersects the frustum
     * @param min Minimum corner of AABB
     * @param max Maximum corner of AABB
     * @return True if AABB intersects frustum
     */
    bool intersectsAABB(const glm::vec3& min, const glm::vec3& max) const;

    /**
     * @brief Gets a specific plane of the frustum
     * @param id Plane identifier
     * @return Reference to the plane
     */
    const Plane& getPlane(PlaneID id) const { return planes_[id]; }

    /**
     * @brief Gets all planes of the frustum
     * @return Array of frustum planes
     */
    const std::array<Plane, PlaneID::COUNT>& getPlanes() const { return planes_; }

private:
    std::array<Plane, PlaneID::COUNT> planes_;

    // Extracts a plane from the view-projection matrix
    static Plane extractPlane(const glm::mat4& viewProj, int row);
};

} // namespace Graphics
} // namespace RebelCAD
