#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    transaction.Record(openhouse::kernel::ChangeRecord(1));

    auto result = transaction.Commit();

    assert(result.Success());
    assert(result.ChangeCount() == 1);

    return 0;
}
