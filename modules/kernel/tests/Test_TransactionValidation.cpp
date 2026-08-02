#include <cassert>

#include <openhouse/kernel/Transaction.hpp>

int main()
{
    openhouse::kernel::Transaction transaction;

    auto result = transaction.Commit();

    assert(result.Success());

    return 0;
}
