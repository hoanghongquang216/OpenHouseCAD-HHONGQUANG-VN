#include <cassert>
#include <openhouse/transform/Rotation.hpp>

int main()
{
    auto transform = openhouse::transform::Rotation::AroundZ(0.0);
    assert(transform.Matrix()(0,0) == 1.0);
    assert(transform.Matrix()(1,1) == 1.0);
    return 0;
}
