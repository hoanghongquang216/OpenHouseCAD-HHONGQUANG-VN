#pragma once

#include <cstdint>

namespace openhouse::model
{

using EntityIdValue = std::uint64_t;

class EntityId
{
public:
    explicit EntityId(EntityIdValue value = 0)
        : value_(value)
    {
    }

    EntityIdValue Value() const
    {
        return value_;
    }

    bool IsValid() const
    {
        return value_ != 0;
    }

private:
    EntityIdValue value_;
};

}
