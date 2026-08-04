#pragma once

#include <openhouse/document/Command.hpp>

#include <memory>
#include <vector>

namespace openhouse::document {

// Manages the undo/redo stacks for a Document. This is the single entry
// point Spiral 5 expects callers (UI, later Spirals) to go through --
// callers should not call ICommand::Execute() directly and manage their
// own history; CommandHistory::Execute() is the one place that both
// performs a command AND records it correctly.
//
// Implemented with std::vector (used as a stack via push_back/pop_back)
// rather than std::stack -- functionally equivalent for the LIFO access
// this class needs, but a vector also allows read-only inspection (e.g.
// UndoCount()/RedoCount() for a UI showing "12 actions to undo") without
// the awkwardness of std::stack's minimal interface.
class CommandHistory {
public:
    // Executes `command` against `doc`. If it succeeds, ownership moves
    // into the undo stack and the redo stack is cleared (standard undo/
    // redo discipline: performing a new action after an Undo discards
    // whatever "future" that Undo had made available to Redo -- the
    // classic example being Undo, then do something different, then the
    // old Redo no longer makes sense to offer). If `command` fails
    // (returns false from its own Execute()), it is destroyed
    // immediately and NOT pushed onto history -- there is nothing
    // meaningful to undo for an operation that never changed anything.
    bool Execute(Document& doc, std::unique_ptr<ICommand> command) {
        if (!command->Execute(doc)) {
            return false;
        }
        undoStack_.push_back(std::move(command));
        redoStack_.clear();
        return true;
    }

    // Undoes the most recent command, moving it onto the redo stack.
    // Returns false (a no-op) if there is nothing to undo.
    bool Undo(Document& doc) {
        if (undoStack_.empty()) {
            return false;
        }
        undoStack_.back()->Undo(doc);
        redoStack_.push_back(std::move(undoStack_.back()));
        undoStack_.pop_back();
        return true;
    }

    // Re-applies the most recently undone command, moving it back onto
    // the undo stack. Returns false (a no-op) if there is nothing to
    // redo.
    bool Redo(Document& doc) {
        if (redoStack_.empty()) {
            return false;
        }
        redoStack_.back()->Redo(doc);
        undoStack_.push_back(std::move(redoStack_.back()));
        redoStack_.pop_back();
        return true;
    }

    [[nodiscard]] bool CanUndo() const noexcept { return !undoStack_.empty(); }
    [[nodiscard]] bool CanRedo() const noexcept { return !redoStack_.empty(); }
    [[nodiscard]] std::size_t UndoCount() const noexcept { return undoStack_.size(); }
    [[nodiscard]] std::size_t RedoCount() const noexcept { return redoStack_.size(); }

    // Discards all history without touching the Document itself (the
    // entities stay exactly as they currently are -- this only forgets
    // how to undo/redo further, it does not revert anything).
    void Clear() noexcept {
        undoStack_.clear();
        redoStack_.clear();
    }

private:
    std::vector<std::unique_ptr<ICommand>> undoStack_;
    std::vector<std::unique_ptr<ICommand>> redoStack_;
};

}
