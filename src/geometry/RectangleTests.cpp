#include <gtest/gtest.h>
#include "sketching/Rectangle.h"
#include "sketching/Line.h"
#include "sketching/Circle.h"
#include "core/Error.h"
#include "graphics/Color.h"
#include <cmath>

using namespace RebelCAD::Sketching;
using namespace RebelCAD::Graphics;
using namespace RebelCAD::Core;

class RectangleTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a 10x10 rectangle centered at origin
        rect = std::make_unique<Rectangle>(-5.0f, -5.0f, 5.0f, 5.0f);
    }

    std::unique_ptr<Rectangle> rect;
};

TEST_F(RectangleTest, Construction) {
    // Test valid construction
    EXPECT_NO_THROW(Rectangle(0.0f, 0.0f, 10.0f, 10.0f));
    
    // Test coordinate normalization
    Rectangle r(10.0f, 10.0f, 0.0f, 0.0f);
    auto corner1 = r.getFirstCorner();
    auto corner2 = r.getSecondCorner();
    EXPECT_FLOAT_EQ(corner1[0], 0.0f);
    EXPECT_FLOAT_EQ(corner1[1], 0.0f);
    EXPECT_FLOAT_EQ(corner2[0], 10.0f);
    EXPECT_FLOAT_EQ(corner2[1], 10.0f);
    
    // Test invalid coordinates
    EXPECT_THROW(Rectangle(INFINITY, 0.0f, 10.0f, 10.0f), Error);
    EXPECT_THROW(Rectangle(0.0f, NAN, 10.0f, 10.0f), Error);
    EXPECT_THROW(Rectangle(0.0f, 0.0f, INFINITY, 10.0f), Error);
    EXPECT_THROW(Rectangle(0.0f, 0.0f, 10.0f, NAN), Error);
}

TEST_F(RectangleTest, BasicProperties) {
    EXPECT_FLOAT_EQ(rect->getWidth(), 10.0f);
    EXPECT_FLOAT_EQ(rect->getHeight(), 10.0f);
    EXPECT_FLOAT_EQ(rect->getArea(), 100.0f);
    EXPECT_FLOAT_EQ(rect->getPerimeter(), 40.0f);
    
    auto center = rect->getCenter();
    EXPECT_FLOAT_EQ(center[0], 0.0f);
    EXPECT_FLOAT_EQ(center[1], 0.0f);
}

TEST_F(RectangleTest, Translation) {
    rect->translate(10.0f, -5.0f);
    
    auto corner1 = rect->getFirstCorner();
    auto corner2 = rect->getSecondCorner();
    EXPECT_FLOAT_EQ(corner1[0], 5.0f);
    EXPECT_FLOAT_EQ(corner1[1], -10.0f);
    EXPECT_FLOAT_EQ(corner2[0], 15.0f);
    EXPECT_FLOAT_EQ(corner2[1], 0.0f);
}

TEST_F(RectangleTest, Rotation) {
    // Rotate 90 degrees counterclockwise
    rect->rotate(M_PI / 2.0f);
    
    auto corners = rect->getCorners();
    // Check first corner (originally top-left)
    EXPECT_NEAR(corners[0][0], 5.0f, 1e-5f);
    EXPECT_NEAR(corners[0][1], -5.0f, 1e-5f);
    
    // Area and perimeter should remain unchanged
    EXPECT_FLOAT_EQ(rect->getArea(), 100.0f);
    EXPECT_FLOAT_EQ(rect->getPerimeter(), 40.0f);
}

TEST_F(RectangleTest, PointContainment) {
    // Test points inside
    EXPECT_TRUE(rect->containsPoint(0.0f, 0.0f));
    EXPECT_TRUE(rect->containsPoint(4.0f, 4.0f));
    
    // Test points outside
    EXPECT_FALSE(rect->containsPoint(6.0f, 0.0f));
    EXPECT_FALSE(rect->containsPoint(0.0f, 6.0f));
    
    // Test points on perimeter
    EXPECT_TRUE(rect->containsPointOnPerimeter(5.0f, 0.0f));
    EXPECT_TRUE(rect->containsPointOnPerimeter(0.0f, 5.0f));
    EXPECT_FALSE(rect->containsPointOnPerimeter(0.0f, 0.0f));
    
    // Test with rotation
    rect->rotate(M_PI / 4.0f); // 45 degrees
    EXPECT_TRUE(rect->containsPoint(0.0f, 0.0f));
    EXPECT_FALSE(rect->containsPoint(5.0f, 5.0f));
}

