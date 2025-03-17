#include "modeling/patterns/PatternSpacing.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <numbers>

namespace rebel_cad {
namespace patterns {

PatternSpacing::PatternSpacing(SpacingType type)
    : type_(type)
    , spacingFunc_([this](float t) { return linearSpacing(t); })
    , interpolationType_(InterpolationType::Linear) {
}

void PatternSpacing::setSpacingFunction(SpacingFunction func) {
    if (!func) {
        throw std::invalid_argument("Invalid spacing function provided");
    }
    spacingFunc_ = std::move(func);
    type_ = SpacingType::Custom;
}

bool PatternSpacing::setParameters(const std::vector<float>& params, 
                                 InterpolationType interpolationType) {
    parameters_ = params;
    interpolationType_ = interpolationType;
    return validateParameters();
}

float PatternSpacing::calculateSpacing(float t) const {
    if (t < 0.0f || t > 1.0f) {
        throw std::out_of_range("Position must be between 0 and 1");
    }

    switch (type_) {
        case SpacingType::Linear:
            return linearSpacing(t);
        case SpacingType::Variable:
            return variableSpacing(t);
        case SpacingType::Curve:
            return curveSpacing(t);
        case SpacingType::Custom:
            return spacingFunc_(t);
        default:
            return linearSpacing(t);
    }
}

std::vector<float> PatternSpacing::generateSpacingPositions(size_t count, float baseSpacing) const {
    if (count == 0) {
        return {};
    }

    std::vector<float> positions;
    positions.reserve(count);

    // Generate normalized positions and apply spacing function
    for (size_t i = 0; i < count; ++i) {
        float t = static_cast<float>(i) / (count - 1);
        float spacing = calculateSpacing(t);
        positions.push_back(baseSpacing * spacing * static_cast<float>(i));
    }

    return positions;
}

bool PatternSpacing::validate() const {
    return validateParameters() && validateSpacingFunction();
}

float PatternSpacing::linearSpacing(float t) const {
    return 1.0f; // Constant spacing
}

float PatternSpacing::curveSpacing(float t) const {
    if (parameters_.size() < 2) {
        return 1.0f;
    }

    // parameters_[0] = curve type (0: exponential, 1: sine, 2: polynomial)
    // parameters_[1] = curve parameter (exponent, frequency, or degree)
    int curveType = static_cast<int>(parameters_[0]);
    float param = parameters_[1];
    
    switch (curveType) {
        case 0: // Exponential
            return std::pow(t, param);
        case 1: // Sine
            return 0.5f * (std::sin(param * std::numbers::pi_v<float> * (t - 0.5f)) + 1.0f);
        case 2: { // Polynomial
            int degree = static_cast<int>(param);
            float result = 0.0f;
            for (int i = 0; i <= degree; ++i) {
                if (parameters_.size() > 2 + i) {
                    result += parameters_[2 + i] * std::pow(t, i);
                }
            }
            return result;
        }
        default:
            return 1.0f;
    }
}

float PatternSpacing::linearInterpolate(float t, float y1, float y2) const {
    return std::lerp(y1, y2, t);
}

float PatternSpacing::cosineInterpolate(float t, float y1, float y2) const {
    float t2 = (1.0f - std::cos(t * std::numbers::pi_v<float>)) / 2.0f;
    return std::lerp(y1, y2, t2);
}

float PatternSpacing::cubicInterpolate(float t, const std::vector<float>& points, size_t index) const {
    float t2 = t * t;
    float t3 = t2 * t;
    
    size_t prev = index > 0 ? index - 1 : 0;
    size_t next = std::min(index + 2, points.size() - 1);
    size_t next2 = std::min(index + 3, points.size() - 1);
    
    float a0 = -0.5f * points[prev] + 1.5f * points[index] - 1.5f * points[next] + 0.5f * points[next2];
    float a1 = points[prev] - 2.5f * points[index] + 2.0f * points[next] - 0.5f * points[next2];
    float a2 = -0.5f * points[prev] + 0.5f * points[next];
    float a3 = points[index];
    
    return a0 * t3 + a1 * t2 + a2 * t + a3;
}

float PatternSpacing::catmullRomInterpolate(float t, const std::vector<float>& points, size_t index) const {
    float t2 = t * t;
    float t3 = t2 * t;
    
    size_t prev = index > 0 ? index - 1 : 0;
    size_t next = std::min(index + 2, points.size() - 1);
    size_t next2 = std::min(index + 3, points.size() - 1);
    
    float a0 = -0.5f * points[prev] + 1.5f * points[index] - 1.5f * points[next] + 0.5f * points[next2];
    float a1 = points[prev] - 2.5f * points[index] + 2.0f * points[next] - 0.5f * points[next2];
    float a2 = -0.5f * points[prev] + 0.5f * points[next];
    float a3 = points[index];
    
    return a0 * t3 + a1 * t2 + a2 * t + a3;
}

float PatternSpacing::variableSpacing(float t) const {
    if (parameters_.empty()) {
        return 1.0f;
    }

    size_t numPoints = parameters_.size();
    float segment = 1.0f / (numPoints - 1);
    
    size_t index = static_cast<size_t>(t / segment);
    index = std::min(index, numPoints - 2);
    
    float localT = (t - index * segment) / segment;
    
    switch (interpolationType_) {
        case InterpolationType::Linear:
            return linearInterpolate(localT, parameters_[index], parameters_[index + 1]);
        case InterpolationType::Cosine:
            return cosineInterpolate(localT, parameters_[index], parameters_[index + 1]);
        case InterpolationType::Cubic:
            return cubicInterpolate(localT, parameters_, index);
        case InterpolationType::CatmullRom:
            return catmullRomInterpolate(localT, parameters_, index);
        default:
            return linearInterpolate(localT, parameters_[index], parameters_[index + 1]);
    }
}

bool PatternSpacing::validateParameters() const {
    switch (type_) {
        case SpacingType::Linear:
            return true;
        case SpacingType::Curve: {
            if (parameters_.size() < 2) return false;
            
            int curveType = static_cast<int>(parameters_[0]);
            if (curveType < 0 || curveType > 2) return false;
            
            switch (curveType) {
                case 0: // Exponential
                    return parameters_[1] > 0.0f; // Exponent must be positive
                case 1: // Sine
                    return parameters_[1] > 0.0f; // Frequency must be positive
                case 2: // Polynomial
                    return parameters_.size() >= 3 && // Need at least one coefficient
                           parameters_[1] >= 0.0f && // Degree must be non-negative
                           parameters_[1] <= 5.0f;  // Limit polynomial degree for stability
            }
            return false;
        }
        case SpacingType::Variable:
            // Need at least 4 points for cubic and Catmull-Rom interpolation
            return parameters_.size() >= (interpolationType_ == InterpolationType::Cubic || 
                   interpolationType_ == InterpolationType::CatmullRom ? 4 : 2);
        case SpacingType::Custom:
            return true;
        default:
            return false;
    }
}

bool PatternSpacing::validateSpacingFunction() const {
    if (type_ != SpacingType::Custom) {
        return true;
    }
    
    try {
        // Test spacing function at boundaries and midpoint
        float values[] = {0.0f, 0.5f, 1.0f};
        for (float t : values) {
            float result = spacingFunc_(t);
            if (std::isnan(result) || std::isinf(result)) {
                return false;
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace patterns
} // namespace rebel_cad
