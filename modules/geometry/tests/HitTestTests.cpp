#include <openhouse/geometry/HitTest.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;
bool NearlyEqual(double a, double b, double eps = 1e-9) { return std::abs(a - b) < eps; }
} // namespace

// --- Line2 -------------------------------------------------------------

static void TestLineProjectionFallsWithinSegment() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}};
    OH_CHECK(NearlyEqual(DistanceToShape(line, Point2d{5.0, 3.0}), 3.0));
}

static void TestLineProjectionFallsBeforeStart() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}};
    OH_CHECK(NearlyEqual(DistanceToShape(line, Point2d{-5.0, 0.0}), 5.0));
}

static void TestLineProjectionFallsAfterEnd() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}};
    OH_CHECK(NearlyEqual(DistanceToShape(line, Point2d{15.0, 0.0}), 5.0));
}

static void TestDegenerateLineFallsBackToPointDistance() {
    const Line2d line{Point2d{3.0, 3.0}, Point2d{3.0, 3.0}}; // start == end
    OH_CHECK(NearlyEqual(DistanceToShape(line, Point2d{6.0, 3.0}), 3.0));
}

// --- Circle2 -------------------------------------------------------------

static void TestCirclePointOutside() {
    const Circle2d circle{Point2d{0.0, 0.0}, 10.0};
    OH_CHECK(NearlyEqual(DistanceToShape(circle, Point2d{14.142135623730951, 0.0}), 4.142135623730951));
}

static void TestCirclePointAtCenter() {
    // Explicit edge case from review: distance from the exact center
    // must equal the radius (the center is exactly `radius` away from
    // every point on the outline).
    const Circle2d circle{Point2d{5.0, 5.0}, 7.0};
    OH_CHECK(NearlyEqual(DistanceToShape(circle, Point2d{5.0, 5.0}), 7.0));
}

// --- Arc2 -------------------------------------------------------------

static void TestArcPointWithinSweep() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.0, kPi / 2.0}; // quarter circle
    OH_CHECK(NearlyEqual(DistanceToShape(arc, Point2d{10.0, 10.0}), 4.142135623730951));
}

static void TestArcPointExactlyOnSweepBoundary() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.0, kPi / 2.0};
    // theta == startAngle exactly -- must be treated as inside (inclusive).
    OH_CHECK(NearlyEqual(DistanceToShape(arc, Point2d{20.0, 0.0}), 10.0));
}

static void TestArcPointOutsideSweepUsesNearestEndpoint() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.0, kPi / 2.0};
    // (-5,-5) is symmetric between the two endpoints (10,0) and (0,10).
    OH_CHECK(NearlyEqual(DistanceToShape(arc, Point2d{-5.0, -5.0}), 15.811388300841896, 1e-9));
}

static void TestArcPointNearBoundaryButStillInside() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.0, kPi / 2.0};
    // atan2(1,9) ~= 6.34 degrees, inside [0, 90] -- circle-style distance
    // (10 - hypot(9,1) = 10 - 9.055385138137417).
    OH_CHECK(NearlyEqual(DistanceToShape(arc, Point2d{9.0, 1.0}), 0.9446148618625827, 1e-9));
}

static void TestArcFullSweepBehavesLikeCircle() {
    // Explicit edge case from review: a 360-degree arc must accept
    // EVERY angle as "within the sweep" -- verified across many angles,
    // not just one, since a boundary/wraparound bug could pass at some
    // angles and fail at others.
    const Arc2d fullArc{Point2d{0.0, 0.0}, 10.0, 0.0, 2.0 * kPi};
    for (int deg = 0; deg < 360; deg += 15) {
        const double theta = static_cast<double>(deg) * kPi / 180.0;
        OH_CHECK(AngleOnArc(fullArc, theta));
    }
    OH_CHECK(NearlyEqual(DistanceToShape(fullArc, Point2d{15.0, 0.0}), 5.0));
}

static void TestArcDegenerateZeroRadius() {
    // radius == 0: the arc collapses to a single point at its center.
    // StartPoint/EndPoint/center all coincide, so the existing formula
    // should degrade gracefully to plain point-distance without special
    // casing.
    const Arc2d point{Point2d{3.0, 4.0}, 0.0, 0.0, kPi};
    OH_CHECK(NearlyEqual(DistanceToShape(point, Point2d{3.0, 4.0}), 0.0));
    OH_CHECK(NearlyEqual(DistanceToShape(point, Point2d{6.0, 4.0}), 3.0));
}

int main() {
    TestLineProjectionFallsWithinSegment();
    TestLineProjectionFallsBeforeStart();
    TestLineProjectionFallsAfterEnd();
    TestDegenerateLineFallsBackToPointDistance();

    TestCirclePointOutside();
    TestCirclePointAtCenter();

    TestArcPointWithinSweep();
    TestArcPointExactlyOnSweepBoundary();
    TestArcPointOutsideSweepUsesNearestEndpoint();
    TestArcPointNearBoundaryButStillInside();
    TestArcFullSweepBehavesLikeCircle();
    TestArcDegenerateZeroRadius();

    std::puts("HitTestTests: all tests passed.");
    return 0;
}
