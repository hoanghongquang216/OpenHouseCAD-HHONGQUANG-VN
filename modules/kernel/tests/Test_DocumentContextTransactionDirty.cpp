#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/DocumentContext.hpp>

int main()
{
    openhouse::kernel::DocumentContext context;

    auto transaction = context.BeginTransaction();

    transaction.Record(openhouse::kernel::ChangeRecord(1));

    context.RecordChange(transaction);

    assert(context.IsModified());

    return 0;
}
