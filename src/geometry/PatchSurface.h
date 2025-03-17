#pragma once

#include <vector>
#include <memory>
#include "Geometry.h"

namespace RebelCAD {
namespace Modeling {

/**
 * @brief Represents a parametric surface patch
 * 
 * A patch surface is defined by a network of control points forming
 * a parametric surface. This implementation supports:
 * - Bicubic Bezier patches
 * - Gregory patches for n-sided regions
 * - Blend surfaces between adjacent patches
 * - G1/G2 continuity enforcement
 */
class PatchSurface : public std::enable_shared_from_this<PatchSurface> {
public:
    /**
     * @brief Defines the type of patch surface
     */
    enum class PatchType {
        BicubicBezier,    // Standard 4x4 control point bicubic patch
        Gregory,          // N-sided Gregory patch
        Blend            // Smooth blend between adjacent patches
    };

    /**
     * @brief Creates a bicubic Bezier patch surface
     * @param controlPoints 4x4 grid of control points
     * @param tangentScale Scale factors for boundary derivatives
     * @return Shared pointer to new patch surface
     */
    static std::shared_ptr<PatchSurface> CreateBicubic(
        const std::vector<std::vector<Point3D>>& controlPoints,
        const std::vector<double>& tangentScale = std::vector<double>());

    /**
     * @brief Creates an n-sided Gregory patch
     * @param boundaryPoints Boundary curves control points
     * @param centerPoint Central point for patch
     * @return Shared pointer to new patch surface
     */
    static std::shared_ptr<PatchSurface> CreateGregory(
        const std::vector<std::vector<Point3D>>& boundaryPoints,
        const Point3D& centerPoint);

    /**
     * @brief Creates a blend surface between patches
     * @param sourcePatch First patch to blend from
     * @param targetPatch Second patch to blend to
     * @param blendFactor Blend shape control factor (0-1)
     * @return Shared pointer to new blend surface
     */
    static std::shared_ptr<PatchSurface> CreateBlend(
        std::shared_ptr<PatchSurface> sourcePatch,
        std::shared_ptr<PatchSurface> targetPatch,
        double blendFactor = 0.5);

    /**
     * @brief Evaluates surface point at given parameters
     * @param u First parameter (0-1)
     * @param v Second parameter (0-1)
     * @return 3D point on surface
     */
    Point3D Evaluate(double u, double v) const;

    /**
     * @brief Evaluates surface normal at given parameters
     * @param u First parameter (0-1)
     * @param v Second parameter (0-1)
     * @return Surface normal vector
     */
    Vector3D EvaluateNormal(double u, double v) const;

    /**
     * @brief Gets the type of patch surface
     */
    PatchType GetType() const { return m_type; }

    /**
     * @brief Gets the control points defining the surface
     */
    const std::vector<std::vector<Point3D>>& GetControlPoints() const { return m_controlPoints; }

    /**
     * @brief Sets a control point position
     * @param i First index
     * @param j Second index
     * @param position New control point position
     */
    void SetControlPoint(size_t i, size_t j, const Point3D& position);

    /**
     * @brief Adjusts the surface to maintain G1/G2 continuity with neighbors
     * @param neighbors Adjacent patch surfaces
     */
    void EnforceContinuity(const std::vector<std::shared_ptr<PatchSurface>>& neighbors);

    /**
     * @brief Converts surface to a triangulated mesh
     * @param resolution Grid resolution for triangulation
     * @return Triangle mesh representation
     */
    std::shared_ptr<TriangleMesh> ToMesh(size_t resolution = 20) const;

private:
    PatchSurface(
        PatchType type,
        const std::vector<std::vector<Point3D>>& controlPoints);

    /**
     * @brief Evaluates a bicubic Bezier patch
     */
    Point3D EvaluateBicubic(double u, double v) const;

    /**
     * @brief Evaluates a Gregory patch
     */
    Point3D EvaluateGregory(double u, double v) const;

    /**
     * @brief Evaluates a blend surface
     */
    Point3D EvaluateBlend(double u, double v) const;

    /**
     * @brief Computes basis functions for parameter value
     * @param t Parameter value (0-1)
     * @return Vector of 4 basis function values
     */
    std::vector<double> ComputeBasisFunctions(double t) const;

    PatchType m_type;                             // Type of patch surface
    std::vector<std::vector<Point3D>> m_controlPoints;  // Control point grid
    std::vector<double> m_tangentScale;           // Scale factors for derivatives
    std::shared_ptr<PatchSurface> m_sourcePatch;  // Source patch for blending
    std::shared_ptr<PatchSurface> m_targetPatch;  // Target patch for blending
    double m_blendFactor;                         // Blend shape control
};

} // namespace Modeling
} // namespace RebelCAD
