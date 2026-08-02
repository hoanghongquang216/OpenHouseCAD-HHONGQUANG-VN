#pragma once

#include <openhouse/model/TransactionChangeSet.hpp>

namespace openhouse::model
{

class ModelTransaction
{
public:
    void Begin()
    {
        active_ = true;
    }

    void Record(TransactionChange change)
    {
        if (active_)
        {
            changes_.Add(std::move(change));
        }
    }

    void Commit()
    {
        active_ = false;
    }

    void Rollback()
    {
        changes_ = TransactionChangeSet{};
        active_ = false;
    }

    bool IsActive() const
    {
        return active_;
    }

    const TransactionChangeSet& Changes() const
    {
        return changes_;
    }

private:
    bool active_{false};
    TransactionChangeSet changes_;
};

}
