#include <gtest/gtest.h>
#include "graphics/RayCasting.h"
#include "graphics/SceneGraph.h"
#include <glm/gtc/matrix_transform.hpp>

namespace RebelCAD {
namespace Graphics {
namespace Tests {

class RayCastingTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple scene graph
        sceneGraph = std::make_unique<SceneGraph>();
        
        // Create test nodes at known positions
        node1 = SceneNode::create("Node1");
        node1->setPosition(glm::vec3(0.0f, 0.0f, -10.0f));
        
        node2 = SceneNode::create("Node2");
        node2->setPosition(glm::vec3(5.0f, 0.0f, -10.0f));
        
        node3 = SceneNode::create("Node3");
        node3->setPosition(glm::vec3(-5.0f, 0.0f, -10.0f));
        
        // Add nodes to scene
        sceneGraph->addNode(node1);
        sceneGraph->addNode(node2);
        sceneGraph->addNode(node3);
        
        // Create raycast manager
        raycastManager = std::make_unique<RaycastManager>();
        
        // Set up spatial partitioning with test nodes
        std::vector<std::pair<AABB, SceneNode*>> nodes;
        AABB bounds1(glm::vec3(-1.0f), glm::vec3(1.0f));
        AABB bounds2(glm::vec3(4.0f, -1.0f, -11.0f), glm::vec3(6.0f, 1.0f, -9.0f));
        AABB bounds3(glm::vec3(-6.0f, -1.0f, -11.0f), glm::vec3(-4.0f, 1.0f, -9.0f));
        
        nodes.emplace_back(bounds1, node1.get());
        nodes.emplace_back(bounds2, node2.get());
        nodes.emplace_back(bounds3, node3.get());
        
        raycastManager->updateAccelerationStructure(nodes);
    }

    std::unique_ptr<SceneGraph> sceneGraph;
    std::unique_ptr<RaycastManager> raycastManager;
    SceneNode::Ptr node1, node2, node3;
};

TEST_F(RayCastingTests, BasicRayCast) {
    // Cast ray directly at node1
    Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    RaycastHit hit = raycastManager->castRay(ray);
    
    EXPECT_TRUE(hit.node != nullptr);
    EXPECT_EQ(hit.node, node1.get());
    EXPECT_NEAR(hit.distance, 10.0f, 0.001f);
}

TEST_F(RayCastingTests, RayMiss) {
    // Cast ray between nodes
    Ray ray(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    RaycastHit hit = raycastManager->castRay(ray);
    
    EXPECT_TRUE(hit.node == nullptr);
    EXPECT_EQ(hit.distance, std::numeric_limits<float>::max());
}

TEST_F(RayCastingTests, MultiRayCast) {
    std::vector<Ray> rays;
    // Ray at node1
    rays.emplace_back(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    // Ray at node2
    rays.emplace_back(glm::vec3(5.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    // Ray that misses
    rays.emplace_back(glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    
    std::vector<RaycastHit> hits = raycastManager->castRays(rays);
    
    EXPECT_EQ(hits.size(), 3);
    EXPECT_EQ(hits[0].node, node1.get());
    EXPECT_EQ(hits[1].node, node2.get());
    EXPECT_EQ(hits[2].node, nullptr);
}

TEST_F(RayCastingTests, ScreenToWorldRay) {
    // Create a test view-projection matrix
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
    glm::mat4 viewProj = proj * view;
    
    // Test center of screen
    Ray centerRay = RaycastManager::screenPointToRay(400, 300, 800, 600, viewProj);
    EXPECT_NEAR(centerRay.getDirection().x, 0.0f, 0.001f);
    EXPECT_NEAR(centerRay.getDirection().y, 0.0f, 0.001f);
    EXPECT_NEAR(centerRay.getDirection().z, -1.0f, 0.001f);
    
    // Test corners
    Ray topLeftRay = RaycastManager::screenPointToRay(0, 0, 800, 600, viewProj);
    EXPECT_GT(topLeftRay.getDirection().x, 0.0f);
    EXPECT_GT(topLeftRay.getDirection().y, 0.0f);
    
    Ray bottomRightRay = RaycastManager::screenPointToRay(800, 600, 800, 600, viewProj);
    EXPECT_LT(bottomRightRay.getDirection().x, 0.0f);
    EXPECT_LT(bottomRightRay.getDirection().y, 0.0f);
}

TEST_F(RayCastingTests, MaxDistanceTest) {
    // Cast ray at node1 but with max distance less than node distance
    Ray ray(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    RaycastHit hit = raycastManager->castRay(ray, 5.0f);
    
    EXPECT_TRUE(hit.node == nullptr);
    EXPECT_EQ(hit.distance, std::numeric_limits<float>::max());
    
    // Now test with distance just beyond node
    hit = raycastManager->castRay(ray, 11.0f);
    EXPECT_TRUE(hit.node != nullptr);
    EXPECT_EQ(hit.node, node1.get());
}

TEST_F(RayCastingTests, FrontFaceTest) {
    // Test front face hit
    Ray frontRay(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    RaycastHit frontHit = raycastManager->castRay(frontRay);
    EXPECT_TRUE(frontHit.frontFace);
    
    // Test back face hit
    Ray backRay(glm::vec3(0.0f, 0.0f, -20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    RaycastHit backHit = raycastManager->castRay(backRay);
    EXPECT_FALSE(backHit.frontFace);
}

} // namespace Tests
} // namespace Graphics
} // namespace RebelCAD
