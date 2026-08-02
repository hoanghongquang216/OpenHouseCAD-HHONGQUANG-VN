#pragma once

#include <vector>

#include <openhouse/model/TransactionChange.hpp>

namespace openhouse::model
{

class TransactionChangeSet
{
public:
    void Add(TransactionChange change)
    {
        changes_.push_back(std::move(change));
    }

    std::size_t Count() const
    {
        return changes_.size();
    }

    const TransactionChange& At(std::size_t index) const
    {
        return changes_.at(index);
    }

private:
    std::vector<TransactionChange> changes_;
};

}
