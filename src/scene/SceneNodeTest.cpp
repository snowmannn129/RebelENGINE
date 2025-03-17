#include <gtest/gtest.h>
#include "graphics/SceneNode.h"
#include "graphics/AABBBoundingGeometry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

using namespace RebelCAD::Graphics;

class SceneNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        node = std::make_shared<SceneNode>("TestNode");
    }

    std::shared_ptr<SceneNode> node;
};

TEST_F(SceneNodeTest, InitializesWithBoundingGeometry) {
    EXPECT_NO_THROW(node->getBoundingGeometry());
}

TEST_F(SceneNodeTest, UpdatesTransformPropagateToGeometry) {
    // Create a translation matrix
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 2.0f, 3.0f));
    node->setWorldTransform(transform);

    // Get the transformed AABB
    auto [min, max] = node->getBoundingGeometry().getAABB();

    // Original AABB is (-0.5, -0.5, -0.5) to (0.5, 0.5, 0.5)
    // After translation by (1, 2, 3), should be (0.5, 1.5, 2.5) to (1.5, 2.5, 3.5)
    EXPECT_NEAR(min.x, 0.5f, 0.0001f);
    EXPECT_NEAR(min.y, 1.5f, 0.0001f);
    EXPECT_NEAR(min.z, 2.5f, 0.0001f);
    EXPECT_NEAR(max.x, 1.5f, 0.0001f);
    EXPECT_NEAR(max.y, 2.5f, 0.0001f);
    EXPECT_NEAR(max.z, 3.5f, 0.0001f);
}

TEST_F(SceneNodeTest, IntersectionTest) {
    // Ray origin at (0,0,-2) pointing in +Z direction
    glm::vec3 origin(0.0f, 0.0f, -2.0f);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    float radius = 0.1f;

    // Should intersect with default AABB
    auto result = node->getBoundingGeometry().intersectSphere(origin, direction, radius);
    EXPECT_TRUE(result.hasCollision);
    EXPECT_NEAR(result.distance, 1.5f, 0.0001f); // Distance to front face at -0.5
}

TEST_F(SceneNodeTest, NoIntersectionTest) {
    // Ray origin at (2,0,0) pointing in +X direction (parallel to box face)
    glm::vec3 origin(2.0f, 0.0f, 0.0f);
    glm::vec3 direction(1.0f, 0.0f, 0.0f);
    float radius = 0.1f;

    // Should not intersect with default AABB
    auto result = node->getBoundingGeometry().intersectSphere(origin, direction, radius);
    EXPECT_FALSE(result.hasCollision);
}

TEST_F(SceneNodeTest, ClosestPointTest) {
    // Point outside AABB
    glm::vec3 point(1.0f, 1.0f, 1.0f);

    auto [closest, normal] = node->getBoundingGeometry().findClosestPoint(point);

    // Closest point should be on the corner (0.5, 0.5, 0.5)
    EXPECT_NEAR(closest.x, 0.5f, 0.0001f);
    EXPECT_NEAR(closest.y, 0.5f, 0.0001f);
    EXPECT_NEAR(closest.z, 0.5f, 0.0001f);

    // Normal should point diagonally
    float invSqrt3 = 1.0f / std::sqrt(3.0f);
    EXPECT_NEAR(normal.x, invSqrt3, 0.0001f);
    EXPECT_NEAR(normal.y, invSqrt3, 0.0001f);
    EXPECT_NEAR(normal.z, invSqrt3, 0.0001f);
}

TEST_F(SceneNodeTest, TransformHierarchyTest) {
    auto child = std::make_shared<SceneNode>("ChildNode");
    node->addChild(child);

    // Move parent
    glm::mat4 parentTransform = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    node->setWorldTransform(parentTransform);

    // Move child relative to parent
    glm::mat4 childLocalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    child->setLocalTransform(childLocalTransform);

    // Child's AABB should be offset by both transforms
    auto [min, max] = child->getBoundingGeometry().getAABB();
    EXPECT_NEAR(min.x, 0.5f, 0.0001f); // Parent +1, then -0.5 for AABB
    EXPECT_NEAR(min.y, 0.5f, 0.0001f); // Parent +0, Child +1, then -0.5 for AABB
    EXPECT_NEAR(max.x, 1.5f, 0.0001f); // Parent +1, then +0.5 for AABB
    EXPECT_NEAR(max.y, 1.5f, 0.0001f); // Parent +0, Child +1, then +0.5 for AABB
}
