#include <cassert>
#include <openhouse/transform/Scaling.hpp>

int main()
{
    auto transform = openhouse::transform::Scaling::Create(2.0, 3.0, 4.0);
    assert(transform.Matrix()(0,0) == 2.0);
    assert(transform.Matrix()(1,1) == 3.0);
    assert(transform.Matrix()(2,2) == 4.0);
    return 0;
}
