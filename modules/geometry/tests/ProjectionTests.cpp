#include <openhouse/geometry/Projection.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <cmath>

using namespace openhouse::geometry;

namespace {

bool NearlyEqual(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

} // namespace

static void TestParameterAtStart() {
    const Line2d line{{0.0, 0.0}, {10.0, 0.0}};
    const double t = ParameterOnLine(line, Point2d{0.0, 0.0});
    OH_CHECK(NearlyEqual(t, 0.0));
}

static void TestParameterAtEnd() {
    const Line2d line{{0.0, 0.0}, {10.0, 0.0}};
    const double t = ParameterOnLine(line, Point2d{10.0, 0.0});
    OH_CHECK(NearlyEqual(t, 1.0));
}

static void TestParameterAtMidpoint() {
    const Line2d line{{0.0, 0.0}, {10.0, 0.0}};
    const double t = ParameterOnLine(line, Point2d{5.0, 0.0});
    OH_CHECK(NearlyEqual(t, 0.5));
}

static void TestParameterBeforeStart() {
    const Line2d line{{0.0, 0.0}, {10.0, 0.0}};
    const double t = ParameterOnLine(line, Point2d{-5.0, 0.0});
    OH_CHECK(t < 0.0);
    OH_CHECK(NearlyEqual(t, -0.5));
}

static void TestParameterAfterEnd() {
    const Line2d line{{0.0, 0.0}, {10.0, 0.0}};
    const double t = ParameterOnLine(line, Point2d{15.0, 0.0});
    OH_CHECK(t > 1.0);
    OH_CHECK(NearlyEqual(t, 1.5));
}

static void TestParameterOnDiagonalLine() {
    const Line2d line{{0.0, 0.0}, {10.0, 10.0}};
    const double t = ParameterOnLine(line, Point2d{5.0, 5.0});
    OH_CHECK(NearlyEqual(t, 0.5));
}

static void TestParameterDegenerateLineReturnsZero() {
    const Line2d line{{3.0, 4.0}, {3.0, 4.0}};
    const double t = ParameterOnLine(line, Point2d{99.0, 99.0});
    OH_CHECK(NearlyEqual(t, 0.0));
}

int main() {
    TestParameterAtStart();
    TestParameterAtEnd();
    TestParameterAtMidpoint();
    TestParameterBeforeStart();
    TestParameterAfterEnd();
    TestParameterOnDiagonalLine();
    TestParameterDegenerateLineReturnsZero();
    std::puts("ProjectionTests: all tests passed.");
    return 0;
}
