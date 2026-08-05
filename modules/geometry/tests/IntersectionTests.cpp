#include <openhouse/geometry/Intersection.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool NearlyEqual(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

bool PointNearlyEqual(const Point2d& a, const Point2d& b, double eps = 1e-6) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps);
}

// True if `p` is nearly equal to one of the result's points, in either
// slot -- the two solution formulas here don't promise a particular
// ordering between points[0]/points[1], so tests check membership, not
// position.
bool ContainsPoint(const IntersectionResultd& result, const Point2d& p, double eps = 1e-6) {
    for (std::size_t i = 0; i < result.count; ++i) {
        if (PointNearlyEqual(result.points[i], p, eps)) {
            return true;
        }
    }
    return false;
}
} // namespace

// --- Line2 x Line2 -------------------------------------------------------

static void TestLineLineCrossing() {
    const Line2d a{{0.0, 0.0}, {10.0, 10.0}};
    const Line2d b{{0.0, 10.0}, {10.0, 0.0}};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {5.0, 5.0}));
}

static void TestLineLineParallelNoIntersection() {
    const Line2d a{{0.0, 0.0}, {10.0, 0.0}};
    const Line2d b{{0.0, 1.0}, {10.0, 1.0}};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestLineLineCollinearOverlapReturnsEmpty() {
    const Line2d a{{0.0, 0.0}, {10.0, 0.0}};
    const Line2d b{{5.0, 0.0}, {15.0, 0.0}};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestLineLineIntersectsOutsideSegmentBounds() {
    // The infinite lines cross, but not within either segment's [0,1].
    const Line2d a{{0.0, 0.0}, {1.0, 1.0}};
    const Line2d b{{5.0, 0.0}, {6.0, -1.0}};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestLineLineSharedEndpoint() {
    // Two segments that meet exactly at a shared endpoint -- exercises
    // the epsilon-widened [0,1] bounds check.
    const Line2d a{{0.0, 0.0}, {5.0, 5.0}};
    const Line2d b{{5.0, 5.0}, {10.0, 0.0}};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {5.0, 5.0}));
}

// --- Line2 x Circle2 -------------------------------------------------------

static void TestLineCircleSecant() {
    const Line2d line{{-10.0, 0.0}, {10.0, 0.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto result = Intersect(line, circle);
    OH_CHECK(result.count == 2);
    OH_CHECK(ContainsPoint(result, {-5.0, 0.0}));
    OH_CHECK(ContainsPoint(result, {5.0, 0.0}));
}

static void TestLineCircleTangent() {
    const Line2d line{{-10.0, 5.0}, {10.0, 5.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto result = Intersect(line, circle);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {0.0, 5.0}));
}

static void TestLineCircleMiss() {
    const Line2d line{{-10.0, 20.0}, {10.0, 20.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto result = Intersect(line, circle);
    OH_CHECK(result.count == 0);
}

static void TestLineCircleSegmentTooShortToReachCircle() {
    // The infinite line through this segment crosses the circle twice,
    // but the segment itself stops well before reaching it.
    const Line2d line{{-10.0, 0.0}, {-8.0, 0.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto result = Intersect(line, circle);
    OH_CHECK(result.count == 0);
}

static void TestLineCircleSegmentCoversOnlyOneCrossing() {
    // The infinite line crosses the circle at x=-5 and x=5; this segment
    // only reaches from -10 to 0, so only the x=-5 crossing is in range.
    const Line2d line{{-10.0, 0.0}, {0.0, 0.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto result = Intersect(line, circle);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {-5.0, 0.0}));
}

static void TestCircleLineCommutesWithLineCircle() {
    const Line2d line{{-10.0, 0.0}, {10.0, 0.0}};
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const auto a = Intersect(line, circle);
    const auto b = Intersect(circle, line);
    OH_CHECK(a.count == b.count);
    OH_CHECK(ContainsPoint(b, {-5.0, 0.0}));
    OH_CHECK(ContainsPoint(b, {5.0, 0.0}));
}

// --- Circle2 x Circle2 -------------------------------------------------------

static void TestCircleCircleOverlapping() {
    const Circle2d a{{0.0, 0.0}, 5.0};
    const Circle2d b{{6.0, 0.0}, 5.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 2);
    // By symmetry, both intersection points sit at x=3.
    OH_CHECK(result.count == 2 && NearlyEqual(result.points[0].x, 3.0) &&
              NearlyEqual(result.points[1].x, 3.0));
}

static void TestCircleCircleExternallyTangent() {
    const Circle2d a{{0.0, 0.0}, 5.0};
    const Circle2d b{{10.0, 0.0}, 5.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {5.0, 0.0}));
}

static void TestCircleCircleInternallyTangent() {
    const Circle2d a{{0.0, 0.0}, 10.0};
    const Circle2d b{{4.0, 0.0}, 6.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {10.0, 0.0}));
}

static void TestCircleCircleTooFarApart() {
    const Circle2d a{{0.0, 0.0}, 5.0};
    const Circle2d b{{100.0, 0.0}, 5.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestCircleCircleOneInsideOtherNotTouching() {
    const Circle2d a{{0.0, 0.0}, 10.0};
    const Circle2d b{{1.0, 0.0}, 2.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestCircleCircleConcentricDifferentRadius() {
    const Circle2d a{{0.0, 0.0}, 5.0};
    const Circle2d b{{0.0, 0.0}, 8.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

static void TestCircleCircleConcentricSameRadiusReturnsEmpty() {
    // Coincident circles -- infinitely many intersection points, not one
    // well-defined result.
    const Circle2d a{{2.0, 3.0}, 5.0};
    const Circle2d b{{2.0, 3.0}, 5.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

// --- Line2 x Arc2 -------------------------------------------------------

static void TestLineArcCrossingWithinSweep() {
    // Quarter-circle arc from 0 to pi/2 (the top-right quadrant of the
    // circle). A horizontal line through y=3.53... crosses the full
    // circle at two points, only one of which (positive x) lies on this
    // arc.
    const Arc2d arc{{0.0, 0.0}, 5.0, 0.0, kPi / 2.0};
    const Line2d line{{-10.0, 3.0}, {10.0, 3.0}};
    const auto result = Intersect(line, arc);
    OH_CHECK(result.count == 1);
    OH_CHECK(result.count == 1 && result.points[0].x > 0.0);
}

static void TestLineArcCrossingOutsideSweepIsFiltered() {
    // Same line, but an arc confined to the top-LEFT quadrant instead --
    // the circle-level crossing on the positive-x side must be filtered
    // out since it's not on this arc's sweep.
    const Arc2d arc{{0.0, 0.0}, 5.0, kPi / 2.0, kPi};
    const Line2d line{{-10.0, 3.0}, {10.0, 3.0}};
    const auto result = Intersect(line, arc);
    OH_CHECK(result.count == 1);
    OH_CHECK(result.count == 1 && result.points[0].x < 0.0);
}

static void TestArcLineCommutesWithLineArc() {
    const Arc2d arc{{0.0, 0.0}, 5.0, 0.0, kPi / 2.0};
    const Line2d line{{-10.0, 3.0}, {10.0, 3.0}};
    const auto a = Intersect(line, arc);
    const auto b = Intersect(arc, line);
    OH_CHECK(a.count == b.count);
}

// --- Circle2 x Arc2 -------------------------------------------------------

static void TestCircleArcFiltersToSweep() {
    // Two full circles that would intersect at (3, +-4) (5-5-6 style
    // triangle via the two-circle formula below); confine the arc side
    // to only the upper half so only the +4 point survives.
    const Circle2d circle{{0.0, 0.0}, 5.0};
    const Arc2d arc{{6.0, 0.0}, 5.0, 0.0, kPi}; // upper half of the second circle
    const auto result = Intersect(circle, arc);
    OH_CHECK(result.count == 1);
    OH_CHECK(result.count == 1 && result.points[0].y > 0.0);
}

// --- Arc2 x Arc2 -------------------------------------------------------

static void TestArcArcBothSweepsIncludeBothPoints() {
    // Full-circle intersections sit at (3, 4) and (3, -4) (verified
    // directly). `a`'s sweep is its right half (-90 to 90 degrees),
    // which covers both points (their angles from a.center are
    // +-53.13 degrees). `b`'s sweep (90 to 270 degrees, counter-
    // clockwise) covers both as well (their angles from b.center are
    // 126.87 and 233.13 degrees, i.e. -126.87 -- both within [90,270]).
    // So both points survive filtering by both arcs.
    const Arc2d a{{0.0, 0.0}, 5.0, -kPi / 2.0, kPi / 2.0};
    const Arc2d b{{6.0, 0.0}, 5.0, kPi / 2.0, 3.0 * kPi / 2.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 2);
    OH_CHECK(ContainsPoint(result, {3.0, 4.0}));
    OH_CHECK(ContainsPoint(result, {3.0, -4.0}));
}

static void TestArcArcOnlyOnePointOnBothSweeps() {
    // Same two full circles (intersections at (3,4) and (3,-4)). Their
    // angles as seen from b.center are +126.87 and -126.87 (== 233.13)
    // degrees respectively -- confirmed by printing them directly
    // rather than computed by hand (see the earlier, wrong version of
    // this test, which assumed a [135,225]-degree range would keep
    // (3,-4) and drop (3,4); it was backwards, and the real build
    // caught it -- exactly the failure mode AI-Working-Agreement.md
    // rule 2 exists for). Using [100,200] degrees for b's sweep keeps
    // only the +126.87-degree point, (3, 4).
    const Arc2d a{{0.0, 0.0}, 5.0, -kPi / 2.0, kPi / 2.0};
    const Arc2d b{{6.0, 0.0}, 5.0, 100.0 * kPi / 180.0, 200.0 * kPi / 180.0};
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 1);
    OH_CHECK(ContainsPoint(result, {3.0, 4.0}));
}

static void TestArcArcNoOverlapInSweep() {
    // Same two full circles as TestCircleCircleOverlapping (intersect at
    // x=3, y=+-4), but both arcs confined to the BOTTOM half only where
    // circle a's arc doesn't reach the intersection points at all.
    const Arc2d a{{0.0, 0.0}, 5.0, kPi, 2.0 * kPi}; // bottom half of circle a
    const Arc2d b{{6.0, 0.0}, 5.0, 0.0, kPi};       // top half of circle b
    const auto result = Intersect(a, b);
    OH_CHECK(result.count == 0);
}

int main() {
    TestLineLineCrossing();
    TestLineLineParallelNoIntersection();
    TestLineLineCollinearOverlapReturnsEmpty();
    TestLineLineIntersectsOutsideSegmentBounds();
    TestLineLineSharedEndpoint();

    TestLineCircleSecant();
    TestLineCircleTangent();
    TestLineCircleMiss();
    TestLineCircleSegmentTooShortToReachCircle();
    TestLineCircleSegmentCoversOnlyOneCrossing();
    TestCircleLineCommutesWithLineCircle();

    TestCircleCircleOverlapping();
    TestCircleCircleExternallyTangent();
    TestCircleCircleInternallyTangent();
    TestCircleCircleTooFarApart();
    TestCircleCircleOneInsideOtherNotTouching();
    TestCircleCircleConcentricDifferentRadius();
    TestCircleCircleConcentricSameRadiusReturnsEmpty();

    TestLineArcCrossingWithinSweep();
    TestLineArcCrossingOutsideSweepIsFiltered();
    TestArcLineCommutesWithLineArc();

    TestCircleArcFiltersToSweep();

    TestArcArcBothSweepsIncludeBothPoints();
    TestArcArcOnlyOnePointOnBothSweeps();
    TestArcArcNoOverlapInSweep();

    std::puts("IntersectionTests: all tests passed.");
    return 0;
}
