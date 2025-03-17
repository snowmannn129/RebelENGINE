#include "sketching/Arc.h"
#include "sketching/Line.h"
#include "graphics/Color.h"
#include "core/Error.h"
#include <cmath>

namespace RebelCAD {
namespace Sketching {

Arc::Arc(float centerX, float centerY, float radius, float startAngle, float endAngle)
    : mCenterX(centerX), mCenterY(centerY), mRadius(radius),
      mStartAngle(normalizeAngle(startAngle)), mEndAngle(normalizeAngle(endAngle)) {
    validateCoordinates(centerX, centerY);
    validateRadius(radius);
    validateAngles(startAngle, endAngle);
}

float Arc::getSweepAngle() const {
    float sweep = mEndAngle - mStartAngle;
    if (sweep < 0) {
        sweep += 2 * M_PI;
    }
    return sweep;
}

std::array<float, 2> Arc::getStartPoint() const {
    return getPointAtAngle(mStartAngle);
}

std::array<float, 2> Arc::getEndPoint() const {
    return getPointAtAngle(mEndAngle);
}

float Arc::getLength() const {
    return mRadius * std::abs(getSweepAngle());
}

void Arc::translate(float dx, float dy) {
    mCenterX += dx;
    mCenterY += dy;
}

void Arc::rotate(float angleRadians) {
    mStartAngle = normalizeAngle(mStartAngle + angleRadians);
    mEndAngle = normalizeAngle(mEndAngle + angleRadians);
}

void Arc::setCenter(float x, float y) {
    validateCoordinates(x, y);
    mCenterX = x;
    mCenterY = y;
}

void Arc::setRadius(float radius) {
    validateRadius(radius);
    mRadius = radius;
}

void Arc::setStartAngle(float angleRadians) {
    float normalized = normalizeAngle(angleRadians);
    validateAngles(normalized, mEndAngle);
    mStartAngle = normalized;
}

void Arc::setEndAngle(float angleRadians) {
    float normalized = normalizeAngle(angleRadians);
    validateAngles(mStartAngle, normalized);
    mEndAngle = normalized;
}

std::array<float, 2> Arc::getMidpoint() const {
    float midAngle = mStartAngle + getSweepAngle() / 2.0f;
    return getPointAtAngle(midAngle);
}

bool Arc::containsPoint(float x, float y, float tolerance) const {
    // First check if point is at the correct distance from center
    float dx = x - mCenterX;
    float dy = y - mCenterY;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (std::abs(distance - mRadius) > tolerance) {
        return false;
    }

    // Then check if point lies within the angular range
    float angle = normalizeAngle(std::atan2(dy, dx));
    return isAngleInRange(angle);
}

std::vector<std::array<float, 2>> Arc::getLineIntersections(const Line& line) const {
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
    
    // Check which points lie within the arc's angular range
    for (const auto& point : points) {
        if (containsPoint(point[0], point[1]) && 
            line.containsPoint(point[0], point[1])) {
            intersections.push_back(point);
        }
    }
    
    return intersections;
}

std::vector<std::array<float, 2>> Arc::getArcIntersections(const Arc& other) const {
    std::vector<std::array<float, 2>> intersections;
    
    // Calculate distance between centers
    float dx = other.mCenterX - mCenterX;
    float dy = other.mCenterY - mCenterY;
    float centerDist = std::sqrt(dx * dx + dy * dy);
    
    // Check for no intersection cases
    if (centerDist > mRadius + other.mRadius || 
        centerDist < std::abs(mRadius - other.mRadius)) {
        return intersections;
    }
    
    // Calculate intersection points
    float a = (mRadius * mRadius - other.mRadius * other.mRadius + centerDist * centerDist) 
              / (2 * centerDist);
    float h = std::sqrt(mRadius * mRadius - a * a);
    
    float x2 = mCenterX + (dx * a) / centerDist;
    float y2 = mCenterY + (dy * a) / centerDist;
    
    // Calculate the two potential intersection points
    std::array<std::array<float, 2>, 2> points = {{
        {x2 + (h * dy) / centerDist, y2 - (h * dx) / centerDist},
        {x2 - (h * dy) / centerDist, y2 + (h * dx) / centerDist}
    }};
    
    // Check which points lie within both arcs' angular ranges
    for (const auto& point : points) {
        if (containsPoint(point[0], point[1]) && 
            other.containsPoint(point[0], point[1])) {
            intersections.push_back(point);
        }
    }
    
    return intersections;
}

Arc Arc::createConcentricArc(float newRadius) const {
    validateRadius(newRadius);
    return Arc(mCenterX, mCenterY, newRadius, mStartAngle, mEndAngle);
}

void Arc::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                const Graphics::Color& color,
                float thickness,
                const std::vector<float>* dashPattern) {
    // Number of segments to approximate the arc
    int segments = static_cast<int>(mRadius * getSweepAngle() / 5.0f) + 1;
    float angleStep = getSweepAngle() / segments;
    
    std::vector<float> vertices;
    vertices.reserve((segments + 1) * 2);
    
    // Generate vertices for the arc
    for (int i = 0; i <= segments; ++i) {
        float angle = mStartAngle + i * angleStep;
        auto point = getPointAtAngle(angle);
        vertices.push_back(point[0]);
        vertices.push_back(point[1]);
    }
    
    // Render the arc using the graphics system
    graphics->drawLineStrip(vertices, color, thickness, dashPattern);
}

void Arc::validateCoordinates(float x, float y) const {
    if (!std::isfinite(x) || !std::isfinite(y)) {
        throw Error("Invalid coordinates for arc");
    }
}

void Arc::validateRadius(float radius) const {
    if (!std::isfinite(radius) || radius <= 0) {
        throw Error("Invalid radius for arc");
    }
}

void Arc::validateAngles(float startAngle, float endAngle) const {
    if (!std::isfinite(startAngle) || !std::isfinite(endAngle)) {
        throw Error("Invalid angles for arc");
    }
}

bool Arc::isAngleInRange(float angle) const {
    angle = normalizeAngle(angle);
    if (mStartAngle <= mEndAngle) {
        return angle >= mStartAngle && angle <= mEndAngle;
    } else {
        return angle >= mStartAngle || angle <= mEndAngle;
    }
}

std::array<float, 2> Arc::getPointAtAngle(float angle) const {
    return {
        mCenterX + mRadius * std::cos(angle),
        mCenterY + mRadius * std::sin(angle)
    };
}

float Arc::normalizeAngle(float angle) const {
    // Normalize angle to [0, 2π)
    angle = std::fmod(angle, 2 * M_PI);
    if (angle < 0) {
        angle += 2 * M_PI;
    }
    return angle;
}

} // namespace Sketching
} // namespace RebelCAD
