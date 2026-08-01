#include <cassert>

#include <openhouse/kernel/QueryResult.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::Vertex vertex(1);

    openhouse::kernel::QueryResult<openhouse::kernel::Vertex> result({&vertex});

    assert(result.Size() == 1);

    for (auto* item : result)
    {
        assert(item == &vertex);
    }

    return 0;
}
