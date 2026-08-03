#include <openhouse/math/Angle.hpp>
#include <openhouse/testing/Check.hpp>

#include <cmath>
#include <cstdio>

using namespace openhouse::math;

namespace {
// Local floating-point tolerance comparison for runtime checks (trig and
// fmod-based results are not bit-exact across the operations involved).
bool NearlyEqual(double a, double b, double eps = 1e-9) {
    return std::abs(a - b) < eps;
}
} // namespace

static void TestFactoriesAndConversion() {
    const Angled fromDeg = Angled::FromDegrees(180.0);
    OH_CHECK(NearlyEqual(fromDeg.Radians(), 3.14159265358979323846));

    const Angled fromRad = Angled::FromRadians(3.14159265358979323846);
    OH_CHECK(NearlyEqual(fromRad.Degrees(), 180.0));

    // Round-trip: degrees -> Angle -> degrees.
    const Angled roundTrip = Angled::FromDegrees(37.5);
    OH_CHECK(NearlyEqual(roundTrip.Degrees(), 37.5));
}

static void TestDefaultIsZero() {
    const Angled a{};
    OH_CHECK(a.Radians() == 0.0);
    OH_CHECK(a.Degrees() == 0.0);
}

static void TestEqualityAndOrdering() {
    OH_CHECK(Angled::FromDegrees(90.0) == Angled::FromDegrees(90.0));
    OH_CHECK(Angled::FromDegrees(45.0) < Angled::FromDegrees(90.0));
    OH_CHECK(Angled::FromDegrees(90.0) > Angled::FromDegrees(45.0));
}

static void TestArithmetic() {
    const Angled sum = Angled::FromDegrees(30.0) + Angled::FromDegrees(60.0);
    OH_CHECK(NearlyEqual(sum.Degrees(), 90.0));

    const Angled diff = Angled::FromDegrees(90.0) - Angled::FromDegrees(30.0);
    OH_CHECK(NearlyEqual(diff.Degrees(), 60.0));

    const Angled negated = -Angled::FromDegrees(45.0);
    OH_CHECK(NearlyEqual(negated.Degrees(), -45.0));

    const Angled scaled = Angled::FromDegrees(30.0) * 3.0;
    OH_CHECK(NearlyEqual(scaled.Degrees(), 90.0));

    const Angled scaledLeft = 2.0 * Angled::FromDegrees(45.0);
    OH_CHECK(NearlyEqual(scaledLeft.Degrees(), 90.0));

    const Angled divided = Angled::FromDegrees(90.0) / 3.0;
    OH_CHECK(NearlyEqual(divided.Degrees(), 30.0));
}

static void TestCompoundAssignment() {
    Angled a = Angled::FromDegrees(10.0);
    a += Angled::FromDegrees(20.0);
    OH_CHECK(NearlyEqual(a.Degrees(), 30.0));

    a -= Angled::FromDegrees(5.0);
    OH_CHECK(NearlyEqual(a.Degrees(), 25.0));

    a *= 2.0;
    OH_CHECK(NearlyEqual(a.Degrees(), 50.0));
}

static void TestNormalizedUnsigned() {
    OH_CHECK(NearlyEqual(NormalizedUnsigned(Angled::FromDegrees(370.0)).Degrees(), 10.0));
    OH_CHECK(NearlyEqual(NormalizedUnsigned(Angled::FromDegrees(-10.0)).Degrees(), 350.0));
    OH_CHECK(NearlyEqual(NormalizedUnsigned(Angled::FromDegrees(720.0)).Degrees(), 0.0));
    OH_CHECK(NearlyEqual(NormalizedUnsigned(Angled::FromDegrees(0.0)).Degrees(), 0.0));
}

static void TestNormalizedSigned() {
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(0.0)).Degrees(), 0.0));
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(180.0)).Degrees(), 180.0));
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(-180.0)).Degrees(), 180.0));
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(270.0)).Degrees(), -90.0));
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(-270.0)).Degrees(), 90.0));
    OH_CHECK(NearlyEqual(NormalizedSigned(Angled::FromDegrees(540.0)).Degrees(), 180.0));
}

static void TestTrig() {
    OH_CHECK(NearlyEqual(Sin(Angled::FromDegrees(90.0)), 1.0));
    OH_CHECK(NearlyEqual(Cos(Angled::FromDegrees(0.0)), 1.0));
    OH_CHECK(NearlyEqual(Sin(Angled::FromDegrees(0.0)), 0.0));
    OH_CHECK(NearlyEqual(Cos(Angled::FromDegrees(180.0)), -1.0));
    OH_CHECK(NearlyEqual(Tan(Angled::FromDegrees(45.0)), 1.0));
}

// Compile-time sanity: construction/factories work in a constexpr context.
// Note: we deliberately do NOT static_assert bit-exact equality between
// results reached via different floating-point computation paths (e.g.
// "30 deg + 60 deg == 90 deg" computed via degrees->radians conversion
// then addition, vs. "90 deg" computed directly) -- IEEE 754 does not
// guarantee those are bit-identical, even though they happened to match
// on the compiler this was authored with. Multiplying two DIFFERENT
// finite values by the same positive constant, however, is guaranteed by
// IEEE 754 to preserve strict ordering, so the ordering check below is
// safe as a static_assert; the equality-based checks are verified at
// runtime instead (see TestArithmetic / TestFactoriesAndConversion),
// using a tolerance comparison.
namespace {
constexpr Angled kRightAngle = Angled::FromDegrees(90.0);
static_assert(Angled::FromDegrees(45.0) < Angled::FromDegrees(90.0));
static_assert(kRightAngle.Radians() > Angled::FromDegrees(0.0).Radians());
} // namespace

int main() {
    TestFactoriesAndConversion();
    TestDefaultIsZero();
    TestEqualityAndOrdering();
    TestArithmetic();
    TestCompoundAssignment();
    TestNormalizedUnsigned();
    TestNormalizedSigned();
    TestTrig();

    std::puts("MathTests: all tests passed.");
    return 0;
}
