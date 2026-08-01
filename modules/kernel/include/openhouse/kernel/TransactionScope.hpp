#pragma once

#include <cstddef>

#include <openhouse/kernel/Transaction.hpp>

namespace openhouse::kernel
{

class TransactionScope
{
public:
    TransactionScope() = default;

    TransactionScope(const TransactionScope&) = delete;
    TransactionScope& operator=(const TransactionScope&) = delete;

    TransactionScope(TransactionScope&&) = default;
    TransactionScope& operator=(TransactionScope&&) = default;

    ~TransactionScope()
    {
        if (transaction_.IsActive())
        {
            transaction_.Rollback();
        }
    }

    Transaction& Current()
    {
        return transaction_;
    }

    const Transaction& Current() const
    {
        return transaction_;
    }

    void Record(ChangeRecord change)
    {
        transaction_.Record(change);
    }

    std::size_t ChangeCount() const
    {
        return transaction_.ChangeCount();
    }

    void Commit()
    {
        transaction_.Commit();
    }

    void Rollback()
    {
        transaction_.Rollback();
    }

private:
    Transaction transaction_;
};

}
