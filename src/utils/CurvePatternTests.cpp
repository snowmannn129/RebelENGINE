#include <gtest/gtest.h>
#include "modeling/patterns/CurvePattern.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/epsilon.hpp>

using namespace rebel_cad::patterns;

class CurvePatternTests : public ::testing::Test {
protected:
    void SetUp() override {
        pattern = std::make_unique<CurvePattern>();
        
        // Create a simple curve (quarter circle in XY plane)
        curvePoints = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        };
    }

    std::unique_ptr<CurvePattern> pattern;
    std::vector<glm::vec3> curvePoints;
    
    static bool approxEqual(const glm::vec3& a, const glm::vec3& b, float epsilon = 0.0001f) {
        return glm::all(glm::epsilonEqual(a, b, epsilon));
    }
    
    static bool isOrthonormal(const glm::mat3& matrix, float epsilon = 0.0001f) {
        // Check column vectors are normalized
        for (int i = 0; i < 3; ++i) {
            if (std::abs(glm::length(matrix[i]) - 1.0f) > epsilon) {
                return false;
            }
        }
        
        // Check orthogonality
        for (int i = 0; i < 3; ++i) {
            for (int j = i + 1; j < 3; ++j) {
                if (std::abs(glm::dot(matrix[i], matrix[j])) > epsilon) {
                    return false;
                }
            }
        }
        
        return true;
    }
};

TEST_F(CurvePatternTests, ValidatesMinimumPoints) {
    EXPECT_FALSE(pattern->setCurvePoints({}));
    EXPECT_FALSE(pattern->setCurvePoints({glm::vec3(0)}));
    EXPECT_FALSE(pattern->setCurvePoints({glm::vec3(0), glm::vec3(1), glm::vec3(2)}));
    EXPECT_TRUE(pattern->setCurvePoints(curvePoints));
}

TEST_F(CurvePatternTests, RejectsInvalidPoints) {
    std::vector<glm::vec3> invalidPoints = curvePoints;
    invalidPoints[1] = glm::vec3(std::numeric_limits<float>::infinity());
    EXPECT_FALSE(pattern->setCurvePoints(invalidPoints));
    
    invalidPoints[1] = glm::vec3(std::numeric_limits<float>::quiet_NaN());
    EXPECT_FALSE(pattern->setCurvePoints(invalidPoints));
}

TEST_F(CurvePatternTests, GeneratesCorrectNumberOfPositions) {
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    
    auto positions = pattern->generatePositions(5, 1.0f);
    EXPECT_EQ(positions.size(), 5);
    
    positions = pattern->generatePositions(0, 1.0f);
    EXPECT_TRUE(positions.empty());
}

TEST_F(CurvePatternTests, PositionsFollowCurve) {
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    
    auto positions = pattern->generatePositions(3, 1.0f);
    ASSERT_EQ(positions.size(), 3);
    
    // First point should be near start
    EXPECT_TRUE(approxEqual(positions[0], curvePoints[0]));
    
    // Last point should be near end
    EXPECT_TRUE(approxEqual(positions[2], curvePoints[3]));
    
    // Middle point should be roughly in the middle of the curve
    EXPECT_TRUE(approxEqual(positions[1], glm::vec3(0.7f, 0.7f, 0.0f), 0.1f));
}

TEST_F(CurvePatternTests, GeneratesValidOrientations) {
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    
    auto positions = pattern->generatePositions(5, 1.0f);
    auto orientations = pattern->calculateOrientations(positions);
    
    ASSERT_EQ(orientations.size(), positions.size());
    
    // Check each orientation matrix is valid
    for (const auto& matrix : orientations) {
        EXPECT_TRUE(isOrthonormal(matrix));
    }
}

TEST_F(CurvePatternTests, HandlesVariableSpacing) {
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    
    // Set up variable spacing with control points
    std::vector<float> spacingParams = {0.5f, 1.0f, 0.5f};
    ASSERT_TRUE(pattern->setSpacingParameters(spacingParams, InterpolationType::Linear));
    
    auto positions = pattern->generatePositions(5, 1.0f);
    ASSERT_EQ(positions.size(), 5);
    
    // Check spacing variation
    std::vector<float> distances;
    for (size_t i = 1; i < positions.size(); ++i) {
        distances.push_back(glm::length(positions[i] - positions[i-1]));
    }
    
    // Middle distances should be larger than end distances
    EXPECT_GT(distances[1], distances[0]);
    EXPECT_GT(distances[1], distances[3]);
}

TEST_F(CurvePatternTests, HandlesStraightCurve) {
    std::vector<glm::vec3> straightLine = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f)
    };
    
    ASSERT_TRUE(pattern->setCurvePoints(straightLine));
    
    auto positions = pattern->generatePositions(4, 1.0f);
    auto orientations = pattern->calculateOrientations(positions);
    
    ASSERT_EQ(orientations.size(), 4);
    
    // Check orientations are consistent along straight line
    for (const auto& matrix : orientations) {
        // X axis should point along line
        EXPECT_TRUE(approxEqual(matrix[0], glm::vec3(1.0f, 0.0f, 0.0f)));
        // Y and Z axes should be perpendicular
        EXPECT_TRUE(isOrthonormal(matrix));
    }
}

TEST_F(CurvePatternTests, ThrowsOnInvalidConfiguration) {
    // Don't set curve points
    EXPECT_THROW(pattern->generatePositions(5, 1.0f), std::runtime_error);
    
    // Set invalid spacing parameters
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    std::vector<float> invalidParams = {-1.0f};
    ASSERT_FALSE(pattern->setSpacingParameters(invalidParams));
    EXPECT_THROW(pattern->generatePositions(5, 1.0f), std::runtime_error);
}

TEST_F(CurvePatternTests, HandlesEmptyPositionsForOrientations) {
    ASSERT_TRUE(pattern->setCurvePoints(curvePoints));
    auto orientations = pattern->calculateOrientations({});
    EXPECT_TRUE(orientations.empty());
}
