#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "assembly/ComponentPositioner.h"

namespace rebel_cad {
namespace assembly {

/**
 * @brief RevoluteJoint class represents a rotational joint between two components
 * 
 * A revolute joint, also known as a pin or hinge joint, allows rotation about a single axis
 * while constraining all other degrees of freedom. This joint is commonly used in mechanical
 * assemblies for creating hinges, pivots, and other rotational mechanisms.
 */
class RevoluteJoint {
public:
    /**
     * @brief Construct a new Revolute Joint
     * 
     * @param component1 First component to be joined
     * @param component2 Second component to be joined
     * @param axis Rotation axis vector (will be normalized)
     * @param point Point on the rotation axis
     */
    RevoluteJoint(std::shared_ptr<Component> component1,
                  std::shared_ptr<Component> component2,
                  const glm::vec3& axis,
                  const glm::vec3& point);

    /**
     * @brief Set the angular limits of the joint
     * 
     * @param minAngle Minimum allowed angle in radians
     * @param maxAngle Maximum allowed angle in radians
     */
    void setAngularLimits(double minAngle, double maxAngle);

    /**
     * @brief Get the current angle of the joint
     * 
     * @return Current angle in radians
     */
    double getCurrentAngle() const;

    /**
     * @brief Rotate the joint by a specified angle
     * 
     * @param angle Angle to rotate by in radians
     * @return true if rotation was successful, false if constrained by limits
     */
    bool rotate(double angle);

    /**
     * @brief Update the joint constraints and component positions
     */
    void update();

    /**
     * @brief Get the rotation axis of the joint
     * 
     * @return const glm::vec3& Normalized rotation axis vector
     */
    const glm::vec3& getAxis() const { return m_axis; }

    /**
     * @brief Get the pivot point of the joint
     * 
     * @return const glm::vec3& Point on the rotation axis
     */
    const glm::vec3& getPivotPoint() const { return m_pivotPoint; }

private:
    std::shared_ptr<Component> m_component1;
    std::shared_ptr<Component> m_component2;
    glm::vec3 m_axis;          // Normalized rotation axis
    glm::vec3 m_pivotPoint;    // Point on rotation axis
    double m_currentAngle;   // Current rotation angle in radians
    double m_minAngle;       // Minimum allowed angle
    double m_maxAngle;       // Maximum allowed angle
    bool m_hasLimits;        // Whether angular limits are enabled
};

} // namespace assembly
} // namespace rebel_cad
