#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace rebel_cad {
namespace assembly {

/**
 * @brief Handles the positioning and manipulation of components in 3D space during assembly.
 * 
 * The ComponentPositioner class provides functionality for:
 * - Inserting new components into the assembly
 * - Interactive 3D positioning with real-time preview
 * - Snapping to other components' faces, edges, and vertices
 * - Collision detection during positioning
 */
class ComponentPositioner {
public:
    ComponentPositioner();
    ~ComponentPositioner() = default;

    /**
     * @brief Initializes a new component positioning operation
     * @param componentId Unique identifier of the component being positioned
     * @return True if positioning operation started successfully
     */
    bool startPositioning(uint64_t componentId);

    /**
     * @brief Updates the component's position during interactive manipulation
     * @param position New 3D position
     * @param rotation New rotation quaternion
     */
    void updatePosition(const glm::vec3& position, const glm::quat& rotation);

    /**
     * @brief Checks for potential collisions at current position
     * @return True if collision detected
     */
    bool checkCollision() const;

    /**
     * @brief Attempts to snap to nearby geometry
     * @param snapThreshold Distance threshold for snapping in model units
     * @return True if snapping occurred
     */
    bool trySnap(float snapThreshold = 0.01f);

    /**
     * @brief Finalizes the component position
     * @return True if position is valid and operation completed successfully
     */
    bool finalizePosition();

    /**
     * @brief Cancels the current positioning operation
     */
    void cancelPositioning();

    /**
     * @brief Gets the current position of the active component
     * @return Current 3D position
     */
    glm::vec3 getCurrentPosition() const { return m_currentPosition; }

    /**
     * @brief Gets the current rotation of the active component
     * @return Current rotation as quaternion
     */
    glm::quat getCurrentRotation() const { return m_currentRotation; }

private:
    uint64_t m_activeComponentId;
    glm::vec3 m_currentPosition;
    glm::quat m_currentRotation;
    bool m_isPositioning;

    /**
     * @brief Finds nearby geometry for potential snapping
     * @param radius Search radius in model units
     * @return Vector of potential snap points
     */
    std::vector<glm::vec3> findSnapPoints(float radius) const;

    /**
     * @brief Validates if current position is valid for placement
     * @return True if position is valid
     */
    bool validatePosition() const;
};

} // namespace assembly
} // namespace rebel_cad
