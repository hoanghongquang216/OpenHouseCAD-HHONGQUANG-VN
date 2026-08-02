#pragma once

#include <string>
#include <vector>
#include <utility>

#include <openhouse/model/TransactionChange.hpp>

namespace openhouse::model
{

class HistoryEntry
{
public:
    explicit HistoryEntry(std::string description)
        : description_(std::move(description))
    {
    }

    void AddChange(TransactionChange change)
    {
        changes_.push_back(std::move(change));
    }

    const std::string& Description() const
    {
        return description_;
    }

    const std::vector<TransactionChange>& Changes() const
    {
        return changes_;
    }

private:
    std::string description_;
    std::vector<TransactionChange> changes_;
};

}
