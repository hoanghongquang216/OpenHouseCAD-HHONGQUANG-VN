#pragma once

#include <openhouse/foundation/Assert.hpp>
#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>
#include <openhouse/geometry/Vector2.hpp>

namespace openhouse::geometry {

// Translate, Rotate, Scale: pure geometric transforms on this project's
// three shape types. Deliberately does NOT depend on openhouse::math
// (no Matrix3, no math::Angle) -- geometry is a lower module than math
// (math depends on geometry, e.g. Matrix4 needs Point3/Vector3), so
// geometry taking a dependency back on math would create the exact
// circular-module problem already avoided once for Arc2's own angle
// storage (see Arc2.hpp's own comment) and caught again during SEL-002's
// design review (geometry must not depend on document). Angles here are
// plain radians (T), matching Arc2::startAngle/endAngle's own
// convention.
//
// These functions are intentionally "dumb": no validation, no business
// rules (e.g. nothing here rejects a zero or negative scale factor --
// see Scale() below). That validation belongs at the document layer
// (document::ScaleEntity, see Transform.hpp there), which is also where
// CAD-specific rules like "locked layers reject transforms" live. This
// mirrors the same layering already established for hit-testing:
// geometry answers "what is the geometric result", document answers
// "is this operation allowed to happen at all".
//
// Spiral 4 scope: only Translate is implemented here so far (TRF-001).
// Rotate and Scale (TRF-002/TRF-003) follow the same shape, added as
// their own separately-verified slices rather than all three at once.

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Line2<T> Translate(const Line2<T>& line, Vector2<T> delta) noexcept {
    return Line2<T>{
        Point2<T>{line.start.x + delta.x, line.start.y + delta.y},
        Point2<T>{line.end.x + delta.x, line.end.y + delta.y},
    };
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Circle2<T> Translate(const Circle2<T>& circle, Vector2<T> delta) noexcept {
    return Circle2<T>{
        Point2<T>{circle.center.x + delta.x, circle.center.y + delta.y},
        circle.radius,
    };
}

// Translating an arc moves its center; radius and both angles are
// unaffected (angles are defined relative to the center, not to any
// fixed world-space reference, so a pure translation doesn't change
// them).
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Arc2<T> Translate(const Arc2<T>& arc, Vector2<T> delta) noexcept {
    return Arc2<T>{
        Point2<T>{arc.center.x + delta.x, arc.center.y + delta.y},
        arc.radius,
        arc.startAngle,
        arc.endAngle,
    };
}

namespace detail {

// Rotates a single point around `pivot` by `radians` (counter-
// clockwise, matching std::atan2's convention and Arc2's own sweep
// direction). Shared by every Rotate() overload below rather than each
// one re-deriving the same formula.
template<foundation::FloatingPoint T>
[[nodiscard]] Point2<T> RotatePoint(const Point2<T>& p, const Point2<T>& pivot, T radians) noexcept {
    const T dx = p.x - pivot.x;
    const T dy = p.y - pivot.y;
    const T c = foundation::cos(radians);
    const T s = foundation::sin(radians);
    return Point2<T>{
        pivot.x + dx * c - dy * s,
        pivot.y + dx * s + dy * c,
    };
}

} // namespace detail

template<foundation::FloatingPoint T>
[[nodiscard]] Line2<T> Rotate(const Line2<T>& line, T radians, Point2<T> pivot) noexcept {
    return Line2<T>{
        detail::RotatePoint(line.start, pivot, radians),
        detail::RotatePoint(line.end, pivot, radians),
    };
}

// Rotating a circle around any pivot moves its center; a circle looks
// identical from every angle, so its radius is the only other field,
// and rotation never changes it.
template<foundation::FloatingPoint T>
[[nodiscard]] Circle2<T> Rotate(const Circle2<T>& circle, T radians, Point2<T> pivot) noexcept {
    return Circle2<T>{
        detail::RotatePoint(circle.center, pivot, radians),
        circle.radius,
    };
}

// Rotating an arc moves its center (around `pivot`, same as Circle2)
// and shifts both startAngle and endAngle by `radians` -- NOT
// normalized into any canonical range afterward. This deliberately
// matches Arc2's own existing invariant (see Arc2.hpp's comment on
// Sweep(): "Not clamped/normalized... this type intentionally has no
// dependency on math::NormalizedUnsigned/Signed") rather than
// introducing an inconsistency where Rotate() is the one place Arc2's
// angles suddenly get normalized. PointAt/StartPoint/EndPoint/
// AngleOnArc all already handle angles outside [0, 2*pi) correctly
// (via sin/cos/fmod's periodicity), so there is no correctness reason
// to normalize -- only a cosmetic one, which isn't worth breaking the
// existing invariant for. Radius and the sweep's magnitude (endAngle -
// startAngle) are both unchanged, since both angles shift by the exact
// same amount.
template<foundation::FloatingPoint T>
[[nodiscard]] Arc2<T> Rotate(const Arc2<T>& arc, T radians, Point2<T> pivot) noexcept {
    return Arc2<T>{
        detail::RotatePoint(arc.center, pivot, radians),
        arc.radius,
        arc.startAngle + radians,
        arc.endAngle + radians,
    };
}

namespace detail {

// Scales a single point relative to `pivot` by `factor`: the point
// moves along the ray from pivot through it, `factor` times as far
// away (or closer, for factor < 1) as it started.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Point2<T> ScalePoint(const Point2<T>& p, const Point2<T>& pivot,
                                              T factor) noexcept {
    return Point2<T>{
        pivot.x + (p.x - pivot.x) * factor,
        pivot.y + (p.y - pivot.y) * factor,
    };
}

} // namespace detail

// Scale is restricted to UNIFORM scaling (a single factor applied to
// both axes), per Spiral 4's design review -- a non-uniform scale (sx
// != sy) applied to a Circle2/Arc2 would produce an ellipse, which this
// project has no type to represent (Circle2/Arc2 both store a single
// scalar radius). Non-uniform scale is deliberately deferred until/
// unless an Ellipse2 type is introduced; forcing uniform scale here
// keeps every shape's own type exactly representable after the
// operation, with no silent loss of shape.
//
// `factor` is expected to be strictly positive. This is NOT validated
// here (only OH_ASSERT'd as a debug-time sanity net, consistent with
// this file's role as pure geometry with no business rules -- see this
// file's own top-of-file comment); the real, always-enforced check
// lives in document::ScaleEntity (document/Transform.hpp), which
// rejects factor <= 0 before ever calling this function.
template<foundation::FloatingPoint T>
[[nodiscard]] Line2<T> Scale(const Line2<T>& line, T factor, Point2<T> pivot) noexcept {
    OH_ASSERT(factor > T{0});
    return Line2<T>{
        detail::ScalePoint(line.start, pivot, factor),
        detail::ScalePoint(line.end, pivot, factor),
    };
}

template<foundation::FloatingPoint T>
[[nodiscard]] Circle2<T> Scale(const Circle2<T>& circle, T factor, Point2<T> pivot) noexcept {
    OH_ASSERT(factor > T{0});
    return Circle2<T>{
        detail::ScalePoint(circle.center, pivot, factor),
        circle.radius * factor,
    };
}

// Scaling an arc moves its center (relative to `pivot`, same formula as
// Circle2) and multiplies its radius by `factor` -- but leaves
// startAngle/endAngle completely UNCHANGED. Scaling stretches distances
// from the pivot; it does not rotate anything, so the arc's angular
// span (where it starts and ends, relative to its own center) is
// exactly the same before and after -- only its size and position
// change.
template<foundation::FloatingPoint T>
[[nodiscard]] Arc2<T> Scale(const Arc2<T>& arc, T factor, Point2<T> pivot) noexcept {
    OH_ASSERT(factor > T{0});
    return Arc2<T>{
        detail::ScalePoint(arc.center, pivot, factor),
        arc.radius * factor,
        arc.startAngle,
        arc.endAngle,
    };
}

}
