#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Point2.hpp>

namespace openhouse::geometry {

// A circular arc: center, radius, and a start/end angle sweeping
// counter-clockwise from startAngle to endAngle (standard math
// convention, same as std::atan2's range and Matrix4::RotationZ's
// direction elsewhere in this codebase).
//
// Angles are stored as PLAIN RADIANS (T), not openhouse::math::Angle.
// This is deliberate: openhouse::math already depends on
// openhouse::geometry (Matrix4 needs Point3/Vector3), so geometry taking
// a dependency on math for a type-safe angle here would create a
// circular module dependency. If callers want the unit-safety
// math::Angle provides, they convert at the call site
// (`someAngle.Radians()`); a math-side convenience wrapper can be added
// later in the math module (which is free to depend on geometry) without
// geometry ever depending back on math.
template<typename T>
struct Arc2 {
    Point2<T> center{};
    T radius{};
    T startAngle{}; // radians
    T endAngle{};   // radians, sweeps counter-clockwise from startAngle

    friend constexpr bool operator==(const Arc2&, const Arc2&) = default;
};

using Arc2f = Arc2<float>;
using Arc2d = Arc2<double>;

// Sweep angle in radians. Not clamped/normalized -- if endAngle < startAngle
// the result is negative, reflecting a clockwise sweep; callers that want
// a normalized [0, 2*pi) sweep should handle that themselves (this type
// intentionally has no dependency on math::NormalizedUnsigned/Signed, for
// the same reason it avoids math::Angle).
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr T Sweep(const Arc2<T>& arc) noexcept {
    return arc.endAngle - arc.startAngle;
}

template<foundation::FloatingPoint T>
[[nodiscard]] Point2<T> PointAt(const Arc2<T>& arc, T angleRadians) noexcept {
    return {
        arc.center.x + arc.radius * foundation::cos(angleRadians),
        arc.center.y + arc.radius * foundation::sin(angleRadians),
    };
}

template<foundation::FloatingPoint T>
[[nodiscard]] Point2<T> StartPoint(const Arc2<T>& arc) noexcept {
    return PointAt(arc, arc.startAngle);
}

template<foundation::FloatingPoint T>
[[nodiscard]] Point2<T> EndPoint(const Arc2<T>& arc) noexcept {
    return PointAt(arc, arc.endAngle);
}

// The point at the arc's angular midpoint -- halfway between
// startAngle and endAngle along the sweep direction, matching how a
// CAD tool's Midpoint snap treats an arc (see SNAP-CORE-001). This is
// NOT the midpoint of the chord between StartPoint/EndPoint (that
// would cut inside the arc for anything but a semicircle) -- it's the
// point actually ON the arc, halfway along its curved length.
template<foundation::FloatingPoint T>
[[nodiscard]] Point2<T> Midpoint(const Arc2<T>& arc) noexcept {
    return PointAt(arc, arc.startAngle + Sweep(arc) / T{2});
}

// Arc length = radius * |sweep angle|. Uses the absolute value of Sweep
// so length is always non-negative regardless of sweep direction.
template<foundation::FloatingPoint T>
[[nodiscard]] T Length(const Arc2<T>& arc) noexcept {
    return arc.radius * foundation::abs(Sweep(arc));
}

}
