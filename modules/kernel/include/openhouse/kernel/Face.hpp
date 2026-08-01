#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Face : public TopologyEntity
{
public:
    explicit Face(ObjectId id)
        : TopologyEntity(id)
    {
    }
};

}
