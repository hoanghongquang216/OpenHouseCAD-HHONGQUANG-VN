#pragma once

#include <cmath>

#include <openhouse/geometry/Curve.hpp>

namespace openhouse::geometry
{

class Arc : public Curve
{
public:
    Arc(
        const Point3D& center,
        double radius,
        double startAngle,
        double endAngle)
        : center_(center),
          radius_(radius),
          startAngle_(startAngle),
          endAngle_(endAngle)
    {
    }

    Point3D StartPoint() const override
    {
        return Evaluate(startAngle_);
    }

    Point3D EndPoint() const override
    {
        return Evaluate(endAngle_);
    }

    double Length() const override
    {
        return std::abs(endAngle_ - startAngle_) * radius_;
    }

    Point3D Evaluate(double angle) const
    {
        return Point3D(
            center_.X() + radius_ * std::cos(angle),
            center_.Y() + radius_ * std::sin(angle),
            center_.Z());
    }

private:
    Point3D center_;
    double radius_;
    double startAngle_;
    double endAngle_;
};

}
