#include "sketching/Circle.h"
#include "sketching/Line.h"
#include "sketching/Arc.h"
#include "graphics/Color.h"
#include "core/Error.h"
#include <cmath>

namespace RebelCAD {
namespace Sketching {

Circle::Circle(float centerX, float centerY, float radius)
    : mCenterX(centerX), mCenterY(centerY), mRadius(radius) {
    validateCoordinates(centerX, centerY);
    validateRadius(radius);
}

float Circle::getCircumference() const {
    return 2.0f * M_PI * mRadius;
}

float Circle::getArea() const {
    return M_PI * mRadius * mRadius;
}

void Circle::translate(float dx, float dy) {
    mCenterX += dx;
    mCenterY += dy;
}

void Circle::setCenter(float x, float y) {
    validateCoordinates(x, y);
    mCenterX = x;
    mCenterY = y;
}

void Circle::setRadius(float radius) {
    validateRadius(radius);
    mRadius = radius;
}

bool Circle::containsPoint(float x, float y, float tolerance) const {
    float dx = x - mCenterX;
    float dy = y - mCenterY;
    float distance = std::sqrt(dx * dx + dy * dy);
    return std::abs(distance - mRadius) <= tolerance;
}

bool Circle::containsPointInside(float x, float y) const {
    float dx = x - mCenterX;
    float dy = y - mCenterY;
    float distanceSquared = dx * dx + dy * dy;
    return distanceSquared <= mRadius * mRadius;
}

std::array<float, 2> Circle::getPointAtAngle(float angle) const {
    return {
        mCenterX + mRadius * std::cos(angle),
        mCenterY + mRadius * std::sin(angle)
    };
}

std::vector<std::array<float, 2>> Circle::getLineIntersections(const Line& line) const {
    std::vector<std::array<float, 2>> intersections;
    
    // Get line start and end points
    auto lineStart = line.getStartPoint();
    auto lineEnd = line.getEndPoint();
    
    // Calculate line parameters
    float x1 = lineStart[0] - mCenterX;
    float y1 = lineStart[1] - mCenterY;
    float x2 = lineEnd[0] - mCenterX;
    float y2 = lineEnd[1] - mCenterY;
    
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dr = std::sqrt(dx * dx + dy * dy);
    float D = x1 * y2 - x2 * y1;
    
    // Calculate discriminant
    float discriminant = mRadius * mRadius * dr * dr - D * D;
    
    if (discriminant < 0) {
        return intersections; // No intersection
    }
    
    float sqrtDisc = std::sqrt(discriminant);
    float drSq = dr * dr;
    
    // Calculate intersection points
    std::array<std::array<float, 2>, 2> points = {{
        {mCenterX + (D * dy + (dy >= 0 ? 1 : -1) * dx * sqrtDisc) / drSq,
         mCenterY + (-D * dx + std::abs(dy) * sqrtDisc) / drSq},
        {mCenterX + (D * dy - (dy >= 0 ? 1 : -1) * dx * sqrtDisc) / drSq,
         mCenterY + (-D * dx - std::abs(dy) * sqrtDisc) / drSq}
    }};
    
    // Check which points lie on the line segment
    for (const auto& point : points) {
        if (line.containsPoint(point[0], point[1])) {
            intersections.push_back(point);
        }
    }
    
    return intersections;
}

std::vector<std::array<float, 2>> Circle::getArcIntersections(const Arc& arc) const {
    // Delegate to Arc's intersection method
    return arc.getCircleIntersections(*this);
}

std::vector<std::array<float, 2>> Circle::getCircleIntersections(const Circle& other) const {
    std::vector<std::array<float, 2>> intersections;
    
    // Calculate distance between centers
    float dx = other.mCenterX - mCenterX;
    float dy = other.mCenterY - mCenterY;
    float centerDist = std::sqrt(dx * dx + dy * dy);
    
    // Check for no intersection cases
    if (centerDist > mRadius + other.mRadius || 
        centerDist < std::abs(mRadius - other.mRadius) ||
        (centerDist == 0 && mRadius == other.mRadius)) {
        return intersections;
    }
    
    // Calculate intersection points
    float a = (mRadius * mRadius - other.mRadius * other.mRadius + centerDist * centerDist) 
              / (2 * centerDist);
    float h = std::sqrt(mRadius * mRadius - a * a);
    
    float x2 = mCenterX + (dx * a) / centerDist;
    float y2 = mCenterY + (dy * a) / centerDist;
    
    // Calculate the two intersection points
    float factor = h / centerDist;
    intersections.push_back({
        x2 + factor * dy,
        y2 - factor * dx
    });
    
    // If circles are tangent, return only one point
    if (h > 0) {
        intersections.push_back({
            x2 - factor * dy,
            y2 + factor * dx
        });
    }
    
    return intersections;
}

Circle Circle::createConcentricCircle(float newRadius) const {
    validateRadius(newRadius);
    return Circle(mCenterX, mCenterY, newRadius);
}

Arc Circle::createArc(float startAngle, float endAngle) const {
    return Arc(mCenterX, mCenterY, mRadius, startAngle, endAngle);
}

void Circle::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                   const Graphics::Color& color,
                   float thickness,
                   const std::vector<float>* dashPattern) {
    // Number of segments to approximate the circle
    int segments = static_cast<int>(mRadius / 2.0f) + 32;
    float angleStep = 2.0f * M_PI / segments;
    
    std::vector<float> vertices;
    vertices.reserve((segments + 1) * 2);
    
    // Generate vertices for the circle
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        auto point = getPointAtAngle(angle);
        vertices.push_back(point[0]);
        vertices.push_back(point[1]);
    }
    
    // Render the circle using the graphics system
    graphics->drawLineStrip(vertices, color, thickness, dashPattern);
}

void Circle::validateCoordinates(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw Error("Invalid coordinates for circle");
    }
}

void Circle::validateRadius(float radius) const {
    if (!std::isfinite(radius) || radius <= 0) {
        throw Error("Invalid radius for circle");
    }
}

} // namespace Sketching
} // namespace RebelCAD
