#include <cassert>

#include <openhouse/kernel/TransactionScope.hpp>

int main()
{
    {
        openhouse::kernel::TransactionScope scope;

        assert(scope.Current().IsActive());
    }

    return 0;
}
