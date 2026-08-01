#include <cassert>

#include <openhouse/kernel/DocumentContext.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    auto transaction = context.BeginTransaction();

    assert(!transaction.Current().IsCommitted());

    transaction.Commit();

    assert(transaction.Current().IsCommitted());

    return 0;
}
