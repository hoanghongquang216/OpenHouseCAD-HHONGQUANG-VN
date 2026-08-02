#include <cassert>

#include <openhouse/geometry/Point3.hpp>

int main()
{
    openhouse::geometry::Point3 point(1.0, 2.0, 3.0);
    openhouse::geometry::Vector3 offset(2.0, 3.0, 4.0);

    auto result = point + offset;

    assert(result.X() == 3.0);
    assert(result.Y() == 5.0);
    assert(result.Z() == 7.0);

    return 0;
}
