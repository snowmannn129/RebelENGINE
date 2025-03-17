#include "graphics/SelectionVisualizer.h"
#include "graphics/RenderableNode.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>

namespace {
    // Shader source for depth cues
    const char* depthCueVertexShader = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 FragPos;
        out vec3 ViewPos;
        out vec3 Normal;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            ViewPos = vec3(view * vec4(FragPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* depthCueFragmentShader = R"(
        #version 450 core
        in vec3 FragPos;
        in vec3 ViewPos;
        in vec3 Normal;
        
        uniform vec4 highlightColor;
        uniform float highlightIntensity;
        
        out vec4 FragColor;
        
        void main() {
            // Basic lighting
            vec3 normal = normalize(Normal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normal, lightDir), 0.0);
            
            // Depth-based intensity
            float depth = abs(ViewPos.z);
            float depthFactor = 1.0 - clamp(depth / 100.0, 0.0, 0.9); // Adjust 100.0 for depth range
            
            // Combine lighting and depth effects
            vec4 color = highlightColor;
            color.rgb *= (diff * highlightIntensity + 0.5) * (depthFactor + 0.3);
            color.a *= depthFactor + 0.2;
            
            FragColor = color;
        }
    )";

    // Shader source for highlighting selected objects
    const char* highlightVertexShader = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 FragPos;
        out vec3 Normal;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

    const char* highlightFragmentShader = R"(
        #version 450 core
        in vec3 FragPos;
        in vec3 Normal;
        
        uniform vec4 highlightColor;
        uniform float highlightIntensity;
        uniform bool isPreview;
        
        out vec4 FragColor;
        
        void main() {
            vec3 normal = normalize(Normal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normal, lightDir), 0.0);
            
            vec4 color = highlightColor;
            if (isPreview) {
                color.a *= 0.5; // Semi-transparent for preview
            }
            
            FragColor = color * (diff * highlightIntensity + 0.5);
        }
    )";

    // Shader source for object outlines
    const char* outlineVertexShader = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform float outlineWidth;
        
        void main() {
            vec3 pos = aPos + aNormal * outlineWidth;
            gl_Position = projection * view * model * vec4(pos, 1.0);
        }
    )";

    const char* outlineFragmentShader = R"(
        #version 450 core
        uniform vec4 outlineColor;
        out vec4 FragColor;
        
        void main() {
            FragColor = outlineColor;
        }
    )";

    // Shader source for selection box/lasso
    const char* boxVertexShader = R"(
        #version 450 core
        layout (location = 0) in vec2 aPos;
        
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* boxFragmentShader = R"(
        #version 450 core
        uniform vec4 boxColor;
        out vec4 FragColor;
        
        void main() {
            FragColor = boxColor;
        }
    )";

    // Helper function to compile shader
    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error(std::string("Shader compilation failed: ") + infoLog);
        }
        
        return shader;
    }

    // Helper function to create shader program
    GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
        
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        
        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[512];
            glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error(std::string("Shader program linking failed: ") + infoLog);
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return program;
    }
}

namespace RebelCAD {
namespace Graphics {

SelectionVisualizer::SelectionVisualizer(std::shared_ptr<SceneGraph> sceneGraph)
    : m_sceneGraph(std::move(sceneGraph)) {
    initializeResources();
}

void SelectionVisualizer::initializeResources() {
    if (m_resourcesInitialized) {
        return;
    }

    // Create shader programs
    m_highlightShader = createShaderProgram(highlightVertexShader, highlightFragmentShader);
    m_outlineShader = createShaderProgram(outlineVertexShader, outlineFragmentShader);
    m_boxShader = createShaderProgram(boxVertexShader, boxFragmentShader);
    m_depthCueShader = createShaderProgram(depthCueVertexShader, depthCueFragmentShader);

    // Initialize VAOs and VBOs
    glGenVertexArrays(1, &m_highlightVAO);
    glGenBuffers(1, &m_highlightVBO);
    
    glGenVertexArrays(1, &m_outlineVAO);
    glGenBuffers(1, &m_outlineVBO);
    
    glGenVertexArrays(1, &m_boxVAO);
    glGenBuffers(1, &m_boxVBO);

    m_resourcesInitialized = true;
}

void SelectionVisualizer::updateSelection(const std::vector<SceneNode::Ptr>& selectedNodes) {
    m_selectedNodes = selectedNodes;
    updateHighlightBuffers();
    generateOutlineGeometry();
    calculateBoundingBoxes();
}

void SelectionVisualizer::updateSelectionPreview(const std::vector<SceneNode::Ptr>& previewNodes) {
    m_previewNodes = previewNodes;
    updateHighlightBuffers();
}

void SelectionVisualizer::render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    if (!m_resourcesInitialized) {
        return;
    }

