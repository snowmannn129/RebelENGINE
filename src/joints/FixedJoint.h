#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace rebel_cad {
namespace assembly {

/**
 * @brief Represents a fixed joint constraint between two components in an assembly
 * 
 * A fixed joint completely locks all degrees of freedom (translation and rotation)
 * between two components, effectively making them behave as a single rigid body.
 * This is useful for creating permanent or semi-permanent connections in assemblies.
 */
class FixedJoint {
public:
    /**
     * @brief Creates a new fixed joint between two components
     * @param component1Id ID of the first component
     * @param component2Id ID of the second component
     * @param connectionPoint Point in world space where the joint is located
     */
    FixedJoint(uint64_t component1Id, uint64_t component2Id, const glm::vec3& connectionPoint);

    /**
     * @brief Gets the ID of the first connected component
     * @return Component ID
     */
    uint64_t getComponent1Id() const { return m_component1Id; }

    /**
     * @brief Gets the ID of the second connected component
     * @return Component ID
     */
    uint64_t getComponent2Id() const { return m_component2Id; }

    /**
     * @brief Gets the world space connection point
     * @return 3D connection point
     */
    glm::vec3 getConnectionPoint() const { return m_connectionPoint; }

    /**
     * @brief Gets the relative transform from component1 to component2
     * @return 4x4 transformation matrix
     */
    glm::mat4 getRelativeTransform() const { return m_relativeTransform; }

    /**
     * @brief Updates the joint's internal state based on component movements
     * @param component1Transform Current transform of component 1
     * @param component2Transform Current transform of component 2
     * @return True if the joint remains valid after update
     */
    bool update(const glm::mat4& component1Transform, const glm::mat4& component2Transform);

    /**
     * @brief Validates if the joint constraint is satisfied
     * @param tolerance Maximum allowed deviation in model units
     * @return True if the joint constraint is satisfied within tolerance
     */
    bool validate(float tolerance = 0.001f) const;

    /**
     * @brief Serializes the joint data to JSON
     * @return JSON string containing joint data
     */
    std::string serialize() const;

    /**
     * @brief Creates a FixedJoint from serialized data
     * @param jsonData JSON string containing joint data
     * @return New FixedJoint instance
     */
    static std::unique_ptr<FixedJoint> deserialize(const std::string& jsonData);

private:
    uint64_t m_component1Id;
    uint64_t m_component2Id;
    glm::vec3 m_connectionPoint;
    glm::mat4 m_relativeTransform;

    /**
     * @brief Calculates the relative transform between components
     * @param component1Transform Transform of first component
     * @param component2Transform Transform of second component
     */
    void calculateRelativeTransform(const glm::mat4& component1Transform, 
                                  const glm::mat4& component2Transform);

    /**
     * @brief Checks if the components' transforms satisfy the joint constraint
     * @param component1Transform Current transform of component 1
     * @param component2Transform Current transform of component 2
     * @param tolerance Maximum allowed deviation
     * @return True if constraint is satisfied
     */
    bool checkConstraint(const glm::mat4& component1Transform,
                        const glm::mat4& component2Transform,
                        float tolerance) const;
};

} // namespace assembly
} // namespace rebel_cad
