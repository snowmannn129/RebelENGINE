#include "sketching/Polygon.h"
#include "graphics/Color.h"
#include "core/Error.h"
#include "core/Log.h"
#include <cmath>
#include <limits>
#include <algorithm>

using namespace RebelCAD::Core;

namespace RebelCAD {
namespace Sketching {

Polygon::Polygon(float centerX, float centerY, float radius, int sides, float rotationRadians) {
    validateVertex(centerX, centerY);
    if (radius <= 0.0f) {
        throw Error(ErrorCode::GeometryError, "Radius must be positive");
    }
    validateSideCount(sides);
    
    calculateRegularPolygonVertices(centerX, centerY, radius, sides, rotationRadians);
}

Polygon::Polygon(const std::vector<std::array<float, 2>>& vertices) {
    validateSideCount(vertices.size());
    
    for (const auto& vertex : vertices) {
        validateVertex(vertex[0], vertex[1]);
    }
    
    mVertices = vertices;
}

std::array<float, 2> Polygon::getCenter() const {
    float sumX = 0.0f, sumY = 0.0f;
    
    for (const auto& vertex : mVertices) {
        sumX += vertex[0];
        sumY += vertex[1];
    }
    
    return {sumX / mVertices.size(), sumY / mVertices.size()};
}

float Polygon::getPerimeter() const {
    float perimeter = 0.0f;
    
    for (size_t i = 0; i < mVertices.size(); ++i) {
        const auto& v1 = mVertices[i];
        const auto& v2 = mVertices[(i + 1) % mVertices.size()];
        
        float dx = v2[0] - v1[0];
        float dy = v2[1] - v1[1];
        perimeter += std::sqrt(dx * dx + dy * dy);
    }
    
    return perimeter;
}

float Polygon::getArea() const {
    return std::abs(calculateSignedArea());
}

void Polygon::translate(float dx, float dy) {
    for (auto& vertex : mVertices) {
        vertex[0] += dx;
        vertex[1] += dy;
    }
}

void Polygon::rotate(float angleRadians) {
    auto center = getCenter();
    float cosA = std::cos(angleRadians);
    float sinA = std::sin(angleRadians);
    
    for (auto& vertex : mVertices) {
        float dx = vertex[0] - center[0];
        float dy = vertex[1] - center[1];
        
        vertex[0] = center[0] + dx * cosA - dy * sinA;
        vertex[1] = center[1] + dx * sinA + dy * cosA;
    }
}

void Polygon::scale(float scale) {
    if (scale <= 0.0f) {
        throw Error(ErrorCode::GeometryError, "Scale factor must be positive");
    }
    
    auto center = getCenter();
    
    for (auto& vertex : mVertices) {
        vertex[0] = center[0] + (vertex[0] - center[0]) * scale;
        vertex[1] = center[1] + (vertex[1] - center[1]) * scale;
    }
}

bool Polygon::containsPoint(float x, float y) const {
    // Ray casting algorithm
    bool inside = false;
    
    for (size_t i = 0, j = mVertices.size() - 1; i < mVertices.size(); j = i++) {
        const auto& vi = mVertices[i];
        const auto& vj = mVertices[j];
        
        if (((vi[1] > y) != (vj[1] > y)) &&
            (x < (vj[0] - vi[0]) * (y - vi[1]) / (vj[1] - vi[1]) + vi[0])) {
            inside = !inside;
        }
    }
    
    return inside;
}

std::vector<Line> Polygon::getLines() const {
    std::vector<Line> lines;
    lines.reserve(mVertices.size());
    
    for (size_t i = 0; i < mVertices.size(); ++i) {
        const auto& v1 = mVertices[i];
        const auto& v2 = mVertices[(i + 1) % mVertices.size()];
        lines.emplace_back(v1[0], v1[1], v2[0], v2[1]);
    }
    
    return lines;
}

void Polygon::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                    const Graphics::Color& color,
                    float thickness,
                    bool filled,
                    const std::vector<float>* dashPattern) {
    // Create non-const lines for rendering
    std::vector<Line> lines = getLines();
    for (auto& line : lines) {
        line.render(graphics, color, thickness, dashPattern);
    }

    // For filled polygons, triangulate using fan method for convex polygons
    if (filled && mVertices.size() >= 3) {
        auto center = getCenter();
        std::vector<float> triangles;
        triangles.reserve(mVertices.size() * 6); // 3 vertices per triangle, 2 coords per vertex

        // Create triangles from center to each pair of adjacent vertices
        for (size_t i = 0; i < mVertices.size(); ++i) {
            const auto& v1 = mVertices[i];
            const auto& v2 = mVertices[(i + 1) % mVertices.size()];

            // Add center point
            triangles.push_back(center[0]);
            triangles.push_back(center[1]);

            // Add first vertex
            triangles.push_back(v1[0]);
            triangles.push_back(v1[1]);

            // Add second vertex
            triangles.push_back(v2[0]);
            triangles.push_back(v2[1]);
        }

        graphics->setColor(color);
        graphics->beginPreview();
        graphics->renderTriangleMesh(triangles.data(), triangles.size() / 2);
        graphics->endPreview();
    }
}

void Polygon::validateVertex(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw Error(ErrorCode::InvalidVertex, "Invalid vertex coordinates");
    }
}

void Polygon::validateSideCount(size_t count) const {
    if (count < 3) {
        throw Error(ErrorCode::GeometryError, "Polygon must have at least 3 sides");
    }
}

void Polygon::calculateRegularPolygonVertices(float centerX, float centerY,
                                            float radius, int sides, float rotation) {
    mVertices.resize(sides);
    float angleStep = 2.0f * M_PI / sides;
    
    for (int i = 0; i < sides; ++i) {
        float angle = rotation + i * angleStep;
        mVertices[i] = {
            centerX + radius * std::cos(angle),
            centerY + radius * std::sin(angle)
        };
    }
}

float Polygon::calculateSignedArea() const {
    float area = 0.0f;
    
    for (size_t i = 0; i < mVertices.size(); ++i) {
        const auto& v1 = mVertices[i];
        const auto& v2 = mVertices[(i + 1) % mVertices.size()];
        area += v1[0] * v2[1] - v2[0] * v1[1];
    }
    
    return area / 2.0f;
}

} // namespace Sketching
} // namespace RebelCAD
