#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/ObjectStoreFilter.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    store.Create<openhouse::kernel::Vertex>();
    store.Create<openhouse::kernel::Vertex>();

    auto vertices = openhouse::kernel::FindAll<openhouse::kernel::Vertex>(store);

    assert(vertices.size() == 2);

    return 0;
}
