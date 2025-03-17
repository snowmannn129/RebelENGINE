#include "sketching/Rectangle.h"
#include "sketching/Line.h"
#include "sketching/Circle.h"
#include "core/Error.h"
#include <cmath>
#include <algorithm>

namespace RebelCAD {
namespace Sketching {

Rectangle::Rectangle(float x1, float y1, float x2, float y2)
    : m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_rotation(0.0f)
{
    validateCoordinates(x1, y1);
    validateCoordinates(x2, y2);
    normalizeCoordinates();
}

float Rectangle::getWidth() const {
    return std::abs(m_x2 - m_x1);
}

float Rectangle::getHeight() const {
    return std::abs(m_y2 - m_y1);
}

float Rectangle::getArea() const {
    return getWidth() * getHeight();
}

float Rectangle::getPerimeter() const {
    return 2.0f * (getWidth() + getHeight());
}

std::array<float, 2> Rectangle::getCenter() const {
    return {(m_x1 + m_x2) / 2.0f, (m_y1 + m_y2) / 2.0f};
}

void Rectangle::translate(float dx, float dy) {
    m_x1 += dx;
    m_y1 += dy;
    m_x2 += dx;
    m_y2 += dy;
}

void Rectangle::rotate(float angleRadians) {
    m_rotation += angleRadians;
    // Normalize rotation angle to [0, 2π)
    while (m_rotation >= 2.0f * M_PI) m_rotation -= 2.0f * M_PI;
    while (m_rotation < 0.0f) m_rotation += 2.0f * M_PI;
}

void Rectangle::setFirstCorner(float x, float y) {
    validateCoordinates(x, y);
    m_x1 = x;
    m_y1 = y;
    normalizeCoordinates();
}

void Rectangle::setSecondCorner(float x, float y) {
    validateCoordinates(x, y);
    m_x2 = x;
    m_y2 = y;
    normalizeCoordinates();
}

bool Rectangle::containsPoint(float x, float y) const {
    if (isAxisAligned()) {
        return x >= m_x1 && x <= m_x2 && y >= m_y1 && y <= m_y2;
    }
    
    // For rotated rectangles, transform point to rectangle's local space
    auto center = getCenter();
    auto localPoint = rotatePoint(x, y, center[0], center[1], -m_rotation);
    auto corners = getCorners();
    float minX = std::min({corners[0][0], corners[1][0], corners[2][0], corners[3][0]});
    float maxX = std::max({corners[0][0], corners[1][0], corners[2][0], corners[3][0]});
    float minY = std::min({corners[0][1], corners[1][1], corners[2][1], corners[3][1]});
    float maxY = std::max({corners[0][1], corners[1][1], corners[2][1], corners[3][1]});
    
    return localPoint[0] >= minX && localPoint[0] <= maxX &&
           localPoint[1] >= minY && localPoint[1] <= maxY;
}

bool Rectangle::containsPointOnPerimeter(float x, float y, float tolerance) const {
    for (const auto& edge : getEdges()) {
        if (edge.containsPoint(x, y, tolerance)) {
            return true;
        }
    }
    return false;
}

std::vector<std::array<float, 2>> Rectangle::getCorners() const {
    std::vector<std::array<float, 2>> corners = {
        std::array<float, 2>{m_x1, m_y1}, // Top-left
        std::array<float, 2>{m_x2, m_y1}, // Top-right
        std::array<float, 2>{m_x2, m_y2}, // Bottom-right
        std::array<float, 2>{m_x1, m_y2}  // Bottom-left
    };
    
    if (!isAxisAligned()) {
        auto center = getCenter();
        for (auto& corner : corners) {
            auto rotated = rotatePoint(corner[0], corner[1], center[0], center[1], m_rotation);
            corner = rotated;
        }
    }
    
    return corners;
}

std::vector<Line> Rectangle::getEdges() const {
    auto corners = getCorners();
    std::vector<Line> edges;
    edges.reserve(4);
    
    for (size_t i = 0; i < 4; ++i) {
        size_t nextIndex = (i + 1) % 4;
        edges.emplace_back(corners[i][0], corners[i][1],
                          corners[nextIndex][0], corners[nextIndex][1]);
    }
    
    return edges;
}

std::vector<std::array<float, 2>> Rectangle::getLineIntersections(const Line& line) const {
    std::vector<std::array<float, 2>> intersections;
    
    for (const auto& edge : getEdges()) {
        auto intersection = edge.getIntersection(line);
        if (intersection) {
            intersections.push_back(*intersection);
        }
    }
    
    return intersections;
}

std::vector<std::array<float, 2>> Rectangle::getCircleIntersections(const Circle& circle) const {
    std::vector<std::array<float, 2>> intersections;
    
    for (const auto& edge : getEdges()) {
        auto edgeIntersections = circle.getLineIntersections(edge);
        intersections.insert(intersections.end(),
                           edgeIntersections.begin(),
                           edgeIntersections.end());
    }
    
    return intersections;
}

std::vector<std::array<float, 2>> Rectangle::getRectangleIntersections(const Rectangle& other) const {
    std::vector<std::array<float, 2>> intersections;
    
    // Get intersections of each edge of this rectangle with all edges of other rectangle
    for (const auto& edge : getEdges()) {
        for (const auto& otherEdge : other.getEdges()) {
            auto intersection = edge.getIntersection(otherEdge);
            if (intersection) {
                intersections.push_back(*intersection);
            }
        }
    }
    
    return intersections;
}

bool Rectangle::intersectsWith(const Rectangle& other) const {
    // Quick check for axis-aligned rectangles
    if (isAxisAligned() && other.isAxisAligned()) {
        return !(m_x2 < other.m_x1 || other.m_x2 < m_x1 ||
                m_y2 < other.m_y1 || other.m_y2 < m_y1);
    }
    
    // For rotated rectangles, check if any corner of one rectangle is inside the other
    // or if any edges intersect
    auto thisCorners = getCorners();
    auto otherCorners = other.getCorners();
    
    // Check if any corner of this rectangle is inside the other
    for (const auto& corner : thisCorners) {
        if (other.containsPoint(corner[0], corner[1])) {
            return true;
        }
    }
    
    // Check if any corner of other rectangle is inside this one
    for (const auto& corner : otherCorners) {
        if (containsPoint(corner[0], corner[1])) {
            return true;
        }
    }
    
    // Check for edge intersections
    return !getRectangleIntersections(other).empty();
}

Rectangle Rectangle::createExpanded(float dx, float dy) const {
    if (isAxisAligned()) {
        return Rectangle(m_x1 - dx, m_y1 - dy, m_x2 + dx, m_y2 + dy);
    }
    
    // For rotated rectangles, expand in local space then rotate back
    auto center = getCenter();
    auto corners = getCorners();
    float minX = std::min({corners[0][0], corners[1][0], corners[2][0], corners[3][0]}) - dx;
    float maxX = std::max({corners[0][0], corners[1][0], corners[2][0], corners[3][0]}) + dx;
    float minY = std::min({corners[0][1], corners[1][1], corners[2][1], corners[3][1]}) - dy;
    float maxY = std::max({corners[0][1], corners[1][1], corners[2][1], corners[3][1]}) + dy;
    
    Rectangle expanded(minX, minY, maxX, maxY);
    expanded.rotate(m_rotation);
    return expanded;
}

void Rectangle::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                      const Graphics::Color& color,
                      float thickness,
                      const std::vector<float>* dashPattern) {
    auto edges = getEdges();
    for (auto& edge : edges) {
        edge.render(graphics, color, thickness, dashPattern);
    }
}

void Rectangle::validateCoordinates(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw RebelCAD::Core::Error(RebelCAD::Core::ErrorCode::GeometryError,
                                   "Invalid coordinates: coordinates must be finite numbers");
    }
    
    const float MAX_COORD = 1e6f;
    if (std::abs(x) > MAX_COORD || std::abs(y) > MAX_COORD) {
        throw RebelCAD::Core::Error(RebelCAD::Core::ErrorCode::GeometryError,
                                   "Coordinate values exceed maximum allowed range");
    }
}

void Rectangle::normalizeCoordinates() {
    if (m_x1 > m_x2) std::swap(m_x1, m_x2);
    if (m_y1 > m_y2) std::swap(m_y1, m_y2);
}

bool Rectangle::isAxisAligned() const {
    return std::fabs(std::fmod(m_rotation, M_PI / 2.0f)) < 1e-5f;
}

std::array<float, 2> Rectangle::rotatePoint(float x, float y,
                                          float cx, float cy,
                                          float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);
    
    // Translate point to origin
    float px = x - cx;
    float py = y - cy;
    
    // Rotate point
    float xnew = px * c - py * s;
    float ynew = px * s + py * c;
    
    // Translate back
    return {xnew + cx, ynew + cy};
}

} // namespace Sketching
} // namespace RebelCAD
