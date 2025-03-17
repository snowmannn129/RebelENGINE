#include "modeling/PatchSurface.h"
#include <cmath>
#include <stdexcept>

namespace RebelCAD {
namespace Modeling {

namespace {
    // Cubic Bernstein basis functions
    double B0(double t) { return (1-t)*(1-t)*(1-t); }
    double B1(double t) { return 3*t*(1-t)*(1-t); }
    double B2(double t) { return 3*t*t*(1-t); }
    double B3(double t) { return t*t*t; }
    
    // Derivatives of basis functions
    double dB0(double t) { return -3*(1-t)*(1-t); }
    double dB1(double t) { return 3*(1-3*t+2*t*t); }
    double dB2(double t) { return 3*(2*t-3*t*t); }
    double dB3(double t) { return 3*t*t; }
}

PatchSurface::PatchSurface(
    PatchType type,
    const std::vector<std::vector<Point3D>>& controlPoints)
    : m_type(type)
    , m_controlPoints(controlPoints)
    , m_blendFactor(0.5)
{
}

std::shared_ptr<PatchSurface> PatchSurface::CreateBicubic(
    const std::vector<std::vector<Point3D>>& controlPoints,
    const std::vector<double>& tangentScale)
{
    // Validate control point grid is 4x4
    if (controlPoints.size() != 4) {
        throw std::invalid_argument("Bicubic patch requires 4x4 control points");
    }
    for (const auto& row : controlPoints) {
        if (row.size() != 4) {
            throw std::invalid_argument("Bicubic patch requires 4x4 control points");
        }
    }

    auto patch = std::shared_ptr<PatchSurface>(
        new PatchSurface(PatchType::BicubicBezier, controlPoints));
    patch->m_tangentScale = tangentScale;
    return patch;
}

std::shared_ptr<PatchSurface> PatchSurface::CreateGregory(
    const std::vector<std::vector<Point3D>>& boundaryPoints,
    const Point3D& centerPoint)
{
    // Validate boundary curves
    if (boundaryPoints.empty()) {
        throw std::invalid_argument("Gregory patch requires boundary curves");
    }
    
    // Convert boundary curves to control point grid
    std::vector<std::vector<Point3D>> controlPoints;
    // TODO: Implement Gregory patch control point calculation
    
    return std::shared_ptr<PatchSurface>(
        new PatchSurface(PatchType::Gregory, controlPoints));
}

std::shared_ptr<PatchSurface> PatchSurface::CreateBlend(
    std::shared_ptr<PatchSurface> sourcePatch,
    std::shared_ptr<PatchSurface> targetPatch,
    double blendFactor)
{
    if (!sourcePatch || !targetPatch) {
        throw std::invalid_argument("Blend surface requires valid source and target patches");
    }
    
    if (blendFactor < 0.0 || blendFactor > 1.0) {
        throw std::invalid_argument("Blend factor must be between 0 and 1");
    }

    // Create empty control point grid for blend surface
    std::vector<std::vector<Point3D>> controlPoints;
    auto patch = std::shared_ptr<PatchSurface>(
        new PatchSurface(PatchType::Blend, controlPoints));
    
    patch->m_sourcePatch = sourcePatch;
    patch->m_targetPatch = targetPatch;
    patch->m_blendFactor = blendFactor;
    
    return patch;
}

Point3D PatchSurface::Evaluate(double u, double v) const
{
    if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
        throw std::invalid_argument("Parameters must be between 0 and 1");
    }

