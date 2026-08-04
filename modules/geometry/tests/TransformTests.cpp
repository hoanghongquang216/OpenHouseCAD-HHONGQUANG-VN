#include <openhouse/geometry/Bounds.hpp>
#include <openhouse/geometry/Transform.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;
bool NearlyEqual(double a, double b, double eps = 1e-9) { return std::abs(a - b) < eps; }
} // namespace

static void TestTranslateLine() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{10.0, 0.0}};
    const Line2d moved = Translate(line, Vector2d{5.0, 3.0});
    OH_CHECK(moved.start.x == 5.0 && moved.start.y == 3.0);
    OH_CHECK(moved.end.x == 15.0 && moved.end.y == 3.0);
}

static void TestTranslateLineByZeroIsNoOp() {
    const Line2d line{Point2d{1.0, 2.0}, Point2d{3.0, 4.0}};
    const Line2d moved = Translate(line, Vector2d{0.0, 0.0});
    OH_CHECK(moved == line);
}

static void TestTranslateCircleMovesCenterKeepsRadius() {
    const Circle2d circle{Point2d{10.0, 10.0}, 5.0};
    const Circle2d moved = Translate(circle, Vector2d{-2.0, -2.0});
    OH_CHECK(moved.center.x == 8.0 && moved.center.y == 8.0);
    OH_CHECK(moved.radius == 5.0); // radius must be unaffected by translation
}

static void TestTranslateArcMovesCenterKeepsRadiusAndAngles() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.5, 1.5};
    const Arc2d moved = Translate(arc, Vector2d{100.0, 100.0});
    OH_CHECK(moved.center.x == 100.0 && moved.center.y == 100.0);
    OH_CHECK(moved.radius == 10.0);
    // Angles are relative to the center, not world space -- a pure
    // translation must leave them exactly unchanged.
    OH_CHECK(moved.startAngle == 0.5);
    OH_CHECK(moved.endAngle == 1.5);
}

static void TestTranslateNegativeDelta() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{0.0, 0.0}};
    const Line2d moved = Translate(line, Vector2d{-5.0, -7.0});
    OH_CHECK(moved.start.x == -5.0 && moved.start.y == -7.0);
}

// --- Rotate ---------------------------------------------------------------

static void TestRotateLineAroundOrigin() {
    const Line2d line{Point2d{10.0, 0.0}, Point2d{20.0, 0.0}};
    const Line2d rotated = Rotate(line, kPi / 2.0, Point2d{0.0, 0.0});
    OH_CHECK(NearlyEqual(rotated.start.x, 0.0) && NearlyEqual(rotated.start.y, 10.0));
    OH_CHECK(NearlyEqual(rotated.end.x, 0.0) && NearlyEqual(rotated.end.y, 20.0));
}

static void TestRotateByZeroIsNoOp() {
    const Line2d line{Point2d{3.0, 4.0}, Point2d{7.0, 2.0}};
    const Line2d rotated = Rotate(line, 0.0, Point2d{1.0, 1.0});
    OH_CHECK(NearlyEqual(rotated.start.x, line.start.x) && NearlyEqual(rotated.start.y, line.start.y));
    OH_CHECK(NearlyEqual(rotated.end.x, line.end.x) && NearlyEqual(rotated.end.y, line.end.y));
}

static void TestRotateCircleAroundOffsetPivotMovesCenterKeepsRadiusExactly() {
    const Circle2d circle{Point2d{10.0, 10.0}, 5.0};
    const Circle2d rotated = Rotate(circle, kPi, Point2d{5.0, 5.0}); // 180 deg around (5,5)
    OH_CHECK(NearlyEqual(rotated.center.x, 0.0) && NearlyEqual(rotated.center.y, 0.0));
    // Radius must be EXACTLY unchanged (no multiplication/division occurs
    // for radius during a rotation) -- exact equality, not just "close".
    OH_CHECK(rotated.radius == 5.0);
}

static void TestRotateCircleBy360DegreesReturnsToStart() {
    const Circle2d circle{Point2d{10.0, 20.0}, 3.0};
    const Circle2d rotated = Rotate(circle, 2.0 * kPi, Point2d{0.0, 0.0});
    OH_CHECK(NearlyEqual(rotated.center.x, circle.center.x, 1e-9));
    OH_CHECK(NearlyEqual(rotated.center.y, circle.center.y, 1e-9));
}

