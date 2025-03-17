#pragma once

#include <vector>
#include <memory>
#include <array>
#include <glm/glm.hpp>
#include "graphics/SceneObject.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Manages spatial partitioning for efficient collision detection
 * 
 * Implements an octree-based spatial partitioning system to accelerate
 * spatial queries and collision detection operations.
 */
class SpatialPartitioning {
public:
    SpatialPartitioning();
    ~SpatialPartitioning() = default;

    /**
     * @brief Finds objects that potentially intersect with a ray-swept sphere
     * @param origin Ray origin
     * @param direction Ray direction
     * @param radius Sphere radius
     * @return Vector of potentially intersecting objects
     */
    std::vector<std::shared_ptr<SceneObject>> findIntersectingObjects(
        const glm::vec3& origin,
        const glm::vec3& direction,
        float radius) const;

    /**
     * @brief Finds objects within a radius of a point
     * @param position Center point
     * @param radius Search radius
     * @return Vector of nearby objects
     */
    std::vector<std::shared_ptr<SceneObject>> findNearbyObjects(
        const glm::vec3& position,
        float radius) const;

    /**
     * @brief Updates the spatial index with a new/modified object
     * @param object Object to update
     */
    void updateObject(std::shared_ptr<SceneObject> object);

    /**
     * @brief Removes an object from the spatial index
     * @param object Object to remove
     */
    void removeObject(std::shared_ptr<SceneObject> object);

    /**
     * @brief Rebuilds the entire spatial index
     * @param objects All objects to index
     */
    void rebuild(const std::vector<std::shared_ptr<SceneObject>>& objects);

private:
    struct OctreeNode;
    std::unique_ptr<OctreeNode> root_;

    /**
     * @brief Internal octree node structure
     */
    struct OctreeNode {
        glm::vec3 center;                    ///< Node center point
        float halfSize;                      ///< Half-width of node
        std::vector<std::shared_ptr<SceneObject>> objects;  ///< Objects in node
        std::array<std::unique_ptr<OctreeNode>, 8> children;  ///< Child nodes
        
        OctreeNode(const glm::vec3& c, float size) 
            : center(c), halfSize(size) {}
    };

    /**
     * @brief Creates octree node subdivisions as needed
     * @param node Node to subdivide
     */
    void subdivideNode(OctreeNode* node);

    /**
     * @brief Tests if a ray-swept sphere intersects an octree node
     * @param node Node to test
     * @param origin Ray origin
     * @param direction Ray direction
     * @param radius Sphere radius
     * @return True if potential intersection exists
     */
    bool rayIntersectsNode(const OctreeNode* node,
                          const glm::vec3& origin,
                          const glm::vec3& direction,
                          float radius) const;

    /**
     * @brief Tests if a sphere overlaps an octree node
     * @param node Node to test
     * @param position Sphere center
     * @param radius Sphere radius
     * @return True if overlap exists
     */
    bool sphereOverlapsNode(const OctreeNode* node,
                           const glm::vec3& position,
                           float radius) const;

    /**
     * @brief Recursively collects objects from intersecting nodes
     * @param node Current node
     * @param origin Ray origin
     * @param direction Ray direction
     * @param radius Sphere radius
     * @param results Output vector for results
     */
    void collectIntersectingObjects(const OctreeNode* node,
                                  const glm::vec3& origin,
                                  const glm::vec3& direction,
                                  float radius,
                                  std::vector<std::shared_ptr<SceneObject>>& results) const;

    /**
     * @brief Recursively collects objects from overlapping nodes
     * @param node Current node
     * @param position Sphere center
     * @param radius Sphere radius
     * @param results Output vector for results
     */
    void collectNearbyObjects(const OctreeNode* node,
                             const glm::vec3& position,
                             float radius,
                             std::vector<std::shared_ptr<SceneObject>>& results) const;
};

} // namespace Graphics
} // namespace RebelCAD