    // Enable blending for transparent effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render highlights
    renderHighlights(viewMatrix, projMatrix);

    // Render outlines
    if (m_style.outlineWidth > 0.0f) {
        renderOutlines(viewMatrix, projMatrix);
    }

    // Render bounding boxes if enabled
    if (m_style.showBoundingBox) {
        renderBoundingBoxes(viewMatrix, projMatrix);
    }

    // Render depth cues if enabled
    if (m_style.showDepthCues) {
        renderDepthCues(viewMatrix, projMatrix);
    }

    glDisable(GL_BLEND);
}

void SelectionVisualizer::renderSelectionShape(PickingMode mode, const std::vector<glm::vec2>& points) {
    if (!m_resourcesInitialized || points.empty()) {
        return;
    }

    glUseProgram(m_boxShader);
    
    // Update box vertices based on selection mode
    std::vector<glm::vec2> shapeVertices;
    if (mode == PickingMode::Box && points.size() >= 2) {
        // Create rectangle from two points
        const glm::vec2& min = points[0];
        const glm::vec2& max = points[1];
        shapeVertices = {
            min,
            glm::vec2(max.x, min.y),
            max,
            glm::vec2(min.x, max.y),
            min  // Close the loop
        };
    } else if (mode == PickingMode::Lasso) {
        // Use points directly for lasso
        shapeVertices = points;
        // Close the loop if not already closed
        if (points.front() != points.back()) {
            shapeVertices.push_back(points.front());
        }
    }

    // Upload vertices
    glBindVertexArray(m_boxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(glm::vec2),
                shapeVertices.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

    // Set uniforms
    glm::vec4 boxColor(0.0f, 0.5f, 1.0f, 0.3f);  // Semi-transparent blue
    glUniform4fv(glGetUniformLocation(m_boxShader, "boxColor"), 1, glm::value_ptr(boxColor));

    // Draw selection shape
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(shapeVertices.size()));
}

void SelectionVisualizer::renderHighlights(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    glUseProgram(m_highlightShader);
    
    // Set shared uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_highlightShader, "view"), 1, GL_FALSE,
                       glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_highlightShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projMatrix));
    glUniform1f(glGetUniformLocation(m_highlightShader, "highlightIntensity"),
                m_style.highlightIntensity);

    // Render selected objects
    glUniform4fv(glGetUniformLocation(m_highlightShader, "highlightColor"), 1,
                 glm::value_ptr(m_style.primaryColor));
    glUniform1i(glGetUniformLocation(m_highlightShader, "isPreview"), GL_FALSE);
    
    for (const auto& node : m_selectedNodes) {
        if (!node) continue;
        
        glUniformMatrix4fv(glGetUniformLocation(m_highlightShader, "model"), 1, GL_FALSE,
                          glm::value_ptr(node->getWorldTransform()));
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            renderableNode->renderWithShader(m_highlightShader);
        }
    }

    // Render preview objects
    glUniform4fv(glGetUniformLocation(m_highlightShader, "highlightColor"), 1,
                 glm::value_ptr(m_style.secondaryColor));
    glUniform1i(glGetUniformLocation(m_highlightShader, "isPreview"), GL_TRUE);
    
    for (const auto& node : m_previewNodes) {
        if (!node) continue;
        
        glUniformMatrix4fv(glGetUniformLocation(m_highlightShader, "model"), 1, GL_FALSE,
                          glm::value_ptr(node->getWorldTransform()));
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            renderableNode->renderWithShader(m_highlightShader);
        }
    }
}

