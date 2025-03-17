#pragma once

#include <vector>
#include <memory>
#include <array>
#include "core/Error.h"
#include "graphics/GraphicsSystem.h"
#include "sketching/Line.h"

// Forward declarations
namespace RebelCAD {
namespace Graphics {
    struct Color;
} // namespace Graphics
} // namespace RebelCAD

namespace RebelCAD {
namespace Sketching {

/**
 * @class Polygon
 * @brief Represents a 2D polygon in the sketching system
 * 
 * The Polygon class provides functionality for creating and manipulating
 * 2D polygons. It supports both regular polygons (equal sides and angles)
 * and irregular polygons. The polygon is represented as a collection of
 * connected line segments that form a closed shape.
 */
class Polygon {
public:
    /**
     * @brief Constructs a regular polygon
     * @param centerX X-coordinate of center point
     * @param centerY Y-coordinate of center point
     * @param radius Distance from center to vertices
     * @param sides Number of sides (must be >= 3)
     * @param rotationRadians Initial rotation in radians (0 = first vertex at right)
     * @throws Error if parameters are invalid
     */
    Polygon(float centerX, float centerY, float radius, int sides, float rotationRadians = 0.0f);

    /**
     * @brief Constructs an irregular polygon from a list of vertices
     * @param vertices Vector of vertex coordinates as {x, y} pairs
     * @throws Error if fewer than 3 vertices or vertices are invalid
     */
    explicit Polygon(const std::vector<std::array<float, 2>>& vertices);

    /**
     * @brief Gets all vertex coordinates
     * @return Vector of {x, y} coordinate pairs
     */
    const std::vector<std::array<float, 2>>& getVertices() const { return mVertices; }

    /**
     * @brief Gets the center point of the polygon
     * @return Array containing {x, y} coordinates of centroid
     */
    std::array<float, 2> getCenter() const;

    /**
     * @brief Gets the number of sides/vertices
     * @return Number of polygon sides
     */
    size_t getSideCount() const { return mVertices.size(); }

    /**
     * @brief Calculates the perimeter of the polygon
     * @return Sum of all side lengths
     */
    float getPerimeter() const;

    /**
     * @brief Calculates the area of the polygon
     * @return Area of the polygon
     */
    float getArea() const;

    /**
     * @brief Moves the polygon by the specified offset
     * @param dx X-axis displacement
     * @param dy Y-axis displacement
     */
    void translate(float dx, float dy);

    /**
     * @brief Rotates the polygon around its center
     * @param angleRadians Rotation angle in radians
     */
    void rotate(float angleRadians);

    /**
     * @brief Scales the polygon relative to its center
     * @param scale Scale factor (1.0 = no change)
     * @throws Error if scale factor is <= 0
     */
    void scale(float scale);

    /**
     * @brief Checks if a point lies inside the polygon
     * @param x X-coordinate of point
     * @param y Y-coordinate of point
     * @return true if point is inside polygon
     */
    bool containsPoint(float x, float y) const;

    /**
     * @brief Gets the lines that make up the polygon
     * @return Vector of Line objects representing polygon edges
     */
    std::vector<Line> getLines() const;

    /**
     * @brief Renders the polygon using the graphics system
     * @param graphics Reference to the graphics system
     * @param color Polygon color
     * @param thickness Line thickness in pixels
     * @param filled Whether to fill the polygon
     * @param dashPattern Optional array of dash lengths for dashed lines
     */
    void render(const std::shared_ptr<Graphics::GraphicsSystem>& graphics,
                const Graphics::Color& color,
                float thickness = 1.0f,
                bool filled = false,
                const std::vector<float>* dashPattern = nullptr);

private:
    std::vector<std::array<float, 2>> mVertices;

    // Validates vertex coordinates
    void validateVertex(float x, float y) const;
    
    // Validates number of sides
    void validateSideCount(size_t count) const;

    // Helper methods for geometric calculations
    void calculateRegularPolygonVertices(float centerX, float centerY, 
                                       float radius, int sides, float rotation);
    float calculateSignedArea() const;
};

} // namespace Sketching
} // namespace RebelCAD
