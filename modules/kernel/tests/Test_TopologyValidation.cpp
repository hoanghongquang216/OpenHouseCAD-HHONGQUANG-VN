#include <cassert>

#include <openhouse/kernel/Edge.hpp>
#include <openhouse/kernel/HalfEdge.hpp>
#include <openhouse/kernel/Loop.hpp>
#include <openhouse/kernel/Validation.hpp>
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

    assert(openhouse::kernel::Validation::IsValid(&vertex));
    assert(halfEdge.VertexRef() == &vertex);
    assert(halfEdge.EdgeRef() == &edge);
    assert(halfEdge.LoopRef() == &loop);

    return 0;
}