// The regression test explicitly requested during TRF-002's design
// review: Rotate(Arc2) must NOT normalize startAngle/endAngle into any
// canonical range -- it must match Arc2's own existing, documented
// invariant (see Arc2.hpp's comment on Sweep()) that angles are never
// clamped/normalized. 350+30=380 degrees, NOT wrapped to 20.
static void TestRotateArcDoesNotNormalizeAngles() {
    const Arc2d arc{Point2d{0.0, 0.0}, 5.0, 350.0 * kPi / 180.0, 370.0 * kPi / 180.0};
    const Arc2d rotated = Rotate(arc, 30.0 * kPi / 180.0, Point2d{0.0, 0.0});

    const double expectedStart = 380.0 * kPi / 180.0;
    const double expectedEnd = 400.0 * kPi / 180.0;
    OH_CHECK(NearlyEqual(rotated.startAngle, expectedStart));
    OH_CHECK(NearlyEqual(rotated.endAngle, expectedEnd));
    // Explicitly NOT the normalized-looking values -- if someone later
    // "helpfully" adds normalization to Rotate(), this assertion is what
    // catches it.
    OH_CHECK(!NearlyEqual(rotated.startAngle, 20.0 * kPi / 180.0));
    OH_CHECK(!NearlyEqual(rotated.endAngle, 40.0 * kPi / 180.0));
}

static void TestRotateArcAroundOwnCenterKeepsCenterFixed() {
    const Arc2d arc{Point2d{10.0, 20.0}, 5.0, 30.0 * kPi / 180.0, 120.0 * kPi / 180.0};
    const Arc2d rotated = Rotate(arc, kPi / 2.0, arc.center); // pivot == own center
    OH_CHECK(NearlyEqual(rotated.center.x, arc.center.x));
    OH_CHECK(NearlyEqual(rotated.center.y, arc.center.y));
    OH_CHECK(rotated.radius == arc.radius);
    OH_CHECK(NearlyEqual(rotated.startAngle, arc.startAngle + kPi / 2.0));
    OH_CHECK(NearlyEqual(rotated.endAngle, arc.endAngle + kPi / 2.0));
}

static void TestRotateArcAroundDifferentPivotMovesCenterAndShiftsAngles() {
    const Arc2d arc{Point2d{10.0, 20.0}, 5.0, 30.0 * kPi / 180.0, 120.0 * kPi / 180.0};
    const Arc2d rotated = Rotate(arc, kPi / 2.0, Point2d{0.0, 0.0}); // pivot != center

    // (10,20) rotated 90 degrees around the origin -> (-20,10).
    OH_CHECK(NearlyEqual(rotated.center.x, -20.0));
    OH_CHECK(NearlyEqual(rotated.center.y, 10.0));
    OH_CHECK(rotated.radius == 5.0);
    // Angles still shift by the rotation amount regardless of pivot.
    OH_CHECK(NearlyEqual(rotated.startAngle, 120.0 * kPi / 180.0));
    OH_CHECK(NearlyEqual(rotated.endAngle, 210.0 * kPi / 180.0));
}

static void TestRotatingArcFourTimesBy90DegreesReturnsToStartGeometrically() {
    // Per the design review: verify by GEOMETRIC properties
    // (StartPoint/EndPoint/AngleOnArc), never by comparing the raw
    // angle numbers directly -- after 4x90=360 degrees of unnormalized
    // accumulation, the numeric startAngle is 390 degrees' worth larger
    // than the original, not equal to it, even though the actual arc
    // drawn is identical.
    const Arc2d original{Point2d{0.0, 0.0}, 10.0, 30.0 * kPi / 180.0, 120.0 * kPi / 180.0};
    const Point2d originalStart = StartPoint(original);
    const Point2d originalEnd = EndPoint(original);

    Arc2d rotated = original;
    for (int i = 0; i < 4; ++i) {
        rotated = Rotate(rotated, kPi / 2.0, Point2d{0.0, 0.0});
    }

    const Point2d newStart = StartPoint(rotated);
    const Point2d newEnd = EndPoint(rotated);
    OH_CHECK(NearlyEqual(newStart.x, originalStart.x, 1e-9));
    OH_CHECK(NearlyEqual(newStart.y, originalStart.y, 1e-9));
    OH_CHECK(NearlyEqual(newEnd.x, originalEnd.x, 1e-9));
    OH_CHECK(NearlyEqual(newEnd.y, originalEnd.y, 1e-9));
    OH_CHECK(rotated.radius == original.radius);

    // The numeric angle, by contrast, must NOT equal the original --
    // this confirms the test is actually exercising unnormalized
    // accumulation, not accidentally passing because normalization
    // happened to bring it back to the same number too.
    OH_CHECK(!NearlyEqual(rotated.startAngle, original.startAngle));
}

// --- Scale ------------------------------------------------------------

