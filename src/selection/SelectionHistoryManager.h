#pragma once

#include "graphics/SceneNode.h"
#include <vector>
#include <memory>
#include <functional>
#include <stack>

namespace RebelCAD {
namespace Graphics {

/**
 * @brief Represents a selection operation for undo/redo
 */
struct SelectionOperation {
    std::vector<SceneNode::Ptr> previousSelection;  ///< Selection before operation
    std::vector<SceneNode::Ptr> currentSelection;   ///< Selection after operation
    std::string description;                        ///< Operation description
};

/**
 * @brief Manages selection history with undo/redo support
 */
class SelectionHistoryManager {
public:
    /**
     * @brief Constructor
     */
    SelectionHistoryManager();

    /**
     * @brief Records a selection operation
     * @param previous Previous selection state
     * @param current Current selection state
     * @param description Operation description
     */
    void recordOperation(
        const std::vector<SceneNode::Ptr>& previous,
        const std::vector<SceneNode::Ptr>& current,
        const std::string& description
    );

    /**
     * @brief Undoes the last selection operation
     * @return Previous selection state
     */
    std::vector<SceneNode::Ptr> undo();

    /**
     * @brief Redoes the last undone selection operation
     * @return Next selection state
     */
    std::vector<SceneNode::Ptr> redo();

    /**
     * @brief Checks if undo is available
     * @return true if undo is available
     */
    bool canUndo() const { return !m_undoStack.empty(); }

    /**
     * @brief Checks if redo is available
     * @return true if redo is available
     */
    bool canRedo() const { return !m_redoStack.empty(); }

    /**
     * @brief Gets the description of the next undo operation
     * @return Operation description or empty string if no undo available
     */
    std::string getUndoDescription() const;

    /**
     * @brief Gets the description of the next redo operation
     * @return Operation description or empty string if no redo available
     */
    std::string getRedoDescription() const;

    /**
     * @brief Sets callback for selection changes from undo/redo
     * @param callback Function to call when selection changes
     */
    void setSelectionChangeCallback(
        std::function<void(const std::vector<SceneNode::Ptr>&)> callback
    );

    /**
     * @brief Clears all history
     */
    void clear();

private:
    std::stack<SelectionOperation> m_undoStack;
    std::stack<SelectionOperation> m_redoStack;
    std::function<void(const std::vector<SceneNode::Ptr>&)> m_selectionChangeCallback;

    // Maximum number of operations to store
    static constexpr size_t MAX_HISTORY_SIZE = 100;

    // Helper methods
    void trimUndoStack();
    void notifySelectionChange(const std::vector<SceneNode::Ptr>& selection);
};

} // namespace Graphics
} // namespace RebelCAD
