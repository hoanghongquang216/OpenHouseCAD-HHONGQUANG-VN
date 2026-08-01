#include <cassert>

#include <openhouse/kernel/Handle.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::Vertex vertex(1);

    openhouse::kernel::Handle<openhouse::kernel::Vertex> handle(&vertex);

    assert(handle.IsValid());
    assert(static_cast<bool>(handle));
    assert(handle.Get() == &vertex);
    assert(handle->Id().Value() == 1);
    assert((*handle).Id().Value() == 1);

    handle.Reset();

    assert(!handle.IsValid());

    return 0;
}
