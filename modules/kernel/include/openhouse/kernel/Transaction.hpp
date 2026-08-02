#pragma once

#include <vector>

#include <openhouse/kernel/ChangeRecord.hpp>
#include <openhouse/kernel/TransactionResult.hpp>
#include <openhouse/kernel/TransactionValidator.hpp>

namespace openhouse::kernel
{

class Transaction
{
public:
    enum class State
    {
        Active,
        Committed,
        RolledBack
    };

    void Record(ChangeRecord change)
    {
        if (state_ == State::Active)
        {
            changes_.push_back(change);
        }
    }

    std::size_t ChangeCount() const
    {
        return changes_.size();
    }

    TransactionResult Commit()
    {
        if (!validator_.CanCommit())
        {
            return TransactionResult(false, changes_.size());
        }

        state_ = State::Committed;
        return TransactionResult(true, changes_.size());
    }

    TransactionResult Rollback()
    {
        state_ = State::RolledBack;
        const auto changes = changes_.size();
        changes_.clear();
        return TransactionResult(true, changes);
    }

    bool IsCommitted() const
    {
        return state_ == State::Committed;
    }

    bool IsActive() const
    {
        return state_ == State::Active;
    }

private:
    State state_{State::Active};
    std::vector<ChangeRecord> changes_;
    TransactionValidator validator_;
};

}
