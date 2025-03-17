#include "graphics/RenderableNode.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

namespace RebelCAD {
namespace Graphics {

RenderableNode::RenderableNode(const std::string& name)
    : SceneNode(name) {
}

RenderableNode::~RenderableNode() {
    if (m_buffersInitialized) {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vertexBuffer);
        glDeleteBuffers(1, &m_normalBuffer);
        glDeleteBuffers(1, &m_indexBuffer);
    }
}

void RenderableNode::setVertices(const std::vector<glm::vec3>& vertices) {
    m_vertices = vertices;
    m_dataChanged = true;
}

void RenderableNode::setNormals(const std::vector<glm::vec3>& normals) {
    if (normals.size() != m_vertices.size()) {
        throw std::runtime_error("Normal count must match vertex count");
    }
    m_normals = normals;
    m_dataChanged = true;
}

void RenderableNode::setIndices(const std::vector<unsigned int>& indices) {
    m_indices = indices;
    m_dataChanged = true;
}

void RenderableNode::initializeBuffers() {
    if (m_buffersInitialized) {
        return;
    }

    // Create and bind VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Create buffers
    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_normalBuffer);
    glGenBuffers(1, &m_indexBuffer);

    m_buffersInitialized = true;
    m_dataChanged = true; // Ensure data is uploaded
}

void RenderableNode::updateBuffers() {
    if (!m_dataChanged) {
        return;
    }

    if (!m_buffersInitialized) {
        initializeBuffers();
    }

    glBindVertexArray(m_vao);

    // Update vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3),
                m_vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Update normal buffer
    if (!m_normals.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, m_normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, m_normals.size() * sizeof(glm::vec3),
                    m_normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    // Update index buffer
    if (!m_indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
                    m_indices.data(), GL_STATIC_DRAW);
    }

    glBindVertexArray(0);
    m_dataChanged = false;
}

void RenderableNode::render() const {
    if (m_vertices.empty()) {
        return;
    }

    const_cast<RenderableNode*>(this)->updateBuffers();

    glBindVertexArray(m_vao);
    
    if (!m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                      GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    }

    glBindVertexArray(0);
}

void RenderableNode::renderWithShader(GLuint shader) const {
    if (m_vertices.empty()) {
        return;
    }

    const_cast<RenderableNode*>(this)->updateBuffers();

    // Set color uniform if available
    GLint colorLoc = glGetUniformLocation(shader, "color");
    if (colorLoc != -1) {
        glUniform4fv(colorLoc, 1, glm::value_ptr(m_color));
    }

    glBindVertexArray(m_vao);
    
    if (!m_indices.empty()) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                      GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));
    }

    glBindVertexArray(0);
}

nlohmann::json RenderableNode::serialize() const {
    auto json = SceneNode::serialize();
    
    // Add renderable-specific data
    json["color"] = {
        m_color.r, m_color.g, m_color.b, m_color.a
    };
    
    // Serialize mesh data
    std::vector<float> vertexData;
    vertexData.reserve(m_vertices.size() * 3);
    for (const auto& v : m_vertices) {
        vertexData.push_back(v.x);
        vertexData.push_back(v.y);
        vertexData.push_back(v.z);
    }
    json["vertices"] = vertexData;
    
    if (!m_normals.empty()) {
        std::vector<float> normalData;
        normalData.reserve(m_normals.size() * 3);
        for (const auto& n : m_normals) {
            normalData.push_back(n.x);
            normalData.push_back(n.y);
            normalData.push_back(n.z);
        }
        json["normals"] = normalData;
    }
    
    if (!m_indices.empty()) {
        json["indices"] = m_indices;
    }
    
    return json;
}

void RenderableNode::deserialize(const nlohmann::json& data) {
    SceneNode::deserialize(data);
    
    // Restore color
    if (data.contains("color")) {
        auto& color = data["color"];
        m_color = glm::vec4(
            color[0].get<float>(),
            color[1].get<float>(),
            color[2].get<float>(),
            color[3].get<float>()
        );
    }
    
    // Restore mesh data
    if (data.contains("vertices")) {
        auto& vertexData = data["vertices"];
        m_vertices.clear();
        m_vertices.reserve(vertexData.size() / 3);
        for (size_t i = 0; i < vertexData.size(); i += 3) {
            m_vertices.emplace_back(
                vertexData[i].get<float>(),
                vertexData[i + 1].get<float>(),
                vertexData[i + 2].get<float>()
            );
        }
    }
    
    if (data.contains("normals")) {
        auto& normalData = data["normals"];
        m_normals.clear();
        m_normals.reserve(normalData.size() / 3);
        for (size_t i = 0; i < normalData.size(); i += 3) {
            m_normals.emplace_back(
                normalData[i].get<float>(),
                normalData[i + 1].get<float>(),
                normalData[i + 2].get<float>()
            );
        }
    }
    
    if (data.contains("indices")) {
        m_indices = data["indices"].get<std::vector<unsigned int>>();
    }
    
    m_dataChanged = true;
}

} // namespace Graphics
} // namespace RebelCAD
