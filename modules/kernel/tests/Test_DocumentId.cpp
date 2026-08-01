#include <cassert>

#include <openhouse/kernel/DocumentId.hpp>

int main()
{
    openhouse::kernel::DocumentId first(1);
    openhouse::kernel::DocumentId second(1);
    openhouse::kernel::DocumentId different(2);

    assert(first == second);
    assert(first != different);
    assert(first.Value() == 1);

    return 0;
}
