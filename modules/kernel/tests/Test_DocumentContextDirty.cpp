#include <cassert>

#include <openhouse/kernel/DocumentContext.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    assert(!context.IsModified());

    context.MarkModified();
    assert(context.IsModified());

    context.MarkSaved();
    assert(!context.IsModified());

    return 0;
}
