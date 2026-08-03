#pragma once

#include <openhouse/foundation/Numbers.hpp>
#include <openhouse/geometry/Point2.hpp>

namespace openhouse::geometry {

// A circle: center point + radius. Deliberately does NOT depend on
// openhouse::math (e.g. for an angle-based Arc2 built on top of this) --
// openhouse::math already depends on openhouse::geometry (Matrix4 needs
// Point3/Vector3), so geometry depending back on math would create a
// circular module dependency. Circle2 needs no angle representation at
// all, so it has no such conflict; Arc2 (which does need one) is
// deliberately deferred to its own task with an explicit dependency
// decision, not bundled in here.
//
// `radius` is not validated (e.g. non-negative) at construction -- this
// type is a plain aggregate like Point2/Vector2/Line2. A negative radius
// is meaningless for Area/Circumference/Contains below; callers
// constructing a Circle2 from untrusted input are responsible for
// validating radius >= 0 themselves.
template<typename T>
struct Circle2 {
    Point2<T> center{};
    T radius{};

    friend constexpr bool operator==(const Circle2&, const Circle2&) = default;
};

using Circle2f = Circle2<float>;
using Circle2d = Circle2<double>;
using Circle2i = Circle2<int>;

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr T Circumference(const Circle2<T>& c) noexcept {
    return T{2} * foundation::pi_v<T> * c.radius;
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr T Area(const Circle2<T>& c) noexcept {
    return foundation::pi_v<T> * c.radius * c.radius;
}

// Inclusive: a point exactly on the boundary counts as contained.
template<foundation::FloatingPoint T>
[[nodiscard]] bool Contains(const Circle2<T>& c, const Point2<T>& p) noexcept {
    return DistanceSquared(c.center, p) <= c.radius * c.radius;
}

}
