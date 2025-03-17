#pragma once

#include "Command.h"
#include "EventSystem.h"
#include <vector>
#include <memory>
#include <string>

namespace RebelCAD {
namespace Core {

/**
 * @brief Manages the undo/redo stack and command execution.
 * 
 * The UndoRedoManager maintains two stacks of commands: one for undo operations
 * and one for redo operations. It handles command execution, undoing, redoing,
 * and command merging for operations like continuous sketching or dragging.
 */
class UndoRedoManager {
public:
    UndoRedoManager();
    ~UndoRedoManager() = default;

    /**
     * @brief Execute a new command and add it to the undo stack.
     * @param command The command to execute.
     * @return true if execution was successful, false otherwise.
     */
    bool executeCommand(CommandPtr command);

    /**
     * @brief Undo the most recent command.
     * @return true if undo was successful, false otherwise.
     */
    bool undo();

    /**
     * @brief Redo the most recently undone command.
     * @return true if redo was successful, false otherwise.
     */
    bool redo();

    /**
     * @brief Check if there are commands that can be undone.
     * @return true if there are commands in the undo stack.
     */
    bool canUndo() const;

    /**
     * @brief Check if there are commands that can be redone.
     * @return true if there are commands in the redo stack.
     */
    bool canRedo() const;

    /**
     * @brief Get the description of the next command to be undone.
     * @return Description string, or empty if no command to undo.
     */
    std::string getUndoDescription() const;

    /**
     * @brief Get the description of the next command to be redone.
     * @return Description string, or empty if no command to redo.
     */
    std::string getRedoDescription() const;

    /**
     * @brief Clear both undo and redo stacks.
     */
    void clear();

    /**
     * @brief Begin a command group for combining multiple commands.
     * @param description Description for the command group.
     */
    void beginCommandGroup(const std::string& description);

    /**
     * @brief End the current command group.
     */
    void endCommandGroup();

    /**
     * @brief Enable/disable command merging for continuous operations.
     * @param enable True to enable merging, false to disable.
     */
    void setCommandMerging(bool enable);

private:
    std::vector<CommandPtr> undoStack_;
    std::vector<CommandPtr> redoStack_;
    bool isGrouping_;
    bool allowMerging_;
    std::vector<CommandPtr> currentGroup_;
    std::string currentGroupDescription_;

    /**
     * @brief Try to merge a new command with the last command in the undo stack.
     * @param command The new command to potentially merge.
     * @return true if merge was successful, false if commands couldn't be merged.
     */
    bool tryMergeCommand(const CommandPtr& command);

    /**
     * @brief Clear the redo stack.
     * Called when a new command is executed, as the redo history becomes invalid.
     */
    void clearRedoStack();

    /**
     * @brief Notify observers about changes in the undo/redo state.
     */
    void notifyStateChanged();
};

} // namespace Core
} // namespace RebelCAD
