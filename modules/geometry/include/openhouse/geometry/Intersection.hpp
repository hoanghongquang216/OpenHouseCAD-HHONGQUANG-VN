#pragma once

#include <openhouse/foundation/Algorithm.hpp>
#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Bounds.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

#include <array>
#include <cstddef>
#include <limits>

namespace openhouse::geometry {

// The result of intersecting two shapes: 0, 1 (tangent, or a degenerate
// single-solution case), or 2 points. A plain fixed-capacity aggregate
// (like Line2/Circle2/Arc2 themselves) rather than std::vector -- the
// result is always at most 2 points, so a heap allocation per call would
// be pure overhead for every caller.
template<typename T>
struct IntersectionResult {
    std::array<Point2<T>, 2> points{};
    std::size_t count = 0;

    [[nodiscard]] constexpr bool Empty() const noexcept {
        return count == 0;
    }
};

using IntersectionResultf = IntersectionResult<float>;
using IntersectionResultd = IntersectionResult<double>;

namespace detail {

// Local numerical epsilon for classifying near-zero quantities that
// arise from this file's own formulas (a discriminant near zero at a
// tangency, a cross-product near zero for near-parallel lines, a
// center-distance near zero for near-concentric circles). This is a
// DIFFERENT concept from document::FindSnapPoint's UI-facing `tolerance`
// parameter (how close a cursor must be to count as "snapped") -- this
// epsilon exists purely to make this file's own root-classification
// numerically robust, regardless of who calls it or why.
//
// Deliberately NOT openhouse::math::NumericTraits::DefaultTolerance:
// `geometry` does not depend on `math` (see Circle2.hpp's own comment
// on why -- math depends on geometry, not the reverse) and
// ADR-0006 (docs/ARCHITECTURE_DECISION_RECORDS) confirms that boundary
// should stay as-is rather than growing a new geometry->math edge just
// for this. Same reasoning/formula as NumericTraits::DefaultTolerance
// (machine epsilon scaled by a documented safety factor), intentionally
// duplicated here rather than shared, since the two modules are meant
// to stay non-dependent siblings.
template<foundation::FloatingPoint T>
inline constexpr T kEpsilon = std::numeric_limits<T>::epsilon() * T{100};

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr bool NearlyZero(T value) noexcept {
    return foundation::abs(value) <= kEpsilon<T>;
}

// Angle (as seen from the circle's center) of a point known to already
// lie on that circle. Shared by the Arc-filtering wrappers below.
template<foundation::FloatingPoint T>
[[nodiscard]] T AngleFromCenter(const Point2<T>& center, const Point2<T>& point) noexcept {
    return foundation::atan2(point.y - center.y, point.x - center.x);
}

} // namespace detail

// ---------------------------------------------------------------------
// Line2 x Line2
// ---------------------------------------------------------------------

// Intersection of two bounded SEGMENTS (not the infinite lines through
// them -- see Line2.hpp's own comment on why Line2 is a segment type).
// Standard parametric-form solve: a.start + t*(a.end-a.start) for
// t in [0,1], b.start + s*(b.end-b.start) for s in [0,1]; solve for
// where they coincide, then reject any solution outside either
// segment's own [0,1] range.
template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Line2<T>& a, const Line2<T>& b) noexcept {
    const T ax = a.end.x - a.start.x;
    const T ay = a.end.y - a.start.y;
    const T bx = b.end.x - b.start.x;
    const T by = b.end.y - b.start.y;

    const T denom = ax * by - ay * bx;
    if (detail::NearlyZero(denom)) {
        // Parallel, including the collinear-overlapping case. A
        // collinear-overlapping pair has infinitely many intersection
        // points -- not a single well-defined one -- so this
        // deliberately reports "no result" rather than picking an
        // arbitrary point from the overlap (same treatment as the
        // coincident-circles case in Circle2 x Circle2 below). A
        // caller that specifically needs "do these segments overlap at
        // all" needs a different query than this one.
        return {};
    }

    const T cx = b.start.x - a.start.x;
    const T cy = b.start.y - a.start.y;

    const T t = (cx * by - cy * bx) / denom;
    const T s = (cx * ay - cy * ax) / denom;

    // Epsilon-widened [0,1] bounds check: without this, a segment pair
    // that meets EXACTLY at a shared endpoint can compute t (or s) as
    // e.g. 1.0000000000000002 due to floating-point rounding in the
    // division above, and a strict t <= T{1} would wrongly reject a
    // real, intended endpoint-to-endpoint intersection.
    const T eps = detail::kEpsilon<T>;
    if (t < -eps || t > T{1} + eps || s < -eps || s > T{1} + eps) {
        return {};
    }

    IntersectionResult<T> result;
    result.points[0] = {a.start.x + t * ax, a.start.y + t * ay};
    result.count = 1;
    return result;
}

