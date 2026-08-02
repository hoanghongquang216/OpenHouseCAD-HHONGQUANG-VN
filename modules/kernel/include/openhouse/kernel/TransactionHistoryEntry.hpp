#pragma once

#include <cstddef>

namespace openhouse::kernel
{

class TransactionHistoryEntry
{
public:
    explicit TransactionHistoryEntry(std::size_t changeCount = 0)
        : changeCount_(changeCount)
    {
    }

    std::size_t ChangeCount() const
    {
        return changeCount_;
    }

private:
    std::size_t changeCount_;
};

}
