#pragma once

#include <utility>

#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityStateSnapshot.hpp>

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
    TransactionChange(
        ChangeOperation operation,
        EntityId id,
        EntityStateSnapshot before,
        EntityStateSnapshot after)
        : operation_(operation),
          entityId_(id),
          before_(std::move(before)),
          after_(std::move(after))
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

    const EntityStateSnapshot& Before() const
    {
        return before_;
    }

    const EntityStateSnapshot& After() const
    {
        return after_;
    }

private:
    ChangeOperation operation_;
    EntityId entityId_;
    EntityStateSnapshot before_;
    EntityStateSnapshot after_;
};

}
