#include <cassert>

#include <openhouse/geometry/Transform.hpp>

int main()
{
    openhouse::geometry::Transform transform;

    assert(transform.Matrix().At(0,0) == 1.0);
    assert(transform.Matrix().At(3,3) == 1.0);

    return 0;
}
