#pragma once

#include <cstdint>

#include <openhouse/model/EntityId.hpp>

namespace openhouse::model
{

enum class ChangeOperation
{
    Create,
    Modify,
    Remove
};

class TransactionChange
{
public:
    TransactionChange(ChangeOperation operation, EntityId id)
        : operation_(operation), entityId_(id)
    {
    }

    ChangeOperation Operation() const
    {
        return operation_;
    }

    EntityId Id() const
    {
        return entityId_;
    }

private:
    ChangeOperation operation_;
    EntityId entityId_;
};

}
