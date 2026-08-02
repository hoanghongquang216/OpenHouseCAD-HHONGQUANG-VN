#include <cassert>

#include <openhouse/geometry/Matrix4.hpp>

int main()
{
    openhouse::geometry::Matrix4 matrix;

    assert(matrix.At(0,0) == 1.0);
    assert(matrix.At(1,1) == 1.0);
    assert(matrix.At(2,2) == 1.0);
    assert(matrix.At(3,3) == 1.0);

    return 0;
}
