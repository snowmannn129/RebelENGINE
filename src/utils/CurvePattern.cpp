#include "modeling/patterns/CurvePattern.hpp"
#include <stdexcept>
#include <algorithm>
#include <glm/gtx/quaternion.hpp>

namespace rebel_cad {
namespace patterns {

CurvePattern::CurvePattern(SpacingType spacingType)
    : spacing_(std::make_unique<PatternSpacing>(spacingType)) {
}

bool CurvePattern::setCurvePoints(const std::vector<glm::vec3>& points) {
    curvePoints_ = points;
    return validateCurvePoints();
}

bool CurvePattern::setSpacingParameters(const std::vector<float>& params,
                                      InterpolationType interpolationType) {
    return spacing_->setParameters(params, interpolationType);
}

std::vector<glm::vec3> CurvePattern::generatePositions(size_t count, float baseSpacing) const {
    if (!validate()) {
        throw std::runtime_error("Invalid curve pattern configuration");
    }

    if (count == 0) {
        return {};
    }

    // Get normalized spacing positions
    std::vector<float> spacings = spacing_->generateSpacingPositions(count, baseSpacing);
    std::vector<glm::vec3> positions;
    positions.reserve(count);

    // Convert normalized positions to curve points
    float curveLength = calculateCurveLength();
    for (float t : spacings) {
        positions.push_back(evaluateCurvePoint(t / curveLength));
    }

    return positions;
}

std::vector<glm::mat3> CurvePattern::calculateOrientations(
    const std::vector<glm::vec3>& positions) const {
    
    if (positions.empty()) {
        return {};
    }

    std::vector<glm::mat3> orientations;
    orientations.reserve(positions.size());

    // Calculate Frenet frame for each position
    for (size_t i = 0; i < positions.size(); ++i) {
        float t = static_cast<float>(i) / (positions.size() - 1);
        
        // Calculate tangent
        glm::vec3 tangent = glm::normalize(evaluateCurveTangent(t));
        
        // Calculate normal using curve normal
        glm::vec3 normal = evaluateCurveNormal(t);
        
        // Calculate binormal
        glm::vec3 binormal = glm::cross(tangent, normal);
        
        // Create rotation matrix from frame vectors
        glm::mat3 rotation;
        rotation[0] = tangent;  // X axis
        rotation[1] = normal;   // Y axis
        rotation[2] = binormal; // Z axis
        
        orientations.push_back(rotation);
    }

    return orientations;
}

bool CurvePattern::validate() const {
    return validateCurvePoints() && spacing_->validate();
}

glm::vec3 CurvePattern::evaluateCurvePoint(float t) const {
    if (t < 0.0f || t > 1.0f) {
        throw std::out_of_range("Curve parameter must be between 0 and 1");
    }

    // Find the relevant control points
    float segment = 1.0f / (curvePoints_.size() - 1);
    size_t index = static_cast<size_t>(t / segment);
    index = std::min(index, curvePoints_.size() - 2);
    
    float localT = (t - index * segment) / segment;
    
    // Get control points for Catmull-Rom interpolation
    size_t p0 = index > 0 ? index - 1 : 0;
    size_t p1 = index;
    size_t p2 = index + 1;
    size_t p3 = std::min(index + 2, curvePoints_.size() - 1);
    
    // Catmull-Rom matrix coefficients
    float t2 = localT * localT;
    float t3 = t2 * localT;
    
    glm::vec3 a = -0.5f * curvePoints_[p0] + 1.5f * curvePoints_[p1] - 
                   1.5f * curvePoints_[p2] + 0.5f * curvePoints_[p3];
    glm::vec3 b = curvePoints_[p0] - 2.5f * curvePoints_[p1] + 
                  2.0f * curvePoints_[p2] - 0.5f * curvePoints_[p3];
    glm::vec3 c = -0.5f * curvePoints_[p0] + 0.5f * curvePoints_[p2];
    glm::vec3 d = curvePoints_[p1];
    
    return a * t3 + b * t2 + c * localT + d;
}

glm::vec3 CurvePattern::evaluateCurveTangent(float t) const {
    const float h = 0.0001f; // Small delta for numerical differentiation
    glm::vec3 p1 = evaluateCurvePoint(std::max(0.0f, t - h));
    glm::vec3 p2 = evaluateCurvePoint(std::min(1.0f, t + h));
    return p2 - p1;
}

glm::vec3 CurvePattern::evaluateCurveNormal(float t) const {
    glm::vec3 tangent = glm::normalize(evaluateCurveTangent(t));
    
    // Use numerical differentiation to compute curve normal
    const float h = 0.0001f;
    glm::vec3 nextTangent = glm::normalize(
        evaluateCurveTangent(std::min(1.0f, t + h)));
    
    glm::vec3 binormal = glm::cross(tangent, nextTangent);
    if (glm::length(binormal) < 0.0001f) {
        // If curve is straight at this point, use arbitrary perpendicular vector
        glm::vec3 temp = glm::abs(tangent.x) > 0.9f ? 
            glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
        binormal = glm::cross(tangent, temp);
    }
    
    return glm::normalize(glm::cross(binormal, tangent));
}

float CurvePattern::calculateCurveLength() const {
    const size_t numSamples = 100;
    float length = 0.0f;
    glm::vec3 prevPoint = curvePoints_[0];
    
    for (size_t i = 1; i <= numSamples; ++i) {
        float t = static_cast<float>(i) / numSamples;
        glm::vec3 currentPoint = evaluateCurvePoint(t);
        length += glm::length(currentPoint - prevPoint);
        prevPoint = currentPoint;
    }
    
    return length;
}

bool CurvePattern::validateCurvePoints() const {
    return hasMinimumPoints() && 
           std::all_of(curvePoints_.begin(), curvePoints_.end(),
                      [](const glm::vec3& p) {
                          return !glm::any(glm::isnan(p)) && 
                                 !glm::any(glm::isinf(p));
                      });
}

bool CurvePattern::hasMinimumPoints() const {
    return curvePoints_.size() >= 4; // Minimum points for Catmull-Rom spline
}

} // namespace patterns
} // namespace rebel_cad
