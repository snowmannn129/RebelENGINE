#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace RebelCAD {
namespace Modeling {

/**
 * Represents a solid 3D body with boundary representation.
 * Used for rendering and boolean operations.
 */
class SolidBody {
public:
    using Point3D = glm::dvec3;
    using Normal3D = glm::dvec3;
    using UV = glm::dvec2;

    /**
     * Enum defining supported boolean operations
     */
    enum class BooleanOperation {
        Union,      // Combines two bodies
        Subtract,   // Subtracts second body from first
        Intersect   // Keeps only overlapping regions
    };

    /**
     * Vertex data structure containing position, normal, and texture coordinates.
     */
    struct Vertex {
        Point3D position;
        Normal3D normal;
        UV texCoord;

        Vertex(
            const Point3D& pos = Point3D(0.0),
            const Normal3D& norm = Normal3D(0.0, 0.0, 1.0),
            const UV& tex = UV(0.0)
        )
            : position(pos)
            , normal(norm)
            , texCoord(tex)
        {}
    };

    /**
     * Triangle face structure containing vertex indices.
     */
    struct Triangle {
        size_t v1, v2, v3;  // Vertex indices

        Triangle(size_t v1 = 0, size_t v2 = 0, size_t v3 = 0)
            : v1(v1), v2(v2), v3(v3)
        {}
    };

    /**
     * Creates a new solid body with the given vertices and triangles.
     * 
     * @param vertices List of vertices
     * @param triangles List of triangles
     * @return std::shared_ptr<SolidBody>
     */
    static std::shared_ptr<SolidBody> Create(
        const std::vector<Vertex>& vertices,
        const std::vector<Triangle>& triangles
    );

    /**
     * Gets the vertices of the solid body.
     * 
     * @return const std::vector<Vertex>&
     */
    const std::vector<Vertex>& GetVertices() const { return m_vertices; }

    /**
     * Gets the triangles of the solid body.
     * 
     * @return const std::vector<Triangle>&
     */
    const std::vector<Triangle>& GetTriangles() const { return m_triangles; }

    /**
     * Performs a boolean operation with another solid body.
     * 
     * @param other The other solid body
     * @param operation The boolean operation to perform
     * @return std::shared_ptr<SolidBody> Result of the operation
     * @throws RebelCAD::Error if operation fails
     */
    std::shared_ptr<SolidBody> PerformBoolean(
        std::shared_ptr<const SolidBody> other,
        BooleanOperation operation) const;

    /**
     * Computes the bounding box of the solid body.
     * 
     * @return std::pair<Point3D, Point3D> Min and max points of bounding box
     */
    std::pair<Point3D, Point3D> GetBoundingBox() const;

    /**
     * Validates the mesh topology.
     * Checks for degenerate triangles, invalid indices, etc.
     * 
     * @throws RebelCAD::Error if topology is invalid
     */
    void ValidateTopology() const;

private:
    SolidBody(
        const std::vector<Vertex>& vertices,
        const std::vector<Triangle>& triangles
    );

    std::vector<Vertex> m_vertices;
    std::vector<Triangle> m_triangles;
};

} // namespace Modeling
} // namespace RebelCAD
