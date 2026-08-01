#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Loop : public TopologyEntity
{
public:
    explicit Loop(ObjectId id)
        : TopologyEntity(id)
    {
    }
};

}
