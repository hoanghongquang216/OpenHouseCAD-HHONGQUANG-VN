#include <cassert>

#include <openhouse/kernel/TransactionScope.hpp>

int main()
{
    openhouse::kernel::TransactionScope scope;

    assert(!scope.Current().IsCommitted());

    scope.Commit();
    assert(scope.Current().IsCommitted());

    scope.Rollback();
    assert(!scope.Current().IsCommitted());

    return 0;
}
