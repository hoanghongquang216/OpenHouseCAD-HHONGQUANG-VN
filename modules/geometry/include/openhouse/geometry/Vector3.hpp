#pragma once

namespace openhouse::geometry
{

class Vector3
{
public:
    constexpr Vector3(double x = 0.0, double y = 0.0, double z = 0.0)
        : x_(x), y_(y), z_(z)
    {
    }

    constexpr double X() const { return x_; }
    constexpr double Y() const { return y_; }
    constexpr double Z() const { return z_; }

    constexpr Vector3 operator+(const Vector3& other) const
    {
        return Vector3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
    }

private:
    double x_;
    double y_;
    double z_;
};

}
