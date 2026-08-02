#pragma once

#include <openhouse/model/TransactionChangeSet.hpp>

namespace openhouse::model
{

class TransactionHistoryEntry
{
public:
    explicit TransactionHistoryEntry(TransactionChangeSet changes)
        : changes_(std::move(changes))
    {
    }

    const TransactionChangeSet& Changes() const
    {
        return changes_;
    }

private:
    TransactionChangeSet changes_;
};

}
