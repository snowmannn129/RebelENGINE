#include "modeling/SubdivisionSurface.hpp"
#include "modeling/HalfEdgeMesh.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

namespace RebelCAD {
namespace Modeling {
namespace Tests {

class SubdivisionSurfaceTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple cube mesh for testing
        mesh_ = std::make_shared<HalfEdgeMesh>();
        createCubeMesh();
    }

    void createCubeMesh() {
        // Create vertices
        auto v0 = mesh_->createVertex(glm::vec3(-1.0f, -1.0f, -1.0f));
        auto v1 = mesh_->createVertex(glm::vec3( 1.0f, -1.0f, -1.0f));
        auto v2 = mesh_->createVertex(glm::vec3( 1.0f,  1.0f, -1.0f));
        auto v3 = mesh_->createVertex(glm::vec3(-1.0f,  1.0f, -1.0f));
        auto v4 = mesh_->createVertex(glm::vec3(-1.0f, -1.0f,  1.0f));
        auto v5 = mesh_->createVertex(glm::vec3( 1.0f, -1.0f,  1.0f));
        auto v6 = mesh_->createVertex(glm::vec3( 1.0f,  1.0f,  1.0f));
        auto v7 = mesh_->createVertex(glm::vec3(-1.0f,  1.0f,  1.0f));

        // Create faces (6 faces of the cube)
        mesh_->createFace({v0, v1, v2, v3}); // Front
        mesh_->createFace({v5, v4, v7, v6}); // Back
        mesh_->createFace({v4, v0, v3, v7}); // Left
        mesh_->createFace({v1, v5, v6, v2}); // Right
        mesh_->createFace({v4, v5, v1, v0}); // Bottom
        mesh_->createFace({v3, v2, v6, v7}); // Top
    }

