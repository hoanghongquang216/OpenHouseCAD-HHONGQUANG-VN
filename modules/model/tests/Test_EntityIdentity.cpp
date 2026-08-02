#include <cassert>

#include <openhouse/model/Entity.hpp>

int main()
{
    openhouse::model::Entity entity;

    assert(!entity.Id().IsValid());

    return 0;
}
