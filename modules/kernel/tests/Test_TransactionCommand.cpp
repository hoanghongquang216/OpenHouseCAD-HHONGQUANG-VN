#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    transaction.Record(openhouse::kernel::ChangeRecord(1));

    assert(transaction.CommandCount() == 1);

    return 0;
}
