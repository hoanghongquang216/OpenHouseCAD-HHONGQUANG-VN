#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    auto* first = store.Create<openhouse::kernel::Vertex>();
    auto* second = store.Create<openhouse::kernel::Vertex>();

    assert(first != nullptr);
    assert(second != nullptr);
    assert(first->Id() != second->Id());
    assert(store.Size() == 2);

    return 0;
}
