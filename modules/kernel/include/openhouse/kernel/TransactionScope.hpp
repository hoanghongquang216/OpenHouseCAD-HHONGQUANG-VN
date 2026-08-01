#pragma once

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
