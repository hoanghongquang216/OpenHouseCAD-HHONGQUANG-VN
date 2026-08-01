#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    auto* vertex = store.Create<openhouse::kernel::Vertex>(1);

    assert(vertex != nullptr);
    assert(store.Contains(1));
    assert(store.Size() == 1);
    assert(store.Find(1) == vertex);

    return 0;
}
