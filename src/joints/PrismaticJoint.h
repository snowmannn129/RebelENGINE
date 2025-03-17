#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "assembly/ComponentPositioner.h"

namespace rebel_cad {
namespace assembly {

/**
 * @brief PrismaticJoint class represents a linear sliding joint between two components
 * 
 * A prismatic joint, also known as a sliding joint, allows translation along a single axis
 * while constraining all other degrees of freedom. This joint is commonly used in mechanical
 * assemblies for creating linear slides, pistons, and other translational mechanisms.
 */
class PrismaticJoint {
public:
    /**
     * @brief Construct a new Prismatic Joint
     * 
     * @param component1 First component to be joined
     * @param component2 Second component to be joined
     * @param axis Translation axis vector (will be normalized)
     * @param point Reference point for the joint
     */
    PrismaticJoint(std::shared_ptr<Component> component1,
                   std::shared_ptr<Component> component2,
                   const glm::vec3& axis,
                   const glm::vec3& point);

    /**
     * @brief Set the linear travel limits of the joint
     * 
     * @param minDistance Minimum allowed distance in model units
     * @param maxDistance Maximum allowed distance in model units
     */
    void setLinearLimits(double minDistance, double maxDistance);

    /**
     * @brief Get the current displacement of the joint
     * 
     * @return Current displacement in model units
     */
    double getCurrentDisplacement() const;

    /**
     * @brief Translate the joint by a specified distance
     * 
     * @param distance Distance to translate by in model units
     * @return true if translation was successful, false if constrained by limits
     */
    bool translate(double distance);

    /**
     * @brief Update the joint constraints and component positions
     */
    void update();

    /**
     * @brief Get the translation axis of the joint
     * 
     * @return const glm::vec3& Normalized translation axis vector
     */
    const glm::vec3& getAxis() const { return m_axis; }

    /**
     * @brief Get the reference point of the joint
     * 
     * @return const glm::vec3& Reference point for the joint
     */
    const glm::vec3& getReferencePoint() const { return m_referencePoint; }

private:
    std::shared_ptr<Component> m_component1;
    std::shared_ptr<Component> m_component2;
    glm::vec3 m_axis;              // Normalized translation axis
    glm::vec3 m_referencePoint;    // Reference point for the joint
    double m_currentDisplacement;   // Current displacement in model units
    double m_minDistance;           // Minimum allowed distance
    double m_maxDistance;           // Maximum allowed distance
    bool m_hasLimits;              // Whether linear limits are enabled
};

} // namespace assembly
} // namespace rebel_cad
