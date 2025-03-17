#include "sketching/SnapManager.h"
#include "sketching/Line.h"
#include "sketching/Circle.h"
#include "sketching/Arc.h"
#include "sketching/Rectangle.h"
#include "sketching/Polygon.h"
#include "graphics/ViewportManager.h"
#include <algorithm>
#include <cmath>

namespace RebelCAD {
namespace Sketching {

SnapManager::SnapManager(const SnapSettings& settings)
    : m_settings(settings) {
}

void SnapManager::setSettings(const SnapSettings& settings) {
    m_settings = settings;
}

const SnapSettings& SnapManager::getSettings() const {
    return m_settings;
}

void SnapManager::setSnapTypeEnabled(SnapType type, bool enabled) {
    if (type != SnapType::COUNT) {
        m_settings.enabledTypes[static_cast<size_t>(type)] = enabled;
    }
}

bool SnapManager::isSnapTypeEnabled(SnapType type) const {
    return type != SnapType::COUNT &&
           m_settings.enabledTypes[static_cast<size_t>(type)];
}

std::optional<SnapPoint> SnapManager::findSnapPoint(
    double x, double y,
    int screenX, int screenY) const {
    
    // Convert screen radius to model units
    double radius = calculateSnapRadius(screenX, screenY, x, y);

    // Get all potential snap points
    auto snapPoints = calculateSnapPoints(x, y, radius);

    // Check grid snap first if enabled
    if (isSnapTypeEnabled(SnapType::GRID)) {
        if (auto gridPoint = findGridSnapPoint(x, y)) {
            // Only use grid point if it's the closest option
            double gridDist = std::hypot(
                gridPoint->position[0] - x,
                gridPoint->position[1] - y
            );
            snapPoints.push_back(*gridPoint);
        }
    }

    // Find closest snap point considering priority
    std::optional<SnapPoint> bestPoint;
    double bestScore = std::numeric_limits<double>::infinity();

    for (const auto& point : snapPoints) {
        double dist = std::hypot(
            point.position[0] - x,
            point.position[1] - y
        );

        // Score combines distance and priority
        double score = dist * point.priority;
        if (score < bestScore) {
            bestScore = score;
            bestPoint = point;
        }
    }

    return bestPoint;
}

template<typename T>
void SnapManager::addElement(const std::shared_ptr<T>& element) {
    if (element) {
        m_elements.push_back(std::static_pointer_cast<void>(element));
    }
}

template<typename T>
void SnapManager::removeElement(const std::shared_ptr<T>& element) {
    auto it = std::find(m_elements.begin(), m_elements.end(), 
                       std::static_pointer_cast<void>(element));
    if (it != m_elements.end()) {
        m_elements.erase(it);
    }
}

void SnapManager::clearElements() {
    m_elements.clear();
}

std::vector<SnapPoint> SnapManager::calculateSnapPoints(
    double x, double y, double radius) const {
    
    std::vector<SnapPoint> points;

    // Process each element
    for (const auto& element : m_elements) {
        // Try to cast to each supported type and calculate snap points
        if (auto line = std::dynamic_pointer_cast<Line>(element)) {
            addLineSnapPoints(line, x, y, radius, points);
        }
        else if (auto circle = std::dynamic_pointer_cast<Circle>(element)) {
            addCircleSnapPoints(circle, x, y, radius, points);
        }
        else if (auto arc = std::dynamic_pointer_cast<Arc>(element)) {
            addArcSnapPoints(arc, x, y, radius, points);
        }
        else if (auto rect = std::dynamic_pointer_cast<Rectangle>(element)) {
            addRectangleSnapPoints(rect, x, y, radius, points);
        }
        else if (auto poly = std::dynamic_pointer_cast<Polygon>(element)) {
            addPolygonSnapPoints(poly, x, y, radius, points);
        }
    }

    // Add intersection points if enabled
    if (isSnapTypeEnabled(SnapType::INTERSECTION)) {
        auto intersections = findIntersectionPoints(x, y, radius);
        points.insert(points.end(), intersections.begin(), intersections.end());
    }

    return points;
}

double SnapManager::calculateSnapRadius(
    int screenX, int screenY,
    double modelX, double modelY) const {
    
    // For now, use a fixed scale until ViewportManager is implemented
    // TODO: Replace with actual viewport scale
    const double scale = 1.0;
    return m_settings.snapRadius / scale;
}

std::optional<SnapPoint> SnapManager::findGridSnapPoint(
    double x, double y) const {
    
    // Calculate nearest grid point
    double gridX = std::round(x / m_settings.gridSize) * m_settings.gridSize;
    double gridY = std::round(y / m_settings.gridSize) * m_settings.gridSize;

    // Check if point is within snap radius
    double dist = std::hypot(gridX - x, gridY - y);
    if (dist <= m_settings.snapRadius) {
        return SnapPoint(
            {gridX, gridY},
            SnapType::GRID,
            1.5, // Slightly lower priority than geometric snaps
            "Grid"
        );
    }

    return std::nullopt;
}

std::vector<SnapPoint> SnapManager::findIntersectionPoints(
    double x, double y, double radius) const {
    
    std::vector<SnapPoint> points;

    // For each pair of elements
    for (size_t i = 0; i < m_elements.size(); ++i) {
        for (size_t j = i + 1; j < m_elements.size(); ++j) {
            // Calculate intersection points between elements
            // This would need type-specific intersection calculations
            // TODO: Implement intersection finding for each combination of types
        }
    }

    return points;
}

// Explicit template instantiations
template void SnapManager::addElement<Line>(const std::shared_ptr<Line>&);
template void SnapManager::addElement<Circle>(const std::shared_ptr<Circle>&);
template void SnapManager::addElement<Arc>(const std::shared_ptr<Arc>&);
template void SnapManager::addElement<Rectangle>(const std::shared_ptr<Rectangle>&);
template void SnapManager::addElement<Polygon>(const std::shared_ptr<Polygon>&);

template void SnapManager::removeElement<Line>(const std::shared_ptr<Line>&);
template void SnapManager::removeElement<Circle>(const std::shared_ptr<Circle>&);
template void SnapManager::removeElement<Arc>(const std::shared_ptr<Arc>&);
template void SnapManager::removeElement<Rectangle>(const std::shared_ptr<Rectangle>&);
template void SnapManager::removeElement<Polygon>(const std::shared_ptr<Polygon>&);

bool SnapManager::isPointInRange(
    double px, double py,
    double x, double y,
    double radius) const {
    return std::hypot(px - x, py - y) <= radius;
}

void SnapManager::addLineSnapPoints(
    const std::shared_ptr<Line>& line,
    double x, double y, double radius,
    std::vector<SnapPoint>& points) const {
    
    if (!line || !isSnapTypeEnabled(SnapType::ENDPOINT)) {
        return;
    }

    // Add endpoints
    auto start = line->getStartPoint();
    auto end = line->getEndPoint();

    if (isPointInRange(start[0], start[1], x, y, radius)) {
        points.emplace_back(start, SnapType::ENDPOINT, 1.0, "Line Start");
    }
    if (isPointInRange(end[0], end[1], x, y, radius)) {
        points.emplace_back(end, SnapType::ENDPOINT, 1.0, "Line End");
    }

    // Add midpoint if enabled
    if (isSnapTypeEnabled(SnapType::MIDPOINT)) {
        std::array<double, 2> mid = {
            (start[0] + end[0]) / 2.0,
            (start[1] + end[1]) / 2.0
        };
        if (isPointInRange(mid[0], mid[1], x, y, radius)) {
            points.emplace_back(mid, SnapType::MIDPOINT, 1.2, "Line Midpoint");
        }
    }
}

void SnapManager::addCircleSnapPoints(
    const std::shared_ptr<Circle>& circle,
    double x, double y, double radius,
    std::vector<SnapPoint>& points) const {
    
    if (!circle || !isSnapTypeEnabled(SnapType::CENTER)) {
        return;
    }

    // Add center point
    auto center = circle->getCenter();
    if (isPointInRange(center[0], center[1], x, y, radius)) {
        points.emplace_back(center, SnapType::CENTER, 1.0, "Circle Center");
    }

    // TODO: Add quadrant points when that snap type is added
}

void SnapManager::addArcSnapPoints(
    const std::shared_ptr<Arc>& arc,
    double x, double y, double radius,
    std::vector<SnapPoint>& points) const {
    
    if (!arc) {
        return;
    }

    // Add endpoints if enabled
    if (isSnapTypeEnabled(SnapType::ENDPOINT)) {
        auto start = arc->getStartPoint();
        auto end = arc->getEndPoint();

        if (isPointInRange(start[0], start[1], x, y, radius)) {
            points.emplace_back(start, SnapType::ENDPOINT, 1.0, "Arc Start");
        }
        if (isPointInRange(end[0], end[1], x, y, radius)) {
            points.emplace_back(end, SnapType::ENDPOINT, 1.0, "Arc End");
        }
    }

    // Add center if enabled
    if (isSnapTypeEnabled(SnapType::CENTER)) {
        auto center = arc->getCenter();
        if (isPointInRange(center[0], center[1], x, y, radius)) {
            points.emplace_back(center, SnapType::CENTER, 1.0, "Arc Center");
        }
    }
}

void SnapManager::addRectangleSnapPoints(
    const std::shared_ptr<Rectangle>& rect,
    double x, double y, double radius,
    std::vector<SnapPoint>& points) const {
    
    if (!rect) {
        return;
    }

    // Add corners if endpoint snapping is enabled
    if (isSnapTypeEnabled(SnapType::ENDPOINT)) {
        auto corners = rect->getCorners();
        for (size_t i = 0; i < corners.size(); ++i) {
            if (isPointInRange(corners[i][0], corners[i][1], x, y, radius)) {
                points.emplace_back(corners[i], SnapType::ENDPOINT, 1.0, 
                                  "Rectangle Corner " + std::to_string(i + 1));
            }
        }
    }

    // Add midpoints if enabled
    if (isSnapTypeEnabled(SnapType::MIDPOINT)) {
        auto corners = rect->getCorners();
        for (size_t i = 0; i < corners.size(); ++i) {
            size_t next = (i + 1) % corners.size();
            std::array<double, 2> mid = {
                (corners[i][0] + corners[next][0]) / 2.0,
                (corners[i][1] + corners[next][1]) / 2.0
            };
            if (isPointInRange(mid[0], mid[1], x, y, radius)) {
                points.emplace_back(mid, SnapType::MIDPOINT, 1.2,
                                  "Rectangle Edge Midpoint");
            }
        }
    }
}

void SnapManager::addPolygonSnapPoints(
    const std::shared_ptr<Polygon>& poly,
    double x, double y, double radius,
    std::vector<SnapPoint>& points) const {
    
    if (!poly) {
        return;
    }

    // Add vertices if endpoint snapping is enabled
    if (isSnapTypeEnabled(SnapType::ENDPOINT)) {
        auto vertices = poly->getVertices();
        for (size_t i = 0; i < vertices.size(); ++i) {
            if (isPointInRange(vertices[i][0], vertices[i][1], x, y, radius)) {
                points.emplace_back(vertices[i], SnapType::ENDPOINT, 1.0,
                                  "Polygon Vertex " + std::to_string(i + 1));
            }
        }
    }

    // Add edge midpoints if enabled
    if (isSnapTypeEnabled(SnapType::MIDPOINT)) {
        auto vertices = poly->getVertices();
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t next = (i + 1) % vertices.size();
            std::array<double, 2> mid = {
                (vertices[i][0] + vertices[next][0]) / 2.0,
                (vertices[i][1] + vertices[next][1]) / 2.0
            };
            if (isPointInRange(mid[0], mid[1], x, y, radius)) {
                points.emplace_back(mid, SnapType::MIDPOINT, 1.2,
                                  "Polygon Edge Midpoint");
            }
        }
    }
}

} // namespace Sketching
} // namespace RebelCAD
