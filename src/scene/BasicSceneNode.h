#pragma once

#include "graphics/SceneNode.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief A basic implementation of SceneNode for simple scene graph nodes
 * 
 * This class provides a concrete implementation of SceneNode that can be used
 * for basic scene organization without any specific rendering or geometry.
 */
class BasicSceneNode : public SceneNode {
public:
    using Ptr = std::shared_ptr<BasicSceneNode>;
    using WeakPtr = std::weak_ptr<BasicSceneNode>;

    explicit BasicSceneNode(const std::string& name = "BasicNode") : SceneNode(name) {}
    ~BasicSceneNode() override = default;

    // Override pure virtual methods with basic implementations
    void updateBoundingVolume() override {}
    bool intersectRay(const glm::vec3& rayOrigin, 
                     const glm::vec3& rayDirection,
                     float& distance) const override { return false; }
    bool intersectFrustum(const class Frustum& frustum) const override { return false; }
    void render() const override {}
};

} // namespace Graphics
} // namespace RebelCAD
