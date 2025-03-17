#include <gtest/gtest.h>
#include "Graphics/SelectionManager.h"
#include "Graphics/SceneNode.h"
#include "Graphics/Viewport.h"
#include <memory>

using namespace RebelCAD::Graphics;

// Mock scene node for testing
class MockSceneNode : public SceneNode {
public:
    MockSceneNode(const std::string& name = "") : SceneNode(name) {}

    void updateBoundingVolume() override {}

    bool intersectRay(const glm::vec3& rayOrigin, 
                     const glm::vec3& rayDirection,
                     float& distance) const override {
        // Simple sphere intersection test
        glm::vec3 center = getWorldTransform()[3];
        float radius = 1.0f;
        
        glm::vec3 oc = rayOrigin - center;
        float a = glm::dot(rayDirection, rayDirection);
        float b = 2.0f * glm::dot(oc, rayDirection);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) {
            return false;
        }
        
        distance = (-b - std::sqrt(discriminant)) / (2.0f * a);
        return distance > 0;
    }

    bool intersectFrustum(const Frustum& frustum) const override {
        // Simple sphere-frustum intersection test
        glm::vec3 center = getWorldTransform()[3];
        float radius = 1.0f;
        
        for (size_t i = 0; i < Frustum::PlaneID::COUNT; ++i) {
            const auto& plane = frustum.getPlane(static_cast<Frustum::PlaneID>(i));
            float distance = glm::dot(plane.normal, center) + plane.distance;
            if (distance < -radius) {
                return false;
            }
        }
        return true;
    }
};

class SelectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<SelectionManager>();
        viewport.setViewport(0, 0, 800, 600);
        viewport.setPerspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 1000.0f);
        viewport.setView(glm::vec3(0, 0, 10), glm::vec3(0), glm::vec3(0, 1, 0));
    }

    std::unique_ptr<SelectionManager> manager;
    Viewport viewport;
};

TEST_F(SelectionManagerTest, SingleObjectSelection) {
    auto node = std::make_shared<MockSceneNode>("test");
    node->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));

    // Select object at center of screen
    bool result = manager->selectAtPosition(viewport, glm::vec2(400, 300));
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->getSelectionCount(), 1);
}

TEST_F(SelectionManagerTest, MultiSelection) {
    auto node1 = std::make_shared<MockSceneNode>("test1");
    auto node2 = std::make_shared<MockSceneNode>("test2");
    node1->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0, 0)));
    node2->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0)));

    // Select first object
    bool result1 = manager->selectAtPosition(viewport, glm::vec2(300, 300), true);
    EXPECT_TRUE(result1);
    EXPECT_EQ(manager->getSelectionCount(), 1);

    // Select second object with multi-select
    bool result2 = manager->selectAtPosition(viewport, glm::vec2(500, 300), true);
    EXPECT_TRUE(result2);
    EXPECT_EQ(manager->getSelectionCount(), 2);
}

TEST_F(SelectionManagerTest, RectangleSelection) {
    auto node1 = std::make_shared<MockSceneNode>("test1");
    auto node2 = std::make_shared<MockSceneNode>("test2");
    node1->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-2, 0, 0)));
    node2->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(2, 0, 0)));

    // Select both objects with rectangle
    bool result = manager->selectInRect(viewport, 
                                      glm::vec2(200, 200),
                                      glm::vec2(600, 400));
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->getSelectionCount(), 2);
}

TEST_F(SelectionManagerTest, ClearSelection) {
    auto node = std::make_shared<MockSceneNode>("test");
    node->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));

    // Select and then clear
    manager->selectAtPosition(viewport, glm::vec2(400, 300));
    EXPECT_EQ(manager->getSelectionCount(), 1);

    bool result = manager->clearSelection();
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->getSelectionCount(), 0);
}

TEST_F(SelectionManagerTest, SelectionEvents) {
    bool eventReceived = false;
    auto subscription = manager->onSelectionChanged(
        [&eventReceived](const SelectionChangeEvent& event) {
            eventReceived = true;
            EXPECT_EQ(event.selected.size(), 1);
            EXPECT_EQ(event.deselected.size(), 0);
            EXPECT_FALSE(event.isMultiSelect);
        });

    auto node = std::make_shared<MockSceneNode>("test");
    node->setWorldTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));

    manager->selectAtPosition(viewport, glm::vec2(400, 300));
    EXPECT_TRUE(eventReceived);

    manager->unsubscribeFromSelectionChanges(subscription);
}
