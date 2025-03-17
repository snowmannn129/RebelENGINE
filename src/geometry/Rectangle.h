#pragma once

#include <array>
#include <memory>
#include <vector>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/Color.h"

namespace RebelCAD {
namespace Sketching {

class Line;
class Circle;

/**
 * @class Rectangle
 * @brief Represents a 2D rectangle in the sketching system
 * 
 * The Rectangle class provides functionality for creating and manipulating
 * 2D rectangles. It supports both axis-aligned and rotated rectangles,
 * with comprehensive geometric operations and intersection testing.
 */
class Rectangle {
public:
    /**
     * @brief Constructs a rectangle with specified corners
     * @param x1 X-coordinate of the first corner
     * @param y1 Y-coordinate of the first corner
     * @param x2 X-coordinate of the second corner
     * @param y2 Y-coordinate of the second corner
     * @throws Error if coordinates are invalid
     */
    Rectangle(float x1, float y1, float x2, float y2);

    /**
     * @brief Gets the coordinates of the first corner
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getFirstCorner() const { return {m_x1, m_y1}; }

    /**
     * @brief Gets the coordinates of the second corner
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getSecondCorner() const { return {m_x2, m_y2}; }

    /**
     * @brief Gets the width of the rectangle
     * @return Width value
     */
    float getWidth() const;

    /**
     * @brief Gets the height of the rectangle
     * @return Height value
     */
    float getHeight() const;

    /**
     * @brief Gets the area of the rectangle
     * @return Area value
     */
    float getArea() const;

    /**
     * @brief Gets the perimeter of the rectangle
     * @return Perimeter value
     */
    float getPerimeter() const;

    /**
     * @brief Gets the center point of the rectangle
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getCenter() const;

    /**
     * @brief Moves the rectangle by the specified offset
     * @param dx X-axis displacement
     * @param dy Y-axis displacement
     */
    void translate(float dx, float dy);

    /**
     * @brief Rotates the rectangle around its center
     * @param angleRadians Rotation angle in radians
     */
    void rotate(float angleRadians);

    /**
     * @brief Sets the first corner of the rectangle
     * @param x X-coordinate
     * @param y Y-coordinate
     * @throws Error if coordinates are invalid
     */
    void setFirstCorner(float x, float y);

    /**
     * @brief Sets the second corner of the rectangle
     * @param x X-coordinate
     * @param y Y-coordinate
     * @throws Error if coordinates are invalid
     */
    void setSecondCorner(float x, float y);

    /**
     * @brief Checks if a point lies inside the rectangle
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @return true if point is inside rectangle
     */
    bool containsPoint(float x, float y) const;

    /**
     * @brief Checks if a point lies on the rectangle's perimeter
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @param tolerance Distance tolerance for considering point on perimeter
     * @return true if point lies on perimeter within tolerance
     */
    bool containsPointOnPerimeter(float x, float y, float tolerance = 1e-5f) const;

    /**
     * @brief Gets the four corners of the rectangle
     * @return Vector of arrays containing {x, y} coordinates of corners
     */
    std::vector<std::array<float, 2>> getCorners() const;

    /**
     * @brief Gets the four lines that make up the rectangle
     * @return Vector of Line objects representing the rectangle's edges
     */
    std::vector<Line> getEdges() const;

    /**
     * @brief Finds intersection points with a line
     * @param line The line to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getLineIntersections(const Line& line) const;

    /**
     * @brief Finds intersection points with a circle
     * @param circle The circle to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getCircleIntersections(const Circle& circle) const;

    /**
     * @brief Finds intersection points with another rectangle
     * @param other The other rectangle to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getRectangleIntersections(const Rectangle& other) const;

    /**
     * @brief Checks if this rectangle intersects with another
     * @param other The other rectangle to check
     * @return true if rectangles intersect
     */
    bool intersectsWith(const Rectangle& other) const;

    /**
     * @brief Creates a rectangle expanded by specified amounts
     * @param dx Amount to expand in x direction
     * @param dy Amount to expand in y direction
     * @return New expanded rectangle
     */
    Rectangle createExpanded(float dx, float dy) const;

    /**
     * @brief Renders the rectangle using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Rectangle color
     * @param thickness Line thickness in pixels
     * @param dashPattern Optional array of dash lengths for dashed lines
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                const Graphics::Color& color,
                float thickness = 1.0f,
                const std::vector<float>* dashPattern = nullptr);

private:
    float m_x1, m_y1;  // First corner coordinates
    float m_x2, m_y2;  // Second corner coordinates
    float m_rotation;  // Rotation angle in radians

    // Validates coordinate values
    void validateCoordinates(float x, float y) const;

    // Helper methods
    void normalizeCoordinates();
    bool isAxisAligned() const;
    std::array<float, 2> rotatePoint(float x, float y, float cx, float cy, float angle) const;
};

} // namespace Sketching
} // namespace RebelCAD
