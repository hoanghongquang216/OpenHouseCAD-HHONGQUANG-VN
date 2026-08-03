#include <openhouse/geometry/Bounds.hpp>

#include <cassert>
#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestLineBounds() {
    const Line2d line{Point2d{5.0, -3.0}, Point2d{1.0, 7.0}};
    const auto box = Bounds(line);
    assert(box.min.x == 1.0 && box.min.y == -3.0);
    assert(box.max.x == 5.0 && box.max.y == 7.0);
}

static void TestCircleBounds() {
    const Circle2d circle{Point2d{10.0, 10.0}, 3.0};
    const auto box = Bounds(circle);
    assert(NearlyEqual(box.min.x, 7.0));
    assert(NearlyEqual(box.min.y, 7.0));
    assert(NearlyEqual(box.max.x, 13.0));
    assert(NearlyEqual(box.max.y, 13.0));
}

static void TestAngleOnArcPositiveSweep() {
    const Arc2d arc{Point2d{0.0, 0.0}, 1.0, 0.0, kPi / 2.0};
    assert(AngleOnArc(arc, 0.0));          // start, inclusive
    assert(AngleOnArc(arc, kPi / 2.0));    // end, inclusive
    assert(AngleOnArc(arc, kPi / 4.0));    // middle
    assert(!AngleOnArc(arc, kPi));         // outside
    assert(!AngleOnArc(arc, -kPi / 4.0));  // outside (before start)
}

static void TestAngleOnArcNegativeSweep() {
    // Clockwise from pi/2 down to 0.
    const Arc2d arc{Point2d{0.0, 0.0}, 1.0, kPi / 2.0, 0.0};
    assert(AngleOnArc(arc, kPi / 2.0)); // start
    assert(AngleOnArc(arc, 0.0));       // end
    assert(AngleOnArc(arc, kPi / 4.0)); // middle
    assert(!AngleOnArc(arc, kPi));      // outside
}

static void TestAngleOnArcWraparound() {
    // Arc crossing the 0/2pi boundary: from 350 degrees to 10 degrees
    // (sweeping forward through 0).
    const double start = 350.0 * kPi / 180.0;
    const double end = 370.0 * kPi / 180.0; // equivalent to 10 degrees, expressed unwrapped
    const Arc2d arc{Point2d{0.0, 0.0}, 1.0, start, end};
    assert(AngleOnArc(arc, 0.0));                    // 0 degrees is within [350, 370]
    assert(AngleOnArc(arc, 355.0 * kPi / 180.0));     // 355 degrees, within range
    assert(!AngleOnArc(arc, 180.0 * kPi / 180.0));    // 180 degrees, well outside
}

static void TestArcBoundsWhenNoCardinalCrossed() {
    // A small arc entirely within the first quadrant's interior angles
    // (e.g. 10 to 30 degrees) crosses no cardinal (0/90/180/270) --
    // bounds should just be the endpoints' bounding box.
    const double start = 10.0 * kPi / 180.0;
    const double end = 30.0 * kPi / 180.0;
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, start, end};
    const auto box = Bounds(arc);
    const Point2d s = StartPoint(arc);
    const Point2d e = EndPoint(arc);
    assert(NearlyEqual(box.min.x, std::min(s.x, e.x)));
    assert(NearlyEqual(box.max.x, std::max(s.x, e.x)));
    assert(NearlyEqual(box.min.y, std::min(s.y, e.y)));
    assert(NearlyEqual(box.max.y, std::max(s.y, e.y)));
}

static void TestArcBoundsWhenCrossingTop() {
    // Arc from 45 to 135 degrees crosses 90 degrees (the topmost point
    // of the circle) -- the classic case where endpoint-only bounds
    // would be WRONG. Highest point must be center.y + radius, not
    // either endpoint's y.
    const double start = 45.0 * kPi / 180.0;
    const double end = 135.0 * kPi / 180.0;
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, start, end};
    const auto box = Bounds(arc);

    assert(NearlyEqual(box.max.y, 10.0)); // center.y (0) + radius (10)
    // x should span from the leftmost endpoint's x to the rightmost.
    const Point2d s = StartPoint(arc);
    const Point2d e = EndPoint(arc);
    assert(NearlyEqual(box.min.x, std::min(s.x, e.x)));
    assert(NearlyEqual(box.max.x, std::max(s.x, e.x)));
}

static void TestArcBoundsFullCircleMatchesCircleBounds() {
    // A 360-degree "arc" should have the exact same bounding box as the
    // full circle it traces.
    const Arc2d arc{Point2d{5.0, -5.0}, 7.0, 0.0, 2.0 * kPi};
    const Circle2d circle{Point2d{5.0, -5.0}, 7.0};

    const auto arcBox = Bounds(arc);
    const auto circleBox = Bounds(circle);

    assert(NearlyEqual(arcBox.min.x, circleBox.min.x));
    assert(NearlyEqual(arcBox.min.y, circleBox.min.y));
    assert(NearlyEqual(arcBox.max.x, circleBox.max.x));
    assert(NearlyEqual(arcBox.max.y, circleBox.max.y));
}

static void TestArcBoundsCrossingAllFourCardinalsOffCenter() {
    // Off-center arc crossing multiple cardinals: from 200 to 340
    // degrees crosses 270 degrees (bottom) only among the four
    // cardinals within [200,340]. Verify against manual expectation.
    const double start = 200.0 * kPi / 180.0;
    const double end = 340.0 * kPi / 180.0;
    const Point2d center{3.0, 4.0};
    const double radius = 2.0;
    const Arc2d arc{center, radius, start, end};
    const auto box = Bounds(arc);

    // 270 degrees is crossed -> lowest point is center.y - radius.
    assert(NearlyEqual(box.min.y, center.y - radius));
    // 180 and 0/360 are NOT crossed (range is 200-340) -> min.x/max.x
    // come from whichever of start/end/nothing-else is most extreme;
    // since no left(180)/right(0) cardinal is crossed, x bounds come
    // from the endpoints only.
    const Point2d s = StartPoint(arc);
    const Point2d e = EndPoint(arc);
    assert(NearlyEqual(box.min.x, std::min(s.x, e.x)));
    assert(NearlyEqual(box.max.x, std::max(s.x, e.x)));
}

int main() {
    TestLineBounds();
    TestCircleBounds();
    TestAngleOnArcPositiveSweep();
    TestAngleOnArcNegativeSweep();
    TestAngleOnArcWraparound();
    TestArcBoundsWhenNoCardinalCrossed();
    TestArcBoundsWhenCrossingTop();
    TestArcBoundsFullCircleMatchesCircleBounds();
    TestArcBoundsCrossingAllFourCardinalsOffCenter();

    std::puts("BoundsTests: all tests passed.");
    return 0;
}
