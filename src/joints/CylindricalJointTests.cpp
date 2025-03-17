#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "assembly/CylindricalJoint.h"
#include "assembly/Component.h"

using namespace rebel_cad::assembly;

class CylindricalJointTest : public ::testing::Test {
protected:
    std::shared_ptr<Component> component1;
    std::shared_ptr<Component> component2;
    glm::vec3 axis;
    glm::vec3 position;
    
    void SetUp() override {
        component1 = std::make_shared<Component>();
        component2 = std::make_shared<Component>();
        axis = glm::vec3(0.0f, 1.0f, 0.0f); // Y-axis
        position = glm::vec3(0.0f, 0.0f, 0.0f); // Origin
    }
};

TEST_F(CylindricalJointTest, Construction) {
    ASSERT_NO_THROW({
        CylindricalJoint joint(component1, component2, axis, position);
    });
}

TEST_F(CylindricalJointTest, InvalidConstruction) {
    // Test with null components
    ASSERT_THROW({
        CylindricalJoint joint(nullptr, component2, axis, position);
    }, std::runtime_error);

    ASSERT_THROW({
        CylindricalJoint joint(component1, nullptr, axis, position);
    }, std::runtime_error);

    // Test with zero axis
    ASSERT_THROW({
        CylindricalJoint joint(component1, component2, glm::vec3(0.0f), position);
    }, std::runtime_error);
}

TEST_F(CylindricalJointTest, RotationAndTranslation) {
    CylindricalJoint joint(component1, component2, axis, position);
    
    // Test rotation
    float testAngle = glm::radians(90.0f);
    joint.SetRotationAngle(testAngle);
    ASSERT_FLOAT_EQ(joint.GetRotationAngle(), testAngle);
    
    // Test translation
    float testTranslation = 5.0f;
    joint.SetTranslation(testTranslation);
    ASSERT_FLOAT_EQ(joint.GetTranslation(), testTranslation);
    
    // Verify component transforms
    glm::mat4 transform2 = component2->GetTransform();
    
    // Extract position from transform
    glm::vec3 finalPos = glm::vec3(transform2[3]);
    ASSERT_FLOAT_EQ(finalPos.y, testTranslation); // Should be translated along Y axis
    
    // Verify rotation around Y axis
    glm::vec3 transformedX = glm::vec3(transform2[0]);
    ASSERT_NEAR(transformedX.x, 0.0f, 1e-6f);
    ASSERT_NEAR(transformedX.z, 1.0f, 1e-6f);
}

TEST_F(CylindricalJointTest, GettersAndSetters) {
    CylindricalJoint joint(component1, component2, axis, position);
    
    // Test axis getter
    glm::vec3 retrievedAxis = joint.GetAxis();
    ASSERT_FLOAT_EQ(retrievedAxis.x, axis.x);
    ASSERT_FLOAT_EQ(retrievedAxis.y, axis.y);
    ASSERT_FLOAT_EQ(retrievedAxis.z, axis.z);
    
    // Test position getter
    glm::vec3 retrievedPos = joint.GetPosition();
    ASSERT_FLOAT_EQ(retrievedPos.x, position.x);
    ASSERT_FLOAT_EQ(retrievedPos.y, position.y);
    ASSERT_FLOAT_EQ(retrievedPos.z, position.z);
}
