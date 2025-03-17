#include <gtest/gtest.h>
#include "modeling/patterns/PatternSpacing.hpp"
#include <cmath>
#define _USE_MATH_DEFINES
#include <math.h>
#include <functional>

using namespace rebel_cad::patterns;

class PatternSpacingTests : public ::testing::Test {
protected:
    void SetUp() override {
        spacing = std::make_unique<PatternSpacing>();
    }

    std::unique_ptr<PatternSpacing> spacing;
};

TEST_F(PatternSpacingTests, DefaultConstructorCreatesLinearSpacing) {
    float result = spacing->calculateSpacing(0.5f);
    EXPECT_FLOAT_EQ(result, 1.0f);
}

TEST_F(PatternSpacingTests, LinearSpacingGeneratesEqualDistances) {
    std::vector<float> positions = spacing->generateSpacingPositions(5, 2.0f);
    ASSERT_EQ(positions.size(), 5);
    
    // Check equal spacing between consecutive positions
    float expectedDiff = 2.0f;
    for (size_t i = 1; i < positions.size(); ++i) {
        EXPECT_NEAR(positions[i] - positions[i-1], expectedDiff * i, 0.0001f);
    }
}

TEST_F(PatternSpacingTests, ExponentialCurveSpacingWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Curve);
    std::vector<float> params = {0.0f, 2.0f}; // Type 0: Exponential, exponent = 2
    EXPECT_TRUE(spacing->setParameters(params));

    float t0 = spacing->calculateSpacing(0.0f);
    float t1 = spacing->calculateSpacing(0.5f);
    float t2 = spacing->calculateSpacing(1.0f);

    EXPECT_FLOAT_EQ(t0, 0.0f);
    EXPECT_FLOAT_EQ(t1, 0.25f);
    EXPECT_FLOAT_EQ(t2, 1.0f);
}

TEST_F(PatternSpacingTests, SineCurveSpacingWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Curve);
    std::vector<float> params = {1.0f, 1.0f}; // Type 1: Sine, frequency = 1
    EXPECT_TRUE(spacing->setParameters(params));

    float t0 = spacing->calculateSpacing(0.0f);
    float t1 = spacing->calculateSpacing(0.25f);
    float t2 = spacing->calculateSpacing(0.5f);
    float t3 = spacing->calculateSpacing(0.75f);
    float t4 = spacing->calculateSpacing(1.0f);

    EXPECT_NEAR(t0, 0.0f, 0.0001f);
    EXPECT_NEAR(t1, 0.5f, 0.0001f);
    EXPECT_NEAR(t2, 1.0f, 0.0001f);
    EXPECT_NEAR(t3, 0.5f, 0.0001f);
    EXPECT_NEAR(t4, 0.0f, 0.0001f);
}

TEST_F(PatternSpacingTests, PolynomialCurveSpacingWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Curve);
    // Type 2: Polynomial, degree = 2, coefficients = [1, -2, 2] (1 - 2x + 2x^2)
    std::vector<float> params = {2.0f, 2.0f, 1.0f, -2.0f, 2.0f};
    EXPECT_TRUE(spacing->setParameters(params));

    float t0 = spacing->calculateSpacing(0.0f);
    float t1 = spacing->calculateSpacing(0.5f);
    float t2 = spacing->calculateSpacing(1.0f);

    EXPECT_FLOAT_EQ(t0, 1.0f);
    EXPECT_FLOAT_EQ(t1, 0.5f);
    EXPECT_FLOAT_EQ(t2, 1.0f);
}

TEST_F(PatternSpacingTests, LinearInterpolationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Variable);
    std::vector<float> params = {1.0f, 2.0f, 4.0f}; // Control points
    EXPECT_TRUE(spacing->setParameters(params, InterpolationType::Linear));

    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.5f), 2.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(1.0f), 4.0f);
}

TEST_F(PatternSpacingTests, CosineInterpolationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Variable);
    std::vector<float> params = {1.0f, 2.0f}; // Control points
    EXPECT_TRUE(spacing->setParameters(params, InterpolationType::Cosine));

    float mid = spacing->calculateSpacing(0.5f);
    EXPECT_GT(mid, 1.5f); // Cosine interpolation should be > linear at midpoint
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(1.0f), 2.0f);
}

