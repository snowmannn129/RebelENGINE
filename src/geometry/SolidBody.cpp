#include "../../include/modeling/SolidBody.hpp"
#include "../../include/core/Error.hpp"
#include <algorithm>
#include <limits>
#include <glm/gtc/epsilon.hpp>

namespace RebelCAD {
namespace Modeling {

std::shared_ptr<SolidBody> SolidBody::Create(
    const std::vector<Vertex>& vertices,
    const std::vector<Triangle>& triangles
) {
    // Validate triangle indices
    for (const auto& triangle : triangles) {
        if (triangle.v1 >= vertices.size() ||
            triangle.v2 >= vertices.size() ||
            triangle.v3 >= vertices.size()) {
            throw Error::InvalidArgument("Triangle index out of range");
        }
    }

    return std::shared_ptr<SolidBody>(
        new SolidBody(vertices, triangles)
    );
}

SolidBody::SolidBody(
    const std::vector<Vertex>& vertices,
    const std::vector<Triangle>& triangles
)
    : m_vertices(vertices)
    , m_triangles(triangles)
{
    // Validate topology on construction
    ValidateTopology();
}

std::pair<SolidBody::Point3D, SolidBody::Point3D> SolidBody::GetBoundingBox() const {
    if (m_vertices.empty()) {
        return {Point3D(0.0), Point3D(0.0)};
    }

    Point3D min(std::numeric_limits<double>::max());
    Point3D max(std::numeric_limits<double>::lowest());

    for (const auto& vertex : m_vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    return {min, max};
}

void SolidBody::ValidateTopology() const {
    const double epsilon = 1e-10;

    // Check for degenerate triangles
    for (const auto& triangle : m_triangles) {
        const auto& v1 = m_vertices[triangle.v1].position;
        const auto& v2 = m_vertices[triangle.v2].position;
        const auto& v3 = m_vertices[triangle.v3].position;

        // Check if any two vertices are too close
        if (glm::all(glm::epsilonEqual(v1, v2, epsilon)) ||
            glm::all(glm::epsilonEqual(v2, v3, epsilon)) ||
            glm::all(glm::epsilonEqual(v3, v1, epsilon))) {
            throw Error::InvalidArgument("Degenerate triangle detected");
        }

        // Check if triangle is too small (area close to zero)
        auto edge1 = v2 - v1;
        auto edge2 = v3 - v1;
        auto normal = glm::cross(edge1, edge2);
        auto area = glm::length(normal) * 0.5;

        if (area < epsilon) {
            throw Error::InvalidArgument("Zero area triangle detected");
        }
    }

    // Check for invalid normals
    for (const auto& vertex : m_vertices) {
        auto length = glm::length(vertex.normal);
        if (std::abs(length - 1.0) > epsilon) {
            throw Error::InvalidArgument("Non-unit normal vector detected");
        }
    }

    // Check for invalid texture coordinates
    for (const auto& vertex : m_vertices) {
        if (vertex.texCoord.x < 0.0 || vertex.texCoord.x > 1.0 ||
            vertex.texCoord.y < 0.0 || vertex.texCoord.y > 1.0) {
            throw Error::InvalidArgument("Texture coordinates out of range [0,1]");
        }
    }

    // Additional checks could be added:
    // - Check for non-manifold edges
    // - Check for consistent winding order
    // - Check for intersecting triangles
    // - Check for watertight mesh
    // These would be important for a production system
}

} // namespace Modeling
} // namespace RebelCAD
