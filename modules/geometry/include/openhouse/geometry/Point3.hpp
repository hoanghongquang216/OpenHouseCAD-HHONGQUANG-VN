#pragma once

#include <openhouse/geometry/Vector3.hpp>

namespace openhouse::geometry
{

class Point3
{
public:
    constexpr Point3(double x = 0.0, double y = 0.0, double z = 0.0)
        : x_(x), y_(y), z_(z)
    {
    }

    constexpr double X() const { return x_; }
    constexpr double Y() const { return y_; }
    constexpr double Z() const { return z_; }

    constexpr Point3 operator+(const Vector3& vector) const
    {
        return Point3(x_ + vector.X(), y_ + vector.Y(), z_ + vector.Z());
    }

    constexpr Point3 operator-(const Vector3& vector) const
    {
        return Point3(x_ - vector.X(), y_ - vector.Y(), z_ - vector.Z());
    }

    constexpr Vector3 operator-(const Point3& other) const
    {
        return Vector3(
            x_ - other.x_,
            y_ - other.y_,
            z_ - other.z_);
    }

private:
    double x_;
    double y_;
    double z_;
};

}
