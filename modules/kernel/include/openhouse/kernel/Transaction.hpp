#pragma once

#include <vector>

#include <openhouse/kernel/ChangeRecord.hpp>

namespace openhouse::kernel
{

class Transaction
{
public:
    void Record(ChangeRecord change)
    {
        changes_.push_back(change);
    }

    std::size_t ChangeCount() const
    {
        return changes_.size();
    }

    void Commit()
    {
        committed_ = true;
    }

    void Rollback()
    {
        committed_ = false;
        changes_.clear();
    }

    bool IsCommitted() const
    {
        return committed_;
    }

private:
    bool committed_{false};
    std::vector<ChangeRecord> changes_;
};

}
