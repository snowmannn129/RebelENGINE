#include <gtest/gtest.h>
#include "graphics/ViewportManager.h"
#include "graphics/ViewportLayout.h"
#include "graphics/ViewSyncManager.h"
#include "graphics/ViewportTypes.h"
#include "graphics/SceneGraph.h"
#include "graphics/TestGeometry.h"

using namespace RebelCAD::Graphics;

class ViewportSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<ViewportManager>(800, 600);
        scene = std::make_unique<SceneGraph>();
        
        // Add test geometry to scene
        auto box = std::make_shared<TestBox>(glm::vec3(0), glm::vec3(2.0f));
        scene->addObject(box);
        
        manager->setActiveScene(scene.get());
    }

    void TearDown() override {
        manager.reset();
        scene.reset();
    }

    std::unique_ptr<ViewportManager> manager;
    std::unique_ptr<SceneGraph> scene;
};

// Layout Management Tests
//----------------------

TEST_F(ViewportSystemTest, CreateSingleViewport) {
    // Create 1x1 layout with single viewport
    ViewportLayoutPreset preset("Single", 1, 1);
    preset.cells.push_back(ViewportCell(0, 0, 1, 1));
    preset.projections.push_back(ViewportProjection::Perspective);
    preset.views.push_back(ViewportView::Isometric);
    
    manager->applyLayoutPreset(preset);
    
    // Verify viewport was created
    auto viewport = manager->getViewport(0, 0);
    ASSERT_NE(viewport, nullptr);
}

TEST_F(ViewportSystemTest, CreateQuadViewport) {
    // Create 2x2 layout with four viewports
    ViewportLayoutPreset preset("Quad", 2, 2);
    
    // Add four cells
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            preset.cells.push_back(ViewportCell(row, col, 1, 1));
            preset.projections.push_back(ViewportProjection::Perspective);
            preset.views.push_back(ViewportView::Isometric);
        }
    }
    
    manager->applyLayoutPreset(preset);
    
    // Verify all viewports were created
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            auto viewport = manager->getViewport(row, col);
            ASSERT_NE(viewport, nullptr);
        }
    }
}

TEST_F(ViewportSystemTest, SaveAndLoadLayout) {
    // Create initial layout
    ViewportLayoutPreset original("Test", 2, 1);
    original.cells.push_back(ViewportCell(0, 0, 1, 1));
    original.cells.push_back(ViewportCell(1, 0, 1, 1));
    original.projections = {ViewportProjection::Perspective, ViewportProjection::Orthographic};
    original.views = {ViewportView::Front, ViewportView::Top};
    
    manager->applyLayoutPreset(original);
    
    // Save current layout
    auto saved = manager->saveLayoutPreset("Saved");
    
    // Verify saved layout matches original
    EXPECT_EQ(saved.rows, original.rows);
    EXPECT_EQ(saved.cols, original.cols);
    EXPECT_EQ(saved.cells.size(), original.cells.size());
    EXPECT_EQ(saved.projections.size(), original.projections.size());
    EXPECT_EQ(saved.views.size(), original.views.size());
}

// View Synchronization Tests
//--------------------------

TEST_F(ViewportSystemTest, CreateSyncGroup) {
    // Create two viewports
    ViewportLayoutPreset preset("Dual", 2, 1);
    preset.cells = {ViewportCell(0, 0), ViewportCell(1, 0)};
    preset.projections = {ViewportProjection::Perspective, ViewportProjection::Perspective};
    preset.views = {ViewportView::Front, ViewportView::Front};
    
    manager->applyLayoutPreset(preset);
    
    // Create sync group
    EXPECT_TRUE(manager->createSyncGroup("test_group"));
    
    // Add viewports to group
    auto v1 = manager->getViewport(0, 0);
    auto v2 = manager->getViewport(1, 0);
    
    EXPECT_TRUE(manager->addToSyncGroup("test_group", v1));
    EXPECT_TRUE(manager->addToSyncGroup("test_group", v2));
}

TEST_F(ViewportSystemTest, SyncViewState) {
    // Create two viewports
    ViewportLayoutPreset preset("Dual", 2, 1);
    preset.cells = {ViewportCell(0, 0), ViewportCell(1, 0)};
    preset.projections = {ViewportProjection::Perspective, ViewportProjection::Perspective};
    preset.views = {ViewportView::Front, ViewportView::Front};
    
    manager->applyLayoutPreset(preset);
    
    // Create sync group and add viewports
    manager->createSyncGroup("test_group");
    auto v1 = manager->getViewport(0, 0);
    auto v2 = manager->getViewport(1, 0);
    manager->addToSyncGroup("test_group", v1);
    manager->addToSyncGroup("test_group", v2);
    
    // Set view state on first viewport
    v1->setView(
        glm::vec3(1.0f, 2.0f, 3.0f),  // Position
        glm::vec3(0.0f),              // Target
        glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
    );
    
    // Verify second viewport synced
    const auto pos1 = v1->getEyePosition();
    const auto pos2 = v2->getEyePosition();
    EXPECT_EQ(pos1, pos2);
}

