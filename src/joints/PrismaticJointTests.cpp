#include <gtest/gtest.h>
#include "assembly/PrismaticJoint.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

using namespace rebel_cad::assembly;

class PrismaticJointTest : public ::testing::Test {
protected:
    void SetUp() override {
        component1 = std::make_shared<Component>();
        component2 = std::make_shared<Component>();
        
        // Set initial positions
        component1->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        component2->setPosition(glm::vec3(1.0f, 1.0f, 1.0f));
    }

    std::shared_ptr<Component> component1;
    std::shared_ptr<Component> component2;
};

TEST_F(PrismaticJointTest, Construction) {
    glm::vec3 axis(1.0f, 0.0f, 0.0f); // X-axis translation
    glm::vec3 point(0.0f, 0.0f, 0.0f); // Origin reference point
    
    PrismaticJoint joint(component1, component2, axis, point);
    
    EXPECT_EQ(joint.getCurrentDisplacement(), 0.0);
    EXPECT_EQ(joint.getAxis(), glm::normalize(axis));
    EXPECT_EQ(joint.getReferencePoint(), point);
}

TEST_F(PrismaticJointTest, Translation) {
    glm::vec3 axis(1.0f, 0.0f, 0.0f); // X-axis translation
    glm::vec3 point(0.0f, 0.0f, 0.0f); // Origin reference point
    
    PrismaticJoint joint(component1, component2, axis, point);
    
    // Test translation without limits
    EXPECT_TRUE(joint.translate(2.0));
    EXPECT_FLOAT_EQ(joint.getCurrentDisplacement(), 2.0);
    
    // Verify component2's new position
    glm::vec3 expectedPos = glm::vec3(3.0f, 1.0f, 1.0f); // Original pos + translation along X
    EXPECT_EQ(component2->getPosition(), expectedPos);
}

TEST_F(PrismaticJointTest, LinearLimits) {
    glm::vec3 axis(1.0f, 0.0f, 0.0f);
    glm::vec3 point(0.0f, 0.0f, 0.0f);
    
    PrismaticJoint joint(component1, component2, axis, point);
    
    // Set limits
    joint.setLinearLimits(-1.0, 1.0);
    
    // Test translation within limits
    EXPECT_TRUE(joint.translate(0.5));
    EXPECT_FLOAT_EQ(joint.getCurrentDisplacement(), 0.5);
    
    // Test translation beyond limits
    EXPECT_FALSE(joint.translate(1.0)); // Would exceed max limit
    EXPECT_FLOAT_EQ(joint.getCurrentDisplacement(), 0.5); // Should remain unchanged
    
    // Test negative translation within limits
    EXPECT_TRUE(joint.translate(-1.0));
    EXPECT_FLOAT_EQ(joint.getCurrentDisplacement(), -0.5);
}

TEST_F(PrismaticJointTest, NonAxisAlignedTranslation) {
    glm::vec3 axis(1.0f, 1.0f, 0.0f); // 45-degree angle in XY plane
    glm::vec3 point(0.0f, 0.0f, 0.0f);
    
    PrismaticJoint joint(component1, component2, axis, point);
    
    // Translate along the diagonal
    EXPECT_TRUE(joint.translate(sqrt(2.0f))); // Length of unit vector along (1,1,0)
    
    // Verify component2's new position
    glm::vec3 expectedPos = glm::vec3(2.0f, 2.0f, 1.0f); // Original pos + translation along diagonal
    
    // Use approximate equality for floating point comparisons
    EXPECT_NEAR(component2->getPosition().x, expectedPos.x, 1e-5f);
    EXPECT_NEAR(component2->getPosition().y, expectedPos.y, 1e-5f);
    EXPECT_NEAR(component2->getPosition().z, expectedPos.z, 1e-5f);
}
