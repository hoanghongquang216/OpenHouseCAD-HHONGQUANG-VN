#include <cassert>

#include <openhouse/kernel/QueryResult.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::Vertex vertex(1);

    const openhouse::kernel::QueryResult<openhouse::kernel::Vertex> result({&vertex});

    assert(result.Size() == 1);
    assert(!result.Empty());
    assert(result[0] == &vertex);

    for (auto* item : result)
    {
        assert(item == &vertex);
    }

    return 0;
}
