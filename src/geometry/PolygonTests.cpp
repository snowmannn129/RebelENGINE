#include "sketching/Polygon.h"
#include "graphics/Color.h"
#include "core/Error.h"
#include <gtest/gtest.h>
#include <cmath>
#include <array>
#include <limits>
#include <vector>

using namespace RebelCAD::Sketching;
using namespace RebelCAD::Graphics;
using namespace RebelCAD::Core;

namespace {

class PolygonTests : public ::testing::Test {
protected:
    // Helper function to compare floating point values
    static bool nearlyEqual(float a, float b, float epsilon = 1e-5f) {
        return std::abs(a - b) <= epsilon;
    }
    
    // Helper function to compare points
    static bool pointsEqual(const std::array<float, 2>& p1, 
                          const std::array<float, 2>& p2,
                          float epsilon = 1e-5f) {
        return nearlyEqual(p1[0], p2[0], epsilon) && 
               nearlyEqual(p1[1], p2[1], epsilon);
    }
};

TEST_F(PolygonTests, RegularPolygonConstruction) {
    // Create a regular hexagon
    Polygon hexagon(0.0f, 0.0f, 1.0f, 6);
    EXPECT_EQ(hexagon.getSideCount(), 6);
    
    auto vertices = hexagon.getVertices();
    EXPECT_EQ(vertices.size(), 6);
    
    // Verify first vertex is at (1,0) for 0 rotation
    EXPECT_TRUE(pointsEqual(vertices[0], std::array<float, 2>{{1.0f, 0.0f}}));
    
    // Verify center point
    auto center = hexagon.getCenter();
    EXPECT_TRUE(pointsEqual(center, std::array<float, 2>{{0.0f, 0.0f}}));
}

TEST_F(PolygonTests, IrregularPolygonConstruction) {
    std::vector<std::array<float, 2>> vertices = {
        std::array<float, 2>{{0.0f, 0.0f}},
        std::array<float, 2>{{1.0f, 0.0f}},
        std::array<float, 2>{{1.0f, 1.0f}},
        std::array<float, 2>{{0.0f, 1.0f}}
    };
    
    Polygon square(vertices);
    EXPECT_EQ(square.getSideCount(), 4);
    
    auto actualVertices = square.getVertices();
    EXPECT_EQ(actualVertices.size(), 4);
    for (size_t i = 0; i < vertices.size(); ++i) {
        EXPECT_TRUE(pointsEqual(actualVertices[i], vertices[i]));
    }
}

TEST_F(PolygonTests, AreaCalculation) {
    // Create a 2x2 square
    std::vector<std::array<float, 2>> vertices = {
        std::array<float, 2>{{0.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 2.0f}},
        std::array<float, 2>{{0.0f, 2.0f}}
    };
    
    Polygon square(vertices);
    EXPECT_TRUE(nearlyEqual(square.getArea(), 4.0f));
    
    // Create a regular triangle with radius 1
    Polygon triangle(0.0f, 0.0f, 1.0f, 3);
    float expectedArea = 3.0f * std::sqrt(3.0f) / 4.0f; // Area of equilateral triangle
    EXPECT_TRUE(nearlyEqual(triangle.getArea(), expectedArea));
}

TEST_F(PolygonTests, PerimeterCalculation) {
    // Create a 2x2 square
    std::vector<std::array<float, 2>> vertices = {
        std::array<float, 2>{{0.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 2.0f}},
        std::array<float, 2>{{0.0f, 2.0f}}
    };
    
    Polygon square(vertices);
    EXPECT_TRUE(nearlyEqual(square.getPerimeter(), 8.0f));
}

TEST_F(PolygonTests, Translation) {
    Polygon square(0.0f, 0.0f, 1.0f, 4); // Unit square centered at origin
    square.translate(2.0f, 3.0f);
    
    auto center = square.getCenter();
    EXPECT_TRUE(pointsEqual(center, std::array<float, 2>{{2.0f, 3.0f}}));
}

TEST_F(PolygonTests, Rotation) {
    // Create a square and rotate it 90 degrees
    std::vector<std::array<float, 2>> vertices = {
        std::array<float, 2>{{1.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 0.0f}},
        std::array<float, 2>{{2.0f, 1.0f}},
        std::array<float, 2>{{1.0f, 1.0f}}
    };
    
    Polygon square(vertices);
    square.rotate(M_PI / 2.0f); // 90 degrees
    
    auto rotatedVertices = square.getVertices();
    // First vertex should now be at approximately (2, 1)
    EXPECT_TRUE(pointsEqual(rotatedVertices[0], std::array<float, 2>{{2.0f, 1.0f}}));
}

TEST_F(PolygonTests, Scaling) {
    Polygon triangle(0.0f, 0.0f, 1.0f, 3);
    float originalArea = triangle.getArea();
    
    triangle.scale(2.0f);
    float newArea = triangle.getArea();
    
    // Area should be 4 times larger after scaling by 2
    EXPECT_TRUE(nearlyEqual(newArea, 4.0f * originalArea));
}

TEST_F(PolygonTests, PointContainment) {
    // Create a 2x2 square centered at origin
    std::vector<std::array<float, 2>> vertices = {
        std::array<float, 2>{{-1.0f, -1.0f}},
        std::array<float, 2>{{1.0f, -1.0f}},
        std::array<float, 2>{{1.0f, 1.0f}},
        std::array<float, 2>{{-1.0f, 1.0f}}
    };
    
    Polygon square(vertices);
    
    // Test points
    EXPECT_TRUE(square.containsPoint(0.0f, 0.0f));  // Center
    EXPECT_TRUE(square.containsPoint(0.5f, 0.5f));  // Inside
    EXPECT_FALSE(square.containsPoint(2.0f, 0.0f)); // Outside
    EXPECT_TRUE(square.containsPoint(-1.0f, -1.0f)); // Vertex
    EXPECT_TRUE(square.containsPoint(0.0f, 1.0f));   // Edge
    
    // Test edge cases
    EXPECT_TRUE(square.containsPoint(-1.0f, 0.0f));  // Left edge
    EXPECT_TRUE(square.containsPoint(1.0f, 0.0f));   // Right edge
    EXPECT_TRUE(square.containsPoint(0.0f, -1.0f));  // Bottom edge
    EXPECT_TRUE(square.containsPoint(0.0f, 1.0f));   // Top edge
    
    // Test points very close to edges
    EXPECT_TRUE(square.containsPoint(-1.0f + 1e-6f, 0.0f));  // Just inside left edge
    EXPECT_FALSE(square.containsPoint(-1.0f - 1e-6f, 0.0f)); // Just outside left edge
}

TEST_F(PolygonTests, InvalidConstruction) {
    // Test invalid radius
    try {
        Polygon invalidRadius(0.0f, 0.0f, -1.0f, 4);
        FAIL() << "Expected Error exception for negative radius";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::GeometryError);
    }
    