TEST_F(RectangleTest, Corners) {
    auto corners = rect->getCorners();
    ASSERT_EQ(corners.size(), 4);
    
    // Check corners before rotation
    EXPECT_FLOAT_EQ(corners[0][0], -5.0f); // Top-left
    EXPECT_FLOAT_EQ(corners[0][1], -5.0f);
    EXPECT_FLOAT_EQ(corners[1][0], 5.0f);  // Top-right
    EXPECT_FLOAT_EQ(corners[1][1], -5.0f);
    EXPECT_FLOAT_EQ(corners[2][0], 5.0f);  // Bottom-right
    EXPECT_FLOAT_EQ(corners[2][1], 5.0f);
    EXPECT_FLOAT_EQ(corners[3][0], -5.0f); // Bottom-left
    EXPECT_FLOAT_EQ(corners[3][1], 5.0f);
    
    // Check corners after rotation
    rect->rotate(M_PI / 2.0f); // 90 degrees
    corners = rect->getCorners();
    EXPECT_NEAR(corners[0][0], 5.0f, 1e-5f);
    EXPECT_NEAR(corners[0][1], -5.0f, 1e-5f);
}

TEST_F(RectangleTest, Edges) {
    auto edges = rect->getEdges();
    ASSERT_EQ(edges.size(), 4);
    
    // Check edge lengths
    for (const auto& edge : edges) {
        EXPECT_FLOAT_EQ(edge.getLength(), 10.0f);
    }
    
    // Check edge connections
    for (size_t i = 0; i < edges.size(); ++i) {
        size_t nextIndex = (i + 1) % edges.size();
        auto end = edges[i].getEndPoint();
        auto start = edges[nextIndex].getStartPoint();
        EXPECT_FLOAT_EQ(end[0], start[0]);
        EXPECT_FLOAT_EQ(end[1], start[1]);
    }
}

TEST_F(RectangleTest, LineIntersection) {
    // Line through center
    Line line(0.0f, -10.0f, 0.0f, 10.0f);
    auto intersections = rect->getLineIntersections(line);
    ASSERT_EQ(intersections.size(), 2);
    EXPECT_FLOAT_EQ(intersections[0][1], -5.0f);
    EXPECT_FLOAT_EQ(intersections[1][1], 5.0f);
    
    // Line not intersecting
    Line noIntersect(10.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(rect->getLineIntersections(noIntersect).empty());
}

TEST_F(RectangleTest, CircleIntersection) {
    // Circle centered at origin with radius 5
    Circle circle(0.0f, 0.0f, 5.0f);
    auto intersections = rect->getCircleIntersections(circle);
    ASSERT_EQ(intersections.size(), 4);
    
    // Circle not intersecting
    Circle noIntersect(20.0f, 20.0f, 5.0f);
    EXPECT_TRUE(rect->getCircleIntersections(noIntersect).empty());
}

TEST_F(RectangleTest, RectangleIntersection) {
    // Overlapping rectangles
    Rectangle other(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(rect->intersectsWith(other));
    auto intersections = rect->getRectangleIntersections(other);
    ASSERT_EQ(intersections.size(), 2);
    
    // Non-overlapping rectangles
    Rectangle noIntersect(10.0f, 10.0f, 20.0f, 20.0f);
    EXPECT_FALSE(rect->intersectsWith(noIntersect));
    EXPECT_TRUE(rect->getRectangleIntersections(noIntersect).empty());
}

TEST_F(RectangleTest, Expansion) {
    auto expanded = rect->createExpanded(2.0f, 2.0f);
    EXPECT_FLOAT_EQ(expanded.getWidth(), 14.0f);
    EXPECT_FLOAT_EQ(expanded.getHeight(), 14.0f);
    
    // Test with rotation
    rect->rotate(M_PI / 4.0f); // 45 degrees
    auto rotatedExpanded = rect->createExpanded(2.0f, 2.0f);
    EXPECT_FLOAT_EQ(rotatedExpanded.getWidth(), 14.0f);
    EXPECT_FLOAT_EQ(rotatedExpanded.getHeight(), 14.0f);
}

TEST_F(RectangleTest, Rendering) {
    auto graphics = std::make_shared<GraphicsSystem>();
    Color color(1.0f, 0.0f, 0.0f); // Red
    std::vector<float> dashPattern = {5.0f, 2.0f};
    
    // Test different rendering styles
    EXPECT_NO_THROW(rect->render(graphics, color));
    EXPECT_NO_THROW(rect->render(graphics, color, 2.0f));
    EXPECT_NO_THROW(rect->render(graphics, color, 2.0f, &dashPattern));
}
