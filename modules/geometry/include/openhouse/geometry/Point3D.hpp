#pragma once

#include <cmath>

#include <openhouse/geometry/Vector3D.hpp>

namespace openhouse::geometry
{

class Point3D
{
public:
    constexpr Point3D()
        : x_(0.0), y_(0.0), z_(0.0)
    {
    }

    constexpr Point3D(double x, double y, double z)
        : x_(x), y_(y), z_(z)
    {
    }

    constexpr double X() const { return x_; }
    constexpr double Y() const { return y_; }
    constexpr double Z() const { return z_; }

    double DistanceTo(const Point3D& other) const
    {
        const double dx = x_ - other.x_;
        const double dy = y_ - other.y_;
        const double dz = z_ - other.z_;

        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    Point3D operator+(const Vector3D& vector) const
    {
        return Point3D(
            x_ + vector.X(),
            y_ + vector.Y(),
            z_ + vector.Z());
    }

    Vector3D operator-(const Point3D& other) const
    {
        return Vector3D(
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
