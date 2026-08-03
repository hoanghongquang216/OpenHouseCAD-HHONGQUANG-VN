#include <openhouse/geometry/BoundingBox2.hpp>

#include <cassert>
#include <cstdio>

using namespace openhouse::geometry;

static void TestWidthHeight() {
    const BoundingBox2d box{Point2d{1.0, 2.0}, Point2d{5.0, 8.0}};
    assert(Width(box) == 4.0);
    assert(Height(box) == 6.0);
}

static void TestCenter() {
    const BoundingBox2d box{Point2d{0.0, 0.0}, Point2d{10.0, 20.0}};
    const Point2d c = Center(box);
    assert(c.x == 5.0);
    assert(c.y == 10.0);
}

static void TestContainsInclusive() {
    const BoundingBox2d box{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}};
    assert(Contains(box, Point2d{5.0, 5.0}));
    assert(Contains(box, Point2d{0.0, 0.0}));   // corner, inclusive
    assert(Contains(box, Point2d{10.0, 10.0})); // corner, inclusive
    assert(!Contains(box, Point2d{-1.0, 5.0}));
    assert(!Contains(box, Point2d{5.0, 10.1}));
}

static void TestIntersectsOverlapping() {
    const BoundingBox2d a{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}};
    const BoundingBox2d b{Point2d{5.0, 5.0}, Point2d{15.0, 15.0}};
    assert(Intersects(a, b));
    assert(Intersects(b, a)); // symmetric
}

static void TestIntersectsTouchingEdge() {
    const BoundingBox2d a{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}};
    const BoundingBox2d b{Point2d{10.0, 0.0}, Point2d{20.0, 10.0}};
    assert(Intersects(a, b)); // touching at x=10 edge counts as intersecting
}

static void TestIntersectsSeparate() {
    const BoundingBox2d a{Point2d{0.0, 0.0}, Point2d{10.0, 10.0}};
    const BoundingBox2d b{Point2d{20.0, 20.0}, Point2d{30.0, 30.0}};
    assert(!Intersects(a, b));
}

static void TestUnion() {
    const BoundingBox2d a{Point2d{0.0, 0.0}, Point2d{5.0, 5.0}};
    const BoundingBox2d b{Point2d{3.0, -2.0}, Point2d{10.0, 4.0}};
    const BoundingBox2d u = Union(a, b);
    assert(u.min.x == 0.0 && u.min.y == -2.0);
    assert(u.max.x == 10.0 && u.max.y == 5.0);
}

static void TestExpand() {
    BoundingBox2d box = FromPoint(Point2d{5.0, 5.0});
    assert(box.min == box.max);

    box = Expand(box, Point2d{0.0, 10.0});
    assert(box.min.x == 0.0 && box.min.y == 5.0);
    assert(box.max.x == 5.0 && box.max.y == 10.0);

    box = Expand(box, Point2d{8.0, 2.0});
    assert(box.min.x == 0.0 && box.min.y == 2.0);
    assert(box.max.x == 8.0 && box.max.y == 10.0);
}

// Compile-time sanity (integer box -- Width/Height/Contains/Intersects/
// Union/Expand don't require FloatingPoint, only Center does).
namespace {
constexpr BoundingBox2i kBox{Point2i{0, 0}, Point2i{10, 10}};
static_assert(Width(kBox) == 10);
static_assert(Height(kBox) == 10);
static_assert(Contains(kBox, Point2i{5, 5}));
static_assert(!Contains(kBox, Point2i{11, 5}));
} // namespace

int main() {
    TestWidthHeight();
    TestCenter();
    TestContainsInclusive();
    TestIntersectsOverlapping();
    TestIntersectsTouchingEdge();
    TestIntersectsSeparate();
    TestUnion();
    TestExpand();

    std::puts("BoundingBox2Tests: all tests passed.");
    return 0;
}
