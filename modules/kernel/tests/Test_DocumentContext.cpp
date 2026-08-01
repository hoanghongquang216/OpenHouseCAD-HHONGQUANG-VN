#include <cassert>

#include <openhouse/kernel/DocumentContext.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    context.Objects().Create<openhouse::kernel::Vertex>();

    assert(context.Objects().Size() == 1);

    context.Clear();

    assert(context.Objects().Size() == 0);

    return 0;
}
