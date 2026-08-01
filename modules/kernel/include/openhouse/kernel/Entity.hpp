#pragma once

#include <openhouse/kernel/ObjectId.hpp>

namespace openhouse::kernel
{

class Entity
{
public:
    explicit Entity(ObjectId id)
        : id_(id)
    {
    }

    virtual ~Entity() = default;

    ObjectId Id() const
    {
        return id_;
    }

private:
    ObjectId id_;
};

}
