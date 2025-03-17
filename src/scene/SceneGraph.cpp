#include "graphics/SceneGraph.h"
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

SceneGraph::SceneGraph() {
    // Initialize spatial index with reasonable default size
    spatialIndex_.rebuild(objects_);
}

void SceneGraph::addObject(std::shared_ptr<SceneObject> object) {
    if (!object) {
        return;
    }

    // Add to object list
    objects_.push_back(object);

    // Update spatial index
    spatialIndex_.updateObject(object);
}

void SceneGraph::removeObject(std::shared_ptr<SceneObject> object) {
    if (!object) {
        return;
    }

    // Remove from spatial index first
    spatialIndex_.removeObject(object);

    // Remove from object list
    auto it = std::find(objects_.begin(), objects_.end(), object);
    if (it != objects_.end()) {
        objects_.erase(it);
    }
}

void SceneGraph::update() {
    // Update all objects
    for (auto& object : objects_) {
        object->update();
    }

    // Rebuild spatial index if needed
    // Note: In a real implementation, you might want to track dirty objects
    // and only update those in the spatial index, rather than rebuilding
    // the entire structure every frame
    spatialIndex_.rebuild(objects_);
}

std::vector<std::shared_ptr<SceneNode>> SceneGraph::getSelectedNodes() const {
    std::vector<std::shared_ptr<SceneNode>> selectedNodes;
    for (const auto& object : objects_) {
        // Cast SceneObject to SceneNode if possible
        if (auto node = std::dynamic_pointer_cast<SceneNode>(object)) {
            if (node->isSelectable() && node->isSelected()) {
                selectedNodes.push_back(node);
            }
            // Check child nodes recursively
            auto children = node->getChildren();
            for (const auto& child : children) {
                if (child->isSelectable() && child->isSelected()) {
                    selectedNodes.push_back(child);
                }
            }
        }
    }
    return selectedNodes;
}

std::vector<std::shared_ptr<SceneNode>> SceneGraph::getAllNodes() const {
    std::vector<std::shared_ptr<SceneNode>> allNodes;
    for (const auto& object : objects_) {
        collectNodes(allNodes, object);
    }
    return allNodes;
}

std::shared_ptr<SceneNode> SceneGraph::findNode(uint64_t id) const {
    for (const auto& object : objects_) {
        if (auto node = std::dynamic_pointer_cast<SceneNode>(object)) {
            if (node->getId() == id) {
                return node;
            }
            // Check child nodes recursively
            auto children = node->getChildren();
            for (const auto& child : children) {
                if (child->getId() == id) {
                    return child;
                }
            }
        }
    }
    return nullptr;
}

void SceneGraph::collectNodes(std::vector<std::shared_ptr<SceneNode>>& nodes, 
                            const std::shared_ptr<SceneObject>& object) const {
    if (auto node = std::dynamic_pointer_cast<SceneNode>(object)) {
        nodes.push_back(node);
        // Recursively collect child nodes
        auto children = node->getChildren();
        for (const auto& child : children) {
            // Since child is already a SceneNode::Ptr, pass it directly
            collectNodes(nodes, std::static_pointer_cast<SceneObject>(child));
        }
    }
}

} // namespace Graphics
} // namespace RebelCAD
