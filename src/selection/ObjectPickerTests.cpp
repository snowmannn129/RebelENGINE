#include <gtest/gtest.h>
#include "graphics/ObjectPicker.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace RebelCAD::Graphics;

class ObjectPickerTests : public ::testing::Test {
protected:
    void SetUp() override {
        sceneGraph = std::make_shared<SceneGraph>();
        picker = std::make_unique<ObjectPicker>(sceneGraph);
        
        // Setup test view and projection matrices
        viewMatrix = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 10.0f),  // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at point
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
        
        projMatrix = glm::perspective(
            glm::radians(45.0f),  // FOV
            800.0f / 600.0f,      // Aspect ratio
            0.1f,                 // Near plane
            100.0f               // Far plane
        );
        
        // Create test nodes
        createTestScene();
    }
    
    void createTestScene() {
        // Create a cube at origin
        auto cube = std::make_shared<SceneNode>("Cube");
        cube->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        sceneGraph->addNode(cube);
        
        // Create a sphere to the right
        auto sphere = std::make_shared<SceneNode>("Sphere");
        sphere->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));
        sceneGraph->addNode(sphere);
        
        // Create a cylinder above
        auto cylinder = std::make_shared<SceneNode>("Cylinder");
        cylinder->setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
        sceneGraph->addNode(cylinder);
    }

    std::shared_ptr<SceneGraph> sceneGraph;
    std::unique_ptr<ObjectPicker> picker;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
};

TEST_F(ObjectPickerTests, PointPickingAtOrigin) {
    // Test picking at screen center (should hit the cube)
    glm::vec2 screenCenter(400.0f, 300.0f);
    auto picked = picker->pickAtPoint(screenCenter, viewMatrix, projMatrix);
    
    ASSERT_FALSE(picked.empty());
    EXPECT_EQ(picked[0]->getName(), "Cube");
}

TEST_F(ObjectPickerTests, BoxSelectionMultipleObjects) {
    // Test box selection that should encompass multiple objects
    glm::vec2 min(300.0f, 200.0f);
    glm::vec2 max(500.0f, 400.0f);
    
    auto picked = picker->pickInBox(min, max, viewMatrix, projMatrix);
    
    EXPECT_GT(picked.size(), 1);
}

TEST_F(ObjectPickerTests, LassoSelectionSingleObject) {
    // Test lasso selection around a single object
    std::vector<glm::vec2> lassoPoints = {
        glm::vec2(380.0f, 280.0f),
        glm::vec2(420.0f, 280.0f),
        glm::vec2(420.0f, 320.0f),
        glm::vec2(380.0f, 320.0f)
    };
    
    auto picked = picker->pickInLasso(lassoPoints, viewMatrix, projMatrix);
    
    ASSERT_EQ(picked.size(), 1);
}

TEST_F(ObjectPickerTests, SelectionFilter) {
    // Test selection filter
    picker->setSelectionFilter([](const SceneNode::Ptr& node) {
        return node->getName() == "Sphere";
    });
    
    // Try to pick the cube (should fail due to filter)
    glm::vec2 screenCenter(400.0f, 300.0f);
    auto picked = picker->pickAtPoint(screenCenter, viewMatrix, projMatrix);
    
    EXPECT_TRUE(picked.empty());
    
    // Try to pick the sphere (should succeed)
    glm::vec2 spherePos(500.0f, 300.0f);
    picked = picker->pickAtPoint(spherePos, viewMatrix, projMatrix);
    
    ASSERT_FALSE(picked.empty());
    EXPECT_EQ(picked[0]->getName(), "Sphere");
}

TEST_F(ObjectPickerTests, PickingPrecision) {
    // Test different picking precisions
    picker->setPickingPrecision(1.0f);  // Very precise
    
    glm::vec2 nearCube(402.0f, 301.0f);  // Just outside cube with default precision
    auto picked = picker->pickAtPoint(nearCube, viewMatrix, projMatrix);
    
    EXPECT_TRUE(picked.empty());
    
    // Increase precision, should now pick the cube
    picker->setPickingPrecision(5.0f);
    picked = picker->pickAtPoint(nearCube, viewMatrix, projMatrix);
    
    ASSERT_FALSE(picked.empty());
    EXPECT_EQ(picked[0]->getName(), "Cube");
}

TEST_F(ObjectPickerTests, EmptyScene) {
    // Test picking in empty scene
    auto emptyGraph = std::make_shared<SceneGraph>();
    auto emptyPicker = std::make_unique<ObjectPicker>(emptyGraph);
    
    glm::vec2 screenPos(400.0f, 300.0f);
    auto picked = emptyPicker->pickAtPoint(screenPos, viewMatrix, projMatrix);
    
    EXPECT_TRUE(picked.empty());
}

TEST_F(ObjectPickerTests, InvalidLassoPoints) {
    // Test lasso selection with invalid points
    std::vector<glm::vec2> twoPoints = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(100.0f, 100.0f)
    };
    
    auto picked = picker->pickInLasso(twoPoints, viewMatrix, projMatrix);
    EXPECT_TRUE(picked.empty());
}

TEST_F(ObjectPickerTests, DepthSorting) {
    // Create test nodes at different depths
    auto front = std::make_shared<SceneNode>("Front");
    front->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    sceneGraph->addNode(front);
    
    auto back = std::make_shared<SceneNode>("Back");
    back->setPosition(glm::vec3(0.0f, 0.0f, -5.0f));
    sceneGraph->addNode(back);
    
    // Pick at a point where both objects overlap
    glm::vec2 screenPos(400.0f, 300.0f);
    auto picked = picker->pickAtPoint(screenPos, viewMatrix, projMatrix);
    
    ASSERT_GE(picked.size(), 2);
    // Front object should be first in the list
    EXPECT_EQ(picked[0]->getName(), "Front");
}
