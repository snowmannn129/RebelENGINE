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
 * @class Arc
 * @brief Represents a 2D circular arc in the sketching system
 * 
 * The Arc class provides functionality for creating and manipulating
 * 2D circular arcs. It handles both the geometric representation and
 * rendering of arcs in the sketching system.
 */
class Arc {
public:
    /**
     * @brief Constructs an arc defined by center, radius, and angles
     * @param centerX X-coordinate of center point
     * @param centerY Y-coordinate of center point
     * @param radius Radius of the arc
     * @param startAngle Starting angle in radians
     * @param endAngle Ending angle in radians
     * @throws Error if parameters are invalid
     */
    Arc(float centerX, float centerY, float radius, float startAngle, float endAngle);

    /**
     * @brief Gets the center point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getCenter() const { return {mCenterX, mCenterY}; }

    /**
     * @brief Gets the radius of the arc
     * @return Radius value
     */
    float getRadius() const { return mRadius; }

    /**
     * @brief Gets the start angle in radians
     * @return Start angle
     */
    float getStartAngle() const { return mStartAngle; }

    /**
     * @brief Gets the end angle in radians
     * @return End angle
     */
    float getEndAngle() const { return mEndAngle; }

    /**
     * @brief Gets the sweep angle in radians
     * @return Sweep angle (end - start)
     */
    float getSweepAngle() const;

    /**
     * @brief Gets the start point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getStartPoint() const;

    /**
     * @brief Gets the end point coordinates
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getEndPoint() const;

    /**
     * @brief Calculates the length of the arc
     * @return Length of the arc
     */
    float getLength() const;

    /**
     * @brief Moves the arc by the specified offset
     * @param dx X-axis displacement
     * @param dy Y-axis displacement
     */
    void translate(float dx, float dy);

    /**
     * @brief Rotates the arc around its center
     * @param angleRadians Rotation angle in radians
     */
    void rotate(float angleRadians);

    /**
     * @brief Sets the center point of the arc
     * @param x X-coordinate of center point
     * @param y Y-coordinate of center point
     * @throws Error if coordinates are invalid
     */
    void setCenter(float x, float y);

    /**
     * @brief Sets the radius of the arc
     * @param radius New radius value
     * @throws Error if radius is invalid
     */
    void setRadius(float radius);

    /**
     * @brief Sets the start angle of the arc
     * @param angleRadians Start angle in radians
     */
    void setStartAngle(float angleRadians);

    /**
     * @brief Sets the end angle of the arc
     * @param angleRadians End angle in radians
     */
    void setEndAngle(float angleRadians);

    /**
     * @brief Gets the midpoint of the arc
     * @return Array containing {x, y} coordinates of midpoint
     */
    std::array<float, 2> getMidpoint() const;

    /**
     * @brief Checks if a point lies on the arc
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @param tolerance Distance tolerance for considering point on arc
     * @return true if point lies on arc within tolerance
     */
    bool containsPoint(float x, float y, float tolerance = 1e-5f) const;

    /**
     * @brief Finds intersection points with a line
     * @param line The line to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getLineIntersections(const Line& line) const;

    /**
     * @brief Finds intersection points with another arc
     * @param other The other arc to intersect with
     * @return Vector of arrays containing {x, y} coordinates of intersection points
     */
    std::vector<std::array<float, 2>> getArcIntersections(const Arc& other) const;

    /**
     * @brief Creates a concentric arc at specified radius
     * @param newRadius Radius for the new concentric arc
     * @return New arc concentric with this one
     */
    Arc createConcentricArc(float newRadius) const;

    /**
     * @brief Renders the arc using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Arc color
     * @param thickness Arc thickness in pixels
     * @param dashPattern Optional array of dash lengths for dashed arcs
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
               const Graphics::Color& color,
               float thickness = 1.0f,
               const std::vector<float>* dashPattern = nullptr);

private:
    float mCenterX;
    float mCenterY;
    float mRadius;
    float mStartAngle;
    float mEndAngle;

    // Validates parameters
    void validateCoordinates(float x, float y) const;
    void validateRadius(float radius) const;
    void validateAngles(float startAngle, float endAngle) const;

    // Helper methods for geometric calculations
    bool isAngleInRange(float angle) const;
    std::array<float, 2> getPointAtAngle(float angle) const;
    float normalizeAngle(float angle) const;
};

} // namespace Sketching
} // namespace RebelCAD
