#include <gtest/gtest.h>
#include "sketching/Line.h"
#include "graphics/Color.h"
#include <cmath>
#include <vector>

using namespace RebelCAD::Sketching;

class LineTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple horizontal line from (0,0) to (10,0)
        line = std::make_unique<Line>(0.0f, 0.0f, 10.0f, 0.0f);
    }

    std::unique_ptr<Line> line;
};

TEST_F(LineTests, Constructor) {
    // Test valid construction
    EXPECT_NO_THROW(Line(0.0f, 0.0f, 1.0f, 1.0f));
    
    // Test invalid coordinates
    EXPECT_THROW(Line(INFINITY, 0.0f, 1.0f, 1.0f), RebelCAD::Error);
    EXPECT_THROW(Line(0.0f, NAN, 1.0f, 1.0f), RebelCAD::Error);
    EXPECT_THROW(Line(0.0f, 0.0f, INFINITY, 1.0f), RebelCAD::Error);
    EXPECT_THROW(Line(0.0f, 0.0f, 1.0f, NAN), RebelCAD::Error);
}

TEST_F(LineTests, GetLength) {
    // Test horizontal line length
    EXPECT_FLOAT_EQ(line->getLength(), 10.0f);
    
    // Test diagonal line length (3-4-5 triangle)
    Line diagonal(0.0f, 0.0f, 3.0f, 4.0f);
    EXPECT_FLOAT_EQ(diagonal.getLength(), 5.0f);
}

TEST_F(LineTests, Translation) {
    line->translate(5.0f, 10.0f);
    
    auto start = line->getStartPoint();
    auto end = line->getEndPoint();
    
    EXPECT_FLOAT_EQ(start[0], 5.0f);
    EXPECT_FLOAT_EQ(start[1], 10.0f);
    EXPECT_FLOAT_EQ(end[0], 15.0f);
    EXPECT_FLOAT_EQ(end[1], 10.0f);
}

TEST_F(LineTests, Rotation) {
    // Rotate 90 degrees (π/2 radians) counterclockwise
    line->rotate(M_PI / 2.0f);
    
    auto end = line->getEndPoint();
    
    // After 90-degree rotation, x should be ~0 and y should be ~10
    EXPECT_NEAR(end[0], 0.0f, 1e-5f);
    EXPECT_NEAR(end[1], 10.0f, 1e-5f);
    
    // Length should remain unchanged after rotation
    EXPECT_FLOAT_EQ(line->getLength(), 10.0f);
}

TEST_F(LineTests, GetPoints) {
    auto start = line->getStartPoint();
    auto end = line->getEndPoint();
    
    EXPECT_FLOAT_EQ(start[0], 0.0f);
    EXPECT_FLOAT_EQ(start[1], 0.0f);
    EXPECT_FLOAT_EQ(end[0], 10.0f);
    EXPECT_FLOAT_EQ(end[1], 0.0f);
}

TEST_F(LineTests, SetPoints) {
    line->setStartPoint(1.0f, 2.0f);
    line->setEndPoint(3.0f, 4.0f);
    
    auto start = line->getStartPoint();
    auto end = line->getEndPoint();
    
    EXPECT_FLOAT_EQ(start[0], 1.0f);
    EXPECT_FLOAT_EQ(start[1], 2.0f);
    EXPECT_FLOAT_EQ(end[0], 3.0f);
    EXPECT_FLOAT_EQ(end[1], 4.0f);
    
    // Test invalid coordinates
    EXPECT_THROW(line->setStartPoint(INFINITY, 0.0f), RebelCAD::Error);
    EXPECT_THROW(line->setEndPoint(0.0f, NAN), RebelCAD::Error);
}

TEST_F(LineTests, GetMidpoint) {
    auto mid = line->getMidpoint();
    EXPECT_FLOAT_EQ(mid[0], 5.0f);
    EXPECT_FLOAT_EQ(mid[1], 0.0f);
    
    // Test with diagonal line
    Line diagonal(0.0f, 0.0f, 2.0f, 2.0f);
    mid = diagonal.getMidpoint();
    EXPECT_FLOAT_EQ(mid[0], 1.0f);
    EXPECT_FLOAT_EQ(mid[1], 1.0f);
}