// Camera Collision Tests
//---------------------

TEST_F(ViewportSystemTest, CollisionDetection) {
    // Create viewport
    ViewportLayoutPreset preset("Single", 1, 1);
    preset.cells.push_back(ViewportCell(0, 0));
    preset.projections.push_back(ViewportProjection::Perspective);
    preset.views.push_back(ViewportView::Front);
    
    manager->applyLayoutPreset(preset);
    auto viewport = manager->getViewport(0, 0);
    
    // Initial camera state
    glm::vec3 initialPos(0.0f, 0.0f, 5.0f);
    glm::vec3 target(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    viewport->setView(initialPos, target, up);
    
    // Try to move camera inside box (should be prevented)
    ViewState targetState;
    targetState.cameraPosition = glm::vec3(0.0f); // Inside box
    targetState.cameraTarget = target;
    targetState.upVector = up;
    
    ViewTransitionConfig config;
    config.duration = 0.5f;
    
    manager->startViewTransition(viewport, targetState, config);
    
    // Let transition complete
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    manager->update();
    
    // Verify camera stayed outside box
    const auto finalPos = viewport->getEyePosition();
    float distToCenter = glm::length(finalPos);
    EXPECT_GT(distToCenter, 1.0f); // Box radius is 1.0
}

TEST_F(ViewportSystemTest, CollisionConfiguration) {
    // Set custom collision parameters
    CameraCollision::Config config;
    config.minDistance = 2.0f;
    config.probeRadius = 0.5f;
    config.smoothingFactor = 0.5f;
    manager->setCollisionConfig(config);
    
    // Create viewport
    ViewportLayoutPreset preset("Single", 1, 1);
    preset.cells.push_back(ViewportCell(0, 0));
    preset.projections.push_back(ViewportProjection::Perspective);
    preset.views.push_back(ViewportView::Front);
    
    manager->applyLayoutPreset(preset);
    auto viewport = manager->getViewport(0, 0);
    
    // Try to move camera close to box
    ViewState targetState;
    targetState.cameraPosition = glm::vec3(1.5f, 0.0f, 0.0f);
    targetState.cameraTarget = glm::vec3(0.0f);
    targetState.upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    
    ViewTransitionConfig transConfig;
    transConfig.duration = 0.5f;
    
    manager->startViewTransition(viewport, targetState, transConfig);
    
    // Let transition complete
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    manager->update();
    
    // Verify minimum distance maintained
    const auto finalPos = viewport->getEyePosition();
    float distToCenter = glm::length(finalPos);
    EXPECT_GT(distToCenter, config.minDistance);
}

TEST_F(ViewportSystemTest, CollisionDuringOrbit) {
    // Create viewport
    ViewportLayoutPreset preset("Single", 1, 1);
    preset.cells.push_back(ViewportCell(0, 0));
    preset.projections.push_back(ViewportProjection::Perspective);
    preset.views.push_back(ViewportView::Front);
    
    manager->applyLayoutPreset(preset);
    auto viewport = manager->getViewport(0, 0);
    
    // Initial camera state
    glm::vec3 initialPos(3.0f, 0.0f, 0.0f);
    glm::vec3 target(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    viewport->setView(initialPos, target, up);
    
    // Orbit camera around box
    for (float angle = 0.0f; angle < glm::pi<float>(); angle += glm::pi<float>() / 4.0f) {
        glm::vec3 newPos(
            3.0f * cos(angle),
            0.0f,
            3.0f * sin(angle)
        );
        
        ViewState targetState;
        targetState.cameraPosition = newPos;
        targetState.cameraTarget = target;
        targetState.upVector = up;
        
        ViewTransitionConfig config;
        config.duration = 0.2f;
        
        manager->startViewTransition(viewport, targetState, config);
        
        // Let transition complete
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        manager->update();
        
        // Verify minimum distance maintained
        const auto finalPos = viewport->getEyePosition();
        float distToCenter = glm::length(finalPos);
        EXPECT_GT(distToCenter, 1.0f);
    }
}
