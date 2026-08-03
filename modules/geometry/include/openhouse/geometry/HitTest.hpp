#pragma once

#include <openhouse/foundation/Algorithm.hpp>
#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/Bounds.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>
#include <openhouse/geometry/Point2.hpp>

namespace openhouse::geometry {

// Shortest distance from `point` to the nearest point ON the segment
// (not the infinite line it lies on). Standard point-to-segment
// projection: project onto the line, clamp the projection parameter to
// [0, 1] to stay within the segment, measure to that clamped point.
//
// A degenerate line (start == end) is handled explicitly: the
// projection formula would divide by zero (the segment's squared
// length), so this falls back to a plain point-to-point distance.
template<foundation::FloatingPoint T>
[[nodiscard]] T DistanceToShape(const Line2<T>& line, const Point2<T>& point) noexcept {
    const T dx = line.end.x - line.start.x;
    const T dy = line.end.y - line.start.y;
    const T lengthSquared = dx * dx + dy * dy;
    if (lengthSquared == T{0}) {
        return foundation::hypot(point.x - line.start.x, point.y - line.start.y);
    }
    T t = ((point.x - line.start.x) * dx + (point.y - line.start.y) * dy) / lengthSquared;
    t = foundation::clamp(t, T{0}, T{1});
    const T closestX = line.start.x + t * dx;
    const T closestY = line.start.y + t * dy;
    return foundation::hypot(point.x - closestX, point.y - closestY);
}

// Distance to the circle's OUTLINE, not to a filled disk -- matches how
// Circle2 is actually rendered (see SvgDocument::AddCircle: fill="none").
// A point inside the circle and a point outside it at the same distance
// from the boundary are equally "close" to the shape as drawn.
template<foundation::FloatingPoint T>
[[nodiscard]] T DistanceToShape(const Circle2<T>& circle, const Point2<T>& point) noexcept {
    return foundation::abs(
        foundation::hypot(point.x - circle.center.x, point.y - circle.center.y) - circle.radius);
}

// Distance to an arc: if the point's angular position (as seen from the
// arc's center) falls within the arc's swept range, the nearest point
// on the arc lies along that same ray -- exactly the circle case above.
// Otherwise, the nearest point on the arc is one of its two endpoints
// (whichever is geometrically closer), not anywhere along the
// underlying full circle.
//
// Reuses AngleOnArc (Bounds.hpp) rather than reimplementing the sweep-
// direction/wraparound logic -- that helper was written specifically
// with this use in mind (see its own doc comment).
template<foundation::FloatingPoint T>
[[nodiscard]] T DistanceToShape(const Arc2<T>& arc, const Point2<T>& point) noexcept {
    const T pointAngle = foundation::atan2(point.y - arc.center.y, point.x - arc.center.x);
    if (AngleOnArc(arc, pointAngle)) {
        return foundation::abs(
            foundation::hypot(point.x - arc.center.x, point.y - arc.center.y) - arc.radius);
    }
    const Point2<T> start = StartPoint(arc);
    const Point2<T> end = EndPoint(arc);
    return foundation::min(foundation::hypot(point.x - start.x, point.y - start.y),
                            foundation::hypot(point.x - end.x, point.y - end.y));
}

}
