#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace rebel_cad {
namespace modeling {

/**
 * @brief Represents a 2D profile that can be used for 3D operations
 * 
 * A Profile consists of an outer boundary and optional inner boundaries (holes).
 * It provides validation and triangulation capabilities needed for 3D operations
 * like extrusion.
 */
class Profile {
public:
    struct Curve {
        std::vector<glm::vec2> points;
        bool closed{true};
    };

    Profile();
    ~Profile();

    /**
     * @brief Sets the outer boundary of the profile
     * 
     * @param boundary Vector of 2D points defining the boundary
     * @param closed Whether the boundary should be closed
     * @return true if boundary is valid
     */
    bool setBoundary(const std::vector<glm::vec2>& boundary, bool closed = true);

    /**
     * @brief Adds a hole to the profile
     * 
     * @param hole Vector of 2D points defining the hole
     * @param closed Whether the hole should be closed
     * @return size_t Index of the added hole
     */
    size_t addHole(const std::vector<glm::vec2>& hole, bool closed = true);

    /**
     * @brief Removes a hole from the profile
     * 
     * @param index Index of the hole to remove
     */
    void removeHole(size_t index);

    /**
     * @brief Gets the outer boundary curve
     */
    const Curve& getBoundary() const { return boundary_; }

    /**
     * @brief Gets all holes in the profile
     */
    const std::vector<Curve>& getHoles() const { return holes_; }

    /**
     * @brief Validates the profile geometry
     * 
     * Checks for:
     * - Self-intersections
     * - Valid hole placement
     * - Proper orientation
     * - Minimum point count
     * 
     * @return true if profile is valid
     */
    bool validate() const;

    /**
     * @brief Triangulates the profile
     * 
     * @return std::vector<glm::vec2> Triangle vertices
     */
    std::vector<glm::vec2> triangulate() const;

    /**
     * @brief Discretizes curves into line segments
     * 
     * @param tolerance Maximum deviation from curve
     */
    void discretize(float tolerance = 0.01f);

    /**
     * @brief Transforms the profile
     * 
     * @param matrix 3x3 transformation matrix
     */
    void transform(const glm::mat3& matrix);

    /**
     * @brief Computes the profile's area
     */
    float computeArea() const;

    /**
     * @brief Checks if a point is inside the profile
     */
    bool containsPoint(const glm::vec2& point) const;

private:
    Curve boundary_;
    std::vector<Curve> holes_;

    bool validateBoundary() const;
    bool validateHoles() const;
    bool checkSelfIntersections(const Curve& curve) const;
    void orientBoundaries();
};

} // namespace modeling
} // namespace rebel_cad
