#pragma once

#include <openhouse/model/EntityId.hpp>

namespace openhouse::model
{

enum class EntityChangeType
{
    Create
};

class EntityChangeRecord
{
public:
    EntityChangeRecord(EntityChangeType type, EntityId id)
        : type_(type), id_(id)
    {
    }

    EntityChangeType Type() const
    {
        return type_;
    }

    EntityId Id() const
    {
        return id_;
    }

private:
    EntityChangeType type_;
    EntityId id_;
};

}
