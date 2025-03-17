#include "sketching/Line.h"
#include "graphics/Color.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace RebelCAD {
namespace Sketching {

Line::Line(float startX, float startY, float endX, float endY)
    : mStartX(startX), mStartY(startY), mEndX(endX), mEndY(endY) {
    validateCoordinates(startX, startY);
    validateCoordinates(endX, endY);
}

float Line::getLength() const {
    float dx = mEndX - mStartX;
    float dy = mEndY - mStartY;
    return std::sqrt(dx * dx + dy * dy);
}

void Line::translate(float dx, float dy) {
    mStartX += dx;
    mStartY += dy;
    mEndX += dx;
    mEndY += dy;
}

void Line::rotate(float angleRadians) {
    // Rotate around start point
    float dx = mEndX - mStartX;
    float dy = mEndY - mStartY;
    
    float cosA = std::cos(angleRadians);
    float sinA = std::sin(angleRadians);
    
    // Apply rotation matrix
    float newDx = dx * cosA - dy * sinA;
    float newDy = dx * sinA + dy * cosA;
    
    mEndX = mStartX + newDx;
    mEndY = mStartY + newDy;
}

void Line::setStartPoint(float x, float y) {
    validateCoordinates(x, y);
    mStartX = x;
    mStartY = y;
}

void Line::setEndPoint(float x, float y) {
    validateCoordinates(x, y);
    mEndX = x;
    mEndY = y;
}

std::array<float, 2> Line::getMidpoint() const {
    return {(mStartX + mEndX) / 2.0f, (mStartY + mEndY) / 2.0f};
}

float Line::getAngleTo(const Line& other) const {
    float dx1 = mEndX - mStartX;
    float dy1 = mEndY - mStartY;
    float dx2 = other.mEndX - other.mStartX;
    float dy2 = other.mEndY - other.mStartY;
    
    float dot = dx1 * dx2 + dy1 * dy2;
    float len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    float len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
    
    float angle = std::acos(std::clamp(dot / (len1 * len2), -1.0f, 1.0f));
    return angle;
}

bool Line::containsPoint(float x, float y, float tolerance) const {
    // Check if point is within bounding box (expanded by tolerance)
    float minX = std::min(mStartX, mEndX) - tolerance;
    float maxX = std::max(mStartX, mEndX) + tolerance;
    float minY = std::min(mStartY, mEndY) - tolerance;
    float maxY = std::max(mStartY, mEndY) + tolerance;
    
    if (x < minX || x > maxX || y < minY || y > maxY) {
        return false;
    }
    
    // Check distance from point to line
    return getDistanceToPoint(x, y) <= tolerance;
}

std::optional<std::array<float, 2>> Line::getIntersection(const Line& other) const {
    float x1 = mStartX, y1 = mStartY;
    float x2 = mEndX, y2 = mEndY;
    float x3 = other.mStartX, y3 = other.mStartY;
    float x4 = other.mEndX, y4 = other.mEndY;
    
    float denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denominator) < 1e-5f) {
        return std::nullopt; // Lines are parallel
    }
    
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denominator;
    
    // Check if intersection point lies on both line segments
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        float x = x1 + t * (x2 - x1);
        float y = y1 + t * (y2 - y1);
        return std::array<float, 2>{x, y};
    }
    
    return std::nullopt;
}

Line Line::createParallelLine(float distance) const {
    float dx = mEndX - mStartX;
    float dy = mEndY - mStartY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    // Calculate perpendicular vector
    float perpX = -dy / length * distance;
    float perpY = dx / length * distance;
    
    return Line(mStartX + perpX, mStartY + perpY, 
               mEndX + perpX, mEndY + perpY);
}

Line Line::createPerpendicularLine(float x, float y) const {
    float dx = mEndX - mStartX;
    float dy = mEndY - mStartY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    // Create perpendicular vector of same length
    float perpX = -dy / length * length;
    float perpY = dx / length * length;
    
    return Line(x, y, x + perpX, y + perpY);
}

void Line::render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                 const Graphics::Color& color,
                 float thickness,
                 const std::vector<float>* dashPattern) {
    graphics->setLineColor(color);
    graphics->setLineThickness(thickness);
    if (dashPattern) {
        graphics->setLineDashPattern(*dashPattern);
    }
    graphics->drawLine(mStartX, mStartY, mEndX, mEndY);
}

float Line::getSlope() const {
    float dx = mEndX - mStartX;
    if (std::abs(dx) < 1e-5f) {
        return std::numeric_limits<float>::infinity();
    }
    return (mEndY - mStartY) / dx;
}

float Line::getYIntercept() const {
    float slope = getSlope();
    if (std::isinf(slope)) {
        return std::numeric_limits<float>::infinity();
    }
    return mStartY - slope * mStartX;
}

float Line::getDistanceToPoint(float x, float y) const {
    float dx = mEndX - mStartX;
    float dy = mEndY - mStartY;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 1e-5f) {
        return std::sqrt((x - mStartX) * (x - mStartX) + 
                        (y - mStartY) * (y - mStartY));
    }
    
    // Calculate cross product and divide by length
    return std::abs((dx * (mStartY - y) - (mStartX - x) * dy) / length);
}

void Line::validateCoordinates(float x, float y) const {
    // Check for NaN or infinite values
    if (std::isnan(x) || std::isnan(y) || 
        std::isinf(x) || std::isinf(y)) {
        throw Error("Invalid coordinate values: coordinates must be finite numbers");
    }

    // Check for values exceeding reasonable bounds
    // Using a large but finite limit for coordinate values
    const float MAX_COORD = 1e6f;
    if (std::abs(x) > MAX_COORD || std::abs(y) > MAX_COORD) {
        throw Error("Coordinate values exceed maximum allowed range");
    }
}

} // namespace Sketching
} // namespace RebelCAD
