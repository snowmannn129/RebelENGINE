#pragma once

#include <vector>
#include <memory>
#include "Geometry.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Represents a Catmull-Clark subdivision surface
 * 
 * A subdivision surface starts with a coarse control mesh and generates
 * a smooth surface through iterative refinement. This implementation uses
 * the Catmull-Clark subdivision scheme which:
 * - Preserves sharp features
 * - Handles arbitrary topology
 * - Produces C2 continuous surfaces except at extraordinary vertices
 */
class SubdivisionSurface : public std::enable_shared_from_this<SubdivisionSurface> {
public:
    /**
     * @brief Creates a subdivision surface from a control mesh
     * @param vertices Initial control vertices
     * @param edges Edge connectivity
     * @param faces Face definitions
     * @param sharpEdges Optional indices of edges to maintain sharp features
     * @return Shared pointer to new subdivision surface
     */
    static std::shared_ptr<SubdivisionSurface> Create(
        const std::vector<Point3D>& vertices,
        const std::vector<Edge>& edges,
        const std::vector<Face>& faces,
        const std::vector<size_t>& sharpEdges = std::vector<size_t>());

    /**
     * @brief Performs one level of subdivision
     * @return New subdivision surface after refinement
     */
    std::shared_ptr<SubdivisionSurface> Subdivide() const;

    /**
     * @brief Converts surface to a solid body after specified subdivisions
     * @param iterations Number of subdivision iterations
     * @return Solid body representation
     */
    std::shared_ptr<SolidBody> ToSolidBody(size_t iterations = 2) const;

    /**
     * @brief Gets the current control vertices
     */
    const std::vector<Point3D>& GetVertices() const { return m_vertices; }

    /**
     * @brief Gets the current edges
     */
    const std::vector<Edge>& GetEdges() const { return m_edges; }

    /**
     * @brief Gets the current faces
     */
    const std::vector<Face>& GetFaces() const { return m_faces; }

    /**
     * @brief Gets indices of sharp edges
     */
    const std::vector<size_t>& GetSharpEdges() const { return m_sharpEdges; }

    /**
     * @brief Sets a vertex position
     * @param index Vertex index
     * @param position New vertex position
     */
    void SetVertex(size_t index, const Point3D& position);

    /**
     * @brief Marks or unmarks an edge as sharp
     * @param edgeIndex Edge index
     * @param isSharp Whether edge should be sharp
     */
    void SetEdgeSharpness(size_t edgeIndex, bool isSharp);

private:
    SubdivisionSurface(
        const std::vector<Point3D>& vertices,
        const std::vector<Edge>& edges,
        const std::vector<Face>& faces,
        const std::vector<size_t>& sharpEdges);

    /**
     * @brief Computes face points for subdivision
     * @return Vector of new face points
     */
    std::vector<Point3D> ComputeFacePoints() const;

    /**
     * @brief Computes edge points for subdivision
     * @param facePoints Pre-computed face points
     * @return Vector of new edge points
     */
    std::vector<Point3D> ComputeEdgePoints(
        const std::vector<Point3D>& facePoints) const;

    /**
     * @brief Updates vertex positions for subdivision
     * @param facePoints Pre-computed face points
     * @param edgePoints Pre-computed edge points
     * @return Vector of new vertex positions
     */
    std::vector<Point3D> UpdateVertexPoints(
        const std::vector<Point3D>& facePoints,
        const std::vector<Point3D>& edgePoints) const;

    std::vector<Point3D> m_vertices;      // Control vertices
    std::vector<Edge> m_edges;            // Edge connectivity
    std::vector<Face> m_faces;            // Face definitions  
    std::vector<size_t> m_sharpEdges;     // Indices of sharp edges
};

} // namespace Modeling
} // namespace RebelCAD
