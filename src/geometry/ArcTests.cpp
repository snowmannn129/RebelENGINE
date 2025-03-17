#include "gtest/gtest.h"
#include "sketching/Arc.h"
#include "sketching/Line.h"
#include "core/Error.h"
#include <cmath>

using namespace RebelCAD::Sketching;

class ArcTest : public ::testing::Test {
protected:
    // Common test values
    const float centerX = 0.0f;
    const float centerY = 0.0f;
    const float radius = 10.0f;
    const float startAngle = 0.0f;
    const float endAngle = M_PI / 2.0f; // 90 degrees
};

TEST_F(ArcTest, Construction) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    
    auto center = arc.getCenter();
    EXPECT_FLOAT_EQ(center[0], centerX);
    EXPECT_FLOAT_EQ(center[1], centerY);
    EXPECT_FLOAT_EQ(arc.getRadius(), radius);
    EXPECT_FLOAT_EQ(arc.getStartAngle(), startAngle);
    EXPECT_FLOAT_EQ(arc.getEndAngle(), endAngle);
}

TEST_F(ArcTest, InvalidConstruction) {
    EXPECT_THROW(Arc(centerX, centerY, -1.0f, startAngle, endAngle), Error);
    EXPECT_THROW(Arc(INFINITY, centerY, radius, startAngle, endAngle), Error);
    EXPECT_THROW(Arc(centerX, centerY, radius, INFINITY, endAngle), Error);
}

TEST_F(ArcTest, SweepAngle) {
    // Test positive sweep
    Arc arc1(centerX, centerY, radius, 0.0f, M_PI / 2.0f);
    EXPECT_FLOAT_EQ(arc1.getSweepAngle(), M_PI / 2.0f);

    // Test sweep across 0
    Arc arc2(centerX, centerY, radius, 3.0f * M_PI / 2.0f, M_PI / 2.0f);
    EXPECT_FLOAT_EQ(arc2.getSweepAngle(), M_PI);
}

TEST_F(ArcTest, EndPoints) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    
    auto start = arc.getStartPoint();
    EXPECT_FLOAT_EQ(start[0], radius); // cos(0) = 1
    EXPECT_FLOAT_EQ(start[1], 0.0f);   // sin(0) = 0
    
    auto end = arc.getEndPoint();
    EXPECT_NEAR(end[0], 0.0f, 1e-5f);      // cos(π/2) = 0
    EXPECT_NEAR(end[1], radius, 1e-5f);     // sin(π/2) = 1
}

TEST_F(ArcTest, Length) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    float expectedLength = radius * (endAngle - startAngle);
    EXPECT_FLOAT_EQ(arc.getLength(), expectedLength);
}

TEST_F(ArcTest, Translation) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    float dx = 5.0f;
    float dy = -3.0f;
    
    arc.translate(dx, dy);
    auto center = arc.getCenter();
    EXPECT_FLOAT_EQ(center[0], centerX + dx);
    EXPECT_FLOAT_EQ(center[1], centerY + dy);
}

TEST_F(ArcTest, Rotation) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    float rotation = M_PI / 4.0f; // 45 degrees
    
    arc.rotate(rotation);
    EXPECT_FLOAT_EQ(arc.getStartAngle(), startAngle + rotation);
    EXPECT_FLOAT_EQ(arc.getEndAngle(), endAngle + rotation);
}

TEST_F(ArcTest, ContainsPoint) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    
    // Point on arc at 45 degrees
    float x = radius * std::cos(M_PI / 4.0f);
    float y = radius * std::sin(M_PI / 4.0f);
    EXPECT_TRUE(arc.containsPoint(x, y));
    
    // Point not on arc (wrong radius)
    EXPECT_FALSE(arc.containsPoint(x * 2, y * 2));
    
    // Point not on arc (outside angular range)
    x = radius * std::cos(3.0f * M_PI / 4.0f);
    y = radius * std::sin(3.0f * M_PI / 4.0f);
    EXPECT_FALSE(arc.containsPoint(x, y));
}

TEST_F(ArcTest, LineIntersection) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    
    // Line that intersects arc at 45 degrees
    Line line(-radius, -radius, radius, radius);
    auto intersections = arc.getLineIntersections(line);
    ASSERT_EQ(intersections.size(), 1);
    
    float expectedX = radius * std::cos(M_PI / 4.0f);
    float expectedY = radius * std::sin(M_PI / 4.0f);
    EXPECT_NEAR(intersections[0][0], expectedX, 1e-5f);
    EXPECT_NEAR(intersections[0][1], expectedY, 1e-5f);
    
    // Line that doesn't intersect
    Line noIntersect(-radius, -radius, -radius, radius);
    EXPECT_TRUE(arc.getLineIntersections(noIntersect).empty());
}

TEST_F(ArcTest, ArcIntersection) {
    Arc arc1(centerX, centerY, radius, startAngle, endAngle);
    Arc arc2(5.0f, 0.0f, radius, M_PI, 3.0f * M_PI / 2.0f);
    
    auto intersections = arc1.getArcIntersections(arc2);
    ASSERT_EQ(intersections.size(), 1);
    
    // Verify intersection point lies on both arcs
    EXPECT_TRUE(arc1.containsPoint(intersections[0][0], intersections[0][1]));
    EXPECT_TRUE(arc2.containsPoint(intersections[0][0], intersections[0][1]));
}

TEST_F(ArcTest, ConcentricArc) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    float newRadius = radius * 2.0f;
    
    Arc concentric = arc.createConcentricArc(newRadius);
    auto center = concentric.getCenter();
    
    EXPECT_FLOAT_EQ(center[0], centerX);
    EXPECT_FLOAT_EQ(center[1], centerY);
    EXPECT_FLOAT_EQ(concentric.getRadius(), newRadius);
    EXPECT_FLOAT_EQ(concentric.getStartAngle(), startAngle);
    EXPECT_FLOAT_EQ(concentric.getEndAngle(), endAngle);
}

TEST_F(ArcTest, Midpoint) {
    Arc arc(centerX, centerY, radius, startAngle, endAngle);
    auto midpoint = arc.getMidpoint();
    
    float expectedAngle = (startAngle + endAngle) / 2.0f;
    float expectedX = radius * std::cos(expectedAngle);
    float expectedY = radius * std::sin(expectedAngle);
    
    EXPECT_NEAR(midpoint[0], expectedX, 1e-5f);
    EXPECT_NEAR(midpoint[1], expectedY, 1e-5f);
}
