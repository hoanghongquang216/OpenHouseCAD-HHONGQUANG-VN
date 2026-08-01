#pragma once

#include <openhouse/transform/Transform.hpp>

namespace openhouse::transform
{

class Translation
{
public:
    static Transform Create(double x, double y, double z)
    {
        Transform result;
        auto matrix = result.Matrix();
        matrix(0,3) = x;
        matrix(1,3) = y;
        matrix(2,3) = z;
        result.SetMatrix(matrix);
        return result;
    }
};

}
