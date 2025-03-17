#include "modeling/KnotVector.h"
#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace RebelCAD {
namespace Modeling {

class KnotVectorTest : public ::testing::Test {
protected:
    KnotVector knot_vector;

    void SetUp() override {
        // Configure with default parameters
        KnotVectorParams params;
        params.uniform = false;
        params.tolerance = 1e-6f;
        params.normalize = true;
        params.min_multiplicity = 1;
        params.max_multiplicity = 0;
        ASSERT_EQ(knot_vector.Configure(params), KnotVector::ErrorCode::None);
    }

    // Helper to verify knot vector properties
    void VerifyKnotProperties(const std::vector<float>& knots) {
        const auto& result = knot_vector.GetKnots();
        ASSERT_EQ(result.size(), knots.size());
        for (size_t i = 0; i < knots.size(); ++i) {
            EXPECT_NEAR(result[i], knots[i], 1e-6f);
        }
    }

    // Helper to verify knot ordering
    void VerifyKnotOrdering() {
        const auto& knots = knot_vector.GetKnots();
        ASSERT_TRUE(std::is_sorted(knots.begin(), knots.end()));
    }

    // Helper to verify multiplicity
    void VerifyMultiplicity(float knot_value, int expected_mult) {
        const auto& knots = knot_vector.GetKnots();
        auto it = std::find_if(knots.begin(), knots.end(),
            [knot_value](float k) { return std::abs(k - knot_value) <= 1e-6f; });
        ASSERT_NE(it, knots.end());
        int index = std::distance(knots.begin(), it);
        EXPECT_EQ(knot_vector.GetMultiplicity(index), expected_mult);
    }
};

TEST_F(KnotVectorTest, BasicOperations) {
    // Set basic knot vector
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 3.0f, 3.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Verify knots were set correctly
    VerifyKnotProperties(knots);
    
    // Verify ordering
    VerifyKnotOrdering();
    
    // Verify multiplicities
    VerifyMultiplicity(0.0f, 3);
    VerifyMultiplicity(3.0f, 3);
    VerifyMultiplicity(1.0f, 1);
    
    // Verify total and unique counts
    EXPECT_EQ(knot_vector.GetTotalCount(), 8);
    EXPECT_EQ(knot_vector.GetUniqueCount(), 4);
}

TEST_F(KnotVectorTest, KnotInsertion) {
    // Start with simple knot vector
    std::vector<float> knots = {0.0f, 1.0f, 2.0f, 3.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Insert knot with multiplicity
    ASSERT_EQ(knot_vector.InsertKnot(1.5f, 2), KnotVector::ErrorCode::None);
    
    // Verify insertion
    VerifyKnotOrdering();
    VerifyMultiplicity(1.5f, 2);
    EXPECT_EQ(knot_vector.GetTotalCount(), 6);
    
    // Insert at existing knot
    ASSERT_EQ(knot_vector.InsertKnot(1.0f, 1), KnotVector::ErrorCode::None);
    VerifyMultiplicity(1.0f, 2);
}

TEST_F(KnotVectorTest, KnotRemoval) {
    // Start with knots having multiple multiplicities
    std::vector<float> knots = {0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Remove knot
    ASSERT_EQ(knot_vector.RemoveKnot(1.0f, 2), KnotVector::ErrorCode::None);
    
    // Verify removal
    VerifyKnotOrdering();
    VerifyMultiplicity(1.0f, 1);
    EXPECT_EQ(knot_vector.GetTotalCount(), 5);
}

TEST_F(KnotVectorTest, Normalization) {
    // Set knot vector with arbitrary range
    std::vector<float> knots = {-2.0f, -1.0f, 0.0f, 1.0f, 2.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Normalize
    ASSERT_EQ(knot_vector.Normalize(), KnotVector::ErrorCode::None);
    
    // Verify normalization
    const auto& result = knot_vector.GetKnots();
    EXPECT_NEAR(result.front(), 0.0f, 1e-6f);
    EXPECT_NEAR(result.back(), 1.0f, 1e-6f);
    VerifyKnotOrdering();
}

TEST_F(KnotVectorTest, ErrorHandling) {
    // Test empty knot vector
    EXPECT_EQ(knot_vector.SetKnots({}), KnotVector::ErrorCode::InvalidMesh);
    
    // Test invalid knot values
    EXPECT_EQ(knot_vector.SetKnots({0.0f, INFINITY}), KnotVector::ErrorCode::InvalidMesh);
    EXPECT_EQ(knot_vector.SetKnots({0.0f, NAN}), KnotVector::ErrorCode::InvalidMesh);
    
    // Test invalid configuration
    KnotVectorParams invalid_params;
    invalid_params.tolerance = -1.0f;
    EXPECT_EQ(knot_vector.Configure(invalid_params), KnotVector::ErrorCode::InvalidMesh);
    
    // Test invalid knot insertion
    std::vector<float> knots = {0.0f, 1.0f, 2.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    EXPECT_EQ(knot_vector.InsertKnot(-1.0f, 1), KnotVector::ErrorCode::InvalidMesh);
    EXPECT_EQ(knot_vector.InsertKnot(3.0f, 1), KnotVector::ErrorCode::InvalidMesh);
    EXPECT_EQ(knot_vector.InsertKnot(1.0f, 0), KnotVector::ErrorCode::InvalidMesh);
}

TEST_F(KnotVectorTest, MultiplicityConstraints) {
    // Configure with multiplicity constraints
    KnotVectorParams params;
    params.min_multiplicity = 2;
    params.max_multiplicity = 4;
    ASSERT_EQ(knot_vector.Configure(params), KnotVector::ErrorCode::None);
    
    // Set initial knots
    std::vector<float> knots = {0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Test insertion within constraints
    ASSERT_EQ(knot_vector.InsertKnot(1.0f, 2), KnotVector::ErrorCode::None);
    VerifyMultiplicity(1.0f, 4);
    
    // Test insertion exceeding max multiplicity
    EXPECT_EQ(knot_vector.InsertKnot(1.0f, 1), KnotVector::ErrorCode::CADError);
    
    // Test removal respecting min multiplicity
    ASSERT_EQ(knot_vector.RemoveKnot(1.0f, 2), KnotVector::ErrorCode::None);
    VerifyMultiplicity(1.0f, 2);
    EXPECT_EQ(knot_vector.RemoveKnot(1.0f, 1), KnotVector::ErrorCode::CADError);
}

TEST_F(KnotVectorTest, SpanFinding) {
    // Set knot vector with known spans
    std::vector<float> knots = {0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 3.0f, 3.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Test span finding
    EXPECT_EQ(knot_vector.FindSpan(0.0f), 0);
    EXPECT_EQ(knot_vector.FindSpan(0.5f), 2);
    EXPECT_EQ(knot_vector.FindSpan(1.5f), 3);
    EXPECT_EQ(knot_vector.FindSpan(3.0f), 6);
    
    // Test boundary cases
    EXPECT_EQ(knot_vector.FindSpan(-1.0f), 0);
    EXPECT_EQ(knot_vector.FindSpan(4.0f), 6);
}

TEST_F(KnotVectorTest, UniformKnots) {
    // Configure for uniform knots
    KnotVectorParams params;
    params.uniform = true;
    ASSERT_EQ(knot_vector.Configure(params), KnotVector::ErrorCode::None);
    
    // Set uniform knot vector
    std::vector<float> knots = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f};
    ASSERT_EQ(knot_vector.SetKnots(knots), KnotVector::ErrorCode::None);
    
    // Verify uniform spacing
    const auto& result = knot_vector.GetKnots();
    float spacing = result[1] - result[0];
    for (size_t i = 1; i < result.size(); ++i) {
        EXPECT_NEAR(result[i] - result[i-1], spacing, 1e-6f);
    }
}

} // namespace Modeling
} // namespace RebelCAD
