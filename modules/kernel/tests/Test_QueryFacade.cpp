#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Query.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;
    store.Create<openhouse::kernel::Vertex>();

    std::size_t count = 0;

    openhouse::kernel::Query::ForEach(store, [&](auto*)
    {
        ++count;
    });

    assert(count == 1);

    return 0;
}
