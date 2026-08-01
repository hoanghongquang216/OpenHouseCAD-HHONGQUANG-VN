#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{

class TopologyEntity : public Entity
{
public:
    explicit TopologyEntity(ObjectId id)
        : Entity(id)
    {
    }

    virtual ~TopologyEntity() = default;
};

}