// ---------------------------------------------------------------------
// Line2 x Circle2
// ---------------------------------------------------------------------

// Intersection of a bounded SEGMENT with a circle's outline (not a
// filled disk -- matches Circle2's own DistanceToShape treatment in
// HitTest.hpp). Parametrize the segment as start + t*d, substitute into
// the circle equation, and solve the resulting quadratic in t; each
// root is then checked against the segment's own [0,1] range, since the
// infinite line can cross the circle twice while the segment itself
// covers zero, one, or both of those crossings.
template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Line2<T>& line, const Circle2<T>& circle) noexcept {
    const T dx = line.end.x - line.start.x;
    const T dy = line.end.y - line.start.y;
    const T fx = line.start.x - circle.center.x;
    const T fy = line.start.y - circle.center.y;

    const T a = dx * dx + dy * dy;
    if (detail::NearlyZero(a)) {
        // Degenerate line (start == end), same treatment as
        // DistanceToShape(Line2, Point2) in HitTest.hpp: fall back to a
        // plain point-vs-circle test rather than dividing by zero.
        const T distance = foundation::hypot(fx, fy);
        IntersectionResult<T> result;
        if (detail::NearlyZero(distance - circle.radius)) {
            result.points[0] = line.start;
            result.count = 1;
        }
        return result;
    }

    const T b = T{2} * (fx * dx + fy * dy);
    const T c = fx * fx + fy * fy - circle.radius * circle.radius;

    const T discriminant = b * b - T{4} * a * c;
    if (discriminant < T{0} && !detail::NearlyZero(discriminant)) {
        return {};
    }

    const T sqrtDiscriminant = foundation::sqrt(foundation::abs(discriminant) < detail::kEpsilon<T>
                                                      ? T{0}
                                                      : foundation::abs(discriminant));
    const T t1 = (-b - sqrtDiscriminant) / (T{2} * a);
    const T t2 = (-b + sqrtDiscriminant) / (T{2} * a);

    const T eps = detail::kEpsilon<T>;
    IntersectionResult<T> result;
    if (t1 >= -eps && t1 <= T{1} + eps) {
        result.points[result.count++] = {line.start.x + t1 * dx, line.start.y + t1 * dy};
    }
    // Tangent case (t1 == t2): only report the single point once.
    if (!detail::NearlyZero(t2 - t1) && t2 >= -eps && t2 <= T{1} + eps) {
        result.points[result.count++] = {line.start.x + t2 * dx, line.start.y + t2 * dy};
    }
    return result;
}

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Circle2<T>& circle, const Line2<T>& line) noexcept {
    return Intersect(line, circle);
}

// ---------------------------------------------------------------------
// Circle2 x Circle2
// ---------------------------------------------------------------------

