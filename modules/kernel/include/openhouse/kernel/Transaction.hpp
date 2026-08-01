#pragma once

#include <vector>

#include <openhouse/kernel/ChangeRecord.hpp>

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

    void Commit()
    {
        state_ = State::Committed;
    }

    void Rollback()
    {
        state_ = State::RolledBack;
        changes_.clear();
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
};

}
