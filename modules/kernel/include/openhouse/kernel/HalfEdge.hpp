#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class HalfEdge : public TopologyEntity
{
public:
    explicit HalfEdge(ObjectId id)
        : TopologyEntity(id)
    {
    }
};

}
