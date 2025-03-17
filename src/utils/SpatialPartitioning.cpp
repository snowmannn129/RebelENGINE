#include "Graphics/SpatialPartitioning.h"
#include <algorithm>
#include <array>
#include <limits>
#include <functional>
#include <glm/gtx/norm.hpp>

namespace RebelCAD {
namespace Graphics {

namespace {
    // Constants for octree configuration
    constexpr int MAX_OBJECTS_PER_NODE = 8;
    constexpr int MAX_DEPTH = 8;
    constexpr float MIN_NODE_SIZE = 1.0f;
}

SpatialPartitioning::SpatialPartitioning() {
    // Create root node with large bounds
    root_ = std::make_unique<OctreeNode>(glm::vec3(0), 1000.0f);
}

std::vector<std::shared_ptr<SceneObject>> SpatialPartitioning::findIntersectingObjects(
    const glm::vec3& origin,
    const glm::vec3& direction,
    float radius) const {
    
    std::vector<std::shared_ptr<SceneObject>> results;
    if (root_) {
        collectIntersectingObjects(root_.get(), origin, direction, radius, results);
    }
    return results;
}

std::vector<std::shared_ptr<SceneObject>> SpatialPartitioning::findNearbyObjects(
    const glm::vec3& position,
    float radius) const {
    
    std::vector<std::shared_ptr<SceneObject>> results;
    if (root_) {
        collectNearbyObjects(root_.get(), position, radius, results);
    }
    return results;
}

void SpatialPartitioning::updateObject(std::shared_ptr<SceneObject> object) {
    if (!object || !root_) {
        return;
    }

    // Remove object from current nodes
    removeObject(object);

    // Get object bounds
    auto [min, max] = object->getBoundingGeometry().getAABB();
    
    // Insert into appropriate nodes
    std::function<void(OctreeNode*)> insert = [&](OctreeNode* node) {
        if (!node) return;

        // Add object to this node
        node->objects.push_back(object);

        // Check if node needs subdivision
        if (node->objects.size() > MAX_OBJECTS_PER_NODE) {
            subdivideNode(node);
        }
    };

    insert(root_.get());
}

void SpatialPartitioning::removeObject(std::shared_ptr<SceneObject> object) {
    if (!object || !root_) {
        return;
    }

    std::function<void(OctreeNode*)> remove = [&](OctreeNode* node) {
        if (!node) return;

        // Remove from current node
        auto it = std::find(node->objects.begin(), node->objects.end(), object);
        if (it != node->objects.end()) {
            node->objects.erase(it);
        }

        // Remove from children
        for (auto& child : node->children) {
            if (child) {
                remove(child.get());
            }
        }
    };

    remove(root_.get());
}

void SpatialPartitioning::rebuild(const std::vector<std::shared_ptr<SceneObject>>& objects) {
    // Calculate bounds of all objects
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::lowest());

    for (const auto& obj : objects) {
        if (!obj) continue;
        auto [objMin, objMax] = obj->getBoundingGeometry().getAABB();
        min = glm::min(min, objMin);
        max = glm::max(max, objMax);
    }

    // Create new root node
    glm::vec3 center = (min + max) * 0.5f;
    float size = glm::length(max - min) * 0.5f;
    root_ = std::make_unique<OctreeNode>(center, size);

    // Insert all objects
    for (const auto& obj : objects) {
        if (obj) {
            updateObject(obj);
        }
    }
}

void SpatialPartitioning::subdivideNode(OctreeNode* node) {
    if (!node || node->halfSize < MIN_NODE_SIZE) {
        return;
    }

    float childSize = node->halfSize * 0.5f;
    glm::vec3 offset(childSize);

    // Create child nodes
    for (int i = 0; i < 8; ++i) {
        glm::vec3 childCenter = node->center;
        childCenter.x += ((i & 1) ? offset.x : -offset.x);
        childCenter.y += ((i & 2) ? offset.y : -offset.y);
        childCenter.z += ((i & 4) ? offset.z : -offset.z);

        node->children[i] = std::make_unique<OctreeNode>(childCenter, childSize);
    }

    // Redistribute objects to children
    for (const auto& obj : node->objects) {
        auto [min, max] = obj->getBoundingGeometry().getAABB();
        
        for (auto& child : node->children) {
            if (child) {
                glm::vec3 childMin = child->center - glm::vec3(child->halfSize);
                glm::vec3 childMax = child->center + glm::vec3(child->halfSize);
                
                // Check if object overlaps child node
                if (glm::all(glm::lessThanEqual(min, childMax)) &&
                    glm::all(glm::greaterThanEqual(max, childMin))) {
                    child->objects.push_back(obj);
                }
            }
        }
    }
}

bool SpatialPartitioning::rayIntersectsNode(const OctreeNode* node,
                                          const glm::vec3& origin,
                                          const glm::vec3& direction,
                                          float radius) const {
    glm::vec3 min = node->center - glm::vec3(node->halfSize + radius);
    glm::vec3 max = node->center + glm::vec3(node->halfSize + radius);

    // Ray-AABB intersection test
    glm::vec3 invDir = 1.0f / direction;
    glm::vec3 t1 = (min - origin) * invDir;
    glm::vec3 t2 = (max - origin) * invDir;

    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    return tNear <= tFar && tFar >= 0;
}

bool SpatialPartitioning::sphereOverlapsNode(const OctreeNode* node,
                                           const glm::vec3& position,
                                           float radius) const {
    glm::vec3 min = node->center - glm::vec3(node->halfSize);
    glm::vec3 max = node->center + glm::vec3(node->halfSize);

    // Find closest point on AABB to sphere center
    glm::vec3 closest = glm::clamp(position, min, max);

    // Check if closest point is within sphere radius
    return glm::length2(closest - position) <= radius * radius;
}

void SpatialPartitioning::collectIntersectingObjects(
    const OctreeNode* node,
    const glm::vec3& origin,
    const glm::vec3& direction,
    float radius,
    std::vector<std::shared_ptr<SceneObject>>& results) const {
    
    if (!node || !rayIntersectsNode(node, origin, direction, radius)) {
        return;
    }

    // Add objects from current node
    results.insert(results.end(), node->objects.begin(), node->objects.end());

    // Recurse into children
    for (const auto& child : node->children) {
        if (child) {
            collectIntersectingObjects(child.get(), origin, direction, radius, results);
        }
    }
}

void SpatialPartitioning::collectNearbyObjects(
    const OctreeNode* node,
    const glm::vec3& position,
    float radius,
    std::vector<std::shared_ptr<SceneObject>>& results) const {
    
    if (!node || !sphereOverlapsNode(node, position, radius)) {
        return;
    }

    // Add objects from current node
    results.insert(results.end(), node->objects.begin(), node->objects.end());

    // Recurse into children
    for (const auto& child : node->children) {
        if (child) {
            collectNearbyObjects(child.get(), position, radius, results);
        }
    }
}

} // namespace Graphics
} // namespace RebelCAD
