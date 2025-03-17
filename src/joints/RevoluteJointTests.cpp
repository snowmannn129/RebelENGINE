#include <gtest/gtest.h>
#include <memory>
#include <glm/gtc/constants.hpp>
#include "assembly/RevoluteJoint.h"
#include "core/Error.h"

using namespace rebel_cad::assembly;

class RevoluteJointTests : public ::testing::Test {
protected:
    void SetUp() override {
        component1 = std::make_shared<Component>();
        component2 = std::make_shared<Component>();
        
        // Default test joint: rotating around Y axis at origin
        axis = glm::vec3(0.0f, 1.0f, 0.0f);
        point = glm::vec3(0.0f);
        joint = std::make_unique<RevoluteJoint>(component1, component2, axis, point);
    }

    std::shared_ptr<Component> component1;
    std::shared_ptr<Component> component2;
    std::unique_ptr<RevoluteJoint> joint;
    glm::vec3 axis;
    glm::vec3 point;
};

TEST_F(RevoluteJointTests, Construction) {
    EXPECT_NO_THROW(RevoluteJoint(component1, component2, axis, point));
    
    // Test invalid components
    EXPECT_THROW(RevoluteJoint(nullptr, component2, axis, point), Error);
    EXPECT_THROW(RevoluteJoint(component1, nullptr, axis, point), Error);
    
    // Test invalid axis
    EXPECT_THROW(RevoluteJoint(component1, component2, glm::vec3(0.0f), point), Error);
}

TEST_F(RevoluteJointTests, AngularLimits) {
    // Test setting valid limits
    EXPECT_NO_THROW(joint->setAngularLimits(-glm::pi<double>() / 2.0, glm::pi<double>() / 2.0));
    
    // Test invalid limits
    EXPECT_THROW(joint->setAngularLimits(1.0, -1.0), Error);
    
    // Test rotation within limits
    joint->setAngularLimits(-1.0, 1.0);
    EXPECT_TRUE(joint->rotate(0.5));
    EXPECT_NEAR(joint->getCurrentAngle(), 0.5, 1e-6);
    
    // Test rotation beyond limits
    EXPECT_FALSE(joint->rotate(1.0)); // Would exceed max limit
    EXPECT_NEAR(joint->getCurrentAngle(), 0.5, 1e-6);
}

TEST_F(RevoluteJointTests, Rotation) {
    // Test basic rotation
    EXPECT_TRUE(joint->rotate(glm::pi<double>() / 2.0));
    EXPECT_NEAR(joint->getCurrentAngle(), glm::pi<double>() / 2.0, 1e-6);
    
    // Test angle normalization
    EXPECT_TRUE(joint->rotate(2.0 * glm::pi<double>()));
    EXPECT_NEAR(joint->getCurrentAngle(), glm::pi<double>() / 2.0, 1e-6);
    
    // Test negative rotation
    EXPECT_TRUE(joint->rotate(-glm::pi<double>()));
    EXPECT_NEAR(joint->getCurrentAngle(), -glm::pi<double>() / 2.0, 1e-6);
}

TEST_F(RevoluteJointTests, ComponentTransformation) {
    // Test that component1 remains fixed
    auto initialTransform1 = component1->getTransform();
    joint->rotate(glm::pi<double>() / 2.0);
    EXPECT_EQ(component1->getTransform(), initialTransform1);
    
    // Test that component2 is transformed
    auto initialTransform2 = component2->getTransform();
    joint->rotate(glm::pi<double>() / 2.0);
    EXPECT_NE(component2->getTransform(), initialTransform2);
}
