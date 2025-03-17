#include <gtest/gtest.h>
#include "../../include/modeling/PatchSurface.h"
#include <cmath>

using namespace RebelCAD::Modeling;

namespace {
    // Helper function to create a simple test patch
    std::shared_ptr<PatchSurface> CreateTestPatch() {
        std::vector<std::vector<Point3D>> controlPoints = {
            {Point3D(0,0,0), Point3D(1,0,1), Point3D(2,0,1), Point3D(3,0,0)},
            {Point3D(0,1,1), Point3D(1,1,2), Point3D(2,1,2), Point3D(3,1,1)},
            {Point3D(0,2,1), Point3D(1,2,2), Point3D(2,2,2), Point3D(3,2,1)},
            {Point3D(0,3,0), Point3D(1,3,1), Point3D(2,3,1), Point3D(3,3,0)}
        };
        return PatchSurface::CreateBicubic(controlPoints);
    }

    // Helper function to compare points with tolerance
    bool PointsEqual(const Point3D& p1, const Point3D& p2, double tolerance = 1e-6) {
        return std::abs(p1.x - p2.x) < tolerance &&
               std::abs(p1.y - p2.y) < tolerance &&
               std::abs(p1.z - p2.z) < tolerance;
    }

    // Helper function to compare vectors with tolerance
    bool VectorsEqual(const Vector3D& v1, const Vector3D& v2, double tolerance = 1e-6) {
        return std::abs(v1.x - v2.x) < tolerance &&
               std::abs(v1.y - v2.y) < tolerance &&
               std::abs(v1.z - v2.z) < tolerance;
    }
}

TEST(PatchSurfaceTests, CreateBicubicPatch) {
    auto patch = CreateTestPatch();
    ASSERT_NE(patch, nullptr);
    EXPECT_EQ(patch->GetType(), PatchSurface::PatchType::BicubicBezier);
}

TEST(PatchSurfaceTests, CreateBicubicPatchInvalidGrid) {
    std::vector<std::vector<Point3D>> controlPoints = {
        {Point3D(0,0,0), Point3D(1,0,0), Point3D(2,0,0)}  // Only 3 points
    };
    EXPECT_THROW(PatchSurface::CreateBicubic(controlPoints), std::invalid_argument);
}

TEST(PatchSurfaceTests, EvaluateCornerPoints) {
    auto patch = CreateTestPatch();
    
    // Test corners match control points
    EXPECT_TRUE(PointsEqual(patch->Evaluate(0, 0), Point3D(0,0,0)));
    EXPECT_TRUE(PointsEqual(patch->Evaluate(1, 0), Point3D(3,0,0)));
    EXPECT_TRUE(PointsEqual(patch->Evaluate(0, 1), Point3D(0,3,0)));
    EXPECT_TRUE(PointsEqual(patch->Evaluate(1, 1), Point3D(3,3,0)));
}

TEST(PatchSurfaceTests, EvaluateInvalidParameters) {
    auto patch = CreateTestPatch();
    EXPECT_THROW(patch->Evaluate(-0.1, 0.5), std::invalid_argument);
    EXPECT_THROW(patch->Evaluate(0.5, 1.1), std::invalid_argument);
}

TEST(PatchSurfaceTests, EvaluateNormalAtCenter) {
    auto patch = CreateTestPatch();
    Vector3D normal = patch->EvaluateNormal(0.5, 0.5);
    
    // Normal should be normalized
    EXPECT_NEAR(normal.Magnitude(), 1.0, 1e-6);
}

TEST(PatchSurfaceTests, CreateBlendSurface) {
    auto patch1 = CreateTestPatch();
    auto patch2 = CreateTestPatch();
    auto blend = PatchSurface::CreateBlend(patch1, patch2, 0.5);
    
    ASSERT_NE(blend, nullptr);
    EXPECT_EQ(blend->GetType(), PatchSurface::PatchType::Blend);
    
    // Test blend interpolates between patches
    Point3D p1 = patch1->Evaluate(0.5, 0.5);
    Point3D p2 = patch2->Evaluate(0.5, 0.5);
    Point3D pb = blend->Evaluate(0.5, 0.5);
    
    // Should be halfway between p1 and p2
    EXPECT_TRUE(PointsEqual(pb, Point3D(
        (p1.x + p2.x)/2,
        (p1.y + p2.y)/2,
        (p1.z + p2.z)/2
    )));
}

TEST(PatchSurfaceTests, CreateBlendSurfaceInvalid) {
    auto patch = CreateTestPatch();
    EXPECT_THROW(PatchSurface::CreateBlend(nullptr, patch, 0.5), std::invalid_argument);
    EXPECT_THROW(PatchSurface::CreateBlend(patch, nullptr, 0.5), std::invalid_argument);
    EXPECT_THROW(PatchSurface::CreateBlend(patch, patch, -0.1), std::invalid_argument);
    EXPECT_THROW(PatchSurface::CreateBlend(patch, patch, 1.1), std::invalid_argument);
}

TEST(PatchSurfaceTests, GenerateMesh) {
    auto patch = CreateTestPatch();
    auto mesh = patch->ToMesh(4);  // 4x4 resolution for testing
    
    ASSERT_NE(mesh, nullptr);
    
    // For 4x4 resolution:
    // - Should have 25 vertices (5x5 grid)
    // - Should have 32 triangles (4x4 quads, 2 triangles each)
    EXPECT_EQ(mesh->vertices.size(), 25);
    EXPECT_EQ(mesh->normals.size(), 25);
    EXPECT_EQ(mesh->triangles.size(), 32);
    
    // Verify all normals are normalized
    for (const auto& normal : mesh->normals) {
        EXPECT_NEAR(normal.Magnitude(), 1.0, 1e-6);
    }
}

TEST(PatchSurfaceTests, SetControlPoint) {
    auto patch = CreateTestPatch();
    Point3D newPoint(5, 5, 5);
    
    patch->SetControlPoint(1, 1, newPoint);
    EXPECT_TRUE(PointsEqual(patch->GetControlPoints()[1][1], newPoint));
}

TEST(PatchSurfaceTests, SetControlPointOutOfRange) {
    auto patch = CreateTestPatch();
    EXPECT_THROW(patch->SetControlPoint(10, 0, Point3D()), std::out_of_range);
    EXPECT_THROW(patch->SetControlPoint(0, 10, Point3D()), std::out_of_range);
}
