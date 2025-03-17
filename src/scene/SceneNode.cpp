#include "graphics/SceneNode.h"
#include "graphics/AABBBoundingGeometry.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/Error.h"

namespace {
    // Helper function to generate a unique ID
    uint64_t generateUniqueId() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        return dis(gen);
    }
}

namespace RebelCAD {
namespace Graphics {

SceneNode::SceneNode(const std::string& name)
    : m_name(name)
    , m_id(generateUniqueId())
    , m_visible(true)
    , m_selectable(true)
    , m_localTransform(1.0f)  // Identity matrix
    , m_worldTransform(1.0f)  // Identity matrix
    , m_parent()  // Empty weak_ptr
    , m_localBoundingBox(glm::vec3(-0.5f), glm::vec3(0.5f))  // Default unit cube AABB
    , m_position(0.0f)
    , m_rotation(0.0f)
    , m_scale(1.0f)
    , m_selected(false)
    , m_transformDirty(false)
    , m_batchUpdate(false)
    , m_boundingGeometry(std::make_unique<AABBBoundingGeometry>(m_localBoundingBox)) {
}

SceneNode::~SceneNode() {
    // Remove this node from parent's children
    if (auto parent = m_parent.lock()) {
        parent->removeChild(shared_from_this());
    }
    
    // Clear children (shared_ptr will handle cleanup)
    m_children.clear();
}

const BoundingGeometry& SceneNode::getBoundingGeometry() const {
    if (!m_boundingGeometry) {
        throw std::runtime_error("No bounding geometry defined for node");
    }
    return *m_boundingGeometry;
}

void SceneNode::addChild(const SceneNode::Ptr& child) {
    if (!child) return;
    
    // Remove from previous parent if any
    if (auto oldParent = child->getParent()) {
        oldParent->removeChild(child);
    }
    
    // Add to our children
    m_children.push_back(child);
    child->setParent(std::dynamic_pointer_cast<SceneNode>(shared_from_this()));
    
    // Update transform to reflect new parent
    child->updateWorldTransform();
}

void SceneNode::removeChild(const SceneNode::Ptr& child) {
    if (!child) return;
    
    auto it = std::find(m_children.begin(), m_children.end(), child);
    if (it != m_children.end()) {
        (*it)->setParent(SceneNode::WeakPtr());
        m_children.erase(it);
    }
}

void SceneNode::setParent(SceneNode::WeakPtr parent) {
    m_parent = parent;
    updateWorldTransform();
}

SceneNode::Ptr SceneNode::getParent() const {
    return m_parent.lock();
}

const std::vector<SceneNode::Ptr>& SceneNode::getChildren() const {
    return m_children;
}

void SceneNode::setLocalTransform(const glm::mat4& transform) {
    m_localTransform = transform;
    m_transformDirty = true;
    updateWorldTransform();
}

void SceneNode::setWorldTransform(const glm::mat4& transform) {
    if (auto parent = m_parent.lock()) {
        // Calculate local transform based on parent's inverse world transform
        glm::mat4 parentWorldInv = glm::inverse(parent->getWorldTransform());
        m_localTransform = parentWorldInv * transform;
    } else {
        m_localTransform = transform;
    }
    
    m_worldTransform = transform;
    updateBoundingVolume();
    
    // Update children
    for (auto& child : m_children) {
        child->m_transformDirty = true;
        child->updateWorldTransform();
    }
}

void SceneNode::updateWorldTransform() {
    if (auto parent = m_parent.lock()) {
        m_worldTransform = parent->getWorldTransform() * m_localTransform;
    } else {
        m_worldTransform = m_localTransform;
    }
    
    updateBoundingVolume();
    
    // Update children
    for (auto& child : m_children) {
        child->updateWorldTransform();
    }
}

void SceneNode::updateBoundingVolume() {
    if (!m_boundingGeometry) {
        // Create default bounding geometry if none exists
        m_boundingGeometry = std::make_unique<AABBBoundingGeometry>(m_localBoundingBox);
    }
    m_boundingGeometry->updateTransform(m_worldTransform);
}

bool SceneNode::isSelected() const {
    return m_selected.load(std::memory_order_acquire);
}

void SceneNode::setSelected(bool selected) {
    m_selected.store(selected, std::memory_order_release);
}

void SceneNode::update() {
    if (m_transformDirty.load(std::memory_order_acquire)) {
        updateWorldTransform();
    }
    
    // Update all children
    for (auto& child : m_children) {
        child->update();
    }
}

bool SceneNode::intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const {
    float distance;
    return intersectRay(rayOrigin, rayDir, distance);
}

nlohmann::json SceneNode::serialize() const {
    std::shared_lock<std::shared_mutex> lock(m_transformMutex);
    nlohmann::json data;
    
    // Basic properties
    data["name"] = m_name;
    data["id"] = m_id;
    data["selected"] = m_selected.load();
    data["visible"] = m_visible;
    data["selectable"] = m_selectable;
    
    // Transform components
    data["position"] = {
        {"x", m_position.x},
        {"y", m_position.y},
        {"z", m_position.z}
    };
    data["rotation"] = {
        {"x", m_rotation.x},
        {"y", m_rotation.y},
        {"z", m_rotation.z}
    };
    data["scale"] = {
        {"x", m_scale.x},
        {"y", m_scale.y},
        {"z", m_scale.z}
    };
    
    // Children
    data["children"] = nlohmann::json::array();
    for (const auto& child : m_children) {
        data["children"].push_back(child->serialize());
    }
    
    return data;
}

void SceneNode::deserialize(const nlohmann::json& data) {
    std::unique_lock<std::shared_mutex> lock(m_transformMutex);
    
    // Basic properties
    m_name = data["name"].get<std::string>();
    if (data.contains("id")) {
        m_id = data["id"].get<uint64_t>();
    } else {
        m_id = generateUniqueId();
    }
    m_selected.store(data["selected"].get<bool>());
    m_visible = data["visible"].get<bool>();
    m_selectable = data["selectable"].get<bool>();
    
    // Transform components
    m_position = glm::vec3(
        data["position"]["x"].get<float>(),
        data["position"]["y"].get<float>(),
        data["position"]["z"].get<float>()
    );
    m_rotation = glm::vec3(
        data["rotation"]["x"].get<float>(),
        data["rotation"]["y"].get<float>(),
        data["rotation"]["z"].get<float>()
    );
    m_scale = glm::vec3(
        data["scale"]["x"].get<float>(),
        data["scale"]["y"].get<float>(),
        data["scale"]["z"].get<float>()
    );
    
    // Update transforms
    m_localTransform = glm::translate(glm::mat4(1.0f), m_position) *
                      glm::toMat4(glm::quat(m_rotation)) *
                      glm::scale(glm::mat4(1.0f), m_scale);
    updateWorldTransform();
}

} // namespace Graphics
} // namespace RebelCAD
