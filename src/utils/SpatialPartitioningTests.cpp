#include <gtest/gtest.h>
#include "Graphics/SpatialPartitioning.h"
#include "Graphics/TestGeometry.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace RebelCAD::Graphics;

class SpatialPartitioningTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test objects at different positions
        auto box1 = std::make_shared<TestBox>(glm::vec3(0), glm::vec3(1));
        auto box2 = std::make_shared<TestBox>(glm::vec3(5, 0, 0), glm::vec3(1));
        auto box3 = std::make_shared<TestBox>(glm::vec3(0, 5, 0), glm::vec3(1));
        
        // Add objects to spatial index
        spatialIndex_.updateObject(box1);
        spatialIndex_.updateObject(box2);
        spatialIndex_.updateObject(box3);
        
        objects_ = {box1, box2, box3};
    }

    SpatialPartitioning spatialIndex_;
    std::vector<std::shared_ptr<SceneObject>> objects_;
};

TEST_F(SpatialPartitioningTests, FindsNearbyObjects) {
    // Test finding objects near origin
    auto nearOrigin = spatialIndex_.findNearbyObjects(glm::vec3(0), 2.0f);
    EXPECT_EQ(nearOrigin.size(), 1) << "Should find one object near origin";
    
    // Test finding objects near (5,0,0)
    auto nearRight = spatialIndex_.findNearbyObjects(glm::vec3(5, 0, 0), 2.0f);
    EXPECT_EQ(nearRight.size(), 1) << "Should find one object at (5,0,0)";
    
    // Test finding objects with larger radius
    auto multipleObjects = spatialIndex_.findNearbyObjects(glm::vec3(2.5f, 0, 0), 3.0f);
    EXPECT_EQ(multipleObjects.size(), 2) << "Should find two objects within radius";
}

TEST_F(SpatialPartitioningTests, FindsIntersectingObjects) {
    // Test ray from origin in +X direction
    auto rightRay = spatialIndex_.findIntersectingObjects(
        glm::vec3(0), glm::vec3(1, 0, 0), 0.1f);
    EXPECT_EQ(rightRay.size(), 2) << "Should find two objects along X axis";
    
    // Test ray from origin in +Y direction
    auto upRay = spatialIndex_.findIntersectingObjects(
        glm::vec3(0), glm::vec3(0, 1, 0), 0.1f);
    EXPECT_EQ(upRay.size(), 2) << "Should find two objects along Y axis";
    
    // Test diagonal ray that misses all objects
    auto diagonalRay = spatialIndex_.findIntersectingObjects(
        glm::vec3(0), glm::normalize(glm::vec3(1, 1, 1)), 0.1f);
    EXPECT_EQ(diagonalRay.size(), 1) << "Should find one object along diagonal";
}

TEST_F(SpatialPartitioningTests, UpdatesObjects) {
    // Create new object
    auto newBox = std::make_shared<TestBox>(glm::vec3(10, 0, 0), glm::vec3(1));
    
    // Add to spatial index
    spatialIndex_.updateObject(newBox);
    
    // Test finding the new object
    auto nearNew = spatialIndex_.findNearbyObjects(glm::vec3(10, 0, 0), 2.0f);
    EXPECT_EQ(nearNew.size(), 1) << "Should find newly added object";
}

TEST_F(SpatialPartitioningTests, RemovesObjects) {
    // Remove first object
    spatialIndex_.removeObject(objects_[0]);
    
    // Test that object was removed
    auto nearOrigin = spatialIndex_.findNearbyObjects(glm::vec3(0), 2.0f);
    EXPECT_EQ(nearOrigin.size(), 0) << "Should not find removed object";
    
    // Other objects should still be findable
    auto nearRight = spatialIndex_.findNearbyObjects(glm::vec3(5, 0, 0), 2.0f);
    EXPECT_EQ(nearRight.size(), 1) << "Should still find other objects";
}

TEST_F(SpatialPartitioningTests, HandlesEmptyQueries) {
    // Test with no nearby objects
    auto farAway = spatialIndex_.findNearbyObjects(glm::vec3(100, 100, 100), 1.0f);
    EXPECT_TRUE(farAway.empty()) << "Should return empty list when no objects nearby";
    
    // Test with zero radius
    auto zeroRadius = spatialIndex_.findNearbyObjects(glm::vec3(0), 0.0f);
    EXPECT_TRUE(zeroRadius.empty()) << "Should return empty list with zero radius";
}

TEST_F(SpatialPartitioningTests, RebuildsMaintainsObjects) {
    // Rebuild spatial index
    spatialIndex_.rebuild(objects_);
    
    // Test that all objects are still findable
    auto allObjects = spatialIndex_.findNearbyObjects(glm::vec3(0), 10.0f);
    EXPECT_EQ(allObjects.size(), objects_.size()) 
        << "Should find all objects after rebuild";
}

TEST_F(SpatialPartitioningTests, HandlesDynamicObjects) {
    // Create object that will move
    auto movingBox = std::make_shared<TestBox>(glm::vec3(0), glm::vec3(1));
    spatialIndex_.updateObject(movingBox);
    
    // Test initial position
    auto initial = spatialIndex_.findNearbyObjects(glm::vec3(0), 2.0f);
    EXPECT_EQ(initial.size(), 2) << "Should find original and new box at origin";
    
    // Move object and update
    movingBox = std::make_shared<TestBox>(glm::vec3(10, 0, 0), glm::vec3(1));
    spatialIndex_.updateObject(movingBox);
    
    // Test new position
    auto moved = spatialIndex_.findNearbyObjects(glm::vec3(10, 0, 0), 2.0f);
    EXPECT_EQ(moved.size(), 1) << "Should find moved object at new position";
}

TEST_F(SpatialPartitioningTests, HandlesOverlappingObjects) {
    // Create overlapping objects
    auto box1 = std::make_shared<TestBox>(glm::vec3(0), glm::vec3(2));
    auto box2 = std::make_shared<TestBox>(glm::vec3(0.5f, 0, 0), glm::vec3(2));
    
    spatialIndex_.updateObject(box1);
    spatialIndex_.updateObject(box2);
    
    // Test finding overlapping objects
    auto overlapping = spatialIndex_.findNearbyObjects(glm::vec3(0), 1.0f);
    EXPECT_EQ(overlapping.size(), 3) 
        << "Should find all overlapping objects plus original";
}