TEST_F(LineTests, GetAngleTo) {
    // Test perpendicular lines
    Line vertical(5.0f, 0.0f, 5.0f, 10.0f);
    EXPECT_FLOAT_EQ(line->getAngleTo(vertical), M_PI / 2.0f);
    
    // Test parallel lines
    Line parallel(0.0f, 5.0f, 10.0f, 5.0f);
    EXPECT_FLOAT_EQ(line->getAngleTo(parallel), 0.0f);
    
    // Test 45-degree angle
    Line diagonal(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_FLOAT_EQ(line->getAngleTo(diagonal), M_PI / 4.0f);
}

TEST_F(LineTests, ContainsPoint) {
    // Test point on line
    EXPECT_TRUE(line->containsPoint(5.0f, 0.0f));
    
    // Test point near line (within tolerance)
    EXPECT_TRUE(line->containsPoint(5.0f, 0.001f, 0.01f));
    
    // Test point off line
    EXPECT_FALSE(line->containsPoint(5.0f, 1.0f));
    
    // Test point beyond line segment
    EXPECT_FALSE(line->containsPoint(15.0f, 0.0f));
}

TEST_F(LineTests, GetIntersection) {
    // Test intersecting lines
    Line diagonal(0.0f, 10.0f, 10.0f, 0.0f);
    auto intersection = line->getIntersection(diagonal);
    ASSERT_TRUE(intersection.has_value());
    EXPECT_FLOAT_EQ((*intersection)[0], 5.0f);
    EXPECT_FLOAT_EQ((*intersection)[1], 0.0f);
    
    // Test parallel lines
    Line parallel(0.0f, 1.0f, 10.0f, 1.0f);
    EXPECT_FALSE(line->getIntersection(parallel).has_value());
    
    // Test non-intersecting lines
    Line noIntersect(20.0f, 0.0f, 20.0f, 10.0f);
    EXPECT_FALSE(line->getIntersection(noIntersect).has_value());
}

TEST_F(LineTests, CreateParallelLine) {
    Line parallel = line->createParallelLine(5.0f);
    auto start = parallel.getStartPoint();
    auto end = parallel.getEndPoint();
    
    // Parallel line should be 5 units above original
    EXPECT_FLOAT_EQ(start[0], 0.0f);
    EXPECT_FLOAT_EQ(start[1], 5.0f);
    EXPECT_FLOAT_EQ(end[0], 10.0f);
    EXPECT_FLOAT_EQ(end[1], 5.0f);
    
    // Length should be preserved
    EXPECT_FLOAT_EQ(parallel.getLength(), line->getLength());
}

TEST_F(LineTests, CreatePerpendicularLine) {
    Line perpendicular = line->createPerpendicularLine(5.0f, 0.0f);
    auto start = perpendicular.getStartPoint();
    auto end = perpendicular.getEndPoint();
    
    // Should start at given point
    EXPECT_FLOAT_EQ(start[0], 5.0f);
    EXPECT_FLOAT_EQ(start[1], 0.0f);
    
    // Should be perpendicular to original line
    EXPECT_FLOAT_EQ(line->getAngleTo(perpendicular), M_PI / 2.0f);
    
    // Length should be preserved
    EXPECT_FLOAT_EQ(perpendicular.getLength(), line->getLength());
}

TEST_F(LineTests, RenderWithStyle) {
    auto mockGraphics = std::make_shared<Graphics::GraphicsSystem>();
    std::vector<float> dashPattern = {5.0f, 2.0f};
    
    // Test rendering with different styles
    line->render(mockGraphics, Graphics::Color::Red, 2.0f, &dashPattern);
    line->render(mockGraphics, Graphics::Color::Blue, 1.0f);
    
    // Basic render should still work
    line->render(mockGraphics, Graphics::Color::Black);
}
