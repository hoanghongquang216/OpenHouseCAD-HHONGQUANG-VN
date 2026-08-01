#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    auto* vertex = store.Create<openhouse::kernel::Vertex>();
    auto id = vertex->Id();

    assert(store.Contains(id));
    assert(store.Size() == 1);

    assert(store.Remove(id));
    assert(!store.Contains(id));
    assert(store.Size() == 0);

    store.Create<openhouse::kernel::Vertex>();
    store.Clear();

    assert(store.Size() == 0);

    return 0;
}
