#pragma once

#include <cstddef>

namespace openhouse::kernel
{

class TransactionResult
{
public:
    explicit TransactionResult(bool success = false, std::size_t changes = 0)
        : success_(success), changeCount_(changes)
    {
    }

    bool Success() const
    {
        return success_;
    }

    std::size_t ChangeCount() const
    {
        return changeCount_;
    }

private:
    bool success_;
    std::size_t changeCount_;
};

}
