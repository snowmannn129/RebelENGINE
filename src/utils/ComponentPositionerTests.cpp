#include <gtest/gtest.h>
#include "assembly/ComponentPositioner.h"
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/epsilon.hpp>

using namespace rebel_cad::assembly;

class ComponentPositionerTest : public ::testing::Test {
protected:
    ComponentPositioner positioner;
    const uint64_t testComponentId = 12345;
    const float epsilon = 0.0001f;
};

TEST_F(ComponentPositionerTest, StartPositioning) {
    EXPECT_TRUE(positioner.startPositioning(testComponentId));
    // Should not allow starting another positioning operation while one is active
    EXPECT_FALSE(positioner.startPositioning(67890));
}

TEST_F(ComponentPositionerTest, UpdatePosition) {
    ASSERT_TRUE(positioner.startPositioning(testComponentId));
    
    glm::vec3 newPos(1.0f, 2.0f, 3.0f);
    glm::quat newRot = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    positioner.updatePosition(newPos, newRot);
    
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        positioner.getCurrentPosition(),
        newPos,
        epsilon
    )));
    
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        glm::vec4(positioner.getCurrentRotation()),
        glm::vec4(newRot),
        epsilon
    )));
}

TEST_F(ComponentPositionerTest, CancelPositioning) {
    ASSERT_TRUE(positioner.startPositioning(testComponentId));
    
    glm::vec3 newPos(1.0f, 2.0f, 3.0f);
    glm::quat newRot = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    positioner.updatePosition(newPos, newRot);
    positioner.cancelPositioning();
    
    // After canceling, should be able to start a new positioning operation
    EXPECT_TRUE(positioner.startPositioning(67890));
    
    // Position and rotation should be reset
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        positioner.getCurrentPosition(),
        glm::vec3(0.0f),
        epsilon
    )));
    
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        glm::vec4(positioner.getCurrentRotation()),
        glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
        epsilon
    )));
}

TEST_F(ComponentPositionerTest, FinalizePosition) {
    ASSERT_TRUE(positioner.startPositioning(testComponentId));
    
    glm::vec3 newPos(1.0f, 2.0f, 3.0f);
    glm::quat newRot = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    positioner.updatePosition(newPos, newRot);
    EXPECT_TRUE(positioner.finalizePosition());
    
    // After finalizing, should be able to start a new positioning operation
    EXPECT_TRUE(positioner.startPositioning(67890));
}

TEST_F(ComponentPositionerTest, TrySnapWithNoSnapPoints) {
    ASSERT_TRUE(positioner.startPositioning(testComponentId));
    
    glm::vec3 originalPos(1.0f, 2.0f, 3.0f);
    positioner.updatePosition(originalPos, glm::quat());
    
    // With no snap points implemented yet, should return false
    EXPECT_FALSE(positioner.trySnap(0.1f));
    
    // Position should remain unchanged
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        positioner.getCurrentPosition(),
        originalPos,
        epsilon
    )));
}

TEST_F(ComponentPositionerTest, CheckCollision) {
    ASSERT_TRUE(positioner.startPositioning(testComponentId));
    
    // With basic implementation, should always return false
    EXPECT_FALSE(positioner.checkCollision());
}
