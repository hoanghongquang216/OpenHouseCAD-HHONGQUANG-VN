#pragma once

#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Vector2.hpp>

namespace openhouse::geometry {

// A bounded line SEGMENT between two points -- not an infinite line. This
// distinction matters in CAD: what a user draws by clicking a start and
// end point is a segment (has a defined length, a start, an end), not the
// infinite line through those two points. If an infinite-line abstraction
// is ever needed (e.g. for intersection/construction geometry), it should
// be a separate type, not a variant of this one, to avoid ambiguity about
// which operations (Length, Midpoint) even make sense for it.
template<typename T>
struct Line2 {
    Point2<T> start{};
    Point2<T> end{};

    friend constexpr bool operator==(const Line2&, const Line2&) = default;
};

using Line2f = Line2<float>;
using Line2d = Line2<double>;
using Line2i = Line2<int>;

template<Vector2Component T>
[[nodiscard]] constexpr Vector2<T> Displacement(const Line2<T>& line) noexcept {
    return line.end - line.start;
}

template<foundation::FloatingPoint T>
[[nodiscard]] T Length(const Line2<T>& line) noexcept {
    return Length(Displacement(line));
}

// See Vector2.hpp's LengthSquared for why this uses Vector2Component,
// not FloatingPoint.
template<Vector2Component T>
[[nodiscard]] constexpr T LengthSquared(const Line2<T>& line) noexcept {
    return LengthSquared(Displacement(line));
}

// Unit vector pointing from start to end. Undefined (division by zero)
// for a degenerate line where start == end -- callers working with
// possibly-degenerate input should check Length(line) > 0 first.
template<foundation::FloatingPoint T>
[[nodiscard]] Vector2<T> Direction(const Line2<T>& line) noexcept {
    return Normalized(Displacement(line));
}

// Restricted to FloatingPoint (not the more permissive Vector2Component,
// which also allows integral T): dividing by 2 with an integral T would
// silently truncate (e.g. midpoint of (0,0)-(3,3) would give (1,1)
// instead of (1.5,1.5)), which is a correctness bug, not a reasonable
// integer-arithmetic convention, for a geometric midpoint.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Point2<T> Midpoint(const Line2<T>& line) noexcept {
    return line.start + Displacement(line) / T{2};
}

}
