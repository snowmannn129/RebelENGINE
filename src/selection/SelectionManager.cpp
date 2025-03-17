#include "graphics/SelectionManager.h"
#include "graphics/SceneGraph.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <algorithm>

using json = nlohmann::json;

namespace RebelCAD {
namespace Graphics {

SelectionManager::SelectionManager(std::shared_ptr<SceneGraph> sceneGraph,
                                 std::shared_ptr<SelectionVisualizer> visualizer)
    : m_sceneGraph(std::move(sceneGraph))
    , m_visualizer(std::move(visualizer)) {
}

void SelectionManager::select(const std::vector<SceneNode::Ptr>& nodes, SelectionMode mode) {
    pushState();
    
    switch (mode) {
        case SelectionMode::Single:
            m_currentState.selectedNodes.clear();
            for (const auto& node : nodes) {
                if (isNodeSelectable(node)) {
                    m_currentState.selectedNodes.push_back(node);
                }
            }
            break;
            
        case SelectionMode::Add:
            for (const auto& node : nodes) {
                if (isNodeSelectable(node)) {
                    if (std::find(m_currentState.selectedNodes.begin(),
                                m_currentState.selectedNodes.end(), node) ==
                        m_currentState.selectedNodes.end()) {
                        m_currentState.selectedNodes.push_back(node);
                    }
                }
            }
            break;
            
        case SelectionMode::Remove:
            for (const auto& node : nodes) {
                auto it = std::find(m_currentState.selectedNodes.begin(),
                                  m_currentState.selectedNodes.end(), node);
                if (it != m_currentState.selectedNodes.end()) {
                    m_currentState.selectedNodes.erase(it);
                }
            }
            break;
            
        case SelectionMode::Toggle:
            for (const auto& node : nodes) {
                if (isNodeSelectable(node)) {
                    auto it = std::find(m_currentState.selectedNodes.begin(),
                                      m_currentState.selectedNodes.end(), node);
                    if (it == m_currentState.selectedNodes.end()) {
                        m_currentState.selectedNodes.push_back(node);
                    } else {
                        m_currentState.selectedNodes.erase(it);
                    }
                }
            }
            break;
    }
    
    updateVisualization();
}

void SelectionManager::deselect(const std::vector<SceneNode::Ptr>& nodes) {
    pushState();
    
    for (const auto& node : nodes) {
        auto it = std::find(m_currentState.selectedNodes.begin(),
                           m_currentState.selectedNodes.end(), node);
        if (it != m_currentState.selectedNodes.end()) {
            m_currentState.selectedNodes.erase(it);
        }
    }
    
    updateVisualization();
}

void SelectionManager::clearSelection() {
    if (!m_currentState.selectedNodes.empty()) {
        pushState();
        m_currentState.selectedNodes.clear();
        updateVisualization();
    }
}

void SelectionManager::invertSelection() {
    pushState();
    
    std::vector<SceneNode::Ptr> allNodes = m_sceneGraph->getAllNodes();
    std::vector<SceneNode::Ptr> newSelection;
    
    for (const auto& node : allNodes) {
        if (isNodeSelectable(node)) {
            auto it = std::find(m_currentState.selectedNodes.begin(),
                               m_currentState.selectedNodes.end(), node);
            if (it == m_currentState.selectedNodes.end()) {
                newSelection.push_back(node);
            }
        }
    }
    
    m_currentState.selectedNodes = std::move(newSelection);
    updateVisualization();
}

void SelectionManager::addFilter(const std::string& name, SelectionFilter filter) {
    m_filters[name] = std::move(filter);
    m_activeFilters[name] = true;
    applyFilters();
}

void SelectionManager::removeFilter(const std::string& name) {
    m_filters.erase(name);
    m_activeFilters.erase(name);
    applyFilters();
}

void SelectionManager::enableFilter(const std::string& name, bool enable) {
    auto it = m_activeFilters.find(name);
    if (it != m_activeFilters.end()) {
        it->second = enable;
        applyFilters();
    }
}

void SelectionManager::clearFilters() {
    if (!m_filters.empty()) {
        m_filters.clear();
        m_activeFilters.clear();
        applyFilters();
    }
}

void SelectionManager::createGroup(const std::string& name,
                                 const std::vector<SceneNode::Ptr>& nodes) {
    pushState();
    
    SelectionGroup group;
    group.name = name;
    group.nodes = nodes;
    m_currentState.groups[name] = std::move(group);
}

void SelectionManager::deleteGroup(const std::string& name) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end()) {
        pushState();
        m_currentState.groups.erase(it);
    }
}

void SelectionManager::addToGroup(const std::string& name,
                                const std::vector<SceneNode::Ptr>& nodes) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end()) {
        pushState();
        auto& group = it->second;
        for (const auto& node : nodes) {
            if (std::find(group.nodes.begin(), group.nodes.end(), node) ==
                group.nodes.end()) {
                group.nodes.push_back(node);
            }
        }
    }
}

