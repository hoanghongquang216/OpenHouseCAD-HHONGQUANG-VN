#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Vertex : public TopologyEntity
{
public:
    explicit Vertex(ObjectId id)
        : TopologyEntity(id)
    {
    }
};

}
