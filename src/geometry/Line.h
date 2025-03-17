#pragma once

#include <array>
#include <memory>
#include <optional>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"

// Forward declarations
namespace RebelCAD {
namespace Graphics {
    struct Color;
} // namespace Graphics
} // namespace RebelCAD

namespace RebelCAD {
namespace Sketching {

/**
 * @class Line
 * @brief Represents a 2D line segment in the sketching system
 * 
 * The Line class provides functionality for creating and manipulating
 * 2D line segments. It handles both the geometric representation and
 * rendering of lines in the sketching system.
 */
class Line {
public:
    /**
     * @brief Constructs a line segment between two points
     * @param startX X-coordinate of start point
     * @param startY Y-coordinate of start point
     * @param endX X-coordinate of end point
     * @param endY Y-coordinate of end point
     * @throws Error if coordinates are invalid
     */
    Line(float startX, float startY, float endX, float endY);

    /**
     * @brief Gets the start point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getStartPoint() const { return {mStartX, mStartY}; }

    /**
     * @brief Gets the end point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getEndPoint() const { return {mEndX, mEndY}; }

    /**
     * @brief Calculates the length of the line segment
     * @return Length of the line
     */
    float getLength() const;

    /**
     * @brief Moves the line by the specified offset
     * @param dx X-axis displacement
     * @param dy Y-axis displacement
     */
    void translate(float dx, float dy);

    /**
     * @brief Rotates the line around its start point
     * @param angleRadians Rotation angle in radians
     */
    void rotate(float angleRadians);

    /**
     * @brief Sets the start point of the line
     * @param x X-coordinate of start point
     * @param y Y-coordinate of start point
     * @throws Error if coordinates are invalid
     */
    void setStartPoint(float x, float y);

    /**
     * @brief Sets the end point of the line
     * @param x X-coordinate of end point
     * @param y Y-coordinate of end point
     * @throws Error if coordinates are invalid
     */
    void setEndPoint(float x, float y);

    /**
     * @brief Gets the midpoint of the line
     * @return Array containing {x, y} coordinates of midpoint
     */
    std::array<float, 2> getMidpoint() const;

    /**
     * @brief Calculates the angle between this line and another line
     * @param other The other line to calculate angle with
     * @return Angle in radians between the lines
     */
    float getAngleTo(const Line& other) const;

    /**
     * @brief Checks if a point lies on the line segment
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @param tolerance Distance tolerance for considering point on line
     * @return true if point lies on line segment within tolerance
     */
    bool containsPoint(float x, float y, float tolerance = 1e-5f) const;

    /**
     * @brief Finds intersection point with another line segment
     * @param other The other line segment to intersect with
     * @return Optional array containing {x, y} coordinates of intersection point,
     *         or empty optional if lines don't intersect
     */
    std::optional<std::array<float, 2>> getIntersection(const Line& other) const;

    /**
     * @brief Creates a parallel line at specified distance
     * @param distance Perpendicular distance to create parallel line at (positive = left side)
     * @return New line parallel to this one
     */
    Line createParallelLine(float distance) const;

    /**
     * @brief Creates a perpendicular line at specified point
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @return New line perpendicular to this one through given point
     */
    Line createPerpendicularLine(float x, float y) const;

    /**
     * @brief Renders the line using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Line color
     * @param thickness Line thickness in pixels
     * @param dashPattern Optional array of dash lengths for dashed lines
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
               const Graphics::Color& color,
               float thickness = 1.0f,
               const std::vector<float>* dashPattern = nullptr);

private:
    float mStartX;
    float mStartY;
    float mEndX;
    float mEndY;

    // Validates coordinate values
    void validateCoordinates(float x, float y) const;

    // Helper methods for geometric calculations
    float getSlope() const;
    float getYIntercept() const;
    float getDistanceToPoint(float x, float y) const;
};

} // namespace Sketching
} // namespace RebelCAD
