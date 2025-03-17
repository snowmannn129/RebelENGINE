#include <gtest/gtest.h>
#include "../../include/modeling/SolidBody.hpp"
#include "../../include/core/Error.hpp"
#include <glm/gtc/epsilon.hpp>

using namespace RebelCAD::Modeling;

class SolidBodyTest : public ::testing::Test {
protected:
    // Helper function to create a simple cube mesh
    std::shared_ptr<SolidBody> CreateCube() {
        std::vector<SolidBody::Vertex> vertices = {
            // Front face
            {{-1.0, -1.0,  1.0}, {0.0, 0.0, 1.0}, {0.0, 0.0}},  // 0
            {{ 1.0, -1.0,  1.0}, {0.0, 0.0, 1.0}, {1.0, 0.0}},  // 1
            {{ 1.0,  1.0,  1.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}},  // 2
            {{-1.0,  1.0,  1.0}, {0.0, 0.0, 1.0}, {0.0, 1.0}},  // 3

            // Back face
            {{-1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0, 0.0}}, // 4
            {{-1.0,  1.0, -1.0}, {0.0, 0.0, -1.0}, {1.0, 1.0}}, // 5
            {{ 1.0,  1.0, -1.0}, {0.0, 0.0, -1.0}, {0.0, 1.0}}, // 6
            {{ 1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}, {0.0, 0.0}}  // 7
        };

        std::vector<SolidBody::Triangle> triangles = {
            // Front face
            {0, 1, 2}, {0, 2, 3},
            // Back face
            {4, 5, 6}, {4, 6, 7},
            // Top face
            {3, 2, 6}, {3, 6, 5},
            // Bottom face
            {0, 4, 7}, {0, 7, 1},
            // Right face
            {1, 7, 6}, {1, 6, 2},
            // Left face
            {4, 0, 3}, {4, 3, 5}
        };

        return SolidBody::Create(vertices, triangles);
    }

    const double epsilon = 1e-10;
};

TEST_F(SolidBodyTest, Creation) {
    // Valid creation
    EXPECT_NO_THROW(CreateCube());

    // Empty mesh
    EXPECT_NO_THROW(SolidBody::Create({}, {}));

    // Invalid triangle indices
    std::vector<SolidBody::Vertex> vertices = {
        {{0.0, 0.0, 0.0}}, {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}}
    };
    std::vector<SolidBody::Triangle> invalidTriangles = {
        {0, 1, 3}  // Index 3 is out of range
    };
    EXPECT_THROW(
        SolidBody::Create(vertices, invalidTriangles),
        RebelCAD::Error
    );
}

TEST_F(SolidBodyTest, VertexAccess) {
    auto cube = CreateCube();
    const auto& vertices = cube->GetVertices();

    // Check number of vertices
    EXPECT_EQ(vertices.size(), 8);

    // Check first vertex position
    const auto& v0 = vertices[0];
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        v0.position,
        SolidBody::Point3D(-1.0, -1.0, 1.0),
        epsilon
    )));

    // Check first vertex normal
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        v0.normal,
        SolidBody::Normal3D(0.0, 0.0, 1.0),
        epsilon
    )));

    // Check first vertex texture coordinates
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        v0.texCoord,
        SolidBody::UV(0.0, 0.0),
        epsilon
    )));
}

TEST_F(SolidBodyTest, TriangleAccess) {
    auto cube = CreateCube();
    const auto& triangles = cube->GetTriangles();

    // Check number of triangles (6 faces * 2 triangles per face)
    EXPECT_EQ(triangles.size(), 12);

    // Check first triangle indices
    const auto& t0 = triangles[0];
    EXPECT_EQ(t0.v1, 0);
    EXPECT_EQ(t0.v2, 1);
    EXPECT_EQ(t0.v3, 2);
}

TEST_F(SolidBodyTest, BoundingBox) {
    auto cube = CreateCube();
    auto [min, max] = cube->GetBoundingBox();

    // Check bounding box dimensions
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        min,
        SolidBody::Point3D(-1.0, -1.0, -1.0),
        epsilon
    )));
    EXPECT_TRUE(glm::all(glm::epsilonEqual(
        max,
        SolidBody::Point3D(1.0, 1.0, 1.0),
        epsilon
    )));
}

TEST_F(SolidBodyTest, TopologyValidation) {
    auto cube = CreateCube();

    // Valid topology
    EXPECT_NO_THROW(cube->ValidateTopology());

    // Create mesh with degenerate triangle
    std::vector<SolidBody::Vertex> vertices = {
        {{0.0, 0.0, 0.0}}, {{1.0, 0.0, 0.0}}, {{1.0, 0.0, 0.0}}
    };
    std::vector<SolidBody::Triangle> triangles = {
        {0, 1, 2}  // Degenerate triangle (two vertices are the same)
    };
    auto degenerateMesh = SolidBody::Create(vertices, triangles);
    EXPECT_THROW(degenerateMesh->ValidateTopology(), RebelCAD::Error);
}
