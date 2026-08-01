#include <cassert>

#include <openhouse/kernel/ObjectId.hpp>

int main()
{
    openhouse::kernel::ObjectId first(100);
    openhouse::kernel::ObjectId second(100);
    openhouse::kernel::ObjectId different(101);

    assert(first == second);
    assert(first != different);
    assert(first.Value() == 100);

    return 0;
}
