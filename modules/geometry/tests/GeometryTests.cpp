// Minimal, framework-free tests for openhouse::geometry primitives.
//
// These are placeholder tests until the project adopts a test framework
// (e.g. Catch2 or GoogleTest, per docs/CODING_STANDARD.md: "New features
// should include tests when practical"). They use only <cassert> and
// static_assert so they compile and run with zero extra dependencies.

#include <openhouse/geometry/Geometry.hpp>

#include <cassert>
#include <cstdio>
#include <type_traits>

using namespace openhouse::geometry;

// Point2/Point3/Vector2/Vector3 are plain aggregates (no user-declared
// constructors) -- verified here since losing aggregate-ness (e.g. by
// accidentally adding a constructor later) would be a silent breaking
// change for any code relying on aggregate initialization.
static_assert(std::is_aggregate_v<Point2i>);
static_assert(std::is_aggregate_v<Point3i>);
static_assert(std::is_aggregate_v<Vector2i>);
static_assert(std::is_aggregate_v<Vector3i>);

// Point + Point must NOT compile (see ADR-0005: "point plus point" is
// intentionally not defined -- it has no affine meaning). Verified as a
// negative compile-time check via a concept, not just by the absence of
// a positive test -- this fails loudly (a static_assert failure) if
// someone ever accidentally adds an operator+(Point2, Point2) overload,
// rather than silently doing nothing.
template<typename T>
concept Addable = requires(T lhs, T rhs) {
    lhs + rhs;
};
static_assert(!Addable<Point2i>);
static_assert(!Addable<Point3i>);

// --- Compile-time checks: construction ----------------------------------

static_assert(sizeof(Point2f) == sizeof(float) * 2);
static_assert(sizeof(Point3f) == sizeof(float) * 3);
static_assert(sizeof(Vector2f) == sizeof(float) * 2);
static_assert(sizeof(Vector3f) == sizeof(float) * 3);

// Default construction should zero-initialize members.
static_assert(Point2f{}.x == 0.0f && Point2f{}.y == 0.0f);
static_assert(Point3f{}.x == 0.0f && Point3f{}.y == 0.0f && Point3f{}.z == 0.0f);
static_assert(Vector2f{}.x == 0.0f && Vector2f{}.y == 0.0f);
static_assert(Vector3f{}.x == 0.0f && Vector3f{}.y == 0.0f && Vector3f{}.z == 0.0f);

// --- Compile-time checks: Vector2 arithmetic ------------------------------

static_assert(Vector2i{1, 2} + Vector2i{3, 4} == Vector2i{4, 6});
static_assert(Vector2i{1, 2} - Vector2i{3, 4} == Vector2i{-2, -2});
static_assert(-Vector2i{1, -2} == Vector2i{-1, 2});
static_assert(Vector2i{1, 2} * 3 == Vector2i{3, 6});
static_assert(3 * Vector2i{1, 2} == Vector2i{3, 6});
static_assert(Dot(Vector2i{1, 0}, Vector2i{0, 1}) == 0);
static_assert(Dot(Vector2i{2, 3}, Vector2i{2, 3}) == 13);
static_assert(Cross(Vector2i{1, 0}, Vector2i{0, 1}) == 1);
static_assert(Cross(Vector2i{0, 1}, Vector2i{1, 0}) == -1);

// --- Compile-time checks: Vector3 arithmetic ------------------------------

static_assert(Vector3i{1, 2, 3} + Vector3i{1, 1, 1} == Vector3i{2, 3, 4});
static_assert(Vector3i{1, 2, 3} - Vector3i{1, 1, 1} == Vector3i{0, 1, 2});
static_assert(-Vector3i{1, -2, 3} == Vector3i{-1, 2, -3});
static_assert(Vector3i{1, 2, 3} * 2 == Vector3i{2, 4, 6});
static_assert(Dot(Vector3i{1, 0, 0}, Vector3i{0, 1, 0}) == 0);
static_assert(Dot(Vector3i{1, 2, 3}, Vector3i{1, 2, 3}) == 14);
static_assert(Cross(Vector3i{1, 0, 0}, Vector3i{0, 1, 0}) == Vector3i{0, 0, 1});
static_assert(Cross(Vector3i{0, 1, 0}, Vector3i{1, 0, 0}) == Vector3i{0, 0, -1});

