#include <cassert>
#include <openhouse/transform/Translation.hpp>

int main()
{
    auto transform = openhouse::transform::Translation::Create(1.0, 2.0, 3.0);
    assert(transform.Matrix()(0,3) == 1.0);
    assert(transform.Matrix()(1,3) == 2.0);
    assert(transform.Matrix()(2,3) == 3.0);
    return 0;
}
