#include <cassert>

#include <openhouse/kernel/Edge.hpp>
#include <openhouse/kernel/HalfEdge.hpp>
#include <openhouse/kernel/Loop.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::Vertex vertex(1);
    openhouse::kernel::Edge edge(2);
    openhouse::kernel::Loop loop(3);
    openhouse::kernel::HalfEdge halfEdge(4);

    halfEdge.SetVertex(&vertex);
    halfEdge.SetEdge(&edge);
    halfEdge.SetLoop(&loop);

    edge.SetForward(&halfEdge);
    loop.AddHalfEdge(&halfEdge);

    assert(halfEdge.VertexRef() == &vertex);
    assert(halfEdge.EdgeRef() == &edge);
    assert(halfEdge.LoopRef() == &loop);
    assert(edge.Forward() == &halfEdge);
    assert(loop.HalfEdges().size() == 1);

    return 0;
}
