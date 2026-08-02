#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/Transaction.hpp>
#include <openhouse/kernel/TransactionHistory.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;
    openhouse::kernel::TransactionHistory history;

    transaction.Record(openhouse::kernel::ChangeRecord(1));
    auto result = transaction.Commit(&history);

    assert(result.Success());
    assert(history.Count() == 1);

    return 0;
}
