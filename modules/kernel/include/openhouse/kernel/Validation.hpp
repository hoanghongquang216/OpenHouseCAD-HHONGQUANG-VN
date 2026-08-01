#pragma once

namespace openhouse::kernel
{

class Entity;

class Validation
{
public:
    static bool IsValid(const Entity* entity)
    {
        return entity != nullptr;
    }
};

}
