#pragma once

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

}
