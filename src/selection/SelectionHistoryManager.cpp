#include "graphics/SelectionHistoryManager.h"
#include <algorithm>

namespace RebelCAD {
namespace Graphics {

SelectionHistoryManager::SelectionHistoryManager()
    : m_selectionChangeCallback(nullptr)
{
}

void SelectionHistoryManager::recordOperation(
    const std::vector<SceneNode::Ptr>& previous,
    const std::vector<SceneNode::Ptr>& current,
    const std::string& description
) {
    // Don't record if selections are identical
    if (previous == current) return;

    // Clear redo stack when new operation is recorded
    while (!m_redoStack.empty()) {
        m_redoStack.pop();
    }

    // Create new operation
    SelectionOperation operation{
        .previousSelection = previous,
        .currentSelection = current,
        .description = description
    };

    // Add to undo stack
    m_undoStack.push(std::move(operation));

    // Trim stack if it exceeds maximum size
    trimUndoStack();
}

std::vector<SceneNode::Ptr> SelectionHistoryManager::undo() {
    if (!canUndo()) {
        return {};
    }

    // Get the operation to undo
    SelectionOperation operation = std::move(m_undoStack.top());
    m_undoStack.pop();

    // Move operation to redo stack
    m_redoStack.push(std::move(operation));

    // Get previous selection state
    auto previousSelection = m_redoStack.top().previousSelection;

    // Notify of selection change
    notifySelectionChange(previousSelection);

    return previousSelection;
}

std::vector<SceneNode::Ptr> SelectionHistoryManager::redo() {
    if (!canRedo()) {
        return {};
    }

    // Get the operation to redo
    SelectionOperation operation = std::move(m_redoStack.top());
    m_redoStack.pop();

    // Move operation to undo stack
    m_undoStack.push(std::move(operation));

    // Get next selection state
    auto nextSelection = m_undoStack.top().currentSelection;

    // Notify of selection change
    notifySelectionChange(nextSelection);

    return nextSelection;
}

std::string SelectionHistoryManager::getUndoDescription() const {
    if (!canUndo()) {
        return "";
    }
    return m_undoStack.top().description;
}

std::string SelectionHistoryManager::getRedoDescription() const {
    if (!canRedo()) {
        return "";
    }
    return m_redoStack.top().description;
}

void SelectionHistoryManager::setSelectionChangeCallback(
    std::function<void(const std::vector<SceneNode::Ptr>&)> callback
) {
    m_selectionChangeCallback = std::move(callback);
}

void SelectionHistoryManager::clear() {
    while (!m_undoStack.empty()) {
        m_undoStack.pop();
    }
    while (!m_redoStack.empty()) {
        m_redoStack.pop();
    }
}

void SelectionHistoryManager::trimUndoStack() {
    while (m_undoStack.size() > MAX_HISTORY_SIZE) {
        m_undoStack.pop();
    }
}

void SelectionHistoryManager::notifySelectionChange(
    const std::vector<SceneNode::Ptr>& selection
) {
    if (m_selectionChangeCallback) {
        m_selectionChangeCallback(selection);
    }
}

} // namespace Graphics
} // namespace RebelCAD
