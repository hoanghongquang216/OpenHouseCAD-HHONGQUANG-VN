#pragma once

#include <vector>

#include <openhouse/kernel/TransactionHistoryEntry.hpp>

namespace openhouse::kernel
{

class TransactionHistory
{
public:
    void Add(TransactionHistoryEntry entry)
    {
        entries_.push_back(entry);
    }

    std::size_t Count() const
    {
        return entries_.size();
    }

private:
    std::vector<TransactionHistoryEntry> entries_;
};

}
