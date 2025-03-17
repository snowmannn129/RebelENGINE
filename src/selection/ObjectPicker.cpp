#include "graphics/ObjectPicker.h"
#include <algorithm>
#include "graphics/GLMConfig.hpp"

namespace RebelCAD {
namespace Graphics {

ObjectPicker::ObjectPicker(std::shared_ptr<SceneGraph> sceneGraph)
    : m_sceneGraph(std::move(sceneGraph)) {
}

std::vector<SceneNode::Ptr> ObjectPicker::pickAtPoint(const glm::vec2& screenPos,
                                                     const glm::mat4& viewMatrix,
                                                     const glm::mat4& projMatrix) {
    glm::vec3 rayOrigin, rayDir;
    screenToRay(screenPos, viewMatrix, projMatrix, rayOrigin, rayDir);
    return raycastScene(rayOrigin, rayDir);
}

std::vector<SceneNode::Ptr> ObjectPicker::pickInBox(const glm::vec2& min,
                                                   const glm::vec2& max,
                                                   const glm::mat4& viewMatrix,
                                                   const glm::mat4& projMatrix) {
    std::vector<SceneNode::Ptr> result;
    
    // Create rays for box corners
    glm::vec3 rayOrigins[4], rayDirs[4];
    glm::vec2 corners[4] = {
        min,
        glm::vec2(max.x, min.y),
        max,
        glm::vec2(min.x, max.y)
    };
    
    // Generate rays for each corner
    for (int i = 0; i < 4; ++i) {
        screenToRay(corners[i], viewMatrix, projMatrix, rayOrigins[i], rayDirs[i]);
    }
    
    // Find objects within the frustum formed by these rays
    auto selectedNodes = m_sceneGraph->getSelectedNodes(); // Get current selection
    for (const auto& node : selectedNodes) {
        if (m_selectionFilter && !m_selectionFilter(node)) {
            continue;
        }
        
        // Project node bounds to screen space
        if (node->intersect(rayOrigins[0], rayDirs[0])) {
            bool isInside = true;
            glm::vec4 clipSpace = projMatrix * viewMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            glm::vec3 ndcSpace(clipSpace.x / clipSpace.w, clipSpace.y / clipSpace.w, clipSpace.z / clipSpace.w);
            glm::vec2 screenPos((ndcSpace.x + 1.0f) * 0.5f, (ndcSpace.y + 1.0f) * 0.5f);
            
            if (screenPos.x < min.x || screenPos.x > max.x ||
                screenPos.y < min.y || screenPos.y > max.y) {
                isInside = false;
            }
            
            if (isInside) {
                result.push_back(node);
            }
        }
    }
    
    return result;
}

std::vector<SceneNode::Ptr> ObjectPicker::pickInLasso(const std::vector<glm::vec2>& screenPoints,
                                                     const glm::mat4& viewMatrix,
                                                     const glm::mat4& projMatrix) {
    std::vector<SceneNode::Ptr> result;
    
    // Get all nodes and filter based on screen projection
    auto selectedNodes = m_sceneGraph->getSelectedNodes();
    for (const auto& node : selectedNodes) {
        if (m_selectionFilter && !m_selectionFilter(node)) {
            continue;
        }
        
        // Project node center to screen space
        glm::vec4 worldPos(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 clipSpace = projMatrix * viewMatrix * worldPos;
        glm::vec3 ndcSpace(clipSpace.x / clipSpace.w, clipSpace.y / clipSpace.w, clipSpace.z / clipSpace.w);
        glm::vec2 screenPos((ndcSpace.x + 1.0f) * 0.5f, (ndcSpace.y + 1.0f) * 0.5f);
        
        if (isPointInPolygon(screenPos, screenPoints)) {
            result.push_back(node);
        }
    }
    
    return result;
}

std::vector<SceneNode::Ptr> ObjectPicker::raycastScene(const glm::vec3& rayOrigin,
                                                      const glm::vec3& rayDir) {
    std::vector<SceneNode::Ptr> result;
    
    // Get all nodes from scene graph
    auto selectedNodes = m_sceneGraph->getSelectedNodes();
    
    // Sort nodes by distance for proper picking order
    struct PickResult {
        SceneNode::Ptr node;
        float distance;
        
        PickResult(const SceneNode::Ptr& n, float d) : node(n), distance(d) {}
    };
    std::vector<PickResult> picks;
    
    for (const auto& node : selectedNodes) {
        if (m_selectionFilter && !m_selectionFilter(node)) {
            continue;
        }
        
        // Check ray intersection with node
        if (node->intersect(rayOrigin, rayDir)) {
            // For now, use a simple distance metric
            glm::vec3 nodePos(node->getWorldTransform()[3]);
            float distance = glm::length(nodePos - rayOrigin);
            picks.emplace_back(node, distance);
        }
    }
    
    // Sort by distance
    std::sort(picks.begin(), picks.end(),
              [](const PickResult& a, const PickResult& b) {
                  return a.distance < b.distance;
              });
    
    // Convert to result vector
    result.reserve(picks.size());
    for (const auto& pick : picks) {
        result.push_back(pick.node);
    }
    
    return result;
}

void ObjectPicker::screenToRay(const glm::vec2& screenPos,
                              const glm::mat4& viewMatrix,
                              const glm::mat4& projMatrix,
                              glm::vec3& rayOrigin,
                              glm::vec3& rayDir) {
    // Convert screen coordinates to normalized device coordinates
    glm::vec4 rayStart_NDC(
        (screenPos.x * 2.0f) - 1.0f,
        (screenPos.y * 2.0f) - 1.0f,
        -1.0f,
        1.0f
    );
    glm::vec4 rayEnd_NDC(
        (screenPos.x * 2.0f) - 1.0f,
        (screenPos.y * 2.0f) - 1.0f,
        0.0f,
        1.0f
    );
    
    // Convert to world space
    glm::mat4 invVP = glm::inverse(projMatrix * viewMatrix);
    glm::vec4 rayStart_World = invVP * rayStart_NDC;
    glm::vec4 rayEnd_World = invVP * rayEnd_NDC;
    rayStart_World /= rayStart_World.w;
    rayEnd_World /= rayEnd_World.w;
    
    rayOrigin = glm::vec3(rayStart_World);
    rayDir = glm::normalize(glm::vec3(rayEnd_World - rayStart_World));
}

bool ObjectPicker::isPointInPolygon(const glm::vec2& point,
                                   const std::vector<glm::vec2>& polygon) {
    if (polygon.size() < 3) return false;
    
    bool inside = false;
    size_t j = polygon.size() - 1;
    
    for (size_t i = 0; i < polygon.size(); i++) {
        if ((polygon[i].y > point.y) != (polygon[j].y > point.y) &&
            point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) /
                     (polygon[j].y - polygon[i].y) + polygon[i].x) {
            inside = !inside;
        }
        j = i;
    }
    
    return inside;
}

} // namespace Graphics
} // namespace RebelCAD
