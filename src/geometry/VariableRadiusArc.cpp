#include "sketching/VariableRadiusArc.h"
#include "core/Error.h"
#include <cmath>

using rebel::core::Error;
using rebel::core::ErrorCode;

namespace RebelCAD {
namespace Sketching {

VariableRadiusArc::VariableRadiusArc(
    float centerX,
    float centerY,
    float startRadius,
    float endRadius,
    float startAngle,
    float endAngle
) : Arc(centerX, centerY, startRadius, startAngle, endAngle),
    mStartRadius(startRadius),
    mEndRadius(endRadius)
{
    if (startRadius <= 0.0f || endRadius <= 0.0f) {
        throw rebel::core::Error(
            ErrorCode::GeometryError,
            "Invalid radius values for variable radius arc"
        );
    }
}

float VariableRadiusArc::getRadiusAtAngle(float angle) const {
    return interpolateRadius(angle);
}

void VariableRadiusArc::setStartRadius(float radius) {
    if (radius <= 0.0f) {
        throw rebel::core::Error(
            ErrorCode::GeometryError,
            "Invalid start radius value"
        );
    }
    mStartRadius = radius;
    setRadius(radius); // Update base class radius
}

void VariableRadiusArc::setEndRadius(float radius) {
    if (radius <= 0.0f) {
        throw rebel::core::Error(
            ErrorCode::GeometryError,
            "Invalid end radius value"
        );
    }
    mEndRadius = radius;
}

std::array<float, 2> VariableRadiusArc::getPointAtAngle(float angle) const {
    auto center = getCenter();
    float radius = interpolateRadius(angle);
    return {
        center[0] + radius * std::cos(angle),
        center[1] + radius * std::sin(angle)
    };
}

float VariableRadiusArc::interpolateRadius(float angle) const {
    float startAngle = getStartAngle();
    float endAngle = getEndAngle();
    float sweepAngle = getSweepAngle();
    
    if (std::abs(sweepAngle) < 1e-6f) {
        return mStartRadius;
    }
    
    // Calculate position along arc (0 to 1)
    float t = (angle - startAngle) / sweepAngle;
    t = std::max(0.0f, std::min(1.0f, t)); // Clamp to [0,1]
    
    // Linear interpolation between start and end radius
    return mStartRadius + (mEndRadius - mStartRadius) * t;
}

} // namespace Sketching
} // namespace RebelCAD
