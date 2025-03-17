#pragma once

#include "core/EventSystem.h"
#include <string>
#include <unordered_map>

namespace RebelCAD {
namespace Core {

/**
 * @brief Event class for undo/redo state changes
 */
class UndoRedoStateChangedEvent : public Event {
public:
    UndoRedoStateChangedEvent(bool canUndo, bool canRedo, 
                             const std::string& undoDesc, 
                             const std::string& redoDesc)
        : Event("UndoRedoStateChanged")
        , canUndo_(canUndo)
        , canRedo_(canRedo)
        , undoDescription_(undoDesc)
        , redoDescription_(redoDesc) {}

    bool canUndo() const { return canUndo_; }
    bool canRedo() const { return canRedo_; }
    const std::string& getUndoDescription() const { return undoDescription_; }
    const std::string& getRedoDescription() const { return redoDescription_; }

private:
    bool canUndo_;
    bool canRedo_;
    std::string undoDescription_;
    std::string redoDescription_;
};

} // namespace Core
} // namespace RebelCAD
