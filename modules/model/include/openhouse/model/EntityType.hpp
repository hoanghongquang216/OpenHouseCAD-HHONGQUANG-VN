#pragma once

#include <cstdint>

namespace openhouse::model
{

using EntityTypeId = std::uint32_t;

class EntityType
{
public:
    explicit EntityType(EntityTypeId id = 0)
        : id_(id)
    {
    }

    EntityTypeId Id() const
    {
        return id_;
    }

private:
    EntityTypeId id_;
};

}
