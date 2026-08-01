#pragma once

#include <atomic>

#include <openhouse/kernel/ObjectId.hpp>

namespace openhouse::kernel
{

class ObjectIdGenerator
{
public:
    ObjectIdGenerator() = default;

    ObjectId Next()
    {
        return ObjectId(next_.fetch_add(1));
    }

private:
    std::atomic<ObjectId::ValueType> next_{1};
};

}
