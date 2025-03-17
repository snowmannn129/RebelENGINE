#include "graphics/SelectionFilterManager.h"
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

SelectionFilterManager::SelectionFilterManager() {
    createStandardFilters();
}

bool SelectionFilterManager::addFilter(
    const std::string& name,
    std::function<bool(const SceneNode::Ptr&)> predicate,
    const std::string& description
) {
    if (m_filters.find(name) != m_filters.end()) {
        return false; // Filter already exists
    }

    SelectionFilter filter{
        .name = name,
        .predicate = std::move(predicate),
        .enabled = true,
        .description = description
    };

    m_filters[name] = std::move(filter);
    return true;
}

bool SelectionFilterManager::removeFilter(const std::string& name) {
    return m_filters.erase(name) > 0;
}

bool SelectionFilterManager::setFilterEnabled(
    const std::string& name,
    bool enabled
) {
    auto it = m_filters.find(name);
    if (it == m_filters.end()) {
        return false;
    }

    it->second.enabled = enabled;
    return true;
}

bool SelectionFilterManager::isFilterEnabled(const std::string& name) const {
    auto it = m_filters.find(name);
    return it != m_filters.end() && it->second.enabled;
}

std::string SelectionFilterManager::getFilterDescription(
    const std::string& name
) const {
    auto it = m_filters.find(name);
    return it != m_filters.end() ? it->second.description : "";
}

std::vector<std::string> SelectionFilterManager::listFilters() const {
    std::vector<std::string> names;
    names.reserve(m_filters.size());
    for (const auto& [name, _] : m_filters) {
        names.push_back(name);
    }
    return names;
}

std::vector<SceneNode::Ptr> SelectionFilterManager::applyFilters(
    const std::vector<SceneNode::Ptr>& selection
) const {
    std::vector<SceneNode::Ptr> filtered = selection;

    // Apply all enabled filters
    for (const auto& [name, filter] : m_filters) {
        if (filter.enabled) {
            filtered.erase(
                std::remove_if(
                    filtered.begin(),
                    filtered.end(),
                    [&filter](const SceneNode::Ptr& node) {
                        return !filter.predicate(node);
                    }
                ),
                filtered.end()
            );
        }
    }

    return filtered;
}

std::vector<SceneNode::Ptr> SelectionFilterManager::applyFilter(
    const std::string& name,
    const std::vector<SceneNode::Ptr>& selection
) const {
    auto it = m_filters.find(name);
    if (it == m_filters.end() || !it->second.enabled) {
        return selection;
    }

    std::vector<SceneNode::Ptr> filtered = selection;
    filtered.erase(
        std::remove_if(
            filtered.begin(),
            filtered.end(),
            [&filter = it->second](const SceneNode::Ptr& node) {
                return !filter.predicate(node);
            }
        ),
        filtered.end()
    );

    return filtered;
}

void SelectionFilterManager::createStandardFilters() {
    // Visible objects filter
    addFilter(
        "Visible",
        filterVisibleObjects,
        "Only select visible objects"
    );

    // Selectable objects filter
    addFilter(
        "Selectable",
        filterSelectableObjects,
        "Only select objects that can be selected"
    );

    // Mesh objects filter
    addFilter(
        "Meshes",
        filterMeshObjects,
        "Only select mesh objects"
    );

    // Group nodes filter
    addFilter(
        "Groups",
        filterGroupNodes,
        "Only select group nodes"
    );

    // Leaf nodes filter
    addFilter(
        "Leaves",
        filterLeafNodes,
        "Only select leaf nodes (no children)"
    );
}

bool SelectionFilterManager::filterVisibleObjects(const SceneNode::Ptr& node) {
    if (!node) return false;
    return node->isVisible();
}

bool SelectionFilterManager::filterSelectableObjects(const SceneNode::Ptr& node) {
    if (!node) return false;
    return node->isSelectable();
}

bool SelectionFilterManager::filterMeshObjects(const SceneNode::Ptr& node) {
    if (!node) return false;
    return node->hasMeshData();
}

bool SelectionFilterManager::filterGroupNodes(const SceneNode::Ptr& node) {
    // Check if node has children
    return !node->getChildren().empty();
}

bool SelectionFilterManager::filterLeafNodes(const SceneNode::Ptr& node) {
    // Check if node has no children
    return node->getChildren().empty();
}

} // namespace Graphics
} // namespace RebelCAD
