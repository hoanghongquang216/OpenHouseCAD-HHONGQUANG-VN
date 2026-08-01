#include <cassert>

#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    assert(transaction.IsActive());

    transaction.Commit();
    assert(transaction.IsCommitted());
    assert(!transaction.IsActive());

    return 0;
}
