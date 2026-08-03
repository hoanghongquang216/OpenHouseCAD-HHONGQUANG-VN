#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/foundation/Numbers.hpp>

#include <compare>

namespace openhouse::math {

// A unit-safe angle type. Radians is the canonical internal
// representation; construction is only possible via the named factories
// FromRadians / FromDegrees, and there is no implicit conversion from a
// bare T -- this is deliberate. A bare `T angle` parameter/variable gives
// no indication of unit, which is one of the most common and hardest to
// spot bug classes in CAD/geometry code (passing degrees where radians
// are expected, or vice versa, compiles silently and produces wildly
// wrong results). Angle<T> makes the unit part of the type, so mixing
// them up is a compile error instead of a silent runtime bug.
template<foundation::FloatingPoint T>
class Angle {
public:
    constexpr Angle() noexcept = default;

    [[nodiscard]] static constexpr Angle FromRadians(T radians) noexcept {
        return Angle(radians);
    }

    [[nodiscard]] static constexpr Angle FromDegrees(T degrees) noexcept {
        return Angle(degrees * (foundation::pi_v<T> / T{180}));
    }

    [[nodiscard]] constexpr T Radians() const noexcept { return radians_; }

    [[nodiscard]] constexpr T Degrees() const noexcept {
        return radians_ * (T{180} / foundation::pi_v<T>);
    }

    friend constexpr bool operator==(const Angle&, const Angle&) = default;
    friend constexpr auto operator<=>(const Angle&, const Angle&) = default;

    [[nodiscard]] constexpr Angle operator+(Angle other) const noexcept {
        return Angle(radians_ + other.radians_);
    }

    [[nodiscard]] constexpr Angle operator-(Angle other) const noexcept {
        return Angle(radians_ - other.radians_);
    }

    [[nodiscard]] constexpr Angle operator-() const noexcept { return Angle(-radians_); }

    [[nodiscard]] constexpr Angle operator*(T scalar) const noexcept {
        return Angle(radians_ * scalar);
    }

    [[nodiscard]] friend constexpr Angle operator*(T scalar, Angle a) noexcept {
        return a * scalar;
    }

    [[nodiscard]] constexpr Angle operator/(T scalar) const noexcept {
        return Angle(radians_ / scalar);
    }

    // Angle / Angle is a dimensionless ratio, not an Angle -- intentionally
    // not provided as operator/ to avoid an easy-to-misuse overload; use
    // a.Radians() / b.Radians() explicitly if a ratio is genuinely needed.

    constexpr Angle& operator+=(Angle other) noexcept {
        radians_ += other.radians_;
        return *this;
    }

    constexpr Angle& operator-=(Angle other) noexcept {
        radians_ -= other.radians_;
        return *this;
    }

    constexpr Angle& operator*=(T scalar) noexcept {
        radians_ *= scalar;
        return *this;
    }

private:
    explicit constexpr Angle(T radians) noexcept : radians_(radians) {}

    T radians_{};
};

using Anglef = Angle<float>;
using Angled = Angle<double>;

// Normalizes to [0, 2*pi) radians / [0, 360) degrees.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Angle<T> NormalizedUnsigned(Angle<T> a) noexcept {
    const T twoPi = T{2} * foundation::pi_v<T>;
    T r = foundation::fmod(a.Radians(), twoPi);
    if (r < T{0}) {
        r += twoPi;
    }
    return Angle<T>::FromRadians(r);
}

// Normalizes to (-pi, pi] radians / (-180, 180] degrees.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Angle<T> NormalizedSigned(Angle<T> a) noexcept {
    const T twoPi = T{2} * foundation::pi_v<T>;
    T r = foundation::fmod(a.Radians() + foundation::pi_v<T>, twoPi);
    if (r <= T{0}) {
        r += twoPi;
    }
    return Angle<T>::FromRadians(r - foundation::pi_v<T>);
}

template<foundation::FloatingPoint T>
[[nodiscard]] T Sin(Angle<T> a) noexcept {
    return foundation::sin(a.Radians());
}

template<foundation::FloatingPoint T>
[[nodiscard]] T Cos(Angle<T> a) noexcept {
    return foundation::cos(a.Radians());
}

template<foundation::FloatingPoint T>
[[nodiscard]] T Tan(Angle<T> a) noexcept {
    return foundation::tan(a.Radians());
}

}
