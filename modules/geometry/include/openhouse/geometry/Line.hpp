#pragma once

#include <openhouse/geometry/Curve.hpp>
#include <openhouse/geometry/Vector3D.hpp>

namespace openhouse::geometry
{

class Line : public Curve
{
public:
    Line(const Point3D& start, const Point3D& end)
        : start_(start), end_(end)
    {
    }

    Point3D StartPoint() const override
    {
        return start_;
    }

    Point3D EndPoint() const override
    {
        return end_;
    }

    double Length() const override
    {
        return start_.DistanceTo(end_);
    }

    Vector3D Direction() const
    {
        return (end_ - start_).Normalize();
    }

    Point3D Evaluate(double parameter) const
    {
        return start_ + (end_ - start_) * parameter;
    }

private:
    Point3D start_;
    Point3D end_;
};

}
