#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    store.Create<openhouse::kernel::Vertex>();
    store.Create<openhouse::kernel::Vertex>();

    std::size_t count = 0;

    for (const auto& item : store)
    {
        assert(item.second != nullptr);
        ++count;
    }

    assert(count == 2);

    return 0;
}
