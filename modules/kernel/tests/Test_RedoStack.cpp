#include <cassert>

#include <openhouse/kernel/RedoStack.hpp>

int main()
{
    openhouse::kernel::RedoStack stack;

    assert(stack.Count() == 0);

    return 0;
}