void SelectionVisualizer::renderOutlines(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    glUseProgram(m_outlineShader);
    
    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_outlineShader, "view"), 1, GL_FALSE,
                       glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_outlineShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projMatrix));
    glUniform1f(glGetUniformLocation(m_outlineShader, "outlineWidth"), m_style.outlineWidth);
    
    glBindVertexArray(m_outlineVAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_outlineVertices.size()));
}

void SelectionVisualizer::renderBoundingBoxes(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    if (!m_resourcesInitialized || m_selectedNodes.empty()) {
        return;
    }

    // Initialize static box geometry if not already done
    static bool boxGeometryInitialized = false;
    static GLuint boxVBO = 0;
    static GLuint boxEBO = 0;
    static GLsizei indexCount = 0;

    if (!boxGeometryInitialized) {
        const std::vector<glm::vec3> boxVertices = {
            // Front face
            {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f},
            {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
            // Back face
            {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
            {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}
        };

        const std::vector<unsigned int> boxIndices = {
            // Front
            0, 1, 1, 2, 2, 3, 3, 0,
            // Back
            4, 5, 5, 6, 6, 7, 7, 4,
            // Connections
            0, 4, 1, 5, 2, 6, 3, 7
        };

        // Create and initialize box geometry buffers
        glGenBuffers(1, &boxVBO);
        glGenBuffers(1, &boxEBO);

        glBindVertexArray(m_boxVAO);

        glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
        glBufferData(GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3),
                    boxVertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, boxIndices.size() * sizeof(unsigned int),
                    boxIndices.data(), GL_STATIC_DRAW);

        indexCount = static_cast<GLsizei>(boxIndices.size());
        boxGeometryInitialized = true;
    }

    glUseProgram(m_boxShader);
    
    // Set shared uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_boxShader, "view"), 1, GL_FALSE,
                       glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_boxShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projMatrix));

    // Set box color (semi-transparent white)
    glm::vec4 boxColor(1.0f, 1.0f, 1.0f, 0.5f);
    glUniform4fv(glGetUniformLocation(m_boxShader, "boxColor"), 1, glm::value_ptr(boxColor));

    // Enable depth test but disable depth write for transparent boxes
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // Bind box geometry
    glBindVertexArray(m_boxVAO);

    // Draw bounding box for each selected node
    for (const auto& node : m_selectedNodes) {
        if (!node) continue;

        auto it = m_boundingBoxTransforms.find(node);
        if (it != m_boundingBoxTransforms.end()) {
            glUniformMatrix4fv(glGetUniformLocation(m_boxShader, "model"), 1, GL_FALSE,
                              glm::value_ptr(it->second));
            glDrawElements(GL_LINES, indexCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    // Restore state
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
}

void SelectionVisualizer::renderDepthCues(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    glUseProgram(m_depthCueShader);
    
    // Set uniforms for depth-based effects
    glUniformMatrix4fv(glGetUniformLocation(m_depthCueShader, "view"), 1, GL_FALSE,
                       glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_depthCueShader, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projMatrix));

    // Render selected objects with depth-dependent intensity
    for (const auto& node : m_selectedNodes) {
        if (!node) continue;
        
        glUniformMatrix4fv(glGetUniformLocation(m_depthCueShader, "model"), 1, GL_FALSE,
                          glm::value_ptr(node->getWorldTransform()));
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            renderableNode->renderWithShader(m_depthCueShader);
        }
    }
}

