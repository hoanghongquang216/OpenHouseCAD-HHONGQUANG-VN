#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/DocumentContext.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    auto transaction = context.BeginTransaction();
    transaction.Record(openhouse::kernel::ChangeRecord(1));

    auto result = context.CommitTransaction(transaction);

    assert(result.Success());
    assert(result.ChangeCount() == 1);
    assert(context.IsModified());

    return 0;
}
