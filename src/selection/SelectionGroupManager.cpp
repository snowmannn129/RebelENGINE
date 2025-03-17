#include "graphics/SelectionGroupManager.h"
#include <algorithm>
#include <unordered_set>

namespace RebelCAD {
namespace Graphics {

SelectionGroupManager::SelectionGroupManager()
    : m_groupChangeCallback(nullptr)
{
}

bool SelectionGroupManager::createGroup(
    const std::string& name,
    const std::vector<SceneNode::Ptr>& nodes,
    const std::string& description
) {
    if (m_groups.find(name) != m_groups.end()) {
        return false; // Group already exists
    }

    SelectionGroup group{
        .name = name,
        .nodes = nodes,
        .visible = true,
        .locked = false,
        .description = description
    };

    m_groups[name] = std::move(group);
    notifyGroupChange(name);
    return true;
}

bool SelectionGroupManager::removeGroup(const std::string& name) {
    if (!isValidGroup(name)) {
        return false;
    }

    m_groups.erase(name);
    notifyGroupChange(name);
    return true;
}

bool SelectionGroupManager::addToGroup(
    const std::string& name,
    const std::vector<SceneNode::Ptr>& nodes
) {
    auto it = m_groups.find(name);
    if (it == m_groups.end() || it->second.locked) {
        return false;
    }

    // Add new nodes (avoiding duplicates)
    std::unordered_set<SceneNode::Ptr> existing(
        it->second.nodes.begin(),
        it->second.nodes.end()
    );
    bool changed = false;

    for (const auto& node : nodes) {
        if (existing.insert(node).second) {
            it->second.nodes.push_back(node);
            changed = true;
        }
    }

    if (changed) {
        notifyGroupChange(name);
    }
    return true;
}

bool SelectionGroupManager::removeFromGroup(
    const std::string& name,
    const std::vector<SceneNode::Ptr>& nodes
) {
    auto it = m_groups.find(name);
    if (it == m_groups.end() || it->second.locked) {
        return false;
    }

    // Create set of nodes to remove
    std::unordered_set<SceneNode::Ptr> toRemove(nodes.begin(), nodes.end());

    // Remove nodes
    size_t originalSize = it->second.nodes.size();
    it->second.nodes.erase(
        std::remove_if(
            it->second.nodes.begin(),
            it->second.nodes.end(),
            [&toRemove](const SceneNode::Ptr& node) {
                return toRemove.find(node) != toRemove.end();
            }
        ),
        it->second.nodes.end()
    );

    if (it->second.nodes.size() != originalSize) {
        notifyGroupChange(name);
        return true;
    }
    return false;
}

std::vector<SceneNode::Ptr> SelectionGroupManager::getGroupMembers(
    const std::string& name
) const {
    auto it = m_groups.find(name);
    return it != m_groups.end() ? it->second.nodes : std::vector<SceneNode::Ptr>();
}

std::vector<std::string> SelectionGroupManager::getNodeGroups(
    const SceneNode::Ptr& node
) const {
    std::vector<std::string> groups;
    for (const auto& [name, group] : m_groups) {
        if (std::find(group.nodes.begin(), group.nodes.end(), node) != group.nodes.end()) {
            groups.push_back(name);
        }
    }
    return groups;
}

bool SelectionGroupManager::setGroupVisible(
    const std::string& name,
    bool visible
) {
    auto it = m_groups.find(name);
    if (it == m_groups.end()) {
        return false;
    }

    if (it->second.visible != visible) {
        it->second.visible = visible;
        notifyGroupChange(name);
        return true;
    }
    return false;
}

bool SelectionGroupManager::setGroupLocked(
    const std::string& name,
    bool locked
) {
    auto it = m_groups.find(name);
    if (it == m_groups.end()) {
        return false;
    }

    if (it->second.locked != locked) {
        it->second.locked = locked;
        notifyGroupChange(name);
        return true;
    }
    return false;
}

std::vector<std::string> SelectionGroupManager::listGroups() const {
    std::vector<std::string> names;
    names.reserve(m_groups.size());
    for (const auto& [name, _] : m_groups) {
        names.push_back(name);
    }
    return names;
}

std::string SelectionGroupManager::getGroupDescription(
    const std::string& name
) const {
    auto it = m_groups.find(name);
    return it != m_groups.end() ? it->second.description : "";
}

bool SelectionGroupManager::setGroupMetadata(
    const std::string& name,
    const std::string& key,
    const std::string& value
) {
    auto it = m_groups.find(name);
    if (it == m_groups.end() || it->second.locked) {
        return false;
    }

    it->second.metadata[key] = value;
    notifyGroupChange(name);
    return true;
}

std::string SelectionGroupManager::getGroupMetadata(
    const std::string& name,
    const std::string& key
) const {
    auto it = m_groups.find(name);
    if (it == m_groups.end()) {
        return "";
    }

    auto metaIt = it->second.metadata.find(key);
    return metaIt != it->second.metadata.end() ? metaIt->second : "";
}

void SelectionGroupManager::setGroupChangeCallback(
    std::function<void(const std::string&)> callback
) {
    m_groupChangeCallback = std::move(callback);
}

void SelectionGroupManager::notifyGroupChange(const std::string& name) {
    if (m_groupChangeCallback) {
        m_groupChangeCallback(name);
    }
}

bool SelectionGroupManager::isValidGroup(const std::string& name) const {
    return m_groups.find(name) != m_groups.end();
}

} // namespace Graphics
} // namespace RebelCAD
