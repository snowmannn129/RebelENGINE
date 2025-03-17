#include "core/UndoRedoManager.h"
#include "core/EventSystem.h"
#include "core/UndoRedoEvent.h"
#include <cassert>

namespace RebelCAD {
namespace Core {

UndoRedoManager::UndoRedoManager()
    : isGrouping_(false)
    , allowMerging_(true) {
}

bool UndoRedoManager::executeCommand(CommandPtr command) {
    if (!command) {
        return false;
    }

    // If we're grouping commands, add to current group
    if (isGrouping_) {
        if (command->execute()) {
            currentGroup_.push_back(command);
            notifyStateChanged();
            return true;
        }
        return false;
    }

    // Try to merge with previous command if merging is enabled
    if (allowMerging_ && !undoStack_.empty() && tryMergeCommand(command)) {
        notifyStateChanged();
        return true;
    }

    // Execute the command and add to undo stack if successful
    if (command->execute()) {
        undoStack_.push_back(command);
        clearRedoStack();
        notifyStateChanged();
        return true;
    }

    return false;
}

bool UndoRedoManager::undo() {
    if (!canUndo()) {
        return false;
    }

    CommandPtr command = undoStack_.back();
    if (command->undo()) {
        undoStack_.pop_back();
        redoStack_.push_back(command);
        notifyStateChanged();
        return true;
    }

    return false;
}

bool UndoRedoManager::redo() {
    if (!canRedo()) {
        return false;
    }

    CommandPtr command = redoStack_.back();
    if (command->redo()) {
        redoStack_.pop_back();
        undoStack_.push_back(command);
        notifyStateChanged();
        return true;
    }

    return false;
}

bool UndoRedoManager::canUndo() const {
    return !undoStack_.empty() && !isGrouping_;
}

bool UndoRedoManager::canRedo() const {
    return !redoStack_.empty() && !isGrouping_;
}

std::string UndoRedoManager::getUndoDescription() const {
    if (!canUndo()) {
        return "";
    }
    return undoStack_.back()->getDescription();
}

std::string UndoRedoManager::getRedoDescription() const {
    if (!canRedo()) {
        return "";
    }
    return redoStack_.back()->getDescription();
}

void UndoRedoManager::clear() {
    undoStack_.clear();
    redoStack_.clear();
    currentGroup_.clear();
    isGrouping_ = false;
    notifyStateChanged();
}

void UndoRedoManager::beginCommandGroup(const std::string& description) {
    if (isGrouping_) {
        endCommandGroup(); // End current group before starting new one
    }
    
    isGrouping_ = true;
    currentGroupDescription_ = description;
    currentGroup_.clear();
}

void UndoRedoManager::endCommandGroup() {
    if (!isGrouping_) {
        return;
    }

    if (!currentGroup_.empty()) {
        // Create a composite command from the group
        class CompositeCommand : public Command {
        public:
            CompositeCommand(std::vector<CommandPtr> commands, std::string desc)
                : commands_(std::move(commands))
                , description_(std::move(desc)) {}

            bool execute() override {
                for (const auto& cmd : commands_) {
                    if (!cmd->execute()) {
                        return false;
                    }
                }
                return true;
            }

            bool undo() override {
                // Undo in reverse order
                for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
                    if (!(*it)->undo()) {
                        return false;
                    }
                }
                return true;
            }

            bool redo() override {
                for (const auto& cmd : commands_) {
                    if (!cmd->redo()) {
                        return false;
                    }
                }
                return true;
            }

            std::string getDescription() const override {
                return description_;
            }

        private:
            std::vector<CommandPtr> commands_;
            std::string description_;
        };

        auto groupCommand = std::make_shared<CompositeCommand>(
            std::move(currentGroup_),
            std::move(currentGroupDescription_)
        );

        undoStack_.push_back(groupCommand);
        clearRedoStack();
    }

    isGrouping_ = false;
    currentGroup_.clear();
    currentGroupDescription_.clear();
    notifyStateChanged();
}

void UndoRedoManager::setCommandMerging(bool enable) {
    allowMerging_ = enable;
}

bool UndoRedoManager::tryMergeCommand(const CommandPtr& command) {
    if (undoStack_.empty()) {
        return false;
    }

    CommandPtr lastCommand = undoStack_.back();
    if (lastCommand->canMergeWith(command.get())) {
        return lastCommand->mergeWith(command.get());
    }

    return false;
}

void UndoRedoManager::clearRedoStack() {
    redoStack_.clear();
}

void UndoRedoManager::notifyStateChanged() {
    // Create and publish the state change event
    UndoRedoStateChangedEvent event(
        canUndo(),
        canRedo(),
        getUndoDescription(),
        getRedoDescription()
    );
    EventBus::getInstance().publish(event);
}

} // namespace Core
} // namespace RebelCAD
