#include <cassert>

#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    assert(!transaction.IsCommitted());

    transaction.Commit();
    assert(transaction.IsCommitted());

    transaction.Rollback();
    assert(!transaction.IsCommitted());

    return 0;
}
