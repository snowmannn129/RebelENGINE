#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include <functional>
#include "SceneNode.h"
#include "SelectionVisualizer.h"

namespace RebelCAD {
namespace Graphics {

// Forward declarations
class SceneGraph;

// Selection interaction modes
enum class SelectionMode {
    Single,     // Single object selection
    Add,        // Add to current selection
    Remove,     // Remove from current selection
    Toggle,     // Toggle selection state
    Edge,       // Edge selection mode
    Vertex,     // Vertex selection mode
    Face        // Face selection mode
};

// Selection filter predicate type
using SelectionFilter = std::function<bool(const SceneNode::Ptr&)>;

// Selection group definition
struct SelectionGroup {
    std::string name;
    std::vector<SceneNode::Ptr> nodes;
    bool isVisible{true};
    bool isLocked{false};
};

// Selection state for history
struct SelectionState {
    std::vector<SceneNode::Ptr> selectedNodes;
    std::unordered_map<std::string, SelectionGroup> groups;
};

class SelectionManager {
public:
    using Ptr = std::shared_ptr<SelectionManager>;
    
    SelectionManager(std::shared_ptr<SceneGraph> sceneGraph, 
                    std::shared_ptr<SelectionVisualizer> visualizer);
    ~SelectionManager() = default;

    // Selection operations
    void select(const std::vector<SceneNode::Ptr>& nodes, SelectionMode mode = SelectionMode::Single);
    void deselect(const std::vector<SceneNode::Ptr>& nodes);
    void clearSelection();
    void invertSelection();
    
    // Selection filtering
    void addFilter(const std::string& name, SelectionFilter filter);
    void removeFilter(const std::string& name);
    void enableFilter(const std::string& name, bool enable);
    void clearFilters();
    
    // Selection groups
    void createGroup(const std::string& name, const std::vector<SceneNode::Ptr>& nodes);
    void deleteGroup(const std::string& name);
    void addToGroup(const std::string& name, const std::vector<SceneNode::Ptr>& nodes);
    void removeFromGroup(const std::string& name, const std::vector<SceneNode::Ptr>& nodes);
    void selectGroup(const std::string& name);
    void setGroupVisibility(const std::string& name, bool visible);
    void setGroupLocked(const std::string& name, bool locked);
    
    // History operations
    void undo();
    void redo();
    
    // Persistence
    void saveSelectionState(const std::string& filename);
    void loadSelectionState(const std::string& filename);
    
    // Selection mode
    void setSelectionMode(SelectionMode mode) { m_currentMode = mode; }
    SelectionMode getSelectionMode() const { return m_currentMode; }

    // Getters
    const std::vector<SceneNode::Ptr>& getSelectedNodes() const { return m_currentState.selectedNodes; }
    const std::unordered_map<std::string, SelectionGroup>& getGroups() const { return m_currentState.groups; }
    bool hasSelection() const { return !m_currentState.selectedNodes.empty(); }

private:
    // Helper methods
    void pushState();
    void applyFilters();
    void updateVisualization();
    bool isNodeSelectable(const SceneNode::Ptr& node) const;
    
    // Core components
    std::shared_ptr<SceneGraph> m_sceneGraph;
    std::shared_ptr<SelectionVisualizer> m_visualizer;
    SelectionMode m_currentMode{SelectionMode::Single};
    
    // Selection state
    SelectionState m_currentState;
    
    // History
    std::stack<SelectionState> m_undoStack;
    std::stack<SelectionState> m_redoStack;
    
    // Filters
    std::unordered_map<std::string, SelectionFilter> m_filters;
    std::unordered_map<std::string, bool> m_activeFilters;
    
    // Constants
    static constexpr size_t MAX_HISTORY_SIZE = 100;
};

} // namespace Graphics
} // namespace RebelCAD
