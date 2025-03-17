#pragma once

#include <memory>
#include <vector>
#include <array>
#include <optional>
#include <bitset>

namespace RebelCAD {
namespace Sketching {

/**
 * @brief Represents different types of snap points
 */
enum class SnapType {
    ENDPOINT,       ///< Snap to line endpoints, arc endpoints
    MIDPOINT,       ///< Snap to line midpoints
    CENTER,         ///< Snap to circle/arc centers
    INTERSECTION,   ///< Snap to intersections between elements
    PERPENDICULAR,  ///< Snap perpendicular to lines/arcs
    PARALLEL,       ///< Snap parallel to lines
    GRID,          ///< Snap to grid points
    EXTENSION,      ///< Snap to virtual line extensions
    COUNT          ///< Number of snap types (keep last)
};

/**
 * @brief Contains information about a snap point
 */
struct SnapPoint {
    std::array<double, 2> position;  ///< Snap point coordinates
    SnapType type;                   ///< Type of snap point
    double priority;                 ///< Priority value (lower = higher priority)
    std::string description;         ///< Human-readable description for UI

    /**
     * @brief Construct a new snap point
     * @param pos Position coordinates
     * @param t Snap type
     * @param p Priority value (default: 1.0)
     * @param desc Description (default: empty)
     */
    SnapPoint(
        const std::array<double, 2>& pos,
        SnapType t,
        double p = 1.0,
        const std::string& desc = ""
    ) : position(pos), type(t), priority(p), description(desc) {}
};

/**
 * @brief Configuration settings for the snap system
 */
struct SnapSettings {
    double snapRadius = 10.0;        ///< Snap radius in pixels
    double gridSize = 10.0;          ///< Grid size in model units
    std::bitset<static_cast<size_t>(SnapType::COUNT)> enabledTypes; ///< Enabled snap types

    /**
     * @brief Construct default settings with all snap types enabled
     */
    SnapSettings() {
        enabledTypes.set(); // Enable all snap types by default
    }
};

/**
 * @brief Manages snapping functionality for the sketching system
 * 
 * The SnapManager provides snapping capabilities for sketch elements,
 * allowing precise positioning based on geometric relationships.
 */
class SnapManager {
public:
    /**
     * @brief Construct a new Snap Manager
     * @param settings Initial snap settings
     */
    explicit SnapManager(const SnapSettings& settings = SnapSettings());

    /**
     * @brief Update snap settings
     * @param settings New settings to apply
     */
    void setSettings(const SnapSettings& settings);

    /**
     * @brief Get current snap settings
     * @return Current settings
     */
    const SnapSettings& getSettings() const;

    /**
     * @brief Enable or disable a specific snap type
     * @param type Snap type to modify
     * @param enabled Whether to enable or disable
     */
    void setSnapTypeEnabled(SnapType type, bool enabled);

    /**
     * @brief Check if a snap type is enabled
     * @param type Snap type to check
     * @return true if enabled
     */
    bool isSnapTypeEnabled(SnapType type) const;

    /**
     * @brief Find the best snap point near a position
     * @param x X coordinate to snap from
     * @param y Y coordinate to snap from
     * @param screenX X coordinate in screen space (for radius calculation)
     * @param screenY Y coordinate in screen space (for radius calculation)
     * @return Best snap point if found
     */
    std::optional<SnapPoint> findSnapPoint(
        double x, double y,
        int screenX, int screenY) const;

    /**
     * @brief Add a geometric element to consider for snapping
     * @param element Element to add
     */
    template<typename T>
    void addElement(const std::shared_ptr<T>& element);

    /**
     * @brief Remove a geometric element from snap consideration
     * @param element Element to remove
     */
    template<typename T>
    void removeElement(const std::shared_ptr<T>& element);

    /**
     * @brief Clear all elements from snap consideration
     */
    void clearElements();

private:
    SnapSettings m_settings;
    std::vector<std::shared_ptr<void>> m_elements;

    /**
     * @brief Calculate snap points for a specific position
     * @param x X coordinate
     * @param y Y coordinate
     * @param radius Snap radius in model units
     * @return Vector of potential snap points
     */
    std::vector<SnapPoint> calculateSnapPoints(
        double x, double y, double radius) const;

    /**
     * @brief Convert screen distance to model units
     * @param screenX Screen X coordinate
     * @param screenY Screen Y coordinate
     * @param modelX Model X coordinate
     * @param modelY Model Y coordinate
     * @return Snap radius in model units
     */
    double calculateSnapRadius(
        int screenX, int screenY,
        double modelX, double modelY) const;

    /**
     * @brief Find grid snap point if enabled
     * @param x X coordinate
     * @param y Y coordinate
     * @return Grid snap point if found
     */
    std::optional<SnapPoint> findGridSnapPoint(double x, double y) const;

    /**
     * @brief Calculate intersection points between elements
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @return Vector of intersection snap points
     */
    std::vector<SnapPoint> findIntersectionPoints(
        double x, double y, double radius) const;

    /**
     * @brief Add snap points for a line
     * @param line Line to process
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @param points Vector to add snap points to
     */
    void addLineSnapPoints(
        const std::shared_ptr<Line>& line,
        double x, double y, double radius,
        std::vector<SnapPoint>& points) const;

    /**
     * @brief Add snap points for a circle
     * @param circle Circle to process
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @param points Vector to add snap points to
     */
    void addCircleSnapPoints(
        const std::shared_ptr<Circle>& circle,
        double x, double y, double radius,
        std::vector<SnapPoint>& points) const;

    /**
     * @brief Add snap points for an arc
     * @param arc Arc to process
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @param points Vector to add snap points to
     */
    void addArcSnapPoints(
        const std::shared_ptr<Arc>& arc,
        double x, double y, double radius,
        std::vector<SnapPoint>& points) const;

    /**
     * @brief Add snap points for a rectangle
     * @param rect Rectangle to process
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @param points Vector to add snap points to
     */
    void addRectangleSnapPoints(
        const std::shared_ptr<Rectangle>& rect,
        double x, double y, double radius,
        std::vector<SnapPoint>& points) const;

    /**
     * @brief Add snap points for a polygon
     * @param poly Polygon to process
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @param points Vector to add snap points to
     */
    void addPolygonSnapPoints(
        const std::shared_ptr<Polygon>& poly,
        double x, double y, double radius,
        std::vector<SnapPoint>& points) const;

    /**
     * @brief Check if a point is within snap radius
     * @param px Point X coordinate
     * @param py Point Y coordinate
     * @param x Center X of search area
     * @param y Center Y of search area
     * @param radius Search radius
     * @return true if point is within radius
     */
    bool isPointInRange(
        double px, double py,
        double x, double y,
        double radius) const;
};

} // namespace Sketching
} // namespace RebelCAD
