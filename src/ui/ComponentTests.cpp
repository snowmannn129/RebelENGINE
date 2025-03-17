#include <gtest/gtest.h>
#include "core/Component.h"

using namespace RebelCAD::Core;

TEST(ComponentTests, Creation) {
    auto comp = Component::create("TestComponent");
    EXPECT_NE(comp, nullptr);
    EXPECT_EQ(comp->getName(), "TestComponent");
}

TEST(ComponentTests, ParentChildRelationship) {
    auto parent = Component::create("Parent");
    auto child1 = Component::create("Child1");
    auto child2 = Component::create("Child2");

    // Test adding children
    EXPECT_TRUE(parent->addChild(child1));
    EXPECT_TRUE(parent->addChild(child2));
    
    // Verify parent-child relationships
    EXPECT_EQ(parent->getChildren().size(), 2);
    EXPECT_EQ(child1->getParent().lock(), parent);
    EXPECT_EQ(child2->getParent().lock(), parent);

    // Test removing child
    EXPECT_TRUE(parent->removeChild(child1));
    EXPECT_EQ(parent->getChildren().size(), 1);
    EXPECT_TRUE(child1->getParent().expired());

    // Test removing from parent
    EXPECT_TRUE(child2->removeFromParent());
    EXPECT_EQ(parent->getChildren().size(), 0);
    EXPECT_TRUE(child2->getParent().expired());
}

TEST(ComponentTests, InvalidOperations) {
    auto comp1 = Component::create("Comp1");
    auto comp2 = Component::create("Comp2");

    // Test adding self as child
    EXPECT_FALSE(comp1->addChild(comp1));

    // Test adding null child
    EXPECT_FALSE(comp1->addChild(nullptr));

    // Test removing non-existent child
    EXPECT_FALSE(comp1->removeChild(comp2));

    // Test removing from non-existent parent
    EXPECT_FALSE(comp1->removeFromParent());
}

TEST(ComponentTests, ReparentingBehavior) {
    auto parent1 = Component::create("Parent1");
    auto parent2 = Component::create("Parent2");
    auto child = Component::create("Child");

    // Add to first parent
    EXPECT_TRUE(parent1->addChild(child));
    EXPECT_EQ(child->getParent().lock(), parent1);

    // Move to second parent
    EXPECT_TRUE(parent2->addChild(child));
    EXPECT_EQ(child->getParent().lock(), parent2);
    EXPECT_EQ(parent1->getChildren().size(), 0);
}

TEST(ComponentTests, NameManagement) {
    auto comp = Component::create("InitialName");
    EXPECT_EQ(comp->getName(), "InitialName");

    comp->setName("NewName");
    EXPECT_EQ(comp->getName(), "NewName");
}

TEST(ComponentTests, Lifecycle) {
    class TestComponent : public Component {
    public:
        static Ptr create() { return std::shared_ptr<TestComponent>(new TestComponent()); }
        bool wasInitialized = false;
        bool wasDestroyed = false;
        float lastDeltaTime = 0.0f;

    protected:
        TestComponent() : Component("TestComponent") {}

    public:
        void onInitialize() override { wasInitialized = true; }
        void onDestroy() override { wasDestroyed = true; }
        void onUpdate(float deltaTime) override { lastDeltaTime = deltaTime; }
    };

    auto comp = TestComponent::create();
    auto testComp = std::static_pointer_cast<TestComponent>(comp);
    
    // Test initialization
    testComp->onInitialize();
    EXPECT_TRUE(testComp->wasInitialized);

    // Test update
    testComp->onUpdate(0.16f);
    EXPECT_FLOAT_EQ(testComp->lastDeltaTime, 0.16f);

    // Test destruction
    testComp->onDestroy();
    EXPECT_TRUE(testComp->wasDestroyed);
}
