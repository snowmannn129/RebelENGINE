#include "assembly/FixedJoint.h"
#include <nlohmann/json.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <stdexcept>

namespace rebel_cad {
namespace assembly {

FixedJoint::FixedJoint(uint64_t component1Id, uint64_t component2Id, const glm::vec3& connectionPoint)
    : m_component1Id(component1Id)
    , m_component2Id(component2Id)
    , m_connectionPoint(connectionPoint)
    , m_relativeTransform(1.0f) // Identity matrix
{
    if (component1Id == component2Id) {
        throw std::invalid_argument("Cannot create fixed joint between a component and itself");
    }
}

bool FixedJoint::update(const glm::mat4& component1Transform, const glm::mat4& component2Transform) 
{
    calculateRelativeTransform(component1Transform, component2Transform);
    return validate();
}

void FixedJoint::calculateRelativeTransform(const glm::mat4& component1Transform,
                                          const glm::mat4& component2Transform) 
{
    // Calculate the relative transform from component1 to component2
    glm::mat4 component1Inverse = glm::inverse(component1Transform);
    m_relativeTransform = component1Inverse * component2Transform;
}

bool FixedJoint::validate(float tolerance) const 
{
    // A fixed joint is valid if the connection point maintains its relative position
    // between the two components within the specified tolerance
    
    // Get the connection point in component1's local space
    glm::vec4 localPoint1 = glm::inverse(m_relativeTransform) * glm::vec4(m_connectionPoint, 1.0f);
    
    // Transform it back to world space using the relative transform
    glm::vec4 localPoint2 = m_relativeTransform * localPoint1;
    
    // Compare the transformed points
    glm::vec3 diff = glm::vec3(localPoint2) - m_connectionPoint;
    return glm::length(diff) <= tolerance;
}

bool FixedJoint::checkConstraint(const glm::mat4& component1Transform,
                               const glm::mat4& component2Transform,
                               float tolerance) const 
{
    // Get the expected position of component2 based on the relative transform
    glm::mat4 expectedComponent2Transform = component1Transform * m_relativeTransform;
    
    // Compare actual vs expected transforms
    glm::vec3 actualPos = glm::vec3(component2Transform[3]);
    glm::vec3 expectedPos = glm::vec3(expectedComponent2Transform[3]);
    
    // Check position difference
    float posDiff = glm::length(actualPos - expectedPos);
    if (posDiff > tolerance) {
        return false;
    }
    
    // Extract and compare rotations (using matrix decomposition)
    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat actualRotation, expectedRotation;
    
    glm::decompose(component2Transform, scale, actualRotation, translation, skew, perspective);
    glm::decompose(expectedComponent2Transform, scale, expectedRotation, translation, skew, perspective);
    
    float angleDiff = glm::angle(actualRotation * glm::inverse(expectedRotation));
    return angleDiff <= tolerance;
}

std::string FixedJoint::serialize() const 
{
    nlohmann::json j;
    j["component1Id"] = m_component1Id;
    j["component2Id"] = m_component2Id;
    j["connectionPoint"] = {
        {"x", m_connectionPoint.x},
        {"y", m_connectionPoint.y},
        {"z", m_connectionPoint.z}
    };
    
    // Serialize the 4x4 transform matrix
    std::vector<float> transform;
    transform.reserve(16);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            transform.push_back(m_relativeTransform[i][j]);
        }
    }
    j["relativeTransform"] = transform;
    
    return j.dump(4); // Pretty print with 4 spaces
}

std::unique_ptr<FixedJoint> FixedJoint::deserialize(const std::string& jsonData) 
{
    auto j = nlohmann::json::parse(jsonData);
    
    glm::vec3 connectionPoint(
        j["connectionPoint"]["x"].get<float>(),
        j["connectionPoint"]["y"].get<float>(),
        j["connectionPoint"]["z"].get<float>()
    );
    
    auto joint = std::make_unique<FixedJoint>(
        j["component1Id"].get<uint64_t>(),
        j["component2Id"].get<uint64_t>(),
        connectionPoint
    );
    
    // Deserialize the transform matrix
    auto transform = j["relativeTransform"].get<std::vector<float>>();
    if (transform.size() != 16) {
        throw std::runtime_error("Invalid transform matrix data");
    }
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            joint->m_relativeTransform[i][j] = transform[i * 4 + j];
        }
    }
    
    return joint;
}

} // namespace assembly
} // namespace rebel_cad
