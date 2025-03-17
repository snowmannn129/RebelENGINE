#include <gtest/gtest.h>
#include "modeling/Profile.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace rebel_cad::modeling;

class ProfileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple square boundary
        boundary = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}
        };
    }

    std::vector<glm::vec2> createCircularHole(glm::vec2 center, float radius, int segments) {
        std::vector<glm::vec2> points;
        for (int i = 0; i < segments; ++i) {
            float angle = (2.0f * glm::pi<float>() * i) / segments;
            points.push_back({
                center.x + radius * std::cos(angle),
                center.y + radius * std::sin(angle)
            });
        }
        return points;
    }

    std::vector<glm::vec2> boundary;
};

TEST_F(ProfileTest, BoundaryValidation) {
    Profile profile;
    EXPECT_TRUE(profile.setBoundary(boundary));

    // Test invalid boundary (less than 3 points)
    std::vector<glm::vec2> invalid_boundary = {
        {0.0f, 0.0f},
        {1.0f, 0.0f}
    };
    EXPECT_FALSE(profile.setBoundary(invalid_boundary));

    // Test duplicate points
    std::vector<glm::vec2> duplicate_points = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 0.0f},  // Duplicate
        {0.0f, 1.0f}
    };
    EXPECT_FALSE(profile.setBoundary(duplicate_points));
}

TEST_F(ProfileTest, HoleManagement) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));

    // Add a valid hole
    auto hole = createCircularHole({0.5f, 0.5f}, 0.2f, 16);
    size_t hole_index = profile.addHole(hole);
    EXPECT_EQ(hole_index, 0);
    EXPECT_EQ(profile.getHoles().size(), 1);

    // Try to add an invalid hole (less than 3 points)
    std::vector<glm::vec2> invalid_hole = {{0.4f, 0.4f}, {0.6f, 0.6f}};
    EXPECT_THROW(profile.addHole(invalid_hole), std::invalid_argument);

    // Remove hole
    profile.removeHole(hole_index);
    EXPECT_EQ(profile.getHoles().size(), 0);
}

TEST_F(ProfileTest, ProfileValidation) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));
    EXPECT_TRUE(profile.validate());

    // Add a valid hole
    auto hole = createCircularHole({0.5f, 0.5f}, 0.2f, 16);
    profile.addHole(hole);
    EXPECT_TRUE(profile.validate());

    // Add a hole outside the boundary
    auto invalid_hole = createCircularHole({1.5f, 1.5f}, 0.2f, 16);
    profile.addHole(invalid_hole);
    EXPECT_FALSE(profile.validate());
}

TEST_F(ProfileTest, SelfIntersectionCheck) {
    Profile profile;
    
    // Create self-intersecting boundary
    std::vector<glm::vec2> intersecting_boundary = {
        {0.0f, 0.0f},
        {1.0f, 1.0f},
        {1.0f, 0.0f},
        {0.0f, 1.0f}
    };
    ASSERT_TRUE(profile.setBoundary(intersecting_boundary));
    EXPECT_FALSE(profile.validate());
}

TEST_F(ProfileTest, AreaComputation) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));
    
    // Square area should be 1.0
    EXPECT_NEAR(profile.computeArea(), 1.0f, 1e-6f);

    // Add a circular hole
    auto hole = createCircularHole({0.5f, 0.5f}, 0.2f, 16);
    profile.addHole(hole);
    
    // Area should be less than 1.0 now
    EXPECT_LT(profile.computeArea(), 1.0f);
}

TEST_F(ProfileTest, PointContainment) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));

    // Points inside
    EXPECT_TRUE(profile.containsPoint({0.5f, 0.5f}));
    EXPECT_TRUE(profile.containsPoint({0.1f, 0.1f}));
    EXPECT_TRUE(profile.containsPoint({0.9f, 0.9f}));

    // Points outside
    EXPECT_FALSE(profile.containsPoint({-0.1f, -0.1f}));
    EXPECT_FALSE(profile.containsPoint({1.1f, 1.1f}));
}

TEST_F(ProfileTest, Transformation) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));

    // Apply translation
    glm::mat3 translation = glm::mat3(1.0f);
    translation[2][0] = 1.0f;  // Translate x by 1
    translation[2][1] = 2.0f;  // Translate y by 2
    profile.transform(translation);

    // Check transformed points
    const auto& transformed_boundary = profile.getBoundary().points;
    EXPECT_NEAR(transformed_boundary[0].x, 1.0f, 1e-6f);
    EXPECT_NEAR(transformed_boundary[0].y, 2.0f, 1e-6f);
}

TEST_F(ProfileTest, Triangulation) {
    Profile profile;
    ASSERT_TRUE(profile.setBoundary(boundary));

    auto triangles = profile.triangulate();
    
    // Simple square should produce 2 triangles (6 vertices)
    EXPECT_EQ(triangles.size(), 6);

    // Verify triangles form a valid tessellation
    EXPECT_TRUE([&triangles]() {
        for (size_t i = 0; i < triangles.size(); i += 3) {
            // Check triangle area is positive
            glm::vec2 v1 = triangles[i + 1] - triangles[i];
            glm::vec2 v2 = triangles[i + 2] - triangles[i];
            float area = v1.x * v2.y - v1.y * v2.x;
            if (area <= 0) return false;
        }
        return true;
    }());
}

TEST_F(ProfileTest, BoundaryOrientation) {
    Profile profile;
    
    // Create counter-clockwise boundary
    std::vector<glm::vec2> ccw_boundary = {
        {0.0f, 0.0f},
        {0.0f, 1.0f},
        {1.0f, 1.0f},
        {1.0f, 0.0f}
    };
    ASSERT_TRUE(profile.setBoundary(ccw_boundary));

    // Create clockwise hole
    auto hole = createCircularHole({0.5f, 0.5f}, 0.2f, 16);
    profile.addHole(hole);

    // Validate orientations are correct
    float boundary_area = 0.0f;
    const auto& boundary_points = profile.getBoundary().points;
    for (size_t i = 0; i < boundary_points.size(); ++i) {
        size_t j = (i + 1) % boundary_points.size();
        boundary_area += boundary_points[i].x * boundary_points[j].y;
        boundary_area -= boundary_points[j].x * boundary_points[i].y;
    }
    EXPECT_GT(boundary_area, 0.0f);  // Counter-clockwise should give positive area

    const auto& holes = profile.getHoles();
    for (const auto& hole : holes) {
        float hole_area = 0.0f;
        for (size_t i = 0; i < hole.points.size(); ++i) {
            size_t j = (i + 1) % hole.points.size();
            hole_area += hole.points[i].x * hole.points[j].y;
            hole_area -= hole.points[j].x * hole.points[i].y;
        }
        EXPECT_LT(hole_area, 0.0f);  // Clockwise should give negative area
    }
}
