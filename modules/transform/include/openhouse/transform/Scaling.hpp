#pragma once

#include <openhouse/transform/Transform.hpp>

namespace openhouse::transform
{

class Scaling
{
public:
    static Transform Create(double x, double y, double z)
    {
        Transform result;
        auto matrix = result.Matrix();

        matrix(0,0)=x;
        matrix(1,1)=y;
        matrix(2,2)=z;

        result.SetMatrix(matrix);
        return result;
    }

    static Transform Uniform(double value)
    {
        return Create(value, value, value);
    }
};

}
