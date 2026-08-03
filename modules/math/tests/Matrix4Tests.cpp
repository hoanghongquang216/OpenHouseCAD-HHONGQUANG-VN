#include <openhouse/math/Matrix4.hpp>

#include <cassert>
#include <cmath>
#include <cstdio>

using namespace openhouse::math;
using namespace openhouse::geometry;

namespace {
bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
bool NearlyEqual(const Point3d& a, const Point3d& b, double eps = 1e-9) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps) && NearlyEqual(a.z, b.z, eps);
}
bool NearlyEqual(const Vector3d& a, const Vector3d& b, double eps = 1e-9) {
    return NearlyEqual(a.x, b.x, eps) && NearlyEqual(a.y, b.y, eps) && NearlyEqual(a.z, b.z, eps);
}
} // namespace

static void TestIdentityLeavesPointsUnchanged() {
    const Matrix4d id = Matrix4d::Identity();
    const Point3d p{1.0, 2.0, 3.0};
    const Point3d result = id * p;
    assert(NearlyEqual(result, p));
}

static void TestIdentityLeavesVectorsUnchanged() {
    const Matrix4d id = Matrix4d::Identity();
    const Vector3d v{1.0, 2.0, 3.0};
    const Vector3d result = id * v;
    assert(NearlyEqual(result, v));
}

static void TestTranslationMovesPoint() {
    const Matrix4d t = Matrix4d::Translation(Vector3d{10.0, 20.0, 30.0});
    const Point3d p{1.0, 1.0, 1.0};
    const Point3d result = t * p;
    assert(NearlyEqual(result, Point3d{11.0, 21.0, 31.0}));
}

static void TestTranslationDoesNotAffectVector() {
    // Translation should not affect free vectors (directions), only points.
    const Matrix4d t = Matrix4d::Translation(Vector3d{10.0, 20.0, 30.0});
    const Vector3d v{1.0, 1.0, 1.0};
    const Vector3d result = t * v;
    assert(NearlyEqual(result, v));
}

static void TestUniformScale() {
    const Matrix4d s = Matrix4d::UniformScale(2.0);
    const Point3d p{1.0, 2.0, 3.0};
    const Point3d result = s * p;
    assert(NearlyEqual(result, Point3d{2.0, 4.0, 6.0}));
}

static void TestNonUniformScale() {
    const Matrix4d s = Matrix4d::Scale(Vector3d{2.0, 3.0, 4.0});
    const Point3d p{1.0, 1.0, 1.0};
    const Point3d result = s * p;
    assert(NearlyEqual(result, Point3d{2.0, 3.0, 4.0}));
}

static void TestRotationZ90DegreesOnXAxis() {
    // Rotating (1,0,0) by +90 degrees around Z should give (0,1,0)
    // (standard right-handed convention, counter-clockwise looking down
    // the +Z axis toward the origin).
    const Matrix4d r = Matrix4d::RotationZ(Angled::FromDegrees(90.0));
    const Point3d p{1.0, 0.0, 0.0};
    const Point3d result = r * p;
    assert(NearlyEqual(result, Point3d{0.0, 1.0, 0.0}, 1e-9));
}

static void TestRotationX90DegreesOnYAxis() {
    // Rotating (0,1,0) by +90 degrees around X should give (0,0,1).
    const Matrix4d r = Matrix4d::RotationX(Angled::FromDegrees(90.0));
    const Point3d p{0.0, 1.0, 0.0};
    const Point3d result = r * p;
    assert(NearlyEqual(result, Point3d{0.0, 0.0, 1.0}, 1e-9));
}

static void TestRotationY90DegreesOnZAxis() {
    // Rotating (0,0,1) by +90 degrees around Y should give (1,0,0).
    const Matrix4d r = Matrix4d::RotationY(Angled::FromDegrees(90.0));
    const Point3d p{0.0, 0.0, 1.0};
    const Point3d result = r * p;
    assert(NearlyEqual(result, Point3d{1.0, 0.0, 0.0}, 1e-9));
}

static void TestRotationPreservesLength() {
    const Matrix4d r = Matrix4d::RotationZ(Angled::FromDegrees(37.0));
    const Vector3d v{3.0, 4.0, 0.0}; // length 5
    const Vector3d rotated = r * v;
    const double lengthBefore = Length(v);
    const double lengthAfter = Length(rotated);
    assert(NearlyEqual(lengthBefore, lengthAfter, 1e-9));
}

static void TestComposition() {
    // Translate then scale, composed as a single matrix, should match
    // applying translation first and scale second on the point manually
    // (matrix on the left is applied first when composed as scale * translate,
    // since M = scale * translate means for a point p: (scale * translate) * p
    // = scale * (translate * p) -- translate happens first).
    const Matrix4d translate = Matrix4d::Translation(Vector3d{1.0, 0.0, 0.0});
    const Matrix4d scale = Matrix4d::UniformScale(2.0);
    const Matrix4d combined = scale * translate;

    const Point3d p{1.0, 1.0, 1.0};
    const Point3d viaCombined = combined * p;
    const Point3d viaManualSteps = scale * (translate * p);

    assert(NearlyEqual(viaCombined, viaManualSteps));
    // translate(1,1,1) by (1,0,0) -> (2,1,1), then scale by 2 -> (4,2,2)
    assert(NearlyEqual(viaCombined, Point3d{4.0, 2.0, 2.0}));
}

static void TestTransposedIsInvolution() {
    const Matrix4d m = Matrix4d::Translation(Vector3d{1.0, 2.0, 3.0});
    const Matrix4d doubleTransposed = m.Transposed().Transposed();
    assert(m == doubleTransposed);
}

static void TestIdentityEquality() {
    assert(Matrix4d::Identity() == Matrix4d::Identity());
    assert(!(Matrix4d::Identity() == Matrix4d::Translation(Vector3d{1.0, 0.0, 0.0})));
}

// Compile-time sanity: Identity is constructible and indexable at compile time.
namespace {
constexpr Matrix4d kIdentity = Matrix4d::Identity();
static_assert(kIdentity.At(0, 0) == 1.0);
static_assert(kIdentity.At(1, 1) == 1.0);
static_assert(kIdentity.At(0, 1) == 0.0);

constexpr Matrix4d kTranslated = Matrix4d::Translation(Vector3d{5.0, 6.0, 7.0});
static_assert(kTranslated.At(0, 3) == 5.0);
static_assert(kTranslated.At(1, 3) == 6.0);
static_assert(kTranslated.At(2, 3) == 7.0);
} // namespace

int main() {
    TestIdentityLeavesPointsUnchanged();
    TestIdentityLeavesVectorsUnchanged();
    TestTranslationMovesPoint();
    TestTranslationDoesNotAffectVector();
    TestUniformScale();
    TestNonUniformScale();
    TestRotationZ90DegreesOnXAxis();
    TestRotationX90DegreesOnYAxis();
    TestRotationY90DegreesOnZAxis();
    TestRotationPreservesLength();
    TestComposition();
    TestTransposedIsInvolution();
    TestIdentityEquality();

    std::puts("Matrix4Tests: all tests passed.");
    return 0;
}
