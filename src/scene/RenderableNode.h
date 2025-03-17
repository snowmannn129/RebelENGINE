#pragma once

#include "graphics/SceneGraph.h"
#include <glad/glad.h>
#include <vector>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief A scene node that can be rendered with OpenGL
 * 
 * RenderableNode extends SceneNode to add rendering capabilities:
 * - Vertex buffer management
 * - Material properties
 * - Rendering methods
 */
class RenderableNode : public SceneNode {
public:
    using Ptr = std::shared_ptr<RenderableNode>;
    
    static Ptr create(const std::string& name = "RenderableNode") {
        auto node = std::make_shared<RenderableNode>(name);
        return node;
    }

    RenderableNode(const std::string& name = "RenderableNode");
    virtual ~RenderableNode();

    // Mesh data setup
    void setVertices(const std::vector<glm::vec3>& vertices);
    void setNormals(const std::vector<glm::vec3>& normals);
    void setIndices(const std::vector<unsigned int>& indices);

    // Material properties
    void setColor(const glm::vec4& color) { m_color = color; }
    const glm::vec4& getColor() const { return m_color; }

    // Mesh data access
    const std::vector<glm::vec3>& getVertices() const { return m_vertices; }
    const std::vector<glm::vec3>& getNormals() const { return m_normals; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }

    // Rendering
    virtual void render() const;
    virtual void renderWithShader(GLuint shader) const;

    // Override serialization
    nlohmann::json serialize() const override;
    void deserialize(const nlohmann::json& data) override;

protected:
    void initializeBuffers();
    void updateBuffers();

private:
    // OpenGL resources
    GLuint m_vao{0};
    GLuint m_vertexBuffer{0};
    GLuint m_normalBuffer{0};
    GLuint m_indexBuffer{0};

    // Mesh data
    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<unsigned int> m_indices;

    // Material properties
    glm::vec4 m_color{1.0f, 1.0f, 1.0f, 1.0f};

    bool m_buffersInitialized{false};
    bool m_dataChanged{false};
};

} // namespace Graphics
} // namespace RebelCAD
