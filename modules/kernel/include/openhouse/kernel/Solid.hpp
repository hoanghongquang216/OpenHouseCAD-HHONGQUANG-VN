#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{
class Solid : public Entity
{
public:
    explicit Solid(ObjectId id) : Entity(id) {}
};
}
