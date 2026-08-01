#pragma once

#include <cmath>
#include <openhouse/transform/Transform.hpp>

namespace openhouse::transform
{

class Rotation
{
public:
    static Transform AroundZ(double angle)
    {
        Transform result;
        auto matrix = result.Matrix();

        const double c = std::cos(angle);
        const double s = std::sin(angle);

        matrix(0,0)=c;
        matrix(0,1)=-s;
        matrix(1,0)=s;
        matrix(1,1)=c;

        result.SetMatrix(matrix);
        return result;
    }
};

}
