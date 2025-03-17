#pragma once

#include <memory>
#include <string>

namespace RebelCAD {
namespace Core {

/**
 * @brief Abstract base class for all commands in RebelCAD.
 * 
 * The Command class implements the Command pattern, allowing all operations
 * in RebelCAD to be undoable/redoable. Each command represents an atomic
 * operation that can be executed, undone, and redone.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * @brief Execute the command.
     * @return true if execution was successful, false otherwise.
     */
    virtual bool execute() = 0;

    /**
     * @brief Undo the command's effects.
     * @return true if undo was successful, false otherwise.
     */
    virtual bool undo() = 0;

    /**
     * @brief Redo the previously undone command.
     * @return true if redo was successful, false otherwise.
     */
    virtual bool redo() = 0;

    /**
     * @brief Get a description of what the command does.
     * @return String describing the command's action.
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Check if the command can be merged with another command.
     * @param other The command to potentially merge with.
     * @return true if commands can be merged, false otherwise.
     */
    virtual bool canMergeWith(const Command* other) const { return false; }

    /**
     * @brief Merge this command with another command.
     * @param other The command to merge with.
     * @return true if merge was successful, false otherwise.
     */
    virtual bool mergeWith(const Command* other) { return false; }
};

// Smart pointer type for Commands
using CommandPtr = std::shared_ptr<Command>;

} // namespace Core
} // namespace RebelCAD
