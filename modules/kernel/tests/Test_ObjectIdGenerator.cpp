#include <cassert>

#include <openhouse/kernel/ObjectIdGenerator.hpp>

int main()
{
    openhouse::kernel::ObjectIdGenerator generator;

    auto first = generator.Next();
    auto second = generator.Next();

    assert(first.Value() == 1);
    assert(second.Value() == 2);
    assert(first != second);

    return 0;
}
