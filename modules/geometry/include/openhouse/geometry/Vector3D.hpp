#pragma once

#include <cmath>

namespace openhouse::geometry
{

class Vector3D
{
public:
    constexpr Vector3D()
        : x_(0.0), y_(0.0), z_(0.0)
    {
    }

    constexpr Vector3D(double x, double y, double z)
        : x_(x), y_(y), z_(z)
    {
    }

    constexpr double X() const { return x_; }
    constexpr double Y() const { return y_; }
    constexpr double Z() const { return z_; }

    double Length() const
    {
        return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
    }

    Vector3D Normalize() const
    {
        const double length = Length();

        if (length == 0.0)
        {
            return Vector3D();
        }

        return Vector3D(x_ / length, y_ / length, z_ / length);
    }

    double Dot(const Vector3D& other) const
    {
        return x_ * other.X() + y_ * other.Y() + z_ * other.Z();
    }

    Vector3D Cross(const Vector3D& other) const
    {
        return Vector3D(
            y_ * other.Z() - z_ * other.Y(),
            z_ * other.X() - x_ * other.Z(),
            x_ * other.Y() - y_ * other.X());
    }

    Vector3D operator+(const Vector3D& other) const
    {
        return Vector3D(x_ + other.X(), y_ + other.Y(), z_ + other.Z());
    }

    Vector3D operator-(const Vector3D& other) const
    {
        return Vector3D(x_ - other.X(), y_ - other.Y(), z_ - other.Z());
    }

    Vector3D operator*(double value) const
    {
        return Vector3D(x_ * value, y_ * value, z_ * value);
    }

private:
    double x_;
    double y_;
    double z_;
};

}
