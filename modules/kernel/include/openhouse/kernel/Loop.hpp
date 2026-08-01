#pragma once

#include <vector>

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class HalfEdge;

class Loop : public TopologyEntity
{
public:
    explicit Loop(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void AddHalfEdge(HalfEdge* edge)
    {
        half_edges_.push_back(edge);
    }

    const std::vector<HalfEdge*>& HalfEdges() const
    {
        return half_edges_;
    }

private:
    std::vector<HalfEdge*> half_edges_;
};

}
