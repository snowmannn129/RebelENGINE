#include <gtest/gtest.h>
#include "modeling/HalfEdge.hpp"
#include <memory>
#include <vector>

using namespace rebel_cad::modeling;

class HalfEdgeTest : public ::testing::Test {
protected:
    // Helper function to create a simple cube mesh
    void createCubeMesh() {
        // Create 8 vertices
        vertices.clear();
        for (int i = 0; i < 8; ++i) {
            float x = (i & 1) ? 1.0f : -1.0f;
            float y = (i & 2) ? 1.0f : -1.0f;
            float z = (i & 4) ? 1.0f : -1.0f;
            vertices.push_back(HalfEdgeFactory::createVertex(glm::vec3(x, y, z)));
        }

        // Create 12 edges (24 half-edges)
        edges.clear();
        for (int i = 0; i < 24; ++i) {
            edges.push_back(HalfEdgeFactory::createHalfEdge());
        }

        // Create 6 faces
        faces.clear();
        for (int i = 0; i < 6; ++i) {
            faces.push_back(HalfEdgeFactory::createFace());
        }

        // Setup cube topology
        setupCubeTopology();
    }

    void setupCubeTopology() {
        // Front face (0,1,2,3)
        setupFace(0, {0, 1, 2, 3}, {0, 2, 4, 6});
        
        // Right face (1,5,6,2)
        setupFace(1, {1, 5, 6, 2}, {1, 10, 5, 3});
        
        // Back face (5,4,7,6)
        setupFace(2, {5, 4, 7, 6}, {8, 12, 14, 11});
        
        // Left face (4,0,3,7)
        setupFace(3, {4, 0, 3, 7}, {9, 7, 15, 13});
        
        // Top face (3,2,6,7)
        setupFace(4, {3, 2, 6, 7}, {16, 18, 20, 22});
        
        // Bottom face (0,4,5,1)
        setupFace(5, {0, 4, 5, 1}, {17, 19, 21, 23});

        // Setup pairs
        setupHalfEdgePairs();
    }

    void setupFace(int faceIdx, const std::vector<int>& vertexIndices, 
                  const std::vector<int>& edgeIndices) {
        Face* face = faces[faceIdx].get();
        face->index = faceIdx;
        
        // Setup edges for this face
        for (size_t i = 0; i < 4; ++i) {
            HalfEdge* edge = edges[edgeIndices[i]].get();
            edge->face = face;
            edge->vertex = vertices[vertexIndices[(i + 1) % 4]].get();
            edge->next = edges[edgeIndices[(i + 1) % 4]].get();
            
            if (i == 0) {
                face->edge = edge;
            }
        }
    }

    void setupHalfEdgePairs() {
        // Setup edge pairs (opposite half-edges)
        std::vector<std::pair<int, int>> pairs = {
            {0, 17}, {1, 21}, {2, 16}, {3, 18},
            {4, 19}, {5, 20}, {6, 15}, {7, 13},
            {8, 22}, {9, 12}, {10, 23}, {11, 14}
        };

        for (const auto& pair : pairs) {
            edges[pair.first]->pair = edges[pair.second].get();
            edges[pair.second]->pair = edges[pair.first].get();
        }
    }

    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<HalfEdge>> edges;
    std::vector<std::unique_ptr<Face>> faces;
};

TEST_F(HalfEdgeTest, VertexCreation) {
    auto vertex = HalfEdgeFactory::createVertex(glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(vertex->position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(vertex->normal, glm::vec3(0.0f));
    EXPECT_EQ(vertex->uv, glm::vec2(0.0f));
    EXPECT_FALSE(vertex->isSharp);
    EXPECT_EQ(vertex->outgoing, nullptr);
}

TEST_F(HalfEdgeTest, CubeMeshTopology) {
    createCubeMesh();
    
    // Test vertex count
    EXPECT_EQ(vertices.size(), 8);
    
    // Test face count
    EXPECT_EQ(faces.size(), 6);
    
    // Test edge count (24 half-edges = 12 full edges)
    EXPECT_EQ(edges.size(), 24);
    
    // Test face-edge-vertex connectivity
    for (const auto& face : faces) {
        EXPECT_NE(face->edge, nullptr);
        auto faceVertices = face->getVertices();
        EXPECT_EQ(faceVertices.size(), 4);  // Cube faces are quads
    }
}

TEST_F(HalfEdgeTest, VertexAdjacency) {
    createCubeMesh();
    
    // Corner vertex should have 3 adjacent vertices
    auto adjacentVertices = vertices[0]->getAdjacentVertices();
    EXPECT_EQ(adjacentVertices.size(), 3);
    
    // Test incident faces
    auto incidentFaces = vertices[0]->getIncidentFaces();
    EXPECT_EQ(incidentFaces.size(), 3);  // Corner vertex connects to 3 faces
}

TEST_F(HalfEdgeTest, FaceOperations) {
    createCubeMesh();
    
    // Test face normal calculation
    faces[0]->updateNormal();
    EXPECT_FLOAT_EQ(faces[0]->normal.z, -1.0f);  // Front face normal points in -z
    
    // Test face centroid
    auto centroid = faces[0]->getCentroid();
    EXPECT_FLOAT_EQ(centroid.z, -1.0f);  // Front face centroid at z = -1
    
    // Test adjacent faces
    auto adjacentFaces = faces[0]->getAdjacentFaces();
    EXPECT_EQ(adjacentFaces.size(), 4);  // Each face connects to 4 others
}

TEST_F(HalfEdgeTest, EdgeOperations) {
    createCubeMesh();
    
    // Test edge length
    auto length = edges[0]->getLength();
    EXPECT_FLOAT_EQ(length, 2.0f);  // Cube edges have length 2
    
    // Test boundary detection
    EXPECT_FALSE(edges[0]->isBoundary());
    
    // Test edge traversal
    auto startVertex = edges[0]->getStartVertex();
    auto endVertex = edges[0]->vertex;
    EXPECT_NE(startVertex, endVertex);
}

TEST_F(HalfEdgeTest, MeshTraversal) {
    createCubeMesh();
    
    // Test complete mesh traversal
    std::set<Vertex*> visitedVertices;
    std::set<Face*> visitedFaces;
    
    // Start from first face and traverse
    auto firstFace = faces[0].get();
    std::function<void(Face*)> traverseFace = [&](Face* face) {
        if (visitedFaces.find(face) != visitedFaces.end()) return;
        
        visitedFaces.insert(face);
        auto vertices = face->getVertices();
        for (auto vertex : vertices) {
            visitedVertices.insert(vertex);
        }
        
        auto adjacent = face->getAdjacentFaces();
        for (auto adjFace : adjacent) {
            traverseFace(adjFace);
        }
    };
    
    traverseFace(firstFace);
    
    EXPECT_EQ(visitedVertices.size(), 8);  // Should visit all vertices
    EXPECT_EQ(visitedFaces.size(), 6);     // Should visit all faces
}

TEST_F(HalfEdgeTest, SharpFeatures) {
    createCubeMesh();
    
    // Mark an edge as sharp
    edges[0]->isSharp = true;
    auto pair = edges[0]->pair;
    EXPECT_TRUE(edges[0]->isSharp);
    EXPECT_NE(pair, nullptr);
    
    // Mark a vertex as sharp
    vertices[0]->isSharp = true;
    EXPECT_TRUE(vertices[0]->isSharp);
}
