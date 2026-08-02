#pragma once

#include <openhouse/geometry/Point3D.hpp>

namespace openhouse::geometry
{

class Curve
{
public:
    virtual ~Curve() = default;

    virtual Point3D StartPoint() const = 0;
    virtual Point3D EndPoint() const = 0;
    virtual double Length() const = 0;
};

}
