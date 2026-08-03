#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>

#include <compare>

namespace openhouse::geometry {

template<typename T>
struct Vector3 {
    T x{};
    T y{};
    T z{};

    friend constexpr bool operator==(const Vector3&, const Vector3&) = default;
};

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int>;

template<typename T>
concept Vector3Component = foundation::Integral<T> || foundation::FloatingPoint<T>;

template<Vector3Component T>
constexpr Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b) noexcept {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

template<Vector3Component T>
constexpr Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b) noexcept {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

template<Vector3Component T>
constexpr Vector3<T> operator-(const Vector3<T>& a) noexcept {
    return {-a.x, -a.y, -a.z};
}

template<Vector3Component T>
constexpr Vector3<T> operator*(const Vector3<T>& a, T scalar) noexcept {
    return {a.x * scalar, a.y * scalar, a.z * scalar};
}

template<Vector3Component T>
constexpr Vector3<T> operator*(T scalar, const Vector3<T>& a) noexcept {
    return a * scalar;
}

template<Vector3Component T>
constexpr Vector3<T> operator/(const Vector3<T>& a, T scalar) noexcept {
    return {a.x / scalar, a.y / scalar, a.z / scalar};
}

template<Vector3Component T>
constexpr Vector3<T>& operator+=(Vector3<T>& a, const Vector3<T>& b) noexcept {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}

template<Vector3Component T>
constexpr Vector3<T>& operator-=(Vector3<T>& a, const Vector3<T>& b) noexcept {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    return a;
}

template<Vector3Component T>
constexpr Vector3<T>& operator*=(Vector3<T>& a, T scalar) noexcept {
    a.x *= scalar;
    a.y *= scalar;
    a.z *= scalar;
    return a;
}

template<Vector3Component T>
constexpr T Dot(const Vector3<T>& a, const Vector3<T>& b) noexcept {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<Vector3Component T>
constexpr Vector3<T> Cross(const Vector3<T>& a, const Vector3<T>& b) noexcept {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

// See Vector2.hpp's LengthSquared for why this uses Vector3Component,
// not FloatingPoint (no division/sqrt involved, unlike Length).
template<Vector3Component T>
constexpr T LengthSquared(const Vector3<T>& a) noexcept {
    return Dot(a, a);
}

template<foundation::FloatingPoint T>
T Length(const Vector3<T>& a) noexcept {
    return foundation::sqrt(LengthSquared(a));
}

template<foundation::FloatingPoint T>
Vector3<T> Normalized(const Vector3<T>& a) noexcept {
    const T len = Length(a);
    return a / len;
}

}
