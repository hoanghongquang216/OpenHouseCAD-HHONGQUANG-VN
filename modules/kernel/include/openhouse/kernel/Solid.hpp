#pragma once

#include <vector>

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Shell;

class Solid : public TopologyEntity
{
public:
    explicit Solid(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void AddShell(Shell* shell)
    {
        shells_.push_back(shell);
    }

    const std::vector<Shell*>& Shells() const
    {
        return shells_;
    }

private:
    std::vector<Shell*> shells_;
};

}
