#include <cassert>

#include <openhouse/model/EntityRegistry.hpp>

int main()
{
    openhouse::model::EntityRegistry registry;

    assert(registry.Count() == 0);

    return 0;
}
