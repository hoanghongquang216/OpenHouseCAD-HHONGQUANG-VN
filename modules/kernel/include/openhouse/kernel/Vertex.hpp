#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{
class Vertex : public Entity
{
public:
    explicit Vertex(ObjectId id) : Entity(id) {}
};
}
