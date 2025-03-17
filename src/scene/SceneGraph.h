#pragma once

#include <vector>
#include <memory>
#include "graphics/SpatialPartitioning.h"
#include "graphics/SceneObject.h"
#include "graphics/SceneNode.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Manages the hierarchical scene structure and spatial organization
 */
class SceneGraph {
public:
    SceneGraph();
    ~SceneGraph() = default;

    /**
     * @brief Gets the spatial partitioning index
     * @return Reference to spatial index
     */
    const SpatialPartitioning& getSpatialIndex() const { return spatialIndex_; }
    SpatialPartitioning& getSpatialIndex() { return spatialIndex_; }

    /**
     * @brief Adds an object to the scene
     * @param object Object to add
     */
    void addObject(std::shared_ptr<SceneObject> object);

    /**
     * @brief Removes an object from the scene
     * @param object Object to remove
     */
    void removeObject(std::shared_ptr<SceneObject> object);

    /**
     * @brief Updates the scene graph and spatial index
     */
    void update();

    /**
     * @brief Gets all objects in the scene
     * @return Vector of scene objects
     */
    const std::vector<std::shared_ptr<SceneObject>>& getObjects() const { return objects_; }

    /**
     * @brief Gets all selected nodes in the scene
     * @return Vector of selected nodes
     */
    std::vector<std::shared_ptr<SceneNode>> getSelectedNodes() const;

    /**
     * @brief Gets all nodes in the scene graph
     * @return Vector of all scene nodes
     */
    std::vector<std::shared_ptr<SceneNode>> getAllNodes() const;

    /**
     * @brief Finds a node by its ID
     * @param id Node ID to find
     * @return Found node or nullptr if not found
     */
    std::shared_ptr<SceneNode> findNode(uint64_t id) const;

private:
    std::vector<std::shared_ptr<SceneObject>> objects_;
    SpatialPartitioning spatialIndex_;

    // Helper methods
    void collectNodes(std::vector<std::shared_ptr<SceneNode>>& nodes, const std::shared_ptr<SceneObject>& object) const;
};

} // namespace Graphics
} // namespace RebelCAD
