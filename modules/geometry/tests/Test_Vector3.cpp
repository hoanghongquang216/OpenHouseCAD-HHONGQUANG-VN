#include <cassert>

#include <openhouse/geometry/Vector3.hpp>

int main()
{
    openhouse::geometry::Vector3 a(1.0, 2.0, 3.0);
    openhouse::geometry::Vector3 b(2.0, 3.0, 4.0);

    auto c = a + b;

    assert(c.X() == 3.0);
    assert(c.Y() == 5.0);
    assert(c.Z() == 7.0);

    return 0;
}