    // Test too few sides
    try {
        Polygon tooFewSides(0.0f, 0.0f, 1.0f, 2);
        FAIL() << "Expected Error exception for too few sides";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::GeometryError);
    }
    
    // Test invalid vertices
    try {
        std::vector<std::array<float, 2>> twoVertices = {
            std::array<float, 2>{{0.0f, 0.0f}},
            std::array<float, 2>{{1.0f, 0.0f}}
        };
        Polygon invalidVertices(twoVertices);
        FAIL() << "Expected Error exception for too few vertices";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::GeometryError);
    }
    
    // Test invalid coordinates (infinity)
    try {
        std::vector<std::array<float, 2>> infinityVertices = {
            std::array<float, 2>{{0.0f, 0.0f}},
            std::array<float, 2>{{1.0f, 0.0f}},
            std::array<float, 2>{{std::numeric_limits<float>::infinity(), 1.0f}}
        };
        Polygon infinityPolygon(infinityVertices);
        FAIL() << "Expected Error exception for infinite coordinates";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::InvalidVertex);
    }
    
    // Test invalid coordinates (NaN)
    try {
        std::vector<std::array<float, 2>> nanVertices = {
            std::array<float, 2>{{0.0f, 0.0f}},
            std::array<float, 2>{{1.0f, 0.0f}},
            std::array<float, 2>{{std::numeric_limits<float>::quiet_NaN(), 1.0f}}
        };
        Polygon nanPolygon(nanVertices);
        FAIL() << "Expected Error exception for NaN coordinates";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::InvalidVertex);
    }
}

TEST_F(PolygonTests, InvalidScaling) {
    Polygon triangle(0.0f, 0.0f, 1.0f, 3);
    
    // Test zero scale
    try {
        triangle.scale(0.0f);
        FAIL() << "Expected Error exception for zero scale";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::GeometryError);
    }
    
    // Test negative scale
    try {
        triangle.scale(-1.0f);
        FAIL() << "Expected Error exception for negative scale";
    } catch (const Error& e) {
        EXPECT_EQ(e.code(), ErrorCode::GeometryError);
    }
}

TEST_F(PolygonTests, Rendering) {
    Polygon triangle(0.0f, 0.0f, 1.0f, 3);
    auto graphics = std::make_shared<GraphicsSystem>();
    Color color(1.0f, 0.0f, 0.0f); // Red
    std::vector<float> dashPattern = {5.0f, 2.0f};
    
    // Test outline rendering
    EXPECT_NO_THROW(triangle.render(graphics, color));
    EXPECT_NO_THROW(triangle.render(graphics, color, 2.0f));
    EXPECT_NO_THROW(triangle.render(graphics, color, 2.0f, false, &dashPattern));
    
    // Test filled rendering
    EXPECT_NO_THROW(triangle.render(graphics, color, 1.0f, true));
    
    // Test filled rendering with dashed outline
    EXPECT_NO_THROW(triangle.render(graphics, color, 2.0f, true, &dashPattern));
}

TEST_F(PolygonTests, EdgeRetrieval) {
    Polygon square(0.0f, 0.0f, 1.0f, 4);
    auto edges = square.getLines();
    
    ASSERT_EQ(edges.size(), 4);
    
    // Check that edges form a closed loop
    for (size_t i = 0; i < edges.size(); ++i) {
        size_t nextIndex = (i + 1) % edges.size();
        auto end = edges[i].getEndPoint();
        auto start = edges[nextIndex].getStartPoint();
        EXPECT_TRUE(pointsEqual(end, start));
    }
}

} // anonymous namespace
