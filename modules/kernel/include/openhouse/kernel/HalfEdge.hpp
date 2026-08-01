#pragma once

#include <openhouse/kernel/TopologyEntity.hpp>

namespace openhouse::kernel
{

class Vertex;
class Edge;
class Loop;

class HalfEdge : public TopologyEntity
{
public:
    explicit HalfEdge(ObjectId id)
        : TopologyEntity(id)
    {
    }

    void SetVertex(Vertex* vertex)
    {
        vertex_ = vertex;
    }

    void SetEdge(Edge* edge)
    {
        edge_ = edge;
    }

    void SetLoop(Loop* loop)
    {
        loop_ = loop;
    }

    Vertex* VertexRef() const
    {
        return vertex_;
    }

    Edge* EdgeRef() const
    {
        return edge_;
    }

    Loop* LoopRef() const
    {
        return loop_;
    }

private:
    Vertex* vertex_ = nullptr;
    Edge* edge_ = nullptr;
    Loop* loop_ = nullptr;
};

}
