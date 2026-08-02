#include <cassert>
#include <cmath>

#include <openhouse/geometry/Point3.hpp>
#include <openhouse/geometry/Vector3.hpp>

int main()
{
    using namespace openhouse::geometry;

    constexpr Vector3<double> a(1.0, 0.0, 0.0);
    constexpr Vector3<double> b(0.0, 1.0, 0.0);

    static_assert(a.X() == 1.0);

    auto cross = a.Cross(b);
    assert(cross.Z() == 1.0);

    Point3<double> p1(1.0, 2.0, 3.0);
    Point3<double> p2(0.0, 1.0, 3.0);

    auto direction = p1 - p2;
    assert(direction.X() == 1.0);
    assert(direction.Y() == 1.0);
    assert(direction.Z() == 0.0);

    return 0;
}
