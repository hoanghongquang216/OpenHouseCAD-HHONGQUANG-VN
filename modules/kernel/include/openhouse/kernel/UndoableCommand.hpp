#pragma once

namespace openhouse::kernel
{

class UndoableCommand
{
public:
    virtual ~UndoableCommand() = default;

    virtual void Undo()
    {
    }
};

}
