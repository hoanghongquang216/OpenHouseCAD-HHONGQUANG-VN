#pragma once

#include <openhouse/geometry/Vector2.hpp>

namespace openhouse::geometry
{

template<typename T>
struct Point2
{
    T x{};
    T y{};

    friend constexpr bool operator==(const Point2&, const Point2&) = default;
};

using Point2f = Point2<float>;
using Point2d = Point2<double>;
using Point2i = Point2<int>;

template<Vector2Component T>
constexpr Vector2<T> operator-(const Point2<T>& a, const Point2<T>& b) noexcept
{
    return {a.x - b.x, a.y - b.y};
}

template<Vector2Component T>
constexpr Point2<T> operator+(const Point2<T>& p, const Vector2<T>& v) noexcept
{
    return {p.x + v.x, p.y + v.y};
}

template<Vector2Component T>
constexpr Point2<T> operator+(const Vector2<T>& v, const Point2<T>& p) noexcept
{
    return p + v;
}

template<Vector2Component T>
constexpr Point2<T> operator-(const Point2<T>& p, const Vector2<T>& v) noexcept
{
    return {p.x - v.x, p.y - v.y};
}

}
