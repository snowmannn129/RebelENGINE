#include "sketching/SnapManager.h"
#include "sketching/Line.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include "sketching/Rectangle.h"
#include "sketching/Polygon.h"
#include <gtest/gtest.h>
#include <memory>

using namespace RebelCAD::Sketching;

class SnapManagerTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test elements
        line = std::make_shared<Line>(0.0f, 0.0f, 2.0f, 0.0f);
        circle = std::make_shared<Circle>(1.0f, 1.0f, 1.0f);
        arc = std::make_shared<Arc>(0.0f, 1.0f, 1.0f, 0.0f, M_PI);
        rect = std::make_shared<Rectangle>(0.0f, 0.0f, 2.0f, 2.0f);
        
        std::vector<std::array<float, 2>> vertices = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
        poly = std::make_shared<Polygon>(vertices);

        // Create snap manager with default settings
        manager = std::make_unique<SnapManager>();
    }

    std::shared_ptr<Line> line;
    std::shared_ptr<Circle> circle;
    std::shared_ptr<Arc> arc;
    std::shared_ptr<Rectangle> rect;
    std::shared_ptr<Polygon> poly;
    std::unique_ptr<SnapManager> manager;

    static constexpr double kTolerance = 1e-6;

    // Helper function to compare points
    static bool pointsEqual(const std::array<double, 2>& p1,
                          const std::array<double, 2>& p2,
                          double tolerance = kTolerance) {
        return std::abs(p1[0] - p2[0]) <= tolerance &&
               std::abs(p1[1] - p2[1]) <= tolerance;
    }
};

TEST_F(SnapManagerTests, Construction) {
    // Default construction
    EXPECT_NO_THROW(SnapManager());

    // Custom settings
    SnapSettings settings;
    settings.snapRadius = 5.0;
    settings.gridSize = 5.0;
    settings.enabledTypes[static_cast<size_t>(SnapType::GRID)] = false;
    EXPECT_NO_THROW(SnapManager(settings));
}

TEST_F(SnapManagerTests, ElementManagement) {
    // Add elements
    manager->addElement(line);
    manager->addElement(circle);
    manager->addElement(arc);
    manager->addElement(rect);
    manager->addElement(poly);

    // Remove elements
    manager->removeElement(line);
    manager->removeElement(circle);

    // Clear all elements
    manager->clearElements();
}

TEST_F(SnapManagerTests, SnapSettings) {
    SnapSettings settings;
    settings.snapRadius = 5.0;
    settings.gridSize = 5.0;
    settings.enabledTypes[static_cast<size_t>(SnapType::GRID)] = false;

    manager->setSettings(settings);
    auto& currentSettings = manager->getSettings();

    EXPECT_NEAR(currentSettings.snapRadius, 5.0, kTolerance);
    EXPECT_NEAR(currentSettings.gridSize, 5.0, kTolerance);
    EXPECT_FALSE(currentSettings.enabledTypes[static_cast<size_t>(SnapType::GRID)]);
}

TEST_F(SnapManagerTests, LineSnapping) {
    manager->addElement(line);

    // Snap to endpoint
    auto point = manager->findSnapPoint(0.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.0, 0.0}));
    EXPECT_EQ(point->type, SnapType::ENDPOINT);

    // Snap to midpoint
    point = manager->findSnapPoint(1.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {1.0, 0.0}));
    EXPECT_EQ(point->type, SnapType::MIDPOINT);
}

TEST_F(SnapManagerTests, CircleSnapping) {
    manager->addElement(circle);

    // Snap to center
    auto point = manager->findSnapPoint(1.1, 1.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {1.0, 1.0}));
    EXPECT_EQ(point->type, SnapType::CENTER);
}

TEST_F(SnapManagerTests, ArcSnapping) {
    manager->addElement(arc);

    // Snap to endpoint
    auto point = manager->findSnapPoint(0.1, 1.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.0, 1.0}));
    EXPECT_EQ(point->type, SnapType::ENDPOINT);

    // Snap to center
    point = manager->findSnapPoint(0.1, 0.9, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.0, 1.0}));
    EXPECT_EQ(point->type, SnapType::CENTER);
}

TEST_F(SnapManagerTests, RectangleSnapping) {
    manager->addElement(rect);

    // Snap to corner
    auto point = manager->findSnapPoint(0.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.0, 0.0}));
    EXPECT_EQ(point->type, SnapType::ENDPOINT);

    // Snap to edge midpoint
    point = manager->findSnapPoint(1.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {1.0, 0.0}));
    EXPECT_EQ(point->type, SnapType::MIDPOINT);
}

TEST_F(SnapManagerTests, PolygonSnapping) {
    manager->addElement(poly);

    // Snap to vertex
    auto point = manager->findSnapPoint(0.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.0, 0.0}));
    EXPECT_EQ(point->type, SnapType::ENDPOINT);

    // Snap to edge midpoint
    point = manager->findSnapPoint(0.6, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {0.5, 0.0}));
    EXPECT_EQ(point->type, SnapType::MIDPOINT);
}

TEST_F(SnapManagerTests, GridSnapping) {
    // Enable grid snapping
    manager->setSnapTypeEnabled(SnapType::GRID, true);

    // Snap to grid point
    auto point = manager->findSnapPoint(9.9, 10.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_TRUE(pointsEqual(point->position, {10.0, 10.0}));
    EXPECT_EQ(point->type, SnapType::GRID);

    // Disable grid snapping
    manager->setSnapTypeEnabled(SnapType::GRID, false);
    point = manager->findSnapPoint(9.9, 10.1, 0, 0);
    EXPECT_FALSE(point.has_value());
}

TEST_F(SnapManagerTests, SnapPriority) {
    manager->addElement(line);

    // Add both endpoint and grid point at same location
    auto point = manager->findSnapPoint(0.1, 0.1, 0, 0);
    ASSERT_TRUE(point.has_value());
    EXPECT_EQ(point->type, SnapType::ENDPOINT); // Endpoint has higher priority
}

TEST_F(SnapManagerTests, OutOfRange) {
    manager->addElement(line);

    // Try to snap to point outside snap radius
    auto point = manager->findSnapPoint(100.0, 100.0, 0, 0);
    EXPECT_FALSE(point.has_value());
}
