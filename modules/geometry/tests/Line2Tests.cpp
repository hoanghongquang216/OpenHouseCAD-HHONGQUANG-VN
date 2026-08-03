#include <openhouse/geometry/Line2.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestConstruction() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{3.0, 4.0}};
    OH_CHECK(line.start.x == 0.0 && line.start.y == 0.0);
    OH_CHECK(line.end.x == 3.0 && line.end.y == 4.0);
}

static void TestEquality() {
    const Line2d a{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}};
    const Line2d b{Point2d{0.0, 0.0}, Point2d{1.0, 1.0}};
    const Line2d c{Point2d{0.0, 0.0}, Point2d{2.0, 2.0}};
    OH_CHECK(a == b);
    OH_CHECK(!(a == c));
}

static void TestDisplacement() {
    const Line2d line{Point2d{1.0, 1.0}, Point2d{4.0, 5.0}};
    const Vector2d d = Displacement(line);
    OH_CHECK(NearlyEqual(d.x, 3.0));
    OH_CHECK(NearlyEqual(d.y, 4.0));
}

static void TestLength() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{3.0, 4.0}};
    OH_CHECK(NearlyEqual(Length(line), 5.0));
    OH_CHECK(NearlyEqual(LengthSquared(line), 25.0));
}

static void TestLengthOfDegenerateLineIsZero() {
    const Line2d line{Point2d{2.0, 2.0}, Point2d{2.0, 2.0}};
    OH_CHECK(NearlyEqual(Length(line), 0.0));
}

static void TestDirection() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{5.0, 0.0}};
    const Vector2d dir = Direction(line);
    OH_CHECK(NearlyEqual(dir.x, 1.0));
    OH_CHECK(NearlyEqual(dir.y, 0.0));
    OH_CHECK(NearlyEqual(Length(Vector2d{dir.x, dir.y}), 1.0)); // unit length
}

static void TestMidpoint() {
    const Line2d line{Point2d{0.0, 0.0}, Point2d{3.0, 4.0}};
    const Point2d mid = Midpoint(line);
    OH_CHECK(NearlyEqual(mid.x, 1.5));
    OH_CHECK(NearlyEqual(mid.y, 2.0));
}

static void TestMidpointOfNegativeCoordinates() {
    const Line2d line{Point2d{-2.0, -2.0}, Point2d{2.0, 2.0}};
    const Point2d mid = Midpoint(line);
    OH_CHECK(NearlyEqual(mid.x, 0.0));
    OH_CHECK(NearlyEqual(mid.y, 0.0));
}

// Compile-time sanity.
namespace {
constexpr Line2i kLine{Point2i{0, 0}, Point2i{3, 4}};
static_assert(kLine.start == (Point2i{0, 0}));
static_assert(kLine.end == (Point2i{3, 4}));
static_assert(Displacement(kLine) == (Vector2i{3, 4}));
} // namespace

int main() {
    TestConstruction();
    TestEquality();
    TestDisplacement();
    TestLength();
    TestLengthOfDegenerateLineIsZero();
    TestDirection();
    TestMidpoint();
    TestMidpointOfNegativeCoordinates();

    std::puts("Line2Tests: all tests passed.");
    return 0;
}
