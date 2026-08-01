#include <cassert>
#include <openhouse/transform/Matrix4.hpp>

int main()
{
    openhouse::transform::Matrix4 matrix;
    assert(matrix(0,0) == 1.0);
    assert(matrix(1,1) == 1.0);
    assert(matrix(2,2) == 1.0);
    return 0;
}