// --- Compile-time checks: affine Point/Vector interaction ----------------

static_assert(Point2i{5, 7} - Point2i{2, 3} == Vector2i{3, 4});
static_assert(Point2i{1, 1} + Vector2i{2, 3} == Point2i{3, 4});
static_assert(Vector2i{2, 3} + Point2i{1, 1} == Point2i{3, 4});
static_assert(Point2i{5, 5} - Vector2i{1, 1} == Point2i{4, 4});

static_assert(Point3i{5, 7, 9} - Point3i{2, 3, 4} == Vector3i{3, 4, 5});
static_assert(Point3i{1, 1, 1} + Vector3i{2, 3, 4} == Point3i{3, 4, 5});
static_assert(Vector3i{2, 3, 4} + Point3i{1, 1, 1} == Point3i{3, 4, 5});
static_assert(Point3i{5, 5, 5} - Vector3i{1, 1, 1} == Point3i{4, 4, 4});

// --- Runtime checks: construction -----------------------------------------

static void TestPoint2Construction() {
    const Point2d p{1.0, 2.0};
    assert(p.x == 1.0);
    assert(p.y == 2.0);
}

static void TestPoint3Construction() {
    const Point3d p{1.0, 2.0, 3.0};
    assert(p.x == 1.0);
    assert(p.y == 2.0);
    assert(p.z == 3.0);
}

static void TestVector2Construction() {
    const Vector2i v{3, -4};
    assert(v.x == 3);
    assert(v.y == -4);
}

static void TestVector3Construction() {
    const Vector3i v{3, -4, 5};
    assert(v.x == 3);
    assert(v.y == -4);
    assert(v.z == 5);
}

static void TestAggregateEquality() {
    const Point2i a{1, 1};
    const Point2i b{1, 1};
    assert(a.x == b.x && a.y == b.y);
}

// --- Runtime checks: length / normalization / distance --------------------

static void TestVector2LengthAndNormalize() {
    const Vector2f a{3.0f, 4.0f};
    assert(Length(a) == 5.0f);
    assert(LengthSquared(a) == 25.0f);

    const Vector2f n = Normalized(a);
    assert(n.x > 0.59f && n.x < 0.61f);
    assert(n.y > 0.79f && n.y < 0.81f);
}

static void TestVector3LengthAndNormalize() {
    const Vector3f b{1.0f, 2.0f, 2.0f};
    assert(Length(b) == 3.0f);
    assert(LengthSquared(b) == 9.0f);
}

static void TestPoint2Distance() {
    const Point2f a{0.0f, 0.0f};
    const Point2f b{3.0f, 4.0f};
    assert(Distance(a, b) == 5.0f);
    assert(DistanceSquared(a, b) == 25.0f);
}

static void TestPoint3Distance() {
    const Point3f p{0.0f, 0.0f, 0.0f};
    const Point3f q{1.0f, 2.0f, 2.0f};
    assert(Distance(p, q) == 3.0f);
}

// --- Runtime checks: compound assignment -----------------------------------

static void TestVector2CompoundAssignment() {
    Vector2i v{1, 1};
    v += Vector2i{2, 2};
    assert((v == Vector2i{3, 3}));
    v -= Vector2i{1, 1};
    assert((v == Vector2i{2, 2}));
    v *= 2;
    assert((v == Vector2i{4, 4}));
}

static void TestPoint2CompoundAssignment() {
    Point2i pt{1, 1};
    pt += Vector2i{4, 4};
    assert((pt == Point2i{5, 5}));
    pt -= Vector2i{2, 2};
    assert((pt == Point2i{3, 3}));
}

int main() {
    TestPoint2Construction();
    TestPoint3Construction();
    TestVector2Construction();
    TestVector3Construction();
    TestAggregateEquality();

    TestVector2LengthAndNormalize();
    TestVector3LengthAndNormalize();
    TestPoint2Distance();
    TestPoint3Distance();

    TestVector2CompoundAssignment();
    TestPoint2CompoundAssignment();

    std::puts("GeometryTests: all tests passed.");
    return 0;
}
