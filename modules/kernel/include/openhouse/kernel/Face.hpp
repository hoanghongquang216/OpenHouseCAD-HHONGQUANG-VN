#pragma once

#include <openhouse/kernel/Entity.hpp>

namespace openhouse::kernel
{
class Face : public Entity
{
public:
    explicit Face(ObjectId id) : Entity(id) {}
};
}