void SelectionManager::removeFromGroup(const std::string& name,
                                     const std::vector<SceneNode::Ptr>& nodes) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end()) {
        pushState();
        auto& group = it->second;
        for (const auto& node : nodes) {
            auto nodeIt = std::find(group.nodes.begin(), group.nodes.end(), node);
            if (nodeIt != group.nodes.end()) {
                group.nodes.erase(nodeIt);
            }
        }
    }
}

void SelectionManager::selectGroup(const std::string& name) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end() && !it->second.isLocked) {
        select(it->second.nodes);
    }
}

void SelectionManager::setGroupVisibility(const std::string& name, bool visible) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end()) {
        pushState();
        it->second.isVisible = visible;
        // Update node visibility in scene graph
        for (const auto& node : it->second.nodes) {
            node->setVisible(visible);
        }
    }
}

void SelectionManager::setGroupLocked(const std::string& name, bool locked) {
    auto it = m_currentState.groups.find(name);
    if (it != m_currentState.groups.end()) {
        pushState();
        it->second.isLocked = locked;
    }
}

void SelectionManager::undo() {
    if (!m_undoStack.empty()) {
        m_redoStack.push(m_currentState);
        m_currentState = m_undoStack.top();
        m_undoStack.pop();
        updateVisualization();
    }
}

void SelectionManager::redo() {
    if (!m_redoStack.empty()) {
        m_undoStack.push(m_currentState);
        m_currentState = m_redoStack.top();
        m_redoStack.pop();
        updateVisualization();
    }
}

void SelectionManager::saveSelectionState(const std::string& filename) {
    json j;
    
    // Save selected nodes
    j["selectedNodes"] = json::array();
    for (const auto& node : m_currentState.selectedNodes) {
        j["selectedNodes"].push_back(node->getId());
    }
    
    // Save groups
    j["groups"] = json::object();
    for (const auto& [name, group] : m_currentState.groups) {
        json groupJson;
        groupJson["name"] = group.name;
        groupJson["isVisible"] = group.isVisible;
        groupJson["isLocked"] = group.isLocked;
        groupJson["nodes"] = json::array();
        for (const auto& node : group.nodes) {
            groupJson["nodes"].push_back(node->getId());
        }
        j["groups"][name] = groupJson;
    }
    
    // Write to file
    std::ofstream file(filename);
    file << j.dump(4);
}

void SelectionManager::loadSelectionState(const std::string& filename) {
    std::ifstream file(filename);
    json j = json::parse(file);
    
    pushState();
    
    // Load selected nodes
    m_currentState.selectedNodes.clear();
    for (const auto& nodeId : j["selectedNodes"]) {
        if (auto node = m_sceneGraph->findNode(nodeId)) {
            m_currentState.selectedNodes.push_back(node);
        }
    }
    
    // Load groups
    m_currentState.groups.clear();
    for (const auto& [name, groupJson] : j["groups"].items()) {
        SelectionGroup group;
        group.name = groupJson["name"];
        group.isVisible = groupJson["isVisible"];
        group.isLocked = groupJson["isLocked"];
        
        for (const auto& nodeId : groupJson["nodes"]) {
            if (auto node = m_sceneGraph->findNode(nodeId)) {
                group.nodes.push_back(node);
            }
        }
        
        m_currentState.groups[name] = std::move(group);
    }
    
    updateVisualization();
}

void SelectionManager::pushState() {
    m_undoStack.push(m_currentState);
    if (m_undoStack.size() > MAX_HISTORY_SIZE) {
        m_undoStack.pop();
    }
    m_redoStack = std::stack<SelectionState>(); // Clear redo stack
}

void SelectionManager::applyFilters() {
    std::vector<SceneNode::Ptr> filteredNodes;
    
    for (const auto& node : m_currentState.selectedNodes) {
        if (isNodeSelectable(node)) {
            filteredNodes.push_back(node);
        }
    }
    
    if (filteredNodes.size() != m_currentState.selectedNodes.size()) {
        pushState();
        m_currentState.selectedNodes = std::move(filteredNodes);
        updateVisualization();
    }
}

void SelectionManager::updateVisualization() {
    if (m_visualizer) {
        m_visualizer->updateSelection(m_currentState.selectedNodes);
    }
}

bool SelectionManager::isNodeSelectable(const SceneNode::Ptr& node) const {
    if (!node) return false;
    
    // Check if node is in a locked group
    for (const auto& [name, group] : m_currentState.groups) {
        if (group.isLocked) {
            auto it = std::find(group.nodes.begin(), group.nodes.end(), node);
            if (it != group.nodes.end()) {
                return false;
            }
        }
    }
    
    // Apply active filters
    for (const auto& [name, filter] : m_filters) {
        auto activeIt = m_activeFilters.find(name);
        if (activeIt != m_activeFilters.end() && activeIt->second) {
            if (!filter(node)) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace Graphics
} // namespace RebelCAD
