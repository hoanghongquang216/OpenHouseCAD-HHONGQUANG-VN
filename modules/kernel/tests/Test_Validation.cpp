#include <cassert>

#include <openhouse/kernel/Validation.hpp>
#include <openhouse/kernel/Vertex.hpp>

int main()
{
    openhouse::kernel::Vertex vertex(1);

    assert(openhouse::kernel::Validation::IsValid(&vertex));
    assert(!openhouse::kernel::Validation::IsValid(nullptr));

    return 0;
}
