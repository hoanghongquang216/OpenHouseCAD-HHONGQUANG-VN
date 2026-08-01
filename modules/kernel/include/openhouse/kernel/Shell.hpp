#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{
class Shell : public Entity
{
public:
    explicit Shell(ObjectId id) : Entity(id) {}
};
}
