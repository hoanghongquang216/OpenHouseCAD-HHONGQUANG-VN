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

    auto handle = store.Get<openhouse::kernel::Vertex>(1);

    assert(handle.IsValid());
    assert(handle.Get() == vertex);
    assert(handle->Id().Value() == 1);

    auto invalid = store.Get<openhouse::kernel::Vertex>(999);
    assert(!invalid.IsValid());

    return 0;
}