    switch (m_type) {
        case PatchType::BicubicBezier:
            return EvaluateBicubic(u, v);
        case PatchType::Gregory:
            return EvaluateGregory(u, v);
        case PatchType::Blend:
            return EvaluateBlend(u, v);
        default:
            throw std::runtime_error("Unknown patch type");
    }
}

Vector3D PatchSurface::EvaluateNormal(double u, double v) const
{
    const double h = 1e-6;  // Small step for finite differences
    
    // Compute partial derivatives using central differences
    Point3D p1 = Evaluate(u+h, v);
    Point3D p2 = Evaluate(u-h, v);
    Point3D p3 = Evaluate(u, v+h);
    Point3D p4 = Evaluate(u, v-h);
    
    // Create vectors along u and v directions
    Vector3D du(
        (p1.x - p2.x)/(2*h),
        (p1.y - p2.y)/(2*h),
        (p1.z - p2.z)/(2*h)
    );
    
    Vector3D dv(
        (p3.x - p4.x)/(2*h),
        (p3.y - p4.y)/(2*h),
        (p3.z - p4.z)/(2*h)
    );
    
    // Normal is cross product of tangent vectors
    return du.Cross(dv).Normalize();
}

void PatchSurface::SetControlPoint(size_t i, size_t j, const Point3D& position)
{
    if (i >= m_controlPoints.size() || j >= m_controlPoints[0].size()) {
        throw std::out_of_range("Control point index out of range");
    }
    m_controlPoints[i][j] = position;
}

void PatchSurface::EnforceContinuity(
    const std::vector<std::shared_ptr<PatchSurface>>& neighbors)
{
    // TODO: Implement G1/G2 continuity enforcement
}

std::shared_ptr<TriangleMesh> PatchSurface::ToMesh(size_t resolution) const
{
    auto mesh = TriangleMesh::Create();
    
    // Generate grid of points
    for (size_t i = 0; i <= resolution; i++) {
        double u = static_cast<double>(i) / resolution;
        for (size_t j = 0; j <= resolution; j++) {
            double v = static_cast<double>(j) / resolution;
            
            mesh->vertices.push_back(Evaluate(u, v));
            mesh->normals.push_back(EvaluateNormal(u, v));
        }
    }
    
    // Generate triangles
    for (size_t i = 0; i < resolution; i++) {
        for (size_t j = 0; j < resolution; j++) {
            size_t v00 = i*(resolution+1) + j;
            size_t v10 = (i+1)*(resolution+1) + j;
            size_t v01 = i*(resolution+1) + (j+1);
            size_t v11 = (i+1)*(resolution+1) + (j+1);
            
            // First triangle
            TriangleMesh::Triangle t1;
            t1.v1 = v00;
            t1.v2 = v10;
            t1.v3 = v01;
            t1.normal = mesh->normals[v00];  // Use vertex normal for face
            mesh->triangles.push_back(t1);
            
            // Second triangle
            TriangleMesh::Triangle t2;
            t2.v1 = v10;
            t2.v2 = v11;
            t2.v3 = v01;
            t2.normal = mesh->normals[v10];  // Use vertex normal for face
            mesh->triangles.push_back(t2);
        }
    }
    
    return mesh;
}

Point3D PatchSurface::EvaluateBicubic(double u, double v) const
{
    std::vector<double> U = ComputeBasisFunctions(u);
    std::vector<double> V = ComputeBasisFunctions(v);
    
    Point3D result{0, 0, 0};
    
    // Compute surface point using tensor product
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            const Point3D& p = m_controlPoints[i][j];
            double weight = U[i] * V[j];
            result.x += weight * p.x;
            result.y += weight * p.y;
            result.z += weight * p.z;
        }
    }
    
    return result;
}

Point3D PatchSurface::EvaluateGregory(double u, double v) const
{
    // TODO: Implement Gregory patch evaluation
    return Point3D();
}

Point3D PatchSurface::EvaluateBlend(double u, double v) const
{
    Point3D p1 = m_sourcePatch->Evaluate(u, v);
    Point3D p2 = m_targetPatch->Evaluate(u, v);
    
    // Linear interpolation between patches
    return Point3D(
        p1.x + m_blendFactor*(p2.x - p1.x),
        p1.y + m_blendFactor*(p2.y - p1.y),
        p1.z + m_blendFactor*(p2.z - p1.z)
    );
}

std::vector<double> PatchSurface::ComputeBasisFunctions(double t) const
{
    return {B0(t), B1(t), B2(t), B3(t)};
}

} // namespace Modeling
} // namespace RebelCAD
