#include <cassert>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/TransactionScope.hpp>

int main()
{
    openhouse::kernel::TransactionScope scope;

    scope.Record(openhouse::kernel::ChangeRecord(5));

    assert(scope.ChangeCount() == 1);

    scope.Rollback();

    assert(scope.ChangeCount() == 0);

    return 0;
}
