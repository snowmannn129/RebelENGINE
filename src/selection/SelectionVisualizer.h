#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "SceneGraph.h"
#include "ObjectPicker.h"

namespace RebelCAD {
namespace Graphics {

// Visual style configuration for selection highlighting
struct SelectionStyle {
    glm::vec4 primaryColor{1.0f, 0.5f, 0.0f, 1.0f};    // Default orange
    glm::vec4 secondaryColor{0.0f, 0.5f, 1.0f, 1.0f};  // Default blue for multi-selection
    float outlineWidth{2.0f};                           // Outline width in pixels
    float highlightIntensity{0.3f};                     // Intensity of the highlight effect
    bool showBoundingBox{true};                         // Show bounding box for selected objects
    bool showDepthCues{true};                           // Enable depth-based highlighting
};

class SelectionVisualizer {
public:
    using Ptr = std::shared_ptr<SelectionVisualizer>;
    
    SelectionVisualizer(std::shared_ptr<SceneGraph> sceneGraph);
    ~SelectionVisualizer() = default;

    // Update selection visualization
    void updateSelection(const std::vector<SceneNode::Ptr>& selectedNodes);
    
    // Update preview for ongoing selection (e.g., during box/lasso selection)
    void updateSelectionPreview(const std::vector<SceneNode::Ptr>& previewNodes);
    
    // Render selection highlights and visual feedback
    void render(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    
    // Render selection box/lasso preview during selection
    void renderSelectionShape(PickingMode mode, const std::vector<glm::vec2>& points);
    
    // Configure selection visualization style
    void setSelectionStyle(const SelectionStyle& style) { m_style = style; }
    const SelectionStyle& getSelectionStyle() const { return m_style; }

private:
    // Initialize shaders and buffers
    void initializeResources();
    
    // Render methods for different visual elements
    void renderHighlights(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    void renderBoundingBoxes(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    void renderOutlines(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    void renderDepthCues(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    
    // Helper methods for visual effects
    void updateHighlightBuffers();
    void generateOutlineGeometry();
    void calculateBoundingBoxes();
    
    // Shader program IDs
    GLuint m_highlightShader{0};
    GLuint m_outlineShader{0};
    GLuint m_boxShader{0};
    GLuint m_depthCueShader{0};
    
    // Buffer objects
    GLuint m_highlightVBO{0};
    GLuint m_highlightVAO{0};
    GLuint m_outlineVBO{0};
    GLuint m_outlineVAO{0};
    GLuint m_boxVBO{0};
    GLuint m_boxVAO{0};
    
    // Selection data
    std::vector<SceneNode::Ptr> m_selectedNodes;
    std::vector<SceneNode::Ptr> m_previewNodes;
    std::unordered_map<SceneNode::Ptr, glm::mat4> m_boundingBoxTransforms;
    
    // Resources
    std::shared_ptr<SceneGraph> m_sceneGraph;
    SelectionStyle m_style;
    
    // Cached data for rendering
    std::vector<glm::vec3> m_outlineVertices;
    std::vector<glm::vec3> m_boxVertices;
    bool m_resourcesInitialized{false};
};

} // namespace Graphics
} // namespace RebelCAD