// Classic two-circle intersection. Concentric circles (same OR
// different radius) are handled as their own explicit branch rather
// than left to fall out of the general formula's floating-point
// behavior (the general formula divides by the center distance, which
// is exactly the quantity that's near-zero here).
template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Circle2<T>& a, const Circle2<T>& b) noexcept {
    const T dx = b.center.x - a.center.x;
    const T dy = b.center.y - a.center.y;
    const T d = foundation::hypot(dx, dy);

    if (detail::NearlyZero(d)) {
        // Concentric. Same-radius means the two circles are identical
        // -- infinitely many intersection points, reported as "no
        // result" for the same reason the collinear-overlapping Line2
        // case is (see its comment). Different-radius concentric
        // circles never touch at all.
        return {};
    }

    if (d > a.radius + b.radius + detail::kEpsilon<T> ||
        d < foundation::abs(a.radius - b.radius) - detail::kEpsilon<T>) {
        // Too far apart, or one entirely inside the other without
        // touching.
        return {};
    }

    // Standard formula: `h` is the perpendicular distance from the
    // center-to-center line to each intersection point; `aDist` is how
    // far along that line (from a.center toward b.center) the foot of
    // that perpendicular sits.
    const T aDist = (d * d + a.radius * a.radius - b.radius * b.radius) / (T{2} * d);
    // h^2 can come out as a tiny negative number at/near tangency due to
    // floating-point rounding even though it's mathematically >= 0 --
    // clamp rather than let sqrt() of a negative value produce NaN.
    const T hSquared = foundation::max(T{0}, a.radius * a.radius - aDist * aDist);
    const T h = foundation::sqrt(hSquared);

    const Point2<T> mid{a.center.x + aDist * dx / d, a.center.y + aDist * dy / d};

    IntersectionResult<T> result;
    if (detail::NearlyZero(h)) {
        // Tangent (internally or externally) -- exactly one point.
        result.points[0] = mid;
        result.count = 1;
        return result;
    }

    // Perpendicular unit vector to the center-to-center direction.
    const T px = -dy / d;
    const T py = dx / d;

    result.points[0] = {mid.x + h * px, mid.y + h * py};
    result.points[1] = {mid.x - h * px, mid.y - h * py};
    result.count = 2;
    return result;
}

// ---------------------------------------------------------------------
// Arc2 wrappers -- reduce to the underlying full-circle formula above,
// then keep only the candidate points that actually lie on the arc's
// swept range. Reuses AngleOnArc (Bounds.hpp) rather than reimplementing
// sweep-direction/wraparound handling -- exactly the same reuse
// HitTest.hpp's DistanceToShape(Arc2, Point2) already makes of it.
// ---------------------------------------------------------------------

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Line2<T>& line, const Arc2<T>& arc) noexcept {
    const IntersectionResult<T> full = Intersect(line, Circle2<T>{arc.center, arc.radius});

    IntersectionResult<T> filtered;
    for (std::size_t i = 0; i < full.count; ++i) {
        const T angle = detail::AngleFromCenter(arc.center, full.points[i]);
        if (AngleOnArc(arc, angle)) {
            filtered.points[filtered.count++] = full.points[i];
        }
    }
    return filtered;
}

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Arc2<T>& arc, const Line2<T>& line) noexcept {
    return Intersect(line, arc);
}

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Circle2<T>& circle, const Arc2<T>& arc) noexcept {
    const IntersectionResult<T> full = Intersect(circle, Circle2<T>{arc.center, arc.radius});

    IntersectionResult<T> filtered;
    for (std::size_t i = 0; i < full.count; ++i) {
        const T angle = detail::AngleFromCenter(arc.center, full.points[i]);
        if (AngleOnArc(arc, angle)) {
            filtered.points[filtered.count++] = full.points[i];
        }
    }
    return filtered;
}

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Arc2<T>& arc, const Circle2<T>& circle) noexcept {
    return Intersect(circle, arc);
}

template<foundation::FloatingPoint T>
[[nodiscard]] IntersectionResult<T> Intersect(const Arc2<T>& a, const Arc2<T>& b) noexcept {
    const IntersectionResult<T> full =
        Intersect(Circle2<T>{a.center, a.radius}, Circle2<T>{b.center, b.radius});

    IntersectionResult<T> filtered;
    for (std::size_t i = 0; i < full.count; ++i) {
        const T angleOnA = detail::AngleFromCenter(a.center, full.points[i]);
        const T angleOnB = detail::AngleFromCenter(b.center, full.points[i]);
        if (AngleOnArc(a, angleOnA) && AngleOnArc(b, angleOnB)) {
            filtered.points[filtered.count++] = full.points[i];
        }
    }
    return filtered;
}

}
