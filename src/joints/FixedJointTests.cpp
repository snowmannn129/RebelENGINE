#include <gtest/gtest.h>
#include "assembly/FixedJoint.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

using namespace rebel_cad::assembly;

class FixedJointTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a fixed joint between two components at origin
        joint = std::make_unique<FixedJoint>(1, 2, glm::vec3(0.0f));
    }

    std::unique_ptr<FixedJoint> joint;

    // Helper function to create a transform matrix
    glm::mat4 createTransform(const glm::vec3& position, const glm::quat& rotation) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = transform * glm::mat4_cast(rotation);
        return transform;
    }
};

TEST_F(FixedJointTest, Construction) {
    EXPECT_EQ(joint->getComponent1Id(), 1);
    EXPECT_EQ(joint->getComponent2Id(), 2);
    EXPECT_EQ(joint->getConnectionPoint(), glm::vec3(0.0f));
}

TEST_F(FixedJointTest, InvalidConstruction) {
    // Test that creating a joint between a component and itself throws
    EXPECT_THROW(FixedJoint(1, 1, glm::vec3(0.0f)), std::invalid_argument);
}

TEST_F(FixedJointTest, UpdateAndValidate) {
    // Set up initial transforms
    glm::mat4 component1Transform = createTransform(
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    glm::mat4 component2Transform = createTransform(
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );

    // Update joint with initial transforms
    EXPECT_TRUE(joint->update(component1Transform, component2Transform));
    EXPECT_TRUE(joint->validate());

    // Test that the relative transform maintains the relationship
    glm::mat4 relativeTransform = joint->getRelativeTransform();
    glm::mat4 expectedComponent2 = component1Transform * relativeTransform;
    
    // Verify positions match within tolerance
    glm::vec3 actualPos = glm::vec3(component2Transform[3]);
    glm::vec3 expectedPos = glm::vec3(expectedComponent2[3]);
    EXPECT_NEAR(glm::length(actualPos - expectedPos), 0.0f, 0.001f);
}

TEST_F(FixedJointTest, SerializationDeserialization) {
    // Set up a joint with non-trivial values
    FixedJoint originalJoint(42, 84, glm::vec3(1.0f, 2.0f, 3.0f));
    
    // Update with some transforms to set non-identity relative transform
    glm::mat4 transform1 = createTransform(
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    glm::mat4 transform2 = createTransform(
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(60.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    originalJoint.update(transform1, transform2);
    
    // Serialize
    std::string serialized = originalJoint.serialize();
    
    // Deserialize
    auto deserializedJoint = FixedJoint::deserialize(serialized);
    
    // Compare values
    EXPECT_EQ(deserializedJoint->getComponent1Id(), originalJoint.getComponent1Id());
    EXPECT_EQ(deserializedJoint->getComponent2Id(), originalJoint.getComponent2Id());
    EXPECT_EQ(deserializedJoint->getConnectionPoint(), originalJoint.getConnectionPoint());
    
    // Compare transforms (allowing for small floating point differences)
    glm::mat4 originalTransform = originalJoint.getRelativeTransform();
    glm::mat4 deserializedTransform = deserializedJoint->getRelativeTransform();
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(originalTransform[i][j], deserializedTransform[i][j], 0.0001f);
        }
    }
}

TEST_F(FixedJointTest, ConstraintValidation) {
    // Set up initial transforms
    glm::mat4 component1Transform = createTransform(
        glm::vec3(0.0f),
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
    );
    
    glm::mat4 component2Transform = createTransform(
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    // Establish initial constraint
    EXPECT_TRUE(joint->update(component1Transform, component2Transform));
    
    // Test valid movement (moving component1 should move component2 accordingly)
    glm::mat4 newComponent1Transform = createTransform(
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    glm::mat4 expectedComponent2Transform = newComponent1Transform * joint->getRelativeTransform();
    EXPECT_TRUE(joint->checkConstraint(newComponent1Transform, expectedComponent2Transform, 0.001f));
    
    // Test invalid movement (moving component2 independently should break constraint)
    glm::mat4 invalidComponent2Transform = createTransform(
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    );
    
    EXPECT_FALSE(joint->checkConstraint(newComponent1Transform, invalidComponent2Transform, 0.001f));
}
