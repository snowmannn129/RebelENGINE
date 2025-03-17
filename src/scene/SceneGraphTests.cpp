#include <gtest/gtest.h>
#include "graphics/SceneGraph.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace RebelCAD::Graphics;

class SceneGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        sceneGraph = std::make_unique<SceneGraph>();
    }

    std::unique_ptr<SceneGraph> sceneGraph;
};

TEST_F(SceneGraphTest, CreateNode) {
    auto node = std::make_shared<SceneNode>("test");
    EXPECT_EQ(node->getName(), "test");
    EXPECT_FALSE(node->isSelected());
}

TEST_F(SceneGraphTest, AddRemoveNode) {
    auto node = std::make_shared<SceneNode>("test");
    sceneGraph->addNode(node);
    
    auto found = sceneGraph->findNode("test");
    EXPECT_EQ(found, node);
    
    sceneGraph->removeNode(node);
    found = sceneGraph->findNode("test");
    EXPECT_EQ(found, nullptr);
}

TEST_F(SceneGraphTest, HierarchyManagement) {
    auto parent = std::make_shared<SceneNode>("parent");
    auto child = std::make_shared<SceneNode>("child");
    
    sceneGraph->addNode(parent);
    sceneGraph->addNode(child, parent);
    
    EXPECT_EQ(parent->getChildren().size(), 1);
    EXPECT_EQ(parent->getChildren()[0], child);
    EXPECT_EQ(child->getParent().lock(), parent);
}

TEST_F(SceneGraphTest, TransformOperations) {
    auto node = std::make_shared<SceneNode>("test");
    sceneGraph->addNode(node);
    
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    glm::vec3 rotation(0.0f, glm::radians(90.0f), 0.0f);
    glm::vec3 scale(2.0f, 2.0f, 2.0f);
    
    node->setPosition(position);
    node->setRotation(rotation);
    node->setScale(scale);
    
    // Check world transform for root child
    auto worldTransform = node->getWorldTransform();
    glm::vec3 worldPos = glm::vec3(worldTransform[3]);
    EXPECT_FLOAT_EQ(worldPos.x, position.x);
    EXPECT_FLOAT_EQ(worldPos.y, position.y);
    EXPECT_FLOAT_EQ(worldPos.z, position.z);
}

TEST_F(SceneGraphTest, SelectionManagement) {
    auto node1 = std::make_shared<SceneNode>("node1");
    auto node2 = std::make_shared<SceneNode>("node2");
    
    sceneGraph->addNode(node1);
    sceneGraph->addNode(node2);
    
    node1->setSelected(true);
    node2->setSelected(true);
    
    auto selectedNodes = sceneGraph->getSelectedNodes();
    EXPECT_EQ(selectedNodes.size(), 2);
    
    sceneGraph->clearSelection();
    selectedNodes = sceneGraph->getSelectedNodes();
    EXPECT_TRUE(selectedNodes.empty());
    EXPECT_FALSE(node1->isSelected());
    EXPECT_FALSE(node2->isSelected());
}

