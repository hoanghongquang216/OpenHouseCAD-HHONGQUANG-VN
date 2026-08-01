#include <cassert>
#include <stdexcept>

#include <openhouse/kernel/ObjectStore.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::ObjectStore store;

    store.Create<openhouse::kernel::Vertex>(1);

    bool failed = false;

    try
    {
        store.Create<openhouse::kernel::Vertex>(1);
    }
    catch (const std::runtime_error&)
    {
        failed = true;
    }

    assert(failed);

    return 0;
}
