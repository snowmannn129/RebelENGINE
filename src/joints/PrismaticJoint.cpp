#include "assembly/PrismaticJoint.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace rebel_cad {
namespace assembly {

PrismaticJoint::PrismaticJoint(std::shared_ptr<Component> component1,
                             std::shared_ptr<Component> component2,
                             const glm::vec3& axis,
                             const glm::vec3& point)
    : m_component1(component1)
    , m_component2(component2)
    , m_axis(glm::normalize(axis))
    , m_referencePoint(point)
    , m_currentDisplacement(0.0)
    , m_minDistance(0.0)
    , m_maxDistance(0.0)
    , m_hasLimits(false)
{
    // Initialize components' relative positions
    update();
}

void PrismaticJoint::setLinearLimits(double minDistance, double maxDistance)
{
    m_minDistance = std::min(minDistance, maxDistance);
    m_maxDistance = std::max(minDistance, maxDistance);
    m_hasLimits = true;

    // Clamp current displacement to new limits
    if (m_hasLimits) {
        m_currentDisplacement = std::clamp(m_currentDisplacement, m_minDistance, m_maxDistance);
        update();
    }
}

double PrismaticJoint::getCurrentDisplacement() const
{
    return m_currentDisplacement;
}

bool PrismaticJoint::translate(double distance)
{
    double newDisplacement = m_currentDisplacement + distance;

    // Check if the new position would violate limits
    if (m_hasLimits) {
        if (newDisplacement < m_minDistance || newDisplacement > m_maxDistance) {
            return false;
        }
    }

    m_currentDisplacement = newDisplacement;
    update();
    return true;
}

void PrismaticJoint::update()
{
    if (!m_component1 || !m_component2) {
        return;
    }

    // Keep component1 fixed and move component2 along the translation axis
    glm::vec3 translation = m_axis * m_currentDisplacement;
    
    // Create translation matrix
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), translation);
    
    // Update component2's position while maintaining its orientation
    // The translation is applied relative to the reference point
    glm::vec3 originalPos = m_component2->getPosition();
    glm::vec3 relativePos = originalPos - m_referencePoint;
    glm::vec3 newPos = m_referencePoint + glm::vec3(transform * glm::vec4(relativePos, 1.0f));
    
    m_component2->setPosition(newPos);
}

} // namespace assembly
} // namespace rebel_cad
