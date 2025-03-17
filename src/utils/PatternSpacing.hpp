#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace rebel_cad {
namespace patterns {

/**
 * @brief Defines different types of spacing distributions for patterns
 */
enum class SpacingType {
    Linear,     ///< Equal spacing between instances
    Variable,   ///< Custom defined spacing using a distribution function
    Curve,      ///< Spacing follows a mathematical curve
    Custom      ///< User-defined spacing function
};

/**
 * @brief Defines interpolation methods for variable spacing
 */
enum class InterpolationType {
    Linear,     ///< Linear interpolation between control points
    Cosine,     ///< Smooth cosine interpolation
    Cubic,      ///< Cubic spline interpolation
    CatmullRom  ///< Catmull-Rom spline interpolation
};

/**
 * @brief Class handling variable spacing calculations for pattern generation
 * 
 * This class manages different types of spacing distributions for patterns,
 * supporting linear, variable, curve-based, and custom spacing functions.
 */
class PatternSpacing {
public:
    using SpacingFunction = std::function<float(float)>;

    /**
     * @brief Constructs a pattern spacing with specified type
     * @param type The type of spacing distribution
     */
    explicit PatternSpacing(SpacingType type = SpacingType::Linear);

    /**
     * @brief Sets a custom spacing function for variable distributions
     * @param func Function taking normalized position (0-1) and returning spacing factor
     */
    void setSpacingFunction(SpacingFunction func);

    /**
     * @brief Sets parameters for built-in spacing types
     * @param params Vector of parameters specific to the spacing type
     * @param interpolationType Type of interpolation for variable spacing
     * @return true if parameters are valid, false otherwise
     */
    bool setParameters(const std::vector<float>& params, 
                      InterpolationType interpolationType = InterpolationType::Linear);

    /**
     * @brief Calculates spacing at a given normalized position
     * @param t Normalized position (0-1)
     * @return Spacing factor at the given position
     */
    float calculateSpacing(float t) const;

    /**
     * @brief Generates spacing positions for a given count
     * @param count Number of instances
     * @param baseSpacing Base spacing between instances
     * @return Vector of positions for pattern instances
     */
    std::vector<float> generateSpacingPositions(size_t count, float baseSpacing) const;

    /**
     * @brief Validates current spacing configuration
     * @return true if configuration is valid, false otherwise
     */
    bool validate() const;

private:
    SpacingType type_;
    SpacingFunction spacingFunc_;
    std::vector<float> parameters_;
    InterpolationType interpolationType_;

    // Interpolation methods
    float linearInterpolate(float t, float y1, float y2) const;
    float cosineInterpolate(float t, float y1, float y2) const;
    float cubicInterpolate(float t, const std::vector<float>& points, size_t index) const;
    float catmullRomInterpolate(float t, const std::vector<float>& points, size_t index) const;

    // Built-in spacing functions
    float linearSpacing(float t) const;
    float curveSpacing(float t) const;
    float variableSpacing(float t) const;

    // Validation helpers
    bool validateParameters() const;
    bool validateSpacingFunction() const;
};

} // namespace patterns
} // namespace rebel_cad
