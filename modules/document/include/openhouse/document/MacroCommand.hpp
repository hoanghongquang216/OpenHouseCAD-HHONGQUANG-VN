#pragma once

#include <openhouse/document/Command.hpp>

#include <memory>
#include <vector>

namespace openhouse::document {

// A composite of ICommands executed/undone/redone as a single, atomic
// unit -- the Command-pattern counterpart to Spiral 4's
// TranslateSelection/RotateSelection/ScaleSelection: applying an
// operation to a multi-entity SelectionSet should feel to the user like
// ONE undoable action ("I moved 5 objects"), not 5 separate undo steps.
//
// MacroCommand implements ICommand's exact interface (bool Execute,
// void Undo, void Redo) so it can sit interchangeably with any single
// command in a CommandHistory -- but Execute()'s bool return is
// necessarily coarser than the per-entity success counts Spiral 4
// exposed (std::size_t): here it only means "at least one sub-command
// actually changed something, so this macro is worth keeping in
// history". The precise partial-success count (e.g. "3 of 5 succeeded,
// 2 were locked") is available via SuccessCount()/TotalCount() AFTER
// Execute() returns, for a caller (UI) that wants to report it --
// deliberately NOT folded into Execute()'s own return type, since
// ICommand's bool contract needs to stay uniform across every command
// type, single or composite.
class MacroCommand final : public ICommand {
public:
    // Takes ownership. Sub-commands are executed/undone in the order
    // added; Undo() runs them in REVERSE order (standard undo-stack
    // discipline: the last thing done is the first thing undone),
    // matching how the individual sub-commands might depend on prior
    // ones having already run (not currently true for Translate/Rotate/
    // Scale, which are independent per-entity, but this is the correct
    // general rule for a composite command and costs nothing to follow
    // now).
    void Add(std::unique_ptr<ICommand> command) { commands_.push_back(std::move(command)); }

    [[nodiscard]] bool Execute(Document& doc) override {
        executed_.assign(commands_.size(), false);
        std::size_t successCount = 0;
        for (std::size_t i = 0; i < commands_.size(); ++i) {
            if (commands_[i]->Execute(doc)) {
                executed_[i] = true;
                ++successCount;
            }
        }
        successCount_ = successCount;
        return successCount > 0;
    }

    void Undo(Document& doc) override {
        // Reverse order, and only for sub-commands that actually
        // succeeded -- a sub-command whose Execute() returned false
        // never changed anything, so calling its Undo() would be
        // meaningless (and, since it never captured an "after" snapshot
        // distinct from "before", could not correctly restore anything
        // regardless).
        for (std::size_t i = commands_.size(); i-- > 0;) {
            if (executed_[i]) {
                commands_[i]->Undo(doc);
            }
        }
    }

    void Redo(Document& doc) override {
        for (std::size_t i = 0; i < commands_.size(); ++i) {
            if (executed_[i]) {
                commands_[i]->Redo(doc);
            }
        }
    }

    // Valid only after Execute() has been called at least once.
    [[nodiscard]] std::size_t SuccessCount() const noexcept { return successCount_; }
    [[nodiscard]] std::size_t TotalCount() const noexcept { return commands_.size(); }
    [[nodiscard]] bool Empty() const noexcept { return commands_.empty(); }

private:
    std::vector<std::unique_ptr<ICommand>> commands_;
    std::vector<bool> executed_;
    std::size_t successCount_ = 0;
};

}
