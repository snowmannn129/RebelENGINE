#include "assembly/RevoluteJoint.h"
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "core/Error.h"

namespace rebel_cad {
namespace assembly {

RevoluteJoint::RevoluteJoint(std::shared_ptr<Component> component1,
                           std::shared_ptr<Component> component2,
                           const glm::vec3& axis,
                           const glm::vec3& point)
    : m_component1(component1)
    , m_component2(component2)
    , m_axis(glm::normalize(axis))
    , m_pivotPoint(point)
    , m_currentAngle(0.0)
    , m_minAngle(-M_PI)
    , m_maxAngle(M_PI)
    , m_hasLimits(false)
{
    if (!component1 || !component2) {
        throw Error("RevoluteJoint: Invalid component pointers");
    }

    if (glm::length(axis) < 1e-6) {
        throw Error("RevoluteJoint: Invalid rotation axis");
    }
}

void RevoluteJoint::setAngularLimits(double minAngle, double maxAngle) {
    if (minAngle > maxAngle) {
        throw Error("RevoluteJoint: Invalid angular limits");
    }
    
    m_minAngle = minAngle;
    m_maxAngle = maxAngle;
    m_hasLimits = true;
    
    // Clamp current angle to new limits
    if (m_currentAngle < m_minAngle) {
        m_currentAngle = m_minAngle;
        update();
    } else if (m_currentAngle > m_maxAngle) {
        m_currentAngle = m_maxAngle;
        update();
    }
}

double RevoluteJoint::getCurrentAngle() const {
    return m_currentAngle;
}

bool RevoluteJoint::rotate(double angle) {
    double newAngle = m_currentAngle + angle;
    
    if (m_hasLimits) {
        if (newAngle < m_minAngle || newAngle > m_maxAngle) {
            return false;
        }
    }
    
    m_currentAngle = newAngle;
    
    // Normalize angle to [-π, π]
    while (m_currentAngle > M_PI) m_currentAngle -= 2.0 * M_PI;
    while (m_currentAngle < -M_PI) m_currentAngle += 2.0 * M_PI;
    
    update();
    return true;
}

void RevoluteJoint::update() {
    // Create rotation quaternion from axis and current angle
    glm::quat rotation = glm::angleAxis(
        static_cast<float>(m_currentAngle),
        m_axis
    );
    
    // Create rotation matrix
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    
    // Create translation matrices
    glm::mat4 toOrigin = glm::translate(
        glm::mat4(1.0f),
        -m_pivotPoint
    );
    glm::mat4 fromOrigin = glm::translate(
        glm::mat4(1.0f),
        m_pivotPoint
    );
    
    // Combine transformations:
    // 1. Translate to pivot point
    // 2. Apply rotation
    // 3. Translate back
    glm::mat4 transform = fromOrigin * rotationMatrix * toOrigin;
    
    // Apply transformation to component2 (component1 remains fixed)
    m_component2->setTransform(transform);
}

} // namespace assembly
} // namespace rebel_cad
