#pragma once

#include <vector>
#include <utility>

#include <openhouse/model/HistoryEntry.hpp>

namespace openhouse::model
{

class HistoryManager
{
public:
    void Push(HistoryEntry entry)
    {
        undoStack_.push_back(std::move(entry));
        redoStack_.clear();
    }

    bool CanUndo() const
    {
        return !undoStack_.empty();
    }

    bool CanRedo() const
    {
        return !redoStack_.empty();
    }

    HistoryEntry Undo()
    {
        HistoryEntry entry = std::move(undoStack_.back());
        undoStack_.pop_back();
        redoStack_.push_back(entry);
        return entry;
    }

    HistoryEntry Redo()
    {
        HistoryEntry entry = std::move(redoStack_.back());
        redoStack_.pop_back();
        undoStack_.push_back(entry);
        return entry;
    }

    void Clear()
    {
        undoStack_.clear();
        redoStack_.clear();
    }

private:
    std::vector<HistoryEntry> undoStack_;
    std::vector<HistoryEntry> redoStack_;
};

}
