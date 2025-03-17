#pragma once

#include <array>
#include <memory>
#include <vector>
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

class Line;
class Arc;

/**
 * @class Circle
 * @brief Represents a 2D circle in the sketching system
 * 
 * The Circle class provides functionality for creating and manipulating
 * 2D circles. It handles both the geometric representation and
 * rendering of circles in the sketching system.
 */
class Circle {
public:
    /**
     * @brief Constructs a circle with specified center and radius
     * @param centerX X-coordinate of center point
     * @param centerY Y-coordinate of center point
     * @param radius Radius of the circle
     * @throws Error if parameters are invalid
     */
    Circle(float centerX, float centerY, float radius);

    /**
     * @brief Gets the center point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getCenter() const { return {mCenterX, mCenterY}; }

    /**
     * @brief Gets the radius of the circle
     * @return Radius value
     */
    float getRadius() const { return mRadius; }

    /**
     * @brief Gets the circumference of the circle
     * @return Circumference value
     */
    float getCircumference() const;

    /**
     * @brief Gets the area of the circle
     * @return Area value
     */
    float getArea() const;

    /**
     * @brief Moves the circle by the specified offset
     * @param dx X-axis displacement
     * @param dy Y-axis displacement
     */
    void translate(float dx, float dy);

    /**
     * @brief Sets the center point of the circle
     * @param x X-coordinate of center point
     * @param y Y-coordinate of center point
     * @throws Error if coordinates are invalid
     */
    void setCenter(float x, float y);

    /**
     * @brief Sets the radius of the circle
     * @param radius New radius value
     * @throws Error if radius is invalid
     */
    void setRadius(float radius);

    /**
     * @brief Checks if a point lies on the circle
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @param tolerance Distance tolerance for considering point on circle
     * @return true if point lies on circle within tolerance
     */
    bool containsPoint(float x, float y, float tolerance = 1e-5f) const;

    /**
     * @brief Checks if a point lies inside the circle
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @return true if point lies inside circle
     */
    bool containsPointInside(float x, float y) const;

    /**
     * @brief Gets the point on the circle at specified angle
     * @param angle Angle in radians from positive X-axis
     * @return Array containing {x, y} coordinates of point
     */
    std::array<float, 2> getPointAtAngle(float angle) const;

    /**
     * @brief Finds intersection points with a line
     * @param line The line to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getLineIntersections(const Line& line) const;

    /**
     * @brief Finds intersection points with an arc
     * @param arc The arc to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getArcIntersections(const Arc& arc) const;

    /**
     * @brief Finds intersection points with another circle
     * @param other The other circle to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getCircleIntersections(const Circle& other) const;

    /**
     * @brief Creates a concentric circle at specified radius
     * @param newRadius Radius for the new concentric circle
     * @return New circle concentric with this one
     */
    Circle createConcentricCircle(float newRadius) const;

    /**
     * @brief Creates an arc from this circle between specified angles
     * @param startAngle Start angle in radians
     * @param endAngle End angle in radians
     * @return Arc representing the specified portion of this circle
     */
    Arc createArc(float startAngle, float endAngle) const;

    /**
     * @brief Renders the circle using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Circle color
     * @param thickness Circle thickness in pixels
     * @param dashPattern Optional array of dash lengths for dashed circles
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                const Graphics::Color& color,
                float thickness = 1.0f,
                const std::vector<float>* dashPattern = nullptr);

private:
    float mCenterX;
    float mCenterY;
    float mRadius;

    // Validates parameters
    void validateCoordinates(float x, float y) const;
    void validateRadius(float radius) const;
};

} // namespace Sketching
} // namespace RebelCAD
