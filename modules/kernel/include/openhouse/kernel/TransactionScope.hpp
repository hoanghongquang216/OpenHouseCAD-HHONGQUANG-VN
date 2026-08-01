#pragma once

#include <cstddef>

#include <openhouse/kernel/Transaction.hpp>

namespace openhouse::kernel
{

class TransactionScope
{
public:
    TransactionScope()
        : transaction_()
    {
    }

    Transaction& Current()
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
