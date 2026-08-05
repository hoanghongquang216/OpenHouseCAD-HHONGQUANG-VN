#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

#include <limits>
#include <optional>

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

// EXTEND-001: intersection of `target`'s infinite line with `boundary`
// as a bounded SEGMENT. Corresponds to AutoCAD's EDGEMODE=0 ("No
// extend") -- `target` is treated as extending infinitely in both
// directions (that is the entire point of Extend: the target's own
// current endpoints do not constrain where it may meet the boundary),
// while `boundary` keeps its normal segment bounds (s in [0,1]).
// AutoCAD/LibreCAD also support an EDGEMODE=1 ("implied edge") mode
// where the boundary is ALSO treated as infinite -- deliberately not
// implemented here; see EXTEND-002 in the backlog. Adding it later is
// an additional function/overload, not a change to this one's
// contract.
//
// Uses the same parametric-form solve as Intersect(Line2,Line2)
// (Intersection.hpp), but only enforces the [0,1] bound on `boundary`'s
// own parameter (`s`), not `target`'s (`t`).
//
// Returns std::nullopt if the lines are parallel (including collinear
// overlap -- same "no single well-defined point" reasoning as
// Intersect(Line2,Line2), see Intersection.hpp), or if the computed
// point falls outside `boundary`'s own segment bounds.
template<foundation::FloatingPoint T>
[[nodiscard]] std::optional<Point2<T>> FindExtendIntersection(const Line2<T>& target,
                                                                const Line2<T>& boundary) noexcept {
    const T ax = target.end.x - target.start.x;
    const T ay = target.end.y - target.start.y;
    const T bx = boundary.end.x - boundary.start.x;
    const T by = boundary.end.y - boundary.start.y;

    const T denom = ax * by - ay * bx;
    // Same local-epsilon convention as Intersection.hpp's own
    // detail::NearlyZero -- deliberately duplicated rather than shared,
    // for the same non-dependency reasons documented there.
    const T eps = std::numeric_limits<T>::epsilon() * T{100};
    if (foundation::abs(denom) <= eps) {
        return std::nullopt;
    }

    const T cx = boundary.start.x - target.start.x;
    const T cy = boundary.start.y - target.start.y;

    // s is boundary's own parameter -- this IS bounds-checked.
    const T s = (cx * ay - cy * ax) / denom;
    if (s < -eps || s > T{1} + eps) {
        return std::nullopt;
    }

    // t is target's parameter -- deliberately NOT bounds-checked; this
    // is the entire reason this function exists rather than reusing
    // Intersect(Line2,Line2).
    const T t = (cx * by - cy * bx) / denom;
    return Point2<T>{target.start.x + t * ax, target.start.y + t * ay};
}

}
