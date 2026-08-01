#include <cassert>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/ObjectStorePredicate.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    store.Create<openhouse::kernel::Vertex>();
    store.Create<openhouse::kernel::Vertex>();

    auto result = openhouse::kernel::FindIf<openhouse::kernel::Vertex>(store,
        [](const auto&)
        {
            return true;
        });

    assert(result.Size() == 2);

    for (auto* vertex : result)
    {
        assert(vertex != nullptr);
    }

    return 0;
}