static void TestScaleLineAroundOffsetPivot() {
    const Line2d line{Point2d{10.0, 0.0}, Point2d{20.0, 0.0}};
    const Line2d scaled = Scale(line, 2.0, Point2d{0.0, 0.0});
    OH_CHECK(scaled.start.x == 20.0 && scaled.start.y == 0.0);
    OH_CHECK(scaled.end.x == 40.0 && scaled.end.y == 0.0);
}

static void TestScaleByOneIsNoOp() {
    const Line2d line{Point2d{3.0, 4.0}, Point2d{7.0, 2.0}};
    const Line2d scaled = Scale(line, 1.0, Point2d{100.0, 100.0}); // pivot irrelevant at factor=1
    OH_CHECK(scaled.start.x == line.start.x && scaled.start.y == line.start.y);
    OH_CHECK(scaled.end.x == line.end.x && scaled.end.y == line.end.y);
}

static void TestScaleCircleMovesCenterAndMultipliesRadius() {
    const Circle2d circle{Point2d{10.0, 10.0}, 5.0};
    const Circle2d scaled = Scale(circle, 3.0, Point2d{0.0, 0.0});
    OH_CHECK(scaled.center.x == 30.0 && scaled.center.y == 30.0);
    OH_CHECK(scaled.radius == 15.0);
}

static void TestScaleCircleAroundOwnCenterKeepsCenterFixed() {
    const Circle2d circle{Point2d{10.0, 10.0}, 5.0};
    const Circle2d scaled = Scale(circle, 0.5, circle.center); // pivot == own center
    OH_CHECK(scaled.center.x == 10.0 && scaled.center.y == 10.0);
    OH_CHECK(scaled.radius == 2.5);
}

static void TestScaleCircleByFactorLessThanOneShrinks() {
    const Circle2d circle{Point2d{0.0, 0.0}, 10.0};
    const Circle2d scaled = Scale(circle, 0.25, Point2d{0.0, 0.0});
    OH_CHECK(scaled.radius == 2.5);
}

// The core behavior locked in during TRF-003's design review: scaling
// an arc changes its radius (and moves its center relative to the
// pivot), but must NOT touch startAngle/endAngle at all -- scaling
// stretches distance, it doesn't rotate.
static void TestScaleArcChangesRadiusNotAngles() {
    const Arc2d arc{Point2d{0.0, 0.0}, 5.0, 0.5, 1.5};
    const Arc2d scaled = Scale(arc, 4.0, Point2d{0.0, 0.0});
    OH_CHECK(scaled.radius == 20.0);
    OH_CHECK(scaled.startAngle == 0.5); // bit-exact -- untouched, not just "close"
    OH_CHECK(scaled.endAngle == 1.5);
}

static void TestScaleArcAroundDifferentPivotMovesCenterScalesRadius() {
    const Arc2d arc{Point2d{10.0, 10.0}, 5.0, 0.0, kPi};
    const Arc2d scaled = Scale(arc, 2.0, Point2d{0.0, 0.0});
    OH_CHECK(scaled.center.x == 20.0 && scaled.center.y == 20.0);
    OH_CHECK(scaled.radius == 10.0);
    OH_CHECK(scaled.startAngle == 0.0 && scaled.endAngle == kPi);
}

int main() {
    TestTranslateLine();
    TestTranslateLineByZeroIsNoOp();
    TestTranslateCircleMovesCenterKeepsRadius();
    TestTranslateArcMovesCenterKeepsRadiusAndAngles();
    TestTranslateNegativeDelta();

    TestRotateLineAroundOrigin();
    TestRotateByZeroIsNoOp();
    TestRotateCircleAroundOffsetPivotMovesCenterKeepsRadiusExactly();
    TestRotateCircleBy360DegreesReturnsToStart();
    TestRotateArcDoesNotNormalizeAngles();
    TestRotateArcAroundOwnCenterKeepsCenterFixed();
    TestRotateArcAroundDifferentPivotMovesCenterAndShiftsAngles();
    TestRotatingArcFourTimesBy90DegreesReturnsToStartGeometrically();

    TestScaleLineAroundOffsetPivot();
    TestScaleByOneIsNoOp();
    TestScaleCircleMovesCenterAndMultipliesRadius();
    TestScaleCircleAroundOwnCenterKeepsCenterFixed();
    TestScaleCircleByFactorLessThanOneShrinks();
    TestScaleArcChangesRadiusNotAngles();
    TestScaleArcAroundDifferentPivotMovesCenterScalesRadius();

    std::puts("TransformTests: all tests passed.");
    return 0;
}
