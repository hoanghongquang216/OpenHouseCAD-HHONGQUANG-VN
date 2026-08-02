#include <cassert>

#include <openhouse/kernel/UndoableCommand.hpp>

int main()
{
    openhouse::kernel::UndoableCommand command;

    command.Undo();

    assert(true);

    return 0;
}
