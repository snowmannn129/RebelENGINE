#pragma once

#include <memory>
#include "graphics/IPreviewRenderer.h"
#include "graphics/GraphicsSystem.h"

namespace RebelCAD {
namespace Graphics {

class PreviewRenderer {
public:
    /**
     * @brief Render mesh with vertex attributes
     * @param graphics Graphics system for rendering
     * @param vertices Vertex data (x,y,z coordinates)
     * @param normals Normal data (x,y,z vectors)
     * @param uvs UV coordinate data (u,v pairs)
     * @param indices Triangle indices
     * @param vertex_count Number of vertices
     * @param index_count Number of indices
     */
    static void RenderMeshWithAttributes(
        std::shared_ptr<GraphicsSystem> graphics,
        const float* vertices,
        const float* normals,
        const float* uvs,
        const unsigned int* indices,
        size_t vertex_count,
        size_t index_count);

    /**
     * @brief Render curve with attributes
     * @param graphics Graphics system for rendering
     * @param vertices Vertex data (x,y,z coordinates)
     * @param tangents Tangent data (x,y,z vectors)
     * @param indices Line segment indices
     * @param vertex_count Number of vertices
     * @param index_count Number of indices
     */
    static void RenderCurveWithAttributes(
        std::shared_ptr<GraphicsSystem> graphics,
        const float* vertices,
        const float* tangents,
        const unsigned int* indices,
        size_t vertex_count,
        size_t index_count);

    /**
     * @brief Render control points and polygon
     * @param graphics Graphics system for rendering
     * @param points Control point data (x,y,z coordinates)
     * @param point_count Number of control points
     */
    static void RenderControlPoints(
        std::shared_ptr<GraphicsSystem> graphics,
        const float* points,
        size_t point_count);

    /**
     * @brief Render preview mesh
     * @param graphics Graphics system for rendering
     * @param vertices Vertex data (x,y,z coordinates)
     * @param indices Triangle indices
     * @param vertex_count Number of vertices
     * @param index_count Number of indices
     */
    static void RenderPreviewMesh(
        std::shared_ptr<GraphicsSystem> graphics,
        const float* vertices,
        const unsigned int* indices,
        size_t vertex_count,
        size_t index_count);

    /**
     * @brief Render wireframe mesh
     * @param graphics Graphics system for rendering
     * @param vertices Vertex data (x,y,z coordinates)
     * @param indices Line indices
     * @param vertex_count Number of vertices
     * @param index_count Number of indices
     */
    static void RenderWireframe(
        std::shared_ptr<GraphicsSystem> graphics,
        const float* vertices,
        const unsigned int* indices,
        size_t vertex_count,
        size_t index_count);

private:
    PreviewRenderer() = delete;  // Static class
};

} // namespace Graphics
} // namespace RebelCAD
