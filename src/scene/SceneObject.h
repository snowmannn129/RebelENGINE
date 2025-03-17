#pragma once

#include <glm/glm.hpp>
#include "graphics/BoundingGeometry.h"

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Base class for all objects in the scene
 */
class SceneObject {
public:
    SceneObject() = default;
    virtual ~SceneObject() = default;

    /**
     * @brief Gets the object's bounding geometry for collision testing
     * @return Reference to bounding geometry
     */
    virtual const BoundingGeometry& getBoundingGeometry() const = 0;

    /**
     * @brief Gets the object's world-space transform
     * @return 4x4 transformation matrix
     */
    virtual glm::mat4 getWorldTransform() const = 0;

    /**
     * @brief Gets the object's position in world space
     * @return World position
     */
    virtual glm::vec3 getWorldPosition() const = 0;

    /**
     * @brief Updates the object's state
     */
    virtual void update() = 0;
};

} // namespace Graphics
} // namespace RebelCAD
