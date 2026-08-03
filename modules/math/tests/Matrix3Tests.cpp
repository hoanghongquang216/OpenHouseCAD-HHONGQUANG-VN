#include <openhouse/math/Matrix3.hpp>

#include <cassert>
#include <cmath>
#include <cstdio>

using namespace openhouse::math;
using namespace openhouse::geometry;

namespace {
bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
bool NearlyEqual(const Point2d& a, const Point2d& b, double eps = 1e-9) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps);
}
} // namespace

static void TestIdentityLeavesPointUnchanged() {
    const Matrix3d id = Matrix3d::Identity();
    const Point2d p{3.0, 4.0};
    assert(NearlyEqual(id * p, p));
}

static void TestTranslationMovesPoint() {
    const Matrix3d t = Matrix3d::Translation(Vector2d{10.0, -5.0});
    const Point2d p{1.0, 1.0};
    assert(NearlyEqual(t * p, Point2d{11.0, -4.0}));
}

static void TestTranslationDoesNotAffectVector() {
    const Matrix3d t = Matrix3d::Translation(Vector2d{10.0, -5.0});
    const Vector2d v{1.0, 1.0};
    assert(NearlyEqual(Point2d{(t * v).x, (t * v).y}, Point2d{1.0, 1.0}));
}

static void TestUniformScale() {
    const Matrix3d s = Matrix3d::UniformScale(3.0);
    const Point2d p{2.0, 2.0};
    assert(NearlyEqual(s * p, Point2d{6.0, 6.0}));
}

static void TestRotation90Degrees() {
    // Same convention as Matrix4::RotationZ: (1,0) -> (0,1) at +90 deg.
    const Matrix3d r = Matrix3d::Rotation(Angled::FromDegrees(90.0));
    const Point2d p{1.0, 0.0};
    assert(NearlyEqual(r * p, Point2d{0.0, 1.0}));
}

static void TestComposition() {
    const Matrix3d translate = Matrix3d::Translation(Vector2d{1.0, 0.0});
    const Matrix3d scale = Matrix3d::UniformScale(2.0);
    const Matrix3d combined = scale * translate;

    const Point2d p{1.0, 1.0};
    // translate first (1,1)->(2,1), then scale by 2 -> (4,2)
    assert(NearlyEqual(combined * p, Point2d{4.0, 2.0}));
}

static void TestIdentityEquality() {
    assert(Matrix3d::Identity() == Matrix3d::Identity());
    assert(!(Matrix3d::Identity() == Matrix3d::Translation(Vector2d{1.0, 0.0})));
}

namespace {
constexpr Matrix3d kIdentity = Matrix3d::Identity();
static_assert(kIdentity.At(0, 0) == 1.0);
static_assert(kIdentity.At(0, 1) == 0.0);

constexpr Matrix3d kTranslated = Matrix3d::Translation(Vector2d{5.0, 6.0});
static_assert(kTranslated.At(0, 2) == 5.0);
static_assert(kTranslated.At(1, 2) == 6.0);
} // namespace

int main() {
    TestIdentityLeavesPointUnchanged();
    TestTranslationMovesPoint();
    TestTranslationDoesNotAffectVector();
    TestUniformScale();
    TestRotation90Degrees();
    TestComposition();
    TestIdentityEquality();

    std::puts("Matrix3Tests: all tests passed.");
    return 0;
}
