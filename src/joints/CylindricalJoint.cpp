#include "assembly/CylindricalJoint.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "core/Log.h"

namespace rebel_cad {
namespace assembly {

CylindricalJoint::CylindricalJoint(
    std::shared_ptr<Component> component1,
    std::shared_ptr<Component> component2,
    const glm::vec3& axis,
    const glm::vec3& position
)
    : m_Component1(component1)
    , m_Component2(component2)
    , m_Axis(glm::normalize(axis))
    , m_Position(position)
    , m_CurrentAngle(0.0f)
    , m_CurrentTranslation(0.0f)
{
    if (!ValidateConfiguration()) {
        LOG_ERROR("Invalid cylindrical joint configuration");
        throw std::runtime_error("Invalid cylindrical joint configuration");
    }
}

bool CylindricalJoint::UpdateConstraints() {
    if (!ValidateConfiguration()) {
        return false;
    }

    UpdateComponentTransforms();
    return true;
}

void CylindricalJoint::SetRotationAngle(float angle) {
    m_CurrentAngle = angle;
    UpdateComponentTransforms();
}

void CylindricalJoint::SetTranslation(float distance) {
    m_CurrentTranslation = distance;
    UpdateComponentTransforms();
}

float CylindricalJoint::GetRotationAngle() const {
    return m_CurrentAngle;
}

float CylindricalJoint::GetTranslation() const {
    return m_CurrentTranslation;
}

glm::vec3 CylindricalJoint::GetAxis() const {
    return m_Axis;
}

glm::vec3 CylindricalJoint::GetPosition() const {
    return m_Position;
}

bool CylindricalJoint::ValidateConfiguration() const {
    if (!m_Component1 || !m_Component2) {
        LOG_ERROR("CylindricalJoint: Invalid component pointers");
        return false;
    }

    // Check if axis is valid (non-zero)
    if (glm::length(m_Axis) < 1e-6f) {
        LOG_ERROR("CylindricalJoint: Invalid axis vector");
        return false;
    }

    return true;
}

void CylindricalJoint::UpdateComponentTransforms() {
    // Create rotation quaternion from axis and angle
    glm::quat rotation = glm::angleAxis(m_CurrentAngle, m_Axis);
    
    // Calculate translation vector along the axis
    glm::vec3 translation = m_Axis * m_CurrentTranslation;
    
    // Component 1 remains fixed at the joint position
    glm::mat4 transform1 = glm::translate(glm::mat4(1.0f), m_Position);
    m_Component1->SetTransform(transform1);
    
    // Component 2 gets both rotated and translated
    glm::mat4 transform2 = glm::translate(glm::mat4(1.0f), m_Position + translation) *
                          glm::mat4_cast(rotation);
    m_Component2->SetTransform(transform2);
}

} // namespace assembly
} // namespace rebel_cad