void SelectionVisualizer::updateHighlightBuffers() {
    // Update highlight buffers for selected and preview nodes
    std::vector<glm::vec3> highlightVertices;
    std::vector<glm::vec3> highlightNormals;
    
    // Process selected nodes
    for (const auto& node : m_selectedNodes) {
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            const auto& vertices = renderableNode->getVertices();
            const auto& normals = renderableNode->getNormals();
            const auto& indices = renderableNode->getIndices();
            
            if (!vertices.empty()) {
                if (!indices.empty()) {
                    // Use indexed geometry
                    for (unsigned int idx : indices) {
                        highlightVertices.push_back(vertices[idx]);
                        if (!normals.empty()) {
                            highlightNormals.push_back(normals[idx]);
                        }
                    }
                } else {
                    // Use non-indexed geometry
                    highlightVertices.insert(highlightVertices.end(), vertices.begin(), vertices.end());
                    if (!normals.empty()) {
                        highlightNormals.insert(highlightNormals.end(), normals.begin(), normals.end());
                    }
                }
            }
        }
    }
    
    // Update highlight VAO/VBO
    glBindVertexArray(m_highlightVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, highlightVertices.size() * sizeof(glm::vec3),
                highlightVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    
    if (!highlightNormals.empty()) {
        GLuint normalVBO;
        glGenBuffers(1, &normalVBO);
        glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
        glBufferData(GL_ARRAY_BUFFER, highlightNormals.size() * sizeof(glm::vec3),
                    highlightNormals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    }
    
    glBindVertexArray(0);
}

void SelectionVisualizer::generateOutlineGeometry() {
    m_outlineVertices.clear();
    
    // Generate outline geometry for selected nodes
    for (const auto& node : m_selectedNodes) {
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            const auto& vertices = renderableNode->getVertices();
            const auto& indices = renderableNode->getIndices();
            
            if (!vertices.empty()) {
                std::vector<glm::vec3> nodeOutlineVerts;
                
                if (!indices.empty()) {
                    // Process indexed geometry
                    for (size_t i = 0; i < indices.size(); i += 3) {
                        // Get triangle vertices
                        const glm::vec3& v0 = vertices[indices[i]];
                        const glm::vec3& v1 = vertices[indices[i + 1]];
                        const glm::vec3& v2 = vertices[indices[i + 2]];
                        
                        // Add edges to outline if they're on the silhouette
                        // This is a simplified approach - a more sophisticated silhouette
                        // detection could be implemented for better results
                        nodeOutlineVerts.push_back(v0);
                        nodeOutlineVerts.push_back(v1);
                        nodeOutlineVerts.push_back(v1);
                        nodeOutlineVerts.push_back(v2);
                        nodeOutlineVerts.push_back(v2);
                        nodeOutlineVerts.push_back(v0);
                    }
                } else {
                    // Process non-indexed geometry
                    for (size_t i = 0; i < vertices.size(); i += 3) {
                        const glm::vec3& v0 = vertices[i];
                        const glm::vec3& v1 = vertices[i + 1];
                        const glm::vec3& v2 = vertices[i + 2];
                        
                        nodeOutlineVerts.push_back(v0);
                        nodeOutlineVerts.push_back(v1);
                        nodeOutlineVerts.push_back(v1);
                        nodeOutlineVerts.push_back(v2);
                        nodeOutlineVerts.push_back(v2);
                        nodeOutlineVerts.push_back(v0);
                    }
                }
                
                // Transform outline vertices to world space
                glm::mat4 transform = node->getWorldTransform();
                for (const auto& v : nodeOutlineVerts) {
                    m_outlineVertices.push_back(glm::vec3(transform * glm::vec4(v, 1.0f)));
                }
            }
        }
    }
    
    // Update outline VAO/VBO
    glBindVertexArray(m_outlineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, m_outlineVertices.size() * sizeof(glm::vec3),
                m_outlineVertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

void SelectionVisualizer::calculateBoundingBoxes() {
    m_boundingBoxTransforms.clear();
    
    for (const auto& node : m_selectedNodes) {
        if (!node) continue;
        
        if (auto renderableNode = std::dynamic_pointer_cast<RenderableNode>(node)) {
            // Calculate bounding box transform based on vertex data
            glm::vec3 min(std::numeric_limits<float>::max());
            glm::vec3 max(std::numeric_limits<float>::lowest());
            
            const auto& vertices = renderableNode->getVertices();
            if (!vertices.empty()) {
                for (const auto& vertex : vertices) {
                    min = glm::min(min, vertex);
                    max = glm::max(max, vertex);
                }
            } else {
                // If no vertices, use unit cube
                min = glm::vec3(-0.5f);
                max = glm::vec3(0.5f);
            }
            
            // Calculate scale and center
            glm::vec3 scale = max - min;
            glm::vec3 center = (min + max) * 0.5f;
            
            // Create transform matrix
            glm::mat4 transform = node->getWorldTransform();
            transform = glm::translate(transform, center);
            transform = glm::scale(transform, scale);
            
            m_boundingBoxTransforms[node] = transform;
        }
    }
}

} // namespace Graphics
} // namespace RebelCAD
