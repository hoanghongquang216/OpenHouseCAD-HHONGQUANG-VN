#pragma once

#include <vector>

#include <openhouse/kernel/UndoableCommand.hpp>

namespace openhouse::kernel
{

class UndoStack
{
public:
    void Push(UndoableCommand command)
    {
        commands_.push_back(command);
    }

    std::size_t Count() const
    {
        return commands_.size();
    }

private:
    std::vector<UndoableCommand> commands_;
};

}
