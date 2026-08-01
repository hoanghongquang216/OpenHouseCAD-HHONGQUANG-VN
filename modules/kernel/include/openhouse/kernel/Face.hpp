#pragma once

#include <vector>

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Loop;

class Face : public TopologyEntity
{
public:
    explicit Face(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void AddLoop(Loop* loop)
    {
        loops_.push_back(loop);
    }

    const std::vector<Loop*>& Loops() const
    {
        return loops_;
    }

private:
    std::vector<Loop*> loops_;
};

}
