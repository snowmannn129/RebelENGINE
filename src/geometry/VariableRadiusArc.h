#pragma once

#include "sketching/Arc.h"

namespace RebelCAD {
namespace Sketching {

/**
 * @class VariableRadiusArc
 * @brief Extends Arc to support varying radius along its length
 * 
 * This class provides functionality for arcs with linearly interpolated
 * radius values between start and end points. This is particularly useful
 * for operations like variable distance offsetting.
 */
class VariableRadiusArc : public Arc {
public:
    /**
     * @brief Constructs a variable radius arc
     * @param centerX X-coordinate of center point
     * @param centerY Y-coordinate of center point
     * @param startRadius Radius at start point
     * @param endRadius Radius at end point
     * @param startAngle Starting angle in radians
     * @param endAngle Ending angle in radians
     */
    VariableRadiusArc(
        float centerX,
        float centerY,
        float startRadius,
        float endRadius,
        float startAngle,
        float endAngle
    );

    /**
     * @brief Gets the radius at a specific angle
     * @param angle Angle in radians
     * @return Interpolated radius at the given angle
     */
    float getRadiusAtAngle(float angle) const;

    /**
     * @brief Gets the start radius
     * @return Start radius value
     */
    float getStartRadius() const { return mStartRadius; }

    /**
     * @brief Gets the end radius
     * @return End radius value
     */
    float getEndRadius() const { return mEndRadius; }

    /**
     * @brief Sets the start radius
     * @param radius New start radius value
     */
    void setStartRadius(float radius);

    /**
     * @brief Sets the end radius
     * @param radius New end radius value
     */
    void setEndRadius(float radius);

    /**
     * @brief Gets a point on the arc at a specific angle
     * @param angle Angle in radians
     * @return Array containing {x, y} coordinates
     */
    std::array<float, 2> getPointAtAngle(float angle) const;

private:
    float mStartRadius;
    float mEndRadius;

    /**
     * @brief Interpolates radius based on angle position
     * @param angle Current angle
     * @return Interpolated radius value
     */
    float interpolateRadius(float angle) const;
};

} // namespace Sketching
} // namespace RebelCAD