    std::shared_ptr<HalfEdgeMesh> mesh_;
};

TEST_F(SubdivisionSurfaceTests, Construction) {
    EXPECT_NO_THROW({
        SubdivisionSurface surface(mesh_);
    });

    EXPECT_THROW({
        SubdivisionSurface surface(nullptr);
    }, RebelCAD::Error);
}

TEST_F(SubdivisionSurfaceTests, BasicSubdivision) {
    SubdivisionSurface surface(mesh_);
    
    auto subdivided = surface.subdivide();
    ASSERT_TRUE(subdivided != nullptr);
    EXPECT_TRUE(subdivided->validateTopology());
    
    // After one subdivision:
    // - Each face should be split into 4 quads
    // - Number of faces should be 24 (6 original faces * 4)
    EXPECT_EQ(subdivided->getFaces().size(), 24);
    
    // Each face should be a quad
    for (const auto& face : subdivided->getFaces()) {
        int vertexCount = 0;
        auto start = face->halfEdge;
        auto current = start;
        do {
            vertexCount++;
            current = current->next;
        } while (current != start);
        EXPECT_EQ(vertexCount, 4);
    }
}

TEST_F(SubdivisionSurfaceTests, SharpEdgePreservation) {
    SubdivisionSurface surface(mesh_);
    
    // Mark an edge as sharp
    auto edge = mesh_->getEdges()[0];
    mesh_->markSharpEdge(edge, 1.0f);
    
    auto subdivided = surface.subdivide();
    ASSERT_TRUE(subdivided != nullptr);
    
    // The subdivided edge point should be exactly halfway between endpoints
    auto originalV1 = edge->halfEdge->vertex;
    auto originalV2 = edge->halfEdge->prev->vertex;
    glm::vec3 expectedMidpoint = (originalV1->position + originalV2->position) * 0.5f;
    
    // Find the corresponding edge point in subdivided mesh
    bool found = false;
    for (const auto& vertex : subdivided->getVertices()) {
        if (glm::all(glm::epsilonEqual(vertex->position, expectedMidpoint, 0.0001f))) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(SubdivisionSurfaceTests, SharpVertexPreservation) {
    SubdivisionSurface surface(mesh_);
    
    // Mark a vertex as sharp
    auto vertex = mesh_->getVertices()[0];
    mesh_->markSharpVertex(vertex);
    glm::vec3 originalPos = vertex->position;
    
    auto subdivided = surface.subdivide();
    ASSERT_TRUE(subdivided != nullptr);
    
    // Find the corresponding vertex in subdivided mesh
    bool found = false;
    for (const auto& v : subdivided->getVertices()) {
        if (glm::all(glm::epsilonEqual(v->position, originalPos, 0.0001f))) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST_F(SubdivisionSurfaceTests, MultipleLevels) {
    SubdivisionSurface surface(mesh_);
    
    EXPECT_THROW({
        surface.subdivide(-1);
    }, RebelCAD::Error);
    
    auto subdivided = surface.subdivide(2);
    ASSERT_TRUE(subdivided != nullptr);
    EXPECT_TRUE(subdivided->validateTopology());
    
    // After two subdivisions:
    // - Each face should be split into 16 quads
    // - Number of faces should be 96 (6 original faces * 16)
    EXPECT_EQ(subdivided->getFaces().size(), 96);
}

TEST_F(SubdivisionSurfaceTests, SubdivisionLevel) {
    SubdivisionSurface surface(mesh_);
    EXPECT_EQ(surface.getSubdivisionLevel(), 0);
    
    surface.subdivide();
    EXPECT_EQ(surface.getSubdivisionLevel(), 1);
    
    surface.subdivide(2);
    EXPECT_EQ(surface.getSubdivisionLevel(), 3);
}

TEST_F(SubdivisionSurfaceTests, BaseMeshPreservation) {
    SubdivisionSurface surface(mesh_);
    
    // Original mesh should have 6 faces (cube)
    EXPECT_EQ(mesh_->getFaces().size(), 6);
    
    // Subdivide multiple times
    auto subdivided1 = surface.subdivide();
    auto subdivided2 = surface.subdivide();
    
    // Original mesh should remain unchanged
    EXPECT_EQ(mesh_->getFaces().size(), 6);
    EXPECT_EQ(surface.getBaseMesh()->getFaces().size(), 6);
}

TEST_F(SubdivisionSurfaceTests, ToSolidBodyBasicConversion) {
    SubdivisionSurface surface(mesh_);
    
    auto solidBody = surface.toSolidBody();
    ASSERT_TRUE(solidBody != nullptr);
    
    // Cube should have 8 vertices
    EXPECT_EQ(solidBody->GetVertices().size(), 8);
    
    // Each face is triangulated, so 6 faces * 2 triangles per face = 12 triangles
    EXPECT_EQ(solidBody->GetTriangles().size(), 12);
    
    // Verify vertex positions
    bool foundCorners = false;
    const auto& vertices = solidBody->GetVertices();
    for (const auto& vertex : vertices) {
        // Check if this is a corner vertex (should have magnitude sqrt(3))
        if (glm::length(vertex.position) > 1.7f) { // sqrt(3) ≈ 1.732
            foundCorners = true;
            // Verify normal is properly computed
            EXPECT_GT(glm::length(vertex.normal), 0.99f); // Should be normalized
            EXPECT_LT(glm::length(vertex.normal), 1.01f);
        }
    }
    EXPECT_TRUE(foundCorners);
}

TEST_F(SubdivisionSurfaceTests, FromSolidBodyBasicConversion) {
    // First create a solid body
    SubdivisionSurface surface(mesh_);
    auto solidBody = surface.toSolidBody();
    
    // Convert back to subdivision surface
    auto newSurface = SubdivisionSurface::fromSolidBody(solidBody);
    ASSERT_TRUE(newSurface != nullptr);
    
    auto newMesh = newSurface->getBaseMesh();
    ASSERT_TRUE(newMesh != nullptr);
    
    // Should preserve vertex count
    EXPECT_EQ(newMesh->getVertices().size(), mesh_->getVertices().size());
    
    // Should preserve face count (though faces may be triangulated)
    EXPECT_GE(newMesh->getFaces().size(), mesh_->getFaces().size());
}

TEST_F(SubdivisionSurfaceTests, SharpFeatureConversion) {
    // Mark some features as sharp in original mesh
    auto edge = mesh_->getEdges()[0];
    mesh_->markSharpEdge(edge, 1.0f);
    auto vertex = mesh_->getVertices()[0];
    mesh_->markSharpVertex(vertex);
    
    SubdivisionSurface surface(mesh_);
    
    // Convert to solid body and back
    auto solidBody = surface.toSolidBody();
    auto newSurface = SubdivisionSurface::fromSolidBody(solidBody);
    auto newMesh = newSurface->getBaseMesh();
    
    // Find corresponding sharp features in new mesh
    bool foundSharpEdge = false;
    bool foundSharpVertex = false;
    
    for (const auto& e : newMesh->getEdges()) {
        if (e->isSharp && e->sharpness >= 0.99f) {
            foundSharpEdge = true;
            break;
        }
    }
    
    for (const auto& v : newMesh->getVertices()) {
        if (v->isSharp) {
            foundSharpVertex = true;
            break;
        }
    }
    
    EXPECT_TRUE(foundSharpEdge);
    EXPECT_TRUE(foundSharpVertex);
}

TEST_F(SubdivisionSurfaceTests, ConversionErrorHandling) {
    // Test null solid body
    EXPECT_THROW({
        SubdivisionSurface::fromSolidBody(nullptr);
    }, RebelCAD::Error);
    
    // Test invalid topology
    auto invalidSolidBody = SolidBody::Create(
        {SolidBody::Vertex()}, // Single vertex
        {} // No triangles
    );
    
    EXPECT_THROW({
        SubdivisionSurface::fromSolidBody(invalidSolidBody);
    }, RebelCAD::Error);
}

TEST_F(SubdivisionSurfaceTests, NormalPreservation) {
    SubdivisionSurface surface(mesh_);
    
    // Convert to solid body
    auto solidBody = surface.toSolidBody();
    
    // Verify normals are unit length and properly oriented
    for (const auto& vertex : solidBody->GetVertices()) {
        // Check normal is unit length
        EXPECT_NEAR(glm::length(vertex.normal), 1.0, 0.001f);
        
        // For a cube, each normal should align with one of the primary axes
        bool alignedWithAxis = false;
        for (int i = 0; i < 3; ++i) {
            if (std::abs(std::abs(vertex.normal[i]) - 1.0) < 0.001f) {
                alignedWithAxis = true;
                break;
            }
        }
        EXPECT_TRUE(alignedWithAxis);
    }
}

} // namespace Tests
} // namespace Modeling
} // namespace RebelCAD
