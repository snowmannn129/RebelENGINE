#pragma once

#include <memory>
#include <string>
#include <vector>
#include <shared_mutex>
#include <atomic>
#include "graphics/GLMConfig.hpp"
#include "graphics/SceneObject.h"
#include "graphics/BoundingGeometry.h"
#include <nlohmann/json.hpp>

namespace RebelCAD {
namespace Graphics {

// Axis-aligned bounding box
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB(const glm::vec3& min = glm::vec3(-0.5f), const glm::vec3& max = glm::vec3(0.5f))
        : min(min), max(max) {}
};

/**
 * @brief Base class for all scene graph nodes
 */
class SceneNode : public SceneObject, public std::enable_shared_from_this<SceneNode> {
public:
    using Ptr = std::shared_ptr<SceneNode>;
    using WeakPtr = std::weak_ptr<SceneNode>;
    using ConstPtr = std::shared_ptr<const SceneNode>;

    virtual ~SceneNode();

    /**
     * @brief Gets the node's name
     * @return Node name
     */
    const std::string& getName() const { return m_name; }
    
    /**
     * @brief Gets the node's unique ID
     * @return Node ID
     */
    uint64_t getId() const { return m_id; }

    /**
     * @brief Sets the node's name
     * @param name New name
     */
    void setName(const std::string& name) { m_name = name; }

    /**
     * @brief Gets the node's local transform
     * @return Local transform matrix
     */
    const glm::mat4& getLocalTransform() const { return m_localTransform; }

    /**
     * @brief Sets the node's local transform
     * @param transform New transform matrix
     */
    void setLocalTransform(const glm::mat4& transform);

    /**
     * @brief Gets the node's world transform
     * @return World transform matrix
     */
    // Implement SceneObject interface
    const BoundingGeometry& getBoundingGeometry() const override;
    glm::mat4 getWorldTransform() const override { return m_worldTransform; }
    glm::vec3 getWorldPosition() const override { return glm::vec3(m_worldTransform[3]); }
    void update() override;

    /**
     * @brief Gets the node's parent
     * @return Parent node or nullptr if root
     */
    SceneNode::Ptr getParent() const;

    /**
     * @brief Gets the node's full path in the scene graph
     * @return Path from root to this node (e.g. "root/group1/node2")
     */
    std::string getFullPath() const {
        std::string path = m_name;
        auto parent = getParent();
        while (parent) {
            path = parent->getName() + "/" + path;
            parent = parent->getParent();
        }
        return path;
    }

    /**
     * @brief Gets a hash of the node's transform
     * @return Hash value representing current transform state
     */
    size_t getTransformHash() const {
        std::hash<float> hasher;
        size_t hash = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                hash ^= hasher(m_worldTransform[i][j]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
        }
        return hash;
    }

    /**
     * @brief Gets the node's children
     * @return Vector of child nodes
     */
    const std::vector<SceneNode::Ptr>& getChildren() const;

    /**
     * @brief Adds a child node
     * @param child Node to add
     */
    void addChild(const SceneNode::Ptr& child);

    /**
     * @brief Removes a child node
     * @param child Node to remove
     */
    void removeChild(const SceneNode::Ptr& child);

    /**
     * @brief Updates the node's world transform
     * @param parentTransform Parent's world transform
     */
    virtual void updateWorldTransform();
    virtual void setWorldTransform(const glm::mat4& transform);
    virtual void setParent(WeakPtr parent);
    virtual void updateBoundingVolume();
    virtual nlohmann::json serialize() const;
    virtual void deserialize(const nlohmann::json& data);
    bool isSelected() const;
    void setSelected(bool selected);

    /**
     * @brief Checks if the node is visible
     * @return true if node is visible
     */
    bool isVisible() const { return m_visible && (!getParent() || getParent()->isVisible()); }

    /**
     * @brief Sets the node's visibility
     * @param visible New visibility state
     */
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * @brief Checks if the node is selectable
     * @return true if node is selectable
     */
    bool isSelectable() const { return m_selectable && (!getParent() || getParent()->isSelectable()); }

    /**
     * @brief Sets the node's selectability
     * @param selectable New selectability state
     */
    void setSelectable(bool selectable) { m_selectable = selectable; }

    /**
     * @brief Checks if the node has mesh data
     * @return true if node has mesh data
     */
    virtual bool hasMeshData() const { return false; }

    /**
     * @brief Tests if a ray intersects with this node
     * @param rayOrigin Origin of the ray
     * @param rayDir Direction of the ray
     * @return true if ray intersects node
     */
    virtual bool intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDir) const;

protected:
    SceneNode() = default;
    SceneNode(const std::string& name);

private:
    std::string m_name;
    uint64_t m_id;
    glm::mat4 m_localTransform{1.0f};
    glm::mat4 m_worldTransform{1.0f};
    WeakPtr m_parent;
    std::vector<Ptr> m_children;
    bool m_visible{true};
    bool m_selectable{true};
    glm::vec3 m_position{0.0f};
    glm::vec3 m_rotation{0.0f};
    glm::vec3 m_scale{1.0f};
    std::atomic<bool> m_selected{false};
    std::atomic<bool> m_transformDirty{false};
    bool m_batchUpdate{false};
    mutable std::shared_mutex m_transformMutex;
    AABB m_localBoundingBox;
    std::unique_ptr<BoundingGeometry> m_boundingGeometry;

    // Protected virtual methods for derived classes
    virtual bool intersectRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& distance) const { return false; }
};

} // namespace Graphics
} // namespace RebelCAD
