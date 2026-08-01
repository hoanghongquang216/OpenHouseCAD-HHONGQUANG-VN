#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Edge : public TopologyEntity
{
public:
    explicit Edge(ObjectId id)
        : TopologyEntity(id)
    {
    }
};

}