TEST_F(PatternSpacingTests, CubicInterpolationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Variable);
    std::vector<float> params = {1.0f, 2.0f, 3.0f, 4.0f}; // Control points
    EXPECT_TRUE(spacing->setParameters(params, InterpolationType::Cubic));

    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.0f), 2.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(1.0f), 3.0f);
    
    // Test smooth transition
    float t1 = spacing->calculateSpacing(0.3f);
    float t2 = spacing->calculateSpacing(0.31f);
    EXPECT_NEAR(t2 - t1, 0.03f, 0.01f);
}

TEST_F(PatternSpacingTests, CatmullRomInterpolationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Variable);
    std::vector<float> params = {1.0f, 2.0f, 3.0f, 4.0f}; // Control points
    EXPECT_TRUE(spacing->setParameters(params, InterpolationType::CatmullRom));

    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.0f), 2.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(1.0f), 3.0f);
    
    // Test smooth transition
    float t1 = spacing->calculateSpacing(0.3f);
    float t2 = spacing->calculateSpacing(0.31f);
    EXPECT_NEAR(t2 - t1, 0.03f, 0.01f);
}

TEST_F(PatternSpacingTests, CustomSpacingFunctionWorks) {
    std::function<float(float)> sinFunc = [](float t) { return std::sin(t * M_PI) + 1.0f; };
    spacing->setSpacingFunction(std::move(sinFunc));

    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(0.5f), 2.0f);
    EXPECT_FLOAT_EQ(spacing->calculateSpacing(1.0f), 1.0f);
}

TEST_F(PatternSpacingTests, InterpolationValidationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Variable);
    
    // Test insufficient points for cubic interpolation
    std::vector<float> tooFewPoints = {1.0f, 2.0f};
    EXPECT_FALSE(spacing->setParameters(tooFewPoints, InterpolationType::Cubic));
    EXPECT_FALSE(spacing->setParameters(tooFewPoints, InterpolationType::CatmullRom));
    
    // Test sufficient points for cubic interpolation
    std::vector<float> sufficientPoints = {1.0f, 2.0f, 3.0f, 4.0f};
    EXPECT_TRUE(spacing->setParameters(sufficientPoints, InterpolationType::Cubic));
    EXPECT_TRUE(spacing->setParameters(sufficientPoints, InterpolationType::CatmullRom));
}

TEST_F(PatternSpacingTests, CurveParameterValidationWorks) {
    spacing = std::make_unique<PatternSpacing>(SpacingType::Curve);
    
    // Test invalid curve type
    std::vector<float> invalidType = {3.0f, 1.0f};
    EXPECT_FALSE(spacing->setParameters(invalidType));

    // Test negative exponent for exponential
    std::vector<float> negativeExponent = {0.0f, -1.0f};
    EXPECT_FALSE(spacing->setParameters(negativeExponent));

    // Test negative frequency for sine
    std::vector<float> negativeFrequency = {1.0f, -1.0f};
    EXPECT_FALSE(spacing->setParameters(negativeFrequency));

    // Test polynomial degree too high
    std::vector<float> degreeTooBig = {2.0f, 6.0f, 1.0f};
    EXPECT_FALSE(spacing->setParameters(degreeTooBig));

    // Test polynomial missing coefficients
    std::vector<float> missingCoeffs = {2.0f, 2.0f};
    EXPECT_FALSE(spacing->setParameters(missingCoeffs));

    // Test empty parameters
    std::vector<float> emptyParams;
    EXPECT_FALSE(spacing->setParameters(emptyParams));

    // Test single parameter
    std::vector<float> singleParam = {1.0f};
    EXPECT_FALSE(spacing->setParameters(singleParam));
}

TEST_F(PatternSpacingTests, OutOfRangeInputThrowsException) {
    EXPECT_THROW(spacing->calculateSpacing(-0.1f), std::out_of_range);
    EXPECT_THROW(spacing->calculateSpacing(1.1f), std::out_of_range);
}

TEST_F(PatternSpacingTests, ZeroCountReturnsEmptyPositions) {
    auto positions = spacing->generateSpacingPositions(0, 1.0f);
    EXPECT_TRUE(positions.empty());
}

TEST_F(PatternSpacingTests, InvalidCustomFunctionIsRejected) {
    // Function that returns NaN
    auto invalidFunc = [](float t) { return std::numeric_limits<float>::quiet_NaN(); };
    spacing->setSpacingFunction(invalidFunc);
    EXPECT_FALSE(spacing->validate());

    // Function that throws
    auto throwingFunc = [](float t) -> float { throw std::runtime_error("Test error"); };
    spacing->setSpacingFunction(throwingFunc);
    EXPECT_FALSE(spacing->validate());
}
