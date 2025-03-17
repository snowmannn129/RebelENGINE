#include <gtest/gtest.h>
#include "Graphics/CameraCollision.h"
#include "Graphics/SceneGraph.h"
#include "Graphics/Viewport.h"
#include "Graphics/TestGeometry.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace RebelCAD::Graphics;

class CameraCollisionTests : public ::testing::Test {
protected:
    void SetUp() override {
        viewport_.setPerspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);
        viewport_.setView(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
        
        // Create test scene with simple geometry
        auto box = std::make_shared<TestBox>(glm::vec3(0), glm::vec3(1));
        scene_.addObject(box);
    }

    CameraCollision collision_;
    Viewport viewport_;
    SceneGraph scene_;
};

TEST_F(CameraCollisionTests, DetectsCollisionWithBox) {
    // Test collision with box surface
    auto result = collision_.testCollision(viewport_, scene_);
    EXPECT_FALSE(result.hasCollision) << "Should not collide from initial position";

    // Move camera closer to box
    viewport_.setView(glm::vec3(0, 0, 0.5), glm::vec3(0), glm::vec3(0, 1, 0));
    result = collision_.testCollision(viewport_, scene_);
    EXPECT_TRUE(result.hasCollision) << "Should detect collision when too close";
    EXPECT_NEAR(result.penetrationDepth, 0.5f, 0.001f);
}

TEST_F(CameraCollisionTests, ResolvesCollision) {
    // Try to move camera inside box
    glm::vec3 targetPos(0, 0, 0);
    glm::vec3 resolvedPos = collision_.resolveCollision(viewport_, scene_, targetPos);
    
    // Should keep camera outside box at minimum distance
    float dist = glm::length(resolvedPos - glm::vec3(0));
    EXPECT_GT(dist, collision_.getConfig().minDistance) 
        << "Should maintain minimum distance from geometry";
}

TEST_F(CameraCollisionTests, PredictsCollision) {
    // Test movement path that would intersect box
    glm::vec3 targetPos(0, 0, -1);
    bool willCollide = collision_.predictCollision(viewport_, scene_, targetPos);
    EXPECT_TRUE(willCollide) << "Should predict collision with box";

    // Test movement path that avoids box
    targetPos = glm::vec3(2, 2, 5);
    willCollide = collision_.predictCollision(viewport_, scene_, targetPos);
    EXPECT_FALSE(willCollide) << "Should not predict collision when path is clear";
}

TEST_F(CameraCollisionTests, HandlesEdgeCases) {
    // Test with camera at origin
    viewport_.setView(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
    auto result = collision_.testCollision(viewport_, scene_);
    EXPECT_TRUE(result.hasCollision) << "Should detect collision at origin";

    // Test with camera far away
    viewport_.setView(glm::vec3(0, 0, 1000), glm::vec3(0), glm::vec3(0, 1, 0));
    result = collision_.testCollision(viewport_, scene_);
    EXPECT_FALSE(result.hasCollision) << "Should not detect collision when far away";
}

TEST_F(CameraCollisionTests, ConfigurationSettings) {
    CameraCollision::Config config;
    config.minDistance = 2.0f;
    config.probeRadius = 1.0f;
    collision_.setConfig(config);

    // Test with increased minimum distance
    viewport_.setView(glm::vec3(0, 0, 2.5), glm::vec3(0), glm::vec3(0, 1, 0));
    auto result = collision_.testCollision(viewport_, scene_);
    EXPECT_TRUE(result.hasCollision) 
        << "Should detect collision at configured minimum distance";
}

TEST_F(CameraCollisionTests, SmoothTransition) {
    // Test smooth position adjustment
    glm::vec3 startPos(0, 0, 5);
    glm::vec3 targetPos(0, 0, 0);
    
    viewport_.setView(startPos, glm::vec3(0), glm::vec3(0, 1, 0));
    glm::vec3 resolvedPos = collision_.resolveCollision(viewport_, scene_, targetPos);
    
    // Should smoothly stop before collision
    float distToTarget = glm::length(resolvedPos - glm::vec3(0));
    float distToStart = glm::length(resolvedPos - startPos);
    
    EXPECT_GT(distToTarget, collision_.getConfig().minDistance)
        << "Should maintain minimum distance";
    EXPECT_LT(distToStart, glm::length(startPos))
        << "Should move towards target when possible";
}

// Helper class for testing
class TestBox : public SceneObject {
public:
    TestBox(const glm::vec3& center, const glm::vec3& size) 
        : center_(center), size_(size) {}

    const BoundingGeometry& getBoundingGeometry() const override {
        return geometry_;
    }

    glm::mat4 getWorldTransform() const override {
        return glm::translate(glm::mat4(1.0f), center_);
    }

    glm::vec3 getWorldPosition() const override {
        return center_;
    }

    void update() override {}

private:
    glm::vec3 center_;
    glm::vec3 size_;
    BoxGeometry geometry_{center_, size_};
};
