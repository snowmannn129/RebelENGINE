#pragma once

#include "modeling/HalfEdgeMesh.hpp"
#include "modeling/SolidBody.hpp"
#include "core/Error.hpp"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Implements Catmull-Clark subdivision surface algorithm
 * 
 * This class provides functionality for subdividing a mesh using the Catmull-Clark
 * algorithm, with support for sharp features and adaptive subdivision levels.
 */
class SubdivisionSurface {
public:
    /**
     * @brief Constructs a subdivision surface from a base mesh
     * @param baseMesh The initial control mesh
     */
    explicit SubdivisionSurface(std::shared_ptr<HalfEdgeMesh> baseMesh);

    /**
     * @brief Performs one level of Catmull-Clark subdivision
     * @return New mesh after subdivision
     */
    std::shared_ptr<HalfEdgeMesh> subdivide();

    /**
     * @brief Performs multiple levels of subdivision
     * @param levels Number of subdivision levels
     * @return Final subdivided mesh
     * @throws Error if levels is negative
     */
    std::shared_ptr<HalfEdgeMesh> subdivide(int levels);

    /**
     * @brief Gets the current subdivision level
     */
    int getSubdivisionLevel() const { return currentLevel_; }

    /**
     * @brief Gets the base control mesh
     */
    std::shared_ptr<HalfEdgeMesh> getBaseMesh() const { return baseMesh_; }

    /**
     * @brief Converts the current subdivision surface to a SolidBody
     * @return Solid body representation of the subdivided mesh
     */
    std::shared_ptr<SolidBody> toSolidBody() const;

    /**
     * @brief Creates a subdivision surface from a solid body
     * @param solidBody The solid body to convert
     * @return New subdivision surface
     * @throws Error if solid body is invalid
     */
    static std::shared_ptr<SubdivisionSurface> fromSolidBody(
        std::shared_ptr<SolidBody> solidBody);

    /**
     * @brief Fixes T-junctions in the mesh by splitting edges
     * 
     * T-junctions occur when a vertex of one face lies on an edge of another face
     * without being a vertex of that edge. This method detects such cases and
     * splits the edges to create proper topology.
     * 
     * @throws Error if mesh is invalid or operation fails
     */
    void fixTJunctions();

    /**
     * @brief Performs a boolean union operation with another subdivision surface
     * @param other The subdivision surface to union with
     * @return New subdivision surface representing the union
     * @throws Error if operation fails
     */
    std::shared_ptr<SubdivisionSurface> booleanUnion(
        std::shared_ptr<SubdivisionSurface> other) const;

    /**
     * @brief Performs a boolean subtraction operation with another subdivision surface
     * @param other The subdivision surface to subtract
     * @return New subdivision surface representing the subtraction
     * @throws Error if operation fails
     */
    std::shared_ptr<SubdivisionSurface> booleanSubtract(
        std::shared_ptr<SubdivisionSurface> other) const;

    /**
     * @brief Performs a boolean intersection operation with another subdivision surface
     * @param other The subdivision surface to intersect with
     * @return New subdivision surface representing the intersection
     * @throws Error if operation fails
     */
    std::shared_ptr<SubdivisionSurface> booleanIntersect(
        std::shared_ptr<SubdivisionSurface> other) const;

    /**
     * @brief Sets material properties for the surface
     * @param materialName Name of the material to apply
     * @param properties Material properties (albedo, roughness, metallic, etc.)
     */
    void setMaterial(const std::string& materialName, 
                    const std::unordered_map<std::string, float>& properties);

    /**
     * @brief Sets a texture for a specific material property
     * @param materialProperty The property to texture (e.g., "albedo", "roughness")
     * @param texturePath Path to the texture file
     * @throws Error if texture loading fails
     */
    void setTexture(const std::string& materialProperty, 
                   const std::string& texturePath);

    /**
     * @brief Gets the UV coordinates for the mesh
     * @return Vector of UV coordinates for each vertex
     */
    std::vector<glm::vec2> getUVCoordinates() const;

    /**
     * @brief Sets UV coordinates for the mesh
     * @param uvCoords Vector of UV coordinates for each vertex
     * @throws Error if uvCoords size doesn't match vertex count
     */
    void setUVCoordinates(const std::vector<glm::vec2>& uvCoords);

private:
    std::shared_ptr<HalfEdgeMesh> baseMesh_;
    int currentLevel_ = 0;

    // Material properties
    struct Material {
        std::string name;
        std::unordered_map<std::string, float> properties;
        std::unordered_map<std::string, std::string> texturePaths;
    };
    Material material_;

    /**
     * @brief Helper struct for subdivision points with position and texture coordinates
     */
    struct SubdivisionPoint {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    /**
     * @brief Computes a face point for Catmull-Clark subdivision
     * @param face The face to compute the point for
     * @return The computed face point with position and texture coordinates
     */
    SubdivisionPoint computeFacePoint(const HalfEdgeMesh::FacePtr& face) const;

    /**
     * @brief Computes an edge point for Catmull-Clark subdivision
     * @param edge The edge to compute the point for
     * @return The computed edge point with position and texture coordinates
     */
    SubdivisionPoint computeEdgePoint(const HalfEdgeMesh::EdgePtr& edge) const;

    /**
     * @brief Computes a vertex point for Catmull-Clark subdivision
     * @param vertex The vertex to compute the new position for
     * @return The computed vertex point with position and texture coordinates
     */
    SubdivisionPoint computeVertexPoint(const HalfEdgeMesh::VertexPtr& vertex) const;

    /**
     * @brief Handles sharp edge cases in subdivision
     * @param edge The edge to process
     * @param regularEdgePoint The regular (smooth) edge point
     * @return The final edge point considering sharpness
     */
    glm::vec3 handleSharpEdge(const HalfEdgeMesh::EdgePtr& edge, 
                             const glm::vec3& regularEdgePoint) const;

    /**
     * @brief Handles sharp vertex cases in subdivision
     * @param vertex The vertex to process
     * @param regularVertexPoint The regular (smooth) vertex point
     * @return The final vertex point considering sharpness
     */
    glm::vec3 handleSharpVertex(const HalfEdgeMesh::VertexPtr& vertex,
                               const glm::vec3& regularVertexPoint) const;

    /**
     * @brief Creates face topology for the subdivided mesh
     * @param newMesh The mesh being created
     * @param oldFace The original face being subdivided
     * @param facePoint The computed face point
     * @param edgePoints Map of edges to their computed points
     * @param vertexPoints Map of vertices to their computed points
     */
    void createSubdividedFaceTopology(
        std::shared_ptr<HalfEdgeMesh> newMesh,
        const HalfEdgeMesh::FacePtr& oldFace,
        const HalfEdgeMesh::VertexPtr& facePoint,
        const std::unordered_map<HalfEdgeMesh::EdgePtr, HalfEdgeMesh::VertexPtr>& edgePoints,
        const std::unordered_map<HalfEdgeMesh::VertexPtr, HalfEdgeMesh::VertexPtr>& vertexPoints);

    /**
     * @brief Performs a boolean operation between two solid bodies
     * @param other The other subdivision surface
     * @param operation The boolean operation to perform (union, subtract, intersect)
     * @return New subdivision surface representing the result
     */
    std::shared_ptr<SubdivisionSurface> performBooleanOperation(
        std::shared_ptr<SubdivisionSurface> other,
        SolidBody::BooleanOperation operation) const;
};

} // namespace Modeling
} // namespace RebelCAD
