#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;
    store.Create<openhouse::kernel::Vertex>();

    const auto& constStore = store;

    assert(constStore.Size() == 1);

    return 0;
}
