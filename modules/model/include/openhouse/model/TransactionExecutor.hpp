#pragma once

#include <openhouse/model/ModelStore.hpp>
#include <openhouse/model/TransactionChange.hpp>

namespace openhouse::model
{

class TransactionExecutor
{
public:
    explicit TransactionExecutor(ModelStore& store)
        : store_(store)
    {
    }

    bool ApplyUndo(const TransactionChange& change)
    {
        return store_.Restore(change.Before());
    }

    bool ApplyRedo(const TransactionChange& change)
    {
        return store_.Restore(change.After());
    }

private:
    ModelStore& store_;
};

}
