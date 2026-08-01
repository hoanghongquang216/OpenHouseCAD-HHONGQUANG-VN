#include <cassert>

#include <openhouse/kernel/DocumentContext.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::DocumentContext context(openhouse::kernel::DocumentId(42));

    assert(context.Id().Value() == 42);

    context.Objects().Create<openhouse::kernel::Vertex>();

    assert(context.Objects().Size() == 1);

    return 0;
}
