#include <openhouse/geometry/Projection.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

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

static void TestExtendFindsIntersectionPastTargetEnd() {
    const Line2d target{{0.0, 0.0}, {3.0, 0.0}};
    const Line2d boundary{{5.0, -5.0}, {5.0, 5.0}};
    const auto result = FindExtendIntersection(target, boundary);
    OH_CHECK(result.has_value());
    OH_CHECK(NearlyEqual(result->x, 5.0));
    OH_CHECK(NearlyEqual(result->y, 0.0));
}

static void TestExtendFindsIntersectionBeforeTargetStart() {
    const Line2d target{{0.0, 0.0}, {3.0, 0.0}};
    const Line2d boundary{{-5.0, -5.0}, {-5.0, 5.0}};
    const auto result = FindExtendIntersection(target, boundary);
    OH_CHECK(result.has_value());
    OH_CHECK(NearlyEqual(result->x, -5.0));
    OH_CHECK(NearlyEqual(result->y, 0.0));
}

static void TestExtendFailsWhenBoundarySegmentTooShort() {
    const Line2d target{{0.0, 0.0}, {3.0, 0.0}};
    const Line2d boundary{{5.0, 1.0}, {5.0, 5.0}};
    const auto result = FindExtendIntersection(target, boundary);
    OH_CHECK(!result.has_value());
}

static void TestExtendIntersectionAlreadyWithinTargetBounds() {
    const Line2d target{{0.0, 0.0}, {10.0, 0.0}};
    const Line2d boundary{{5.0, -5.0}, {5.0, 5.0}};
    const auto result = FindExtendIntersection(target, boundary);
    OH_CHECK(result.has_value());
    OH_CHECK(NearlyEqual(result->x, 5.0));
    OH_CHECK(NearlyEqual(result->y, 0.0));
}

static void TestExtendParallelLinesReturnsNullopt() {
    const Line2d target{{0.0, 0.0}, {10.0, 0.0}};
    const Line2d boundary{{0.0, 5.0}, {10.0, 5.0}};
    const auto result = FindExtendIntersection(target, boundary);
    OH_CHECK(!result.has_value());
}

int main() {
    TestParameterAtStart();
    TestParameterAtEnd();
    TestParameterAtMidpoint();
    TestParameterBeforeStart();
    TestParameterAfterEnd();
    TestParameterOnDiagonalLine();
    TestParameterDegenerateLineReturnsZero();
    TestExtendFindsIntersectionPastTargetEnd();
    TestExtendFindsIntersectionBeforeTargetStart();
    TestExtendFailsWhenBoundarySegmentTooShort();
    TestExtendIntersectionAlreadyWithinTargetBounds();
    TestExtendParallelLinesReturnsNullopt();
    std::puts("ProjectionTests: all tests passed.");
    return 0;
}
