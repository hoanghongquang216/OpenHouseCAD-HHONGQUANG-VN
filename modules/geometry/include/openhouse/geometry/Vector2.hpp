#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>

#include <compare>

namespace openhouse::geometry {

template<typename T>
struct Vector2 {
    T x{};
    T y{};

    friend constexpr bool operator==(const Vector2&, const Vector2&) = default;
};

using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;
using Vector2i = Vector2<int>;

template<typename T>
concept Vector2Component = foundation::Integral<T> || foundation::FloatingPoint<T>;

template<Vector2Component T>
constexpr Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b) noexcept {
    return {a.x + b.x, a.y + b.y};
}

template<Vector2Component T>
constexpr Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b) noexcept {
    return {a.x - b.x, a.y - b.y};
}

template<Vector2Component T>
constexpr Vector2<T> operator-(const Vector2<T>& a) noexcept {
    return {-a.x, -a.y};
}

template<Vector2Component T>
constexpr Vector2<T> operator*(const Vector2<T>& a, T scalar) noexcept {
    return {a.x * scalar, a.y * scalar};
}

template<Vector2Component T>
constexpr Vector2<T> operator*(T scalar, const Vector2<T>& a) noexcept {
    return a * scalar;
}

template<Vector2Component T>
constexpr Vector2<T> operator/(const Vector2<T>& a, T scalar) noexcept {
    return {a.x / scalar, a.y / scalar};
}

template<Vector2Component T>
constexpr Vector2<T>& operator+=(Vector2<T>& a, const Vector2<T>& b) noexcept {
    a.x += b.x;
    a.y += b.y;
    return a;
}

template<Vector2Component T>
constexpr Vector2<T>& operator-=(Vector2<T>& a, const Vector2<T>& b) noexcept {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

template<Vector2Component T>
constexpr Vector2<T>& operator*=(Vector2<T>& a, T scalar) noexcept {
    a.x *= scalar;
    a.y *= scalar;
    return a;
}

template<Vector2Component T>
constexpr T Dot(const Vector2<T>& a, const Vector2<T>& b) noexcept {
    return a.x * b.x + a.y * b.y;
}

// 2D "cross product" yields a scalar: the z-component of the equivalent 3D cross product.
template<Vector2Component T>
constexpr T Cross(const Vector2<T>& a, const Vector2<T>& b) noexcept {
    return a.x * b.y - a.y * b.x;
}

// LengthSquared uses the broader Vector2Component (not FloatingPoint):
// it's just Dot(a,a), no division or sqrt involved, so it's exactly as
// valid for integral T as Dot/Cross are. Length below genuinely needs
// FloatingPoint since it calls sqrt.
template<Vector2Component T>
constexpr T LengthSquared(const Vector2<T>& a) noexcept {
    return Dot(a, a);
}

template<foundation::FloatingPoint T>
T Length(const Vector2<T>& a) noexcept {
    return foundation::sqrt(LengthSquared(a));
}

template<foundation::FloatingPoint T>
Vector2<T> Normalized(const Vector2<T>& a) noexcept {
    const T len = Length(a);
    return a / len;
}

}
