#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{
class Edge : public Entity
{
public:
    explicit Edge(ObjectId id) : Entity(id) {}
};
}
