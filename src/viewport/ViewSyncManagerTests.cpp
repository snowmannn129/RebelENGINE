#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "graphics/ViewSyncManager.h"

using namespace RebelCAD::Graphics;

class ViewSyncManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<ViewSyncManager>();
        viewport1 = std::make_unique<Viewport>(800, 600);
        viewport2 = std::make_unique<Viewport>(800, 600);
        viewport3 = std::make_unique<Viewport>(800, 600);
    }

    std::unique_ptr<ViewSyncManager> manager;
    std::unique_ptr<Viewport> viewport1;
    std::unique_ptr<Viewport> viewport2;
    std::unique_ptr<Viewport> viewport3;
};

// Test group creation and management
TEST_F(ViewSyncManagerTests, GroupManagement) {
    // Create group
    ViewSyncGroup* group1 = manager->createGroup("Group1");
    EXPECT_NE(group1, nullptr);
    EXPECT_EQ(group1->getName(), "Group1");

    // Try to create duplicate group
    ViewSyncGroup* duplicate = manager->createGroup("Group1");
    EXPECT_EQ(duplicate, nullptr);

    // Get existing group
    ViewSyncGroup* found = manager->getGroup("Group1");
    EXPECT_EQ(found, group1);

    // Get non-existent group
    EXPECT_EQ(manager->getGroup("NonExistent"), nullptr);

    // Remove group
    manager->removeGroup("Group1");
    EXPECT_EQ(manager->getGroup("Group1"), nullptr);
}

// Test viewport management within groups
TEST_F(ViewSyncManagerTests, ViewportManagement) {
    ViewSyncGroup* group = manager->createGroup("TestGroup");
    ASSERT_NE(group, nullptr);

    // Add viewports
    EXPECT_TRUE(manager->addToGroup("TestGroup", viewport1.get()));
    EXPECT_TRUE(manager->addToGroup("TestGroup", viewport2.get()));
    EXPECT_EQ(group->getViewportCount(), 2);

    // Try to add duplicate viewport
    EXPECT_FALSE(manager->addToGroup("TestGroup", viewport1.get()));
    EXPECT_EQ(group->getViewportCount(), 2);

    // Remove viewport
    manager->removeFromGroup("TestGroup", viewport1.get());
    EXPECT_EQ(group->getViewportCount(), 1);
    EXPECT_FALSE(group->containsViewport(viewport1.get()));
    EXPECT_TRUE(group->containsViewport(viewport2.get()));
}

// Test view state synchronization
TEST_F(ViewSyncManagerTests, StateSynchronization) {
    ViewSyncGroup* group = manager->createGroup("SyncGroup");
    ASSERT_NE(group, nullptr);

    manager->addToGroup("SyncGroup", viewport1.get());
    manager->addToGroup("SyncGroup", viewport2.get());

    // Create test state
    ViewState state;
    state.cameraPosition = glm::vec3(1, 2, 3);
    state.cameraTarget = glm::vec3(0, 0, 0);
    state.upVector = glm::vec3(0, 1, 0);
    state.projection = ViewportProjection::Perspective;
    state.fieldOfView = 45.0f;
    state.nearPlane = 0.1f;
    state.farPlane = 1000.0f;

    // Update group state
    manager->updateGroupState("SyncGroup", state);

    // Verify state propagation
    EXPECT_EQ(viewport1->getCameraPosition(), state.cameraPosition);
    EXPECT_EQ(viewport1->getCameraTarget(), state.cameraTarget);
    EXPECT_EQ(viewport2->getCameraPosition(), state.cameraPosition);
    EXPECT_EQ(viewport2->getCameraTarget(), state.cameraTarget);
}

// Test viewport group queries
TEST_F(ViewSyncManagerTests, ViewportGroupQueries) {
    manager->createGroup("Group1");
    manager->createGroup("Group2");
    manager->createGroup("Group3");

    manager->addToGroup("Group1", viewport1.get());
    manager->addToGroup("Group2", viewport1.get());
    manager->addToGroup("Group2", viewport2.get());
    manager->addToGroup("Group3", viewport2.get());

    // Check viewport group memberships
    auto groups1 = manager->getViewportGroups(viewport1.get());
    EXPECT_EQ(groups1.size(), 2);
    EXPECT_TRUE(std::find(groups1.begin(), groups1.end(), "Group1") != groups1.end());
    EXPECT_TRUE(std::find(groups1.begin(), groups1.end(), "Group2") != groups1.end());

    auto groups2 = manager->getViewportGroups(viewport2.get());
    EXPECT_EQ(groups2.size(), 2);
    EXPECT_TRUE(std::find(groups2.begin(), groups2.end(), "Group2") != groups2.end());
    EXPECT_TRUE(std::find(groups2.begin(), groups2.end(), "Group3") != groups2.end());

    auto groups3 = manager->getViewportGroups(viewport3.get());
    EXPECT_TRUE(groups3.empty());
}

// Test thread safety
TEST_F(ViewSyncManagerTests, ThreadSafety) {
    // Create initial groups
    manager->createGroup("Group1");
    manager->createGroup("Group2");

    // Perform concurrent operations
    const int numThreads = 4;
    std::vector<std::thread> threads;

    // Thread 1: Add/remove viewports
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            manager->addToGroup("Group1", viewport1.get());
            manager->removeFromGroup("Group1", viewport1.get());
        }
    });

    // Thread 2: Create/remove groups
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            std::string name = "TestGroup" + std::to_string(i);
            ViewSyncGroup* group = manager->createGroup(name);
            if (group) {
                manager->removeGroup(name);
            }
        }
    });

    // Thread 3: Update view states
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            ViewState state;
            state.cameraPosition = glm::vec3(i);
            manager->updateGroupState("Group1", state);
            manager->updateGroupState("Group2", state);
        }
    });

    // Thread 4: Query viewport groups
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; ++i) {
            manager->getViewportGroups(viewport1.get());
            manager->getViewportGroups(viewport2.get());
        }
    });

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify manager integrity
    EXPECT_NO_THROW(manager->clear());
}

// Test state comparison
TEST_F(ViewSyncManagerTests, StateComparison) {
    ViewState state1;
    state1.cameraPosition = glm::vec3(1, 2, 3);
    state1.cameraTarget = glm::vec3(0);
    state1.upVector = glm::vec3(0, 1, 0);
    state1.projection = ViewportProjection::Perspective;
    state1.fieldOfView = 45.0f;
    state1.orthoScale = 1.0f;
    state1.nearPlane = 0.1f;
    state1.farPlane = 1000.0f;

    ViewState state2 = state1;
    EXPECT_EQ(state1, state2);

    state2.cameraPosition = glm::vec3(2, 3, 4);
    EXPECT_NE(state1, state2);

    state2 = state1;
    state2.projection = ViewportProjection::Orthographic;
    EXPECT_NE(state1, state2);
}

// Test group clear operation
TEST_F(ViewSyncManagerTests, ClearOperation) {
    manager->createGroup("Group1");
    manager->createGroup("Group2");
    manager->addToGroup("Group1", viewport1.get());
    manager->addToGroup("Group2", viewport2.get());

    manager->clear();

    EXPECT_EQ(manager->getGroup("Group1"), nullptr);
    EXPECT_EQ(manager->getGroup("Group2"), nullptr);
    EXPECT_TRUE(manager->getViewportGroups(viewport1.get()).empty());
    EXPECT_TRUE(manager->getViewportGroups(viewport2.get()).empty());
}
