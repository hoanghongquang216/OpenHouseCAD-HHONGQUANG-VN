#pragma once

#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

namespace openhouse::geometry {

// Parametric position of `point` along `line`, using the same
// convention Line2 x Line2 intersection already uses internally
// (Intersection.hpp): line.start + t * (line.end - line.start).
// t == 0 at line.start, t == 1 at line.end, t == 0.5 at the midpoint.
//
// NOT clamped to [0,1] -- a point beyond either endpoint still
// produces a well-defined t outside that range; the caller decides
// what an out-of-range t means (TRIM-001 needs exactly this: telling
// "intersection is on the segment" from "past one end" by reading t
// directly, rather than this function silently clamping the answer
// away).
//
// `point` is expected to already lie ON (or numerically very close
// to) the line -- e.g. a point produced by geometry::Intersect().
// This does NOT perform perpendicular projection of an arbitrary
// off-line point onto its nearest point on the line; that is a
// different operation with no current caller (see TRIM-001 audit /
// ADR-0006 on not introducing capability ahead of an actual need).
//
// Degenerate line (start == end): returns 0. There is no well-defined
// "position along" a zero-length segment.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr T ParameterOnLine(const Line2<T>& line, const Point2<T>& point) noexcept {
    const T dx = line.end.x - line.start.x;
    const T dy = line.end.y - line.start.y;
    const T lengthSquared = dx * dx + dy * dy;
    if (lengthSquared == T{0}) {
        return T{0};
    }
    const T px = point.x - line.start.x;
    const T py = point.y - line.start.y;
    return (px * dx + py * dy) / lengthSquared;
}

}
