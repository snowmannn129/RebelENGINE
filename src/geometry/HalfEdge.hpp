#pragma once

#include "graphics/GLCommon.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <unordered_map>

namespace rebel_cad {
namespace modeling {

/**
 * @brief Forward declarations for half-edge mesh components
 */
class HalfEdge;
class Vertex;
class Face;

/**
 * @brief Represents a vertex in the half-edge data structure
 * Stores position, normal, and a reference to one outgoing half-edge
 */
class Vertex {
public:
    glm::vec3 position;        ///< 3D position of the vertex
    glm::vec3 normal;          ///< Vertex normal (averaged from adjacent faces)
    glm::vec2 uv;             ///< Texture coordinates
    HalfEdge* outgoing;       ///< One of the half-edges originating from this vertex
    bool isSharp;             ///< Flag for vertices that should maintain their position
    size_t index;             ///< Index in the mesh's vertex array

    /**
     * @brief Get all adjacent vertices
     * @return Vector of pointers to adjacent vertices
     */
    std::vector<Vertex*> getAdjacentVertices() const;

    /**
     * @brief Get all incident faces
     * @return Vector of pointers to incident faces
     */
    std::vector<Face*> getIncidentFaces() const;

    /**
     * @brief Get all outgoing half-edges
     * @return Vector of pointers to outgoing half-edges
     */
    std::vector<HalfEdge*> getOutgoingHalfEdges() const;
};

/**
 * @brief Represents a half-edge in the data structure
 * Each edge in the mesh is represented by two half-edges going in opposite directions
 */
class HalfEdge {
public:
    Vertex* vertex;           ///< Vertex at the end of this half-edge
    HalfEdge* pair;          ///< Opposite half-edge
    HalfEdge* next;          ///< Next half-edge in the face
    Face* face;              ///< Face this half-edge belongs to
    bool isSharp;            ///< Flag for edges that should remain sharp

    /**
     * @brief Get the vertex at the start of this half-edge
     * @return Pointer to the start vertex
     */
    Vertex* getStartVertex() const {
        return pair ? pair->vertex : nullptr;
    }

    /**
     * @brief Get the length of this half-edge
     * @return Length of the edge in 3D space
     */
    float getLength() const;

    /**
     * @brief Check if this is a boundary edge
     * @return true if this is a boundary edge
     */
    bool isBoundary() const {
        return face == nullptr;
    }
};

/**
 * @brief Represents a face in the half-edge data structure
 */
class Face {
public:
    HalfEdge* edge;          ///< One of the half-edges of this face
    glm::vec3 normal;        ///< Face normal
    size_t index;            ///< Index in the mesh's face array

    /**
     * @brief Get all vertices of this face
     * @return Vector of pointers to face vertices in order
     */
    std::vector<Vertex*> getVertices() const;

    /**
     * @brief Get all edges of this face
     * @return Vector of pointers to face half-edges in order
     */
    std::vector<HalfEdge*> getEdges() const;

    /**
     * @brief Get adjacent faces
     * @return Vector of pointers to adjacent faces
     */
    std::vector<Face*> getAdjacentFaces() const;

    /**
     * @brief Calculate the centroid of the face
     * @return Face centroid position
     */
    glm::vec3 getCentroid() const;

    /**
     * @brief Calculate face normal
     * Updates the stored normal value
     */
    void updateNormal();

    /**
     * @brief Get the number of edges in this face
     * @return Number of edges
     */
    size_t getEdgeCount() const;
};

/**
 * @brief Factory functions for creating half-edge mesh components
 */
namespace HalfEdgeFactory {
    /**
     * @brief Create a new vertex
     * @param position Vertex position
     * @param normal Optional vertex normal
     * @param uv Optional texture coordinates
     * @return Unique pointer to new vertex
     */
    std::unique_ptr<Vertex> createVertex(
        const glm::vec3& position,
        const glm::vec3& normal = glm::vec3(0.0f),
        const glm::vec2& uv = glm::vec2(0.0f)
    );

    /**
     * @brief Create a new half-edge
     * @return Unique pointer to new half-edge
     */
    std::unique_ptr<HalfEdge> createHalfEdge();

    /**
     * @brief Create a new face
     * @return Unique pointer to new face
     */
    std::unique_ptr<Face> createFace();
}

} // namespace modeling
} // namespace rebel_cad
