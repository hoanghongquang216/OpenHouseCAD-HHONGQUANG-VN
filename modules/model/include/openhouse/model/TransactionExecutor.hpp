#pragma once

#include <openhouse/model/TransactionChange.hpp>

namespace openhouse::model
{

class TransactionExecutor
{
public:
    void ApplyUndo(const TransactionChange& change)
    {
        // ModelStore integration will apply the Before snapshot.
        (void)change;
    }

    void ApplyRedo(const TransactionChange& change)
    {
        // ModelStore integration will apply the After snapshot.
        (void)change;
    }
};

}
