#include <openhouse/geometry/Arc2.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
constexpr double kPi = 3.14159265358979323846;

bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
bool NearlyEqual(const Point2d& a, const Point2d& b, double eps = 1e-9) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps);
}
} // namespace

static void TestConstruction() {
    const Arc2d arc{Point2d{0.0, 0.0}, 5.0, 0.0, kPi};
    OH_CHECK(arc.center.x == 0.0 && arc.center.y == 0.0);
    OH_CHECK(arc.radius == 5.0);
    OH_CHECK(arc.startAngle == 0.0);
    OH_CHECK(NearlyEqual(arc.endAngle, kPi));
}

static void TestEquality() {
    const Arc2d a{Point2d{0.0, 0.0}, 1.0, 0.0, 1.0};
    const Arc2d b{Point2d{0.0, 0.0}, 1.0, 0.0, 1.0};
    const Arc2d c{Point2d{0.0, 0.0}, 1.0, 0.0, 2.0};
    OH_CHECK(a == b);
    OH_CHECK(!(a == c));
}

static void TestSweep() {
    const Arc2d a{Point2d{0.0, 0.0}, 1.0, 0.0, kPi / 2.0};
    OH_CHECK(NearlyEqual(Sweep(a), kPi / 2.0));

    // Clockwise sweep gives a negative result -- documented behavior,
    // not clamped.
    const Arc2d b{Point2d{0.0, 0.0}, 1.0, kPi / 2.0, 0.0};
    OH_CHECK(NearlyEqual(Sweep(b), -kPi / 2.0));
}

static void TestPointAtZeroAngleIsOnPositiveXAxis() {
    const Arc2d arc{Point2d{0.0, 0.0}, 5.0, 0.0, kPi};
    const Point2d p = PointAt(arc, 0.0);
    OH_CHECK(NearlyEqual(p, Point2d{5.0, 0.0}));
}

static void TestPointAtQuarterTurn() {
    // pi/2 radians (90 degrees, counter-clockwise) from angle 0 lands on
    // the positive Y axis, consistent with standard math convention
    // (matches Matrix4::RotationZ's direction elsewhere in this project).
    const Arc2d arc{Point2d{0.0, 0.0}, 5.0, 0.0, kPi};
    const Point2d p = PointAt(arc, kPi / 2.0);
    OH_CHECK(NearlyEqual(p, Point2d{0.0, 5.0}));
}

static void TestStartAndEndPoint() {
    const Arc2d arc{Point2d{10.0, 10.0}, 2.0, 0.0, kPi / 2.0};
    const Point2d start = StartPoint(arc);
    const Point2d end = EndPoint(arc);

    OH_CHECK(NearlyEqual(start, Point2d{12.0, 10.0}));
    OH_CHECK(NearlyEqual(end, Point2d{10.0, 12.0}));
}

static void TestOffCenterArc() {
    const Arc2d arc{Point2d{5.0, 5.0}, 3.0, 0.0, kPi};
    const Point2d start = StartPoint(arc);
    OH_CHECK(NearlyEqual(start, Point2d{8.0, 5.0}));
}

static void TestLengthOfHalfCircle() {
    const Arc2d arc{Point2d{0.0, 0.0}, 10.0, 0.0, kPi};
    // Half a circle of radius 10: length = 10 * pi.
    OH_CHECK(NearlyEqual(Length(arc), 10.0 * kPi));
}

static void TestLengthOfFullCircleMatchesCircumference() {
    const Arc2d arc{Point2d{0.0, 0.0}, 4.0, 0.0, 2.0 * kPi};
    const Circle2d fullCircle{Point2d{0.0, 0.0}, 4.0};
    OH_CHECK(NearlyEqual(Length(arc), Circumference(fullCircle)));
}

static void TestLengthIsNonNegativeRegardlessOfSweepDirection() {
    const Arc2d clockwise{Point2d{0.0, 0.0}, 5.0, kPi / 2.0, 0.0};
    OH_CHECK(Length(clockwise) > 0.0);
    OH_CHECK(NearlyEqual(Length(clockwise), 5.0 * (kPi / 2.0)));
}

// Compile-time sanity.
namespace {
constexpr Arc2d kArc{Point2d{0.0, 0.0}, 5.0, 0.0, 1.0};
static_assert(kArc.radius == 5.0);
static_assert(Sweep(kArc) == 1.0);
} // namespace

int main() {
    TestConstruction();
    TestEquality();
    TestSweep();
    TestPointAtZeroAngleIsOnPositiveXAxis();
    TestPointAtQuarterTurn();
    TestStartAndEndPoint();
    TestOffCenterArc();
    TestLengthOfHalfCircle();
    TestLengthOfFullCircleMatchesCircumference();
    TestLengthIsNonNegativeRegardlessOfSweepDirection();

    std::puts("Arc2Tests: all tests passed.");
    return 0;
}
