#pragma once

#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Numbers.hpp>
#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/geometry/BoundingBox2.hpp>
#include <openhouse/geometry/Circle2.hpp>
#include <openhouse/geometry/Line2.hpp>

namespace openhouse::geometry {

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr BoundingBox2<T> Bounds(const Line2<T>& line) noexcept {
    return {
        {foundation::min(line.start.x, line.end.x), foundation::min(line.start.y, line.end.y)},
        {foundation::max(line.start.x, line.end.x), foundation::max(line.start.y, line.end.y)},
    };
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr BoundingBox2<T> Bounds(const Circle2<T>& circle) noexcept {
    return {
        {circle.center.x - circle.radius, circle.center.y - circle.radius},
        {circle.center.x + circle.radius, circle.center.y + circle.radius},
    };
}

// Returns true if angle `theta` (any real value, not necessarily in
// [0, 2*pi)) lies on the arc's swept range from arc.startAngle to
// arc.endAngle -- correctly handling both sweep directions (positive/
// counter-clockwise and negative/clockwise) and angle wraparound.
//
// Algorithm: shift theta relative to startAngle, then normalize that
// shift into a half-open window matching the sweep's own sign (
// [0, 2*pi) if sweep >= 0, or (-2*pi, 0] if sweep < 0) before comparing
// its magnitude against the sweep. Exposed (not just an implementation
// detail of Bounds()) because it's independently useful for future
// hit-testing on arcs.
template<foundation::FloatingPoint T>
[[nodiscard]] bool AngleOnArc(const Arc2<T>& arc, T theta) noexcept {
    const T twoPi = T{2} * foundation::pi_v<T>;
    const T sweep = Sweep(arc);
    T raw = theta - arc.startAngle;

    if (sweep >= T{0}) {
        raw = foundation::fmod(raw, twoPi);
        if (raw < T{0}) {
            raw += twoPi;
        }
        return raw >= T{0} && raw <= sweep;
    } else {
        raw = foundation::fmod(raw, twoPi);
        if (raw > T{0}) {
            raw -= twoPi;
        }
        return raw <= T{0} && raw >= sweep;
    }
}

// See the algorithmic note above AngleOnArc: an arc's bounding box is
// NOT simply the bounding box of its start/end points -- if the arc
// sweeps through one of the four axis-aligned extremes (0, pi/2, pi,
// 3*pi/2, i.e. the rightmost/topmost/leftmost/bottommost points of the
// full circle), the box must extend to include that point too. Example:
// an arc from 45 to 135 degrees passes through 90 degrees, so its
// highest point is center.y + radius, NOT either endpoint.
template<foundation::FloatingPoint T>
[[nodiscard]] BoundingBox2<T> Bounds(const Arc2<T>& arc) noexcept {
    BoundingBox2<T> box = FromPoint(StartPoint(arc));
    box = Expand(box, EndPoint(arc));

    for (int k = 0; k < 4; ++k) {
        const T cardinal = static_cast<T>(k) * (foundation::pi_v<T> / T{2});
        if (AngleOnArc(arc, cardinal)) {
            box = Expand(box, PointAt(arc, cardinal));
        }
    }
    return box;
}

}
