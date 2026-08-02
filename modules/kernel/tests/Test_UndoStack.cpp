#include <cassert>

#include <openhouse/kernel/UndoStack.hpp>

int main()
{
    openhouse::kernel::UndoStack stack;

    assert(stack.Count() == 0);

    return 0;
}