TEST_F(SceneGraphTest, PickingOperation) {
    auto node = std::make_shared<SceneNode>("test");
    sceneGraph->addNode(node);
    
    // Since base SceneNode::intersect returns false, picking should return nullptr
    auto picked = sceneGraph->pick(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    EXPECT_EQ(picked, nullptr);
}

TEST_F(SceneGraphTest, UpdatePropagation) {
    auto parent = std::make_shared<SceneNode>("parent");
    auto child = std::make_shared<SceneNode>("child");
    
    sceneGraph->addNode(parent);
    sceneGraph->addNode(child, parent);
    
    parent->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
    child->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    
    sceneGraph->update();
    
    // Child's world position should be parent + local
    auto worldTransform = child->getWorldTransform();
    glm::vec3 worldPos = glm::vec3(worldTransform[3]);
    EXPECT_FLOAT_EQ(worldPos.x, 1.0f);
    EXPECT_FLOAT_EQ(worldPos.y, 1.0f);
    EXPECT_FLOAT_EQ(worldPos.z, 0.0f);
}

TEST_F(SceneGraphTest, NodeSerialization) {
    auto node = std::make_shared<SceneNode>("test_node");
    node->setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
    node->setRotation(glm::vec3(0.0f, glm::radians(90.0f), 0.0f));
    node->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
    node->setSelected(true);

    // Serialize node
    auto json = node->serialize();

    // Create new node and deserialize
    auto newNode = std::make_shared<SceneNode>();
    newNode->deserialize(json);

    // Verify properties
    EXPECT_EQ(newNode->getName(), "test_node");
    EXPECT_TRUE(newNode->isSelected());
    
    auto pos = glm::vec3(newNode->getLocalTransform()[3]);
    EXPECT_FLOAT_EQ(pos.x, 1.0f);
    EXPECT_FLOAT_EQ(pos.y, 2.0f);
    EXPECT_FLOAT_EQ(pos.z, 3.0f);
}

TEST_F(SceneGraphTest, SceneGraphSerialization) {
    // Create a test scene
    auto parent = std::make_shared<SceneNode>("parent");
    auto child1 = std::make_shared<SceneNode>("child1");
    auto child2 = std::make_shared<SceneNode>("child2");

    parent->setPosition(glm::vec3(1.0f, 0.0f, 0.0f));
    child1->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    child2->setPosition(glm::vec3(0.0f, 0.0f, 1.0f));

    sceneGraph->addNode(parent);
    sceneGraph->addNode(child1, parent);
    sceneGraph->addNode(child2, parent);

    // Serialize entire scene
    auto json = sceneGraph->serialize();

    // Create new scene graph and deserialize
    auto newSceneGraph = std::make_unique<SceneGraph>();
    newSceneGraph->deserialize(json);

    // Verify scene structure
    auto newParent = newSceneGraph->findNode("parent");
    EXPECT_NE(newParent, nullptr);
    EXPECT_EQ(newParent->getChildren().size(), 2);

    auto newChild1 = newSceneGraph->findNode("child1");
    auto newChild2 = newSceneGraph->findNode("child2");
    EXPECT_NE(newChild1, nullptr);
    EXPECT_NE(newChild2, nullptr);

    // Verify transforms were preserved
    auto parentPos = glm::vec3(newParent->getLocalTransform()[3]);
    EXPECT_FLOAT_EQ(parentPos.x, 1.0f);
    EXPECT_FLOAT_EQ(parentPos.y, 0.0f);
    EXPECT_FLOAT_EQ(parentPos.z, 0.0f);
}

TEST_F(SceneGraphTest, FileIO) {
    // Create a test scene
    auto node = std::make_shared<SceneNode>("test_node");
    node->setPosition(glm::vec3(1.0f, 2.0f, 3.0f));
    sceneGraph->addNode(node);

    // Save to file
    const std::string testFile = "test_scene.json";
    EXPECT_TRUE(sceneGraph->saveToFile(testFile));

    // Load into new scene graph
    auto newSceneGraph = std::make_unique<SceneGraph>();
    EXPECT_TRUE(newSceneGraph->loadFromFile(testFile));

    // Verify loaded scene
    auto loadedNode = newSceneGraph->findNode("test_node");
    EXPECT_NE(loadedNode, nullptr);
    
    auto pos = glm::vec3(loadedNode->getLocalTransform()[3]);
    EXPECT_FLOAT_EQ(pos.x, 1.0f);
    EXPECT_FLOAT_EQ(pos.y, 2.0f);
    EXPECT_FLOAT_EQ(pos.z, 3.0f);

    // Cleanup
    std::remove(testFile.c_str());
}

TEST_F(SceneGraphTest, SerializationErrorHandling) {
    // Test invalid version
    nlohmann::json invalidData;
    invalidData["version"] = "2.0";
    EXPECT_THROW(sceneGraph->deserialize(invalidData), std::runtime_error);

    // Test invalid file path
    EXPECT_FALSE(sceneGraph->loadFromFile("nonexistent_file.json"));
}
