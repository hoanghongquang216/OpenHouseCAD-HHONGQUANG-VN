#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class HalfEdge;

class Edge : public TopologyEntity
{
public:
    explicit Edge(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void SetForward(HalfEdge* edge)
    {
        forward_ = edge;
    }

    void SetBackward(HalfEdge* edge)
    {
        backward_ = edge;
    }

    HalfEdge* Forward() const
    {
        return forward_;
    }

    HalfEdge* Backward() const
    {
        return backward_;
    }

private:
    HalfEdge* forward_ = nullptr;
    HalfEdge* backward_ = nullptr;
};

}
