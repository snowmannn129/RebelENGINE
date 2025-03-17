#pragma once

namespace RebelCAD {
namespace Graphics {

class IPreviewRenderer {
public:
    virtual ~IPreviewRenderer() = default;
    
    virtual void beginPreview() = 0;
    virtual void endPreview() = 0;
    
    virtual void renderTriangleMesh(
        const float* vertices, 
        size_t vertexCount) = 0;
    
    virtual void renderTriangleMeshWithAttributes(
        const float* vertices,
        const float* normals,
        const float* uvs,
        const unsigned int* indices,
        size_t vertexCount,
        size_t indexCount) = 0;
};

} // namespace Graphics
} // namespace RebelCAD
