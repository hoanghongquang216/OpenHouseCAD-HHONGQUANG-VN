#include <openhouse/geometry/Circle2.hpp>

#include <cassert>
#include <cmath>
#include <cstdio>

using namespace openhouse::geometry;

namespace {
bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestConstruction() {
    const Circle2d c{Point2d{1.0, 2.0}, 5.0};
    assert(c.center.x == 1.0 && c.center.y == 2.0);
    assert(c.radius == 5.0);
}

static void TestEquality() {
    const Circle2d a{Point2d{0.0, 0.0}, 1.0};
    const Circle2d b{Point2d{0.0, 0.0}, 1.0};
    const Circle2d c{Point2d{0.0, 0.0}, 2.0};
    assert(a == b);
    assert(!(a == c));
}

static void TestCircumference() {
    const Circle2d c{Point2d{0.0, 0.0}, 1.0};
    assert(NearlyEqual(Circumference(c), 2.0 * 3.14159265358979323846));
}

static void TestArea() {
    const Circle2d c{Point2d{0.0, 0.0}, 2.0};
    assert(NearlyEqual(Area(c), 3.14159265358979323846 * 4.0));
}

static void TestContainsCenterIsInside() {
    const Circle2d c{Point2d{5.0, 5.0}, 3.0};
    assert(Contains(c, c.center));
}

static void TestContainsPointOnBoundary() {
    const Circle2d c{Point2d{0.0, 0.0}, 5.0};
    assert(Contains(c, Point2d{5.0, 0.0}));
    assert(Contains(c, Point2d{0.0, 5.0}));
    assert(Contains(c, Point2d{3.0, 4.0})); // 3-4-5 triangle, exactly on boundary
}

static void TestContainsPointOutside() {
    const Circle2d c{Point2d{0.0, 0.0}, 5.0};
    assert(!Contains(c, Point2d{5.1, 0.0}));
    assert(!Contains(c, Point2d{10.0, 10.0}));
}

static void TestContainsPointInside() {
    const Circle2d c{Point2d{0.0, 0.0}, 5.0};
    assert(Contains(c, Point2d{1.0, 1.0}));
    assert(Contains(c, Point2d{0.0, 0.0}));
}

static void TestOffCenterCircle() {
    const Circle2d c{Point2d{10.0, -10.0}, 2.0};
    assert(Contains(c, Point2d{10.0, -10.0}));
    assert(Contains(c, Point2d{11.0, -10.0}));
    assert(!Contains(c, Point2d{13.0, -10.0}));
}

// Compile-time sanity.
namespace {
constexpr Circle2i kCircle{Point2i{0, 0}, 5};
static_assert(kCircle.center == (Point2i{0, 0}));
static_assert(kCircle.radius == 5);
} // namespace

int main() {
    TestConstruction();
    TestEquality();
    TestCircumference();
    TestArea();
    TestContainsCenterIsInside();
    TestContainsPointOnBoundary();
    TestContainsPointOutside();
    TestContainsPointInside();
    TestOffCenterCircle();

    std::puts("Circle2Tests: all tests passed.");
    return 0;
}
