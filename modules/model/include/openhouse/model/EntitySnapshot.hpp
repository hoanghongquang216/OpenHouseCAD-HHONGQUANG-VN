#pragma once

#include <openhouse/model/EntityId.hpp>
#include <openhouse/model/EntityType.hpp>

namespace openhouse::model
{

class EntitySnapshot
{
public:
    EntitySnapshot(EntityId id, EntityType type)
        : id_(id), type_(type)
    {
    }

    EntityId Id() const
    {
        return id_;
    }

    EntityType Type() const
    {
        return type_;
    }

private:
    EntityId id_;
    EntityType type_;
};

}
