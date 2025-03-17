#include <gtest/gtest.h>
#include "assembly/AssemblyTree.h"
#include <glm/gtx/vector_angle.hpp>

using namespace RebelCAD::Assembly;

class ExplodedViewTests : public ::testing::Test {
protected:
    AssemblyTree tree;
    
    void SetUp() override {
        // Create a simple assembly hierarchy for testing
        tree.addComponent("root", "Root");
        tree.addComponent("child1", "Child 1", "root");
        tree.addComponent("child2", "Child 2", "root");
        tree.addComponent("grandchild1", "Grandchild 1", "child1");
    }
};

TEST_F(ExplodedViewTests, DefaultState) {
    EXPECT_FALSE(tree.isExplodedViewEnabled());
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.0f);
}

TEST_F(ExplodedViewTests, EnableDisableExplodedView) {
    tree.setExplodedViewEnabled(true);
    EXPECT_TRUE(tree.isExplodedViewEnabled());

    tree.setExplodedViewEnabled(false);
    EXPECT_FALSE(tree.isExplodedViewEnabled());
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.0f);
}

TEST_F(ExplodedViewTests, ExplosionFactor) {
    tree.setExplodedViewEnabled(true);
    
    tree.setExplosionFactor(0.5f);
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.5f);

    // Test clamping
    tree.setExplosionFactor(-0.5f);
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.0f);

    tree.setExplosionFactor(1.5f);
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 1.0f);
}

TEST_F(ExplodedViewTests, ComponentExplodeOffset) {
    const glm::vec3 offset(1.0f, 2.0f, 3.0f);
    
    // Set and get offset
    EXPECT_TRUE(tree.setComponentExplodeOffset("child1", offset));
    auto result = tree.getComponentExplodeOffset("child1");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), offset);

    // Invalid component
    EXPECT_FALSE(tree.setComponentExplodeOffset("invalid", offset));
    EXPECT_FALSE(tree.getComponentExplodeOffset("invalid").has_value());
}

TEST_F(ExplodedViewTests, ExplodedPosition) {
    tree.setExplodedViewEnabled(true);
    tree.setExplosionFactor(1.0f);

    // Set offsets
    const glm::vec3 rootOffset(0.0f, 1.0f, 0.0f);
    const glm::vec3 child1Offset(1.0f, 0.0f, 0.0f);
    const glm::vec3 grandchild1Offset(0.0f, 0.0f, 1.0f);

    tree.setComponentExplodeOffset("root", rootOffset);
    tree.setComponentExplodeOffset("child1", child1Offset);
    tree.setComponentExplodeOffset("grandchild1", grandchild1Offset);

    // Test cumulative offsets
    auto pos = tree.getExplodedPosition("grandchild1");
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos.value(), rootOffset + child1Offset + grandchild1Offset);

    // Test with explosion factor
    tree.setExplosionFactor(0.5f);
    pos = tree.getExplodedPosition("grandchild1");
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos.value(), (rootOffset + child1Offset + grandchild1Offset) * 0.5f);

    // Test with exploded view disabled
    tree.setExplodedViewEnabled(false);
    pos = tree.getExplodedPosition("grandchild1");
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(pos.value(), glm::vec3(0.0f));
}

TEST_F(ExplodedViewTests, ResetExplodedView) {
    // Setup exploded view
    tree.setExplodedViewEnabled(true);
    tree.setExplosionFactor(0.5f);
    tree.setComponentExplodeOffset("child1", glm::vec3(1.0f));

    // Reset
    tree.resetExplodedView();

    // Verify state
    EXPECT_FALSE(tree.isExplodedViewEnabled());
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.0f);

    auto offset = tree.getComponentExplodeOffset("child1");
    ASSERT_TRUE(offset.has_value());
    EXPECT_EQ(offset.value(), glm::vec3(0.0f));
}

TEST_F(ExplodedViewTests, ClearResetsExplodedView) {
    // Setup exploded view
    tree.setExplodedViewEnabled(true);
    tree.setExplosionFactor(0.5f);
    tree.setComponentExplodeOffset("child1", glm::vec3(1.0f));

    // Clear tree
    tree.clear();

    // Verify state
    EXPECT_FALSE(tree.isExplodedViewEnabled());
    EXPECT_FLOAT_EQ(tree.getExplosionFactor(), 0.0f);
    EXPECT_TRUE(tree.isEmpty());
}
