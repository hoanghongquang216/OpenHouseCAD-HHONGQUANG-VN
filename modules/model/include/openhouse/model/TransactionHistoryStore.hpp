#pragma once

#include <vector>

#include <openhouse/model/TransactionHistoryEntry.hpp>

namespace openhouse::model
{

class TransactionHistoryStore
{
public:
    void Add(TransactionHistoryEntry entry)
    {
        entries_.push_back(std::move(entry));
    }

    std::size_t Count() const
    {
        return entries_.size();
    }

    const TransactionHistoryEntry& At(std::size_t index) const
    {
        return entries_.at(index);
    }

private:
    std::vector<TransactionHistoryEntry> entries_;
};

}
