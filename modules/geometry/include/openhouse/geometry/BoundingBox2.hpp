#pragma once

#include <openhouse/foundation/Algorithm.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/geometry/Point2.hpp>

namespace openhouse::geometry {

// An axis-aligned bounding box: min/max corner points. Deliberately
// independent of any specific shape type (Line2/Circle2/Arc2) -- this
// header only defines the box itself and generic operations on it.
// Per-shape bounding-box computation (Bounds(Line2), Bounds(Circle2),
// Bounds(Arc2)) lives in Bounds.hpp, which depends on this header plus
// each shape header; keeping that separate means BoundingBox2 itself
// stays a minimal, broadly reusable primitive with no shape-specific
// dependencies.
template<typename T>
struct BoundingBox2 {
    Point2<T> min{};
    Point2<T> max{};

    friend constexpr bool operator==(const BoundingBox2&, const BoundingBox2&) = default;
};

using BoundingBox2f = BoundingBox2<float>;
using BoundingBox2d = BoundingBox2<double>;
using BoundingBox2i = BoundingBox2<int>;

template<typename T>
[[nodiscard]] constexpr T Width(const BoundingBox2<T>& box) noexcept {
    return box.max.x - box.min.x;
}

template<typename T>
[[nodiscard]] constexpr T Height(const BoundingBox2<T>& box) noexcept {
    return box.max.y - box.min.y;
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr Point2<T> Center(const BoundingBox2<T>& box) noexcept {
    return {
        box.min.x + (box.max.x - box.min.x) / T{2},
        box.min.y + (box.max.y - box.min.y) / T{2},
    };
}

// Inclusive: a point exactly on the boundary counts as contained.
template<typename T>
[[nodiscard]] constexpr bool Contains(const BoundingBox2<T>& box, const Point2<T>& p) noexcept {
    return p.x >= box.min.x && p.x <= box.max.x && p.y >= box.min.y && p.y <= box.max.y;
}

// Inclusive: boxes that only touch at an edge/corner count as intersecting.
template<typename T>
[[nodiscard]] constexpr bool Intersects(const BoundingBox2<T>& a, const BoundingBox2<T>& b) noexcept {
    return a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y;
}

// Smallest box containing both inputs.
template<typename T>
[[nodiscard]] constexpr BoundingBox2<T> Union(const BoundingBox2<T>& a, const BoundingBox2<T>& b) noexcept {
    return {
        {foundation::min(a.min.x, b.min.x), foundation::min(a.min.y, b.min.y)},
        {foundation::max(a.max.x, b.max.x), foundation::max(a.max.y, b.max.y)},
    };
}

// Smallest box containing the input box and the given point.
template<typename T>
[[nodiscard]] constexpr BoundingBox2<T> Expand(const BoundingBox2<T>& box, const Point2<T>& p) noexcept {
    return {
        {foundation::min(box.min.x, p.x), foundation::min(box.min.y, p.y)},
        {foundation::max(box.max.x, p.x), foundation::max(box.max.y, p.y)},
    };
}

// A box containing only the given point (min == max == p). Useful as the
// starting accumulator before repeated Expand() calls when building a
// bounding box up from a set of points (e.g. Document::Bounds()).
template<typename T>
[[nodiscard]] constexpr BoundingBox2<T> FromPoint(const Point2<T>& p) noexcept {
    return {p, p};
}

// Expands (or shrinks, for a negative margin) the box equally on all
// four sides. Primarily exists for hit-testing's bounding-box fast-
// reject (see document::HitTest): a shape's exact bounding box, dilated
// by the hit-test tolerance, gives a cheap AABB-vs-point test that
// rejects most entities before the more expensive exact
// geometry::DistanceToShape() computation runs on them. A negative
// margin producing an inverted box (min > max) is the caller's
// responsibility to avoid; this function doesn't validate that.
template<typename T>
[[nodiscard]] constexpr BoundingBox2<T> Dilate(const BoundingBox2<T>& box, T margin) noexcept {
    return {
        {box.min.x - margin, box.min.y - margin},
        {box.max.x + margin, box.max.y + margin},
    };
}

}
