#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "graphics/GLMConfig.hpp"
#include "graphics/SceneGraph.h"
#include "graphics/SceneNode.h"

namespace RebelCAD {
namespace Graphics {

enum class PickingMode {
    Point,      // Single click picking
    Box,        // Box/rectangle selection
    Lasso       // Freeform lasso selection
};

class ObjectPicker {
public:
    ObjectPicker(std::shared_ptr<SceneGraph> sceneGraph);
    ~ObjectPicker() = default;

    // Pick objects at screen coordinates
    std::vector<SceneNode::Ptr> pickAtPoint(const glm::vec2& screenPos, const glm::mat4& viewMatrix, 
                                           const glm::mat4& projMatrix);
    
    // Box selection with screen coordinates
    std::vector<SceneNode::Ptr> pickInBox(const glm::vec2& min, const glm::vec2& max, 
                                         const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
    
    // Lasso selection with screen coordinates
    std::vector<SceneNode::Ptr> pickInLasso(const std::vector<glm::vec2>& screenPoints,
                                           const glm::mat4& viewMatrix, const glm::mat4& projMatrix);

    // Set picking precision (in screen pixels)
    void setPickingPrecision(float precision) { m_pickingPrecision = precision; }
    
    // Set selection filter to only pick specific node types
    void setSelectionFilter(const std::function<bool(const SceneNode::Ptr&)>& filter) { 
        m_selectionFilter = filter; 
    }

private:
    // Ray-cast into scene and find intersecting objects
    std::vector<SceneNode::Ptr> raycastScene(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
    
    // Convert screen coordinates to world ray
    void screenToRay(const glm::vec2& screenPos, const glm::mat4& viewMatrix,
                    const glm::mat4& projMatrix, glm::vec3& rayOrigin, glm::vec3& rayDir);
    
    // Check if point is inside polygon (for lasso selection)
    bool isPointInPolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon);

    std::shared_ptr<SceneGraph> m_sceneGraph;
    float m_pickingPrecision{5.0f};  // Default 5 pixel precision
    std::function<bool(const SceneNode::Ptr&)> m_selectionFilter{nullptr};
};

} // namespace Graphics
} // namespace RebelCAD
