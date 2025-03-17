#include "gtest/gtest.h"
#include "sketching/Circle.h"
#include "sketching/Line.h"
#include "sketching/Arc.h"
#include "core/Error.h"
#include <cmath>

using namespace RebelCAD::Sketching;

class CircleTest : public ::testing::Test {
protected:
    // Common test values
    const float centerX = 0.0f;
    const float centerY = 0.0f;
    const float radius = 10.0f;
};

TEST_F(CircleTest, Construction) {
    Circle circle(centerX, centerY, radius);
    
    auto center = circle.getCenter();
    EXPECT_FLOAT_EQ(center[0], centerX);
    EXPECT_FLOAT_EQ(center[1], centerY);
    EXPECT_FLOAT_EQ(circle.getRadius(), radius);
}

TEST_F(CircleTest, InvalidConstruction) {
    EXPECT_THROW(Circle(centerX, centerY, -1.0f), Error);
    EXPECT_THROW(Circle(INFINITY, centerY, radius), Error);
    EXPECT_THROW(Circle(centerX, centerY, 0.0f), Error);
}

TEST_F(CircleTest, CircumferenceAndArea) {
    Circle circle(centerX, centerY, radius);
    
    EXPECT_FLOAT_EQ(circle.getCircumference(), 2.0f * M_PI * radius);
    EXPECT_FLOAT_EQ(circle.getArea(), M_PI * radius * radius);
}

TEST_F(CircleTest, Translation) {
    Circle circle(centerX, centerY, radius);
    float dx = 5.0f;
    float dy = -3.0f;
    
    circle.translate(dx, dy);
    auto center = circle.getCenter();
    EXPECT_FLOAT_EQ(center[0], centerX + dx);
    EXPECT_FLOAT_EQ(center[1], centerY + dy);
}

TEST_F(CircleTest, PointContainment) {
    Circle circle(centerX, centerY, radius);
    
    // Point on circle
    EXPECT_TRUE(circle.containsPoint(radius, 0.0f));
    
    // Point inside circle
    EXPECT_TRUE(circle.containsPointInside(radius/2.0f, 0.0f));
    
    // Point outside circle
    EXPECT_FALSE(circle.containsPoint(radius * 2.0f, 0.0f));
    EXPECT_FALSE(circle.containsPointInside(radius * 2.0f, 0.0f));
    
    // Point with tolerance
    float tolerance = 0.1f;
    EXPECT_TRUE(circle.containsPoint(radius + tolerance/2.0f, 0.0f, tolerance));
}

TEST_F(CircleTest, PointAtAngle) {
    Circle circle(centerX, centerY, radius);
    
    // Test points at cardinal angles
    auto point0 = circle.getPointAtAngle(0.0f);
    EXPECT_FLOAT_EQ(point0[0], radius);
    EXPECT_FLOAT_EQ(point0[1], 0.0f);
    
    auto point90 = circle.getPointAtAngle(M_PI/2.0f);
    EXPECT_NEAR(point90[0], 0.0f, 1e-5f);
    EXPECT_NEAR(point90[1], radius, 1e-5f);
    
    auto point180 = circle.getPointAtAngle(M_PI);
    EXPECT_NEAR(point180[0], -radius, 1e-5f);
    EXPECT_NEAR(point180[1], 0.0f, 1e-5f);
}

TEST_F(CircleTest, LineIntersection) {
    Circle circle(centerX, centerY, radius);
    
    // Line through center
    Line line1(-radius*2.0f, 0.0f, radius*2.0f, 0.0f);
    auto intersections1 = circle.getLineIntersections(line1);
    ASSERT_EQ(intersections1.size(), 2);
    EXPECT_FLOAT_EQ(intersections1[0][0], radius);
    EXPECT_FLOAT_EQ(intersections1[0][1], 0.0f);
    EXPECT_FLOAT_EQ(intersections1[1][0], -radius);
    EXPECT_FLOAT_EQ(intersections1[1][1], 0.0f);
    
    // Tangent line
    Line line2(0.0f, radius, radius*2.0f, radius);
    auto intersections2 = circle.getLineIntersections(line2);
    ASSERT_EQ(intersections2.size(), 1);
    EXPECT_NEAR(intersections2[0][0], 0.0f, 1e-5f);
    EXPECT_NEAR(intersections2[0][1], radius, 1e-5f);
    
    // No intersection
    Line line3(radius*2.0f, radius*2.0f, radius*3.0f, radius*2.0f);
    EXPECT_TRUE(circle.getLineIntersections(line3).empty());
}

TEST_F(CircleTest, CircleIntersection) {
    Circle circle1(centerX, centerY, radius);
    
    // Two intersection points
    Circle circle2(radius, 0.0f, radius);
    auto intersections1 = circle1.getCircleIntersections(circle2);
    ASSERT_EQ(intersections1.size(), 2);
    
    // One intersection point (tangent)
    Circle circle3(2.0f * radius, 0.0f, radius);
    auto intersections2 = circle1.getCircleIntersections(circle3);
    ASSERT_EQ(intersections2.size(), 1);
    EXPECT_FLOAT_EQ(intersections2[0][0], radius);
    EXPECT_FLOAT_EQ(intersections2[0][1], 0.0f);
    
    // No intersection (too far)
    Circle circle4(3.0f * radius, 0.0f, radius);
    EXPECT_TRUE(circle1.getCircleIntersections(circle4).empty());
    
    // No intersection (one inside other)
    Circle circle5(0.0f, 0.0f, radius/2.0f);
    EXPECT_TRUE(circle1.getCircleIntersections(circle5).empty());
}

TEST_F(CircleTest, ArcCreation) {
    Circle circle(centerX, centerY, radius);
    float startAngle = 0.0f;
    float endAngle = M_PI/2.0f;
    
    Arc arc = circle.createArc(startAngle, endAngle);
    
    auto arcCenter = arc.getCenter();
    EXPECT_FLOAT_EQ(arcCenter[0], centerX);
    EXPECT_FLOAT_EQ(arcCenter[1], centerY);
    EXPECT_FLOAT_EQ(arc.getRadius(), radius);
    EXPECT_FLOAT_EQ(arc.getStartAngle(), startAngle);
    EXPECT_FLOAT_EQ(arc.getEndAngle(), endAngle);
}

TEST_F(CircleTest, ConcentricCircle) {
    Circle circle(centerX, centerY, radius);
    float newRadius = radius * 2.0f;
    
    Circle concentric = circle.createConcentricCircle(newRadius);
    auto center = concentric.getCenter();
    
    EXPECT_FLOAT_EQ(center[0], centerX);
    EXPECT_FLOAT_EQ(center[1], centerY);
    EXPECT_FLOAT_EQ(concentric.getRadius(), newRadius);
}
