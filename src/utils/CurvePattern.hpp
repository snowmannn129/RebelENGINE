#pragma once

#include "modeling/patterns/PatternSpacing.hpp"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace rebel_cad {
namespace patterns {

/**
 * @brief Class for generating patterns that follow a 3D curve
 * 
 * This class combines curve evaluation with pattern spacing to create
 * patterns that follow arbitrary curves in 3D space. It supports various
 * spacing distributions and handles curve parameterization.
 */
class CurvePattern {
public:
    /**
     * @brief Constructs a curve pattern with specified spacing type
     * @param spacingType The type of spacing distribution to use
     */
    explicit CurvePattern(SpacingType spacingType = SpacingType::Linear);

    /**
     * @brief Sets the control points defining the curve
     * @param points Vector of 3D control points
     * @return true if points are valid, false otherwise
     */
    bool setCurvePoints(const std::vector<glm::vec3>& points);

    /**
     * @brief Sets the spacing parameters for pattern distribution
     * @param params Spacing parameters
     * @param interpolationType Type of interpolation for variable spacing
     * @return true if parameters are valid, false otherwise
     */
    bool setSpacingParameters(const std::vector<float>& params,
                            InterpolationType interpolationType = InterpolationType::Linear);

    /**
     * @brief Generates pattern instance positions along the curve
     * @param count Number of instances to generate
     * @param baseSpacing Base spacing between instances
     * @return Vector of 3D positions for pattern instances
     */
    std::vector<glm::vec3> generatePositions(size_t count, float baseSpacing) const;

    /**
     * @brief Calculates orientations for pattern instances
     * @param positions Vector of instance positions
     * @return Vector of rotation matrices for each instance
     */
    std::vector<glm::mat3> calculateOrientations(const std::vector<glm::vec3>& positions) const;

    /**
     * @brief Validates current pattern configuration
     * @return true if configuration is valid, false otherwise
     */
    bool validate() const;

private:
    std::unique_ptr<PatternSpacing> spacing_;
    std::vector<glm::vec3> curvePoints_;
    
    // Curve evaluation helpers
    glm::vec3 evaluateCurvePoint(float t) const;
    glm::vec3 evaluateCurveTangent(float t) const;
    glm::vec3 evaluateCurveNormal(float t) const;
    float calculateCurveLength() const;
    
    // Validation helpers
    bool validateCurvePoints() const;
    bool hasMinimumPoints() const;
};

} // namespace patterns
} // namespace rebel_cad
