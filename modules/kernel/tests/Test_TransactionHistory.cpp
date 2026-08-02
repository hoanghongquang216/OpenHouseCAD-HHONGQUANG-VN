#include <cassert>

#include <openhouse/kernel/TransactionHistory.hpp>

int main()
{
    openhouse::kernel::TransactionHistory history;

    history.Add(openhouse::kernel::TransactionHistoryEntry(1));

    assert(history.Count() == 1);

    return 0;
}
