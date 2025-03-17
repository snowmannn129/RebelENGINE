#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "assembly/ComponentPositioner.h"

namespace rebel_cad {
namespace assembly {

/**
 * @brief Represents a cylindrical joint between two components in an assembly.
 * 
 * A cylindrical joint allows both rotation around and translation along a common axis
 * between two components. This joint has 2 degrees of freedom:
 * 1. Rotation around the axis
 * 2. Translation along the axis
 */
class CylindricalJoint {
public:
    /**
     * @brief Constructs a cylindrical joint between two components
     * @param component1 First component to be joined
     * @param component2 Second component to be joined
     * @param axis The axis of rotation and translation
     * @param position The position of the joint in world space
     */
    CylindricalJoint(
        std::shared_ptr<Component> component1,
        std::shared_ptr<Component> component2,
        const glm::vec3& axis,
        const glm::vec3& position
    );

    /**
     * @brief Updates the joint constraints based on current component positions
     * @return true if constraints were successfully updated
     */
    bool UpdateConstraints();

    /**
     * @brief Sets the current rotation angle around the joint axis
     * @param angle Rotation angle in radians
     */
    void SetRotationAngle(float angle);

    /**
     * @brief Sets the current translation along the joint axis
     * @param distance Translation distance in world units
     */
    void SetTranslation(float distance);

    /**
     * @brief Gets the current rotation angle
     * @return Current rotation in radians
     */
    float GetRotationAngle() const;

    /**
     * @brief Gets the current translation
     * @return Current translation distance
     */
    float GetTranslation() const;

    /**
     * @brief Gets the joint axis
     * @return Normalized axis vector
     */
    glm::vec3 GetAxis() const;

    /**
     * @brief Gets the joint position
     * @return Position in world space
     */
    glm::vec3 GetPosition() const;

private:
    std::shared_ptr<Component> m_Component1;
    std::shared_ptr<Component> m_Component2;
    glm::vec3 m_Axis;
    glm::vec3 m_Position;
    float m_CurrentAngle;
    float m_CurrentTranslation;

    /**
     * @brief Validates the joint configuration
     * @return true if configuration is valid
     */
    bool ValidateConfiguration() const;

    /**
     * @brief Updates the transform matrices for both components
     */
    void UpdateComponentTransforms();
};

} // namespace assembly
} // namespace rebel_cad
