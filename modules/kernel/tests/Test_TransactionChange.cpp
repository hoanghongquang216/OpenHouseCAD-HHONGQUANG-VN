#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    transaction.Record(openhouse::kernel::ChangeRecord(10));

    assert(transaction.ChangeCount() == 1);

    transaction.Rollback();

    assert(transaction.ChangeCount() == 0);

    return 0;
}
