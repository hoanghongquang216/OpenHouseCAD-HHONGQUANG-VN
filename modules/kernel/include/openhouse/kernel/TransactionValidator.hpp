#pragma once

namespace openhouse::kernel
{

class TransactionValidator
{
public:
    bool CanCommit() const
    {
        return true;
    }
};

}
