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

template<typename T>
constexpr Vector3 operator-(const Point3<T>& a, const Point3<T>& b) noexcept
{
    return Vector3(
        static_cast<double>(a.x - b.x),
        static_cast<double>(a.y - b.y),
        static_cast<double>(a.z - b.z));
}

template<typename T>
constexpr Point3<T> operator+(const Point3<T>& p, const Vector3& v) noexcept
{
    return {
        static_cast<T>(p.x + v.X()),
        static_cast<T>(p.y + v.Y()),
        static_cast<T>(p.z + v.Z())};
}

template<typename T>
constexpr Point3<T> operator+(const Vector3& v, const Point3<T>& p) noexcept
{
    return p + v;
}

template<typename T>
constexpr Point3<T> operator-(const Point3<T>& p, const Vector3& v) noexcept
{
    return {
        static_cast<T>(p.x - v.X()),
        static_cast<T>(p.y - v.Y()),
        static_cast<T>(p.z - v.Z())};
}

}
