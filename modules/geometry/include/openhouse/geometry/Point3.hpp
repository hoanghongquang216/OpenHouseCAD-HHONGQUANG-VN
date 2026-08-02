#pragma once

#include <openhouse/geometry/Vector3.hpp>

namespace openhouse::geometry
{

template<typename T>
struct Point3
{
    T x{};
    T y{};
    T z{};

    friend constexpr bool operator==(const Point3&, const Point3&) = default;
};

using Point3f = Point3<float>;
using Point3d = Point3<double>;
using Point3i = Point3<int>;

template<Vector3Component T>
constexpr Vector3<T> operator-(const Point3<T>& a, const Point3<T>& b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

template<Vector3Component T>
constexpr Point3<T> operator+(const Point3<T>& p, const Vector3<T>& v) noexcept
{
    return {p.x + v.x, p.y + v.y, p.z + v.z};
}

template<Vector3Component T>
constexpr Point3<T> operator+(const Vector3<T>& v, const Point3<T>& p) noexcept
{
    return p + v;
}

template<Vector3Component T>
constexpr Point3<T> operator-(const Point3<T>& p, const Vector3<T>& v) noexcept
{
    return {p.x - v.x, p.y - v.y, p.z - v.z};
}

}
