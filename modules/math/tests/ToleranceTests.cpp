#include <openhouse/math/Tolerance.hpp>

#include <cassert>
#include <cstdio>

using namespace openhouse::math;

static void TestExactEquality() {
    assert(NearlyEqual(1.0, 1.0));
    assert(NearlyEqual(0.0, 0.0));
    assert(NearlyEqual(-5.0, -5.0));
}

static void TestExplicitToleranceAccepts() {
    assert(NearlyEqual(1.0, 1.0 + 1e-10, 1e-6));
    assert(NearlyEqual(100.0, 100.0 - 1e-8, 1e-6));
}

static void TestExplicitToleranceRejects() {
    assert(!NearlyEqual(1.0, 1.1, 1e-6));
    assert(!NearlyEqual(0.0, 1.0, 1e-6));
}

static void TestNearZeroUsesAbsoluteComparison() {
    // Near zero, a relative comparison is meaningless (dividing by ~0);
    // the absolute branch must handle this case correctly.
    assert(NearlyEqual(0.0, 1e-15, 1e-9));
    assert(!NearlyEqual(0.0, 1e-6, 1e-9));
}

static void TestRelativeScalingAtLargeMagnitude() {
    // At magnitude 1e10, an absolute difference of 1.0 is a relative
    // difference of only 1e-10 -- "nearly equal" under a generous
    // relative tolerance, but clearly not under a tight one. This
    // exercises the relative-comparison fallback specifically (the
    // absolute branch alone would reject both cases at these tolerances).
    assert(NearlyEqual(1e10, 1e10 + 1.0, 1e-6));   // 1e-10 relative diff <= 1e-6 tolerance
    assert(!NearlyEqual(1e10, 1e10 + 1.0, 1e-12)); // 1e-10 relative diff >  1e-12 tolerance
}

static void TestDefaultToleranceIsUsableAtUnitScale() {
    // Uses NumericTraits<double>::DefaultTolerance directly rather than a
    // hardcoded assumption about its exact value, so this test stays
    // correct even if that scale factor is ever revisited (see
    // NumericTraits.hpp).
    const double halfDefault = NumericTraits<double>::DefaultTolerance / 2.0;
    assert(NearlyEqual(1.0, 1.0 + halfDefault));

    const double wellBeyondDefault = NumericTraits<double>::DefaultTolerance * 1000.0;
    assert(!NearlyEqual(1.0, 1.0 + wellBeyondDefault));
}

static void TestIsZero() {
    assert(IsZero(0.0));
    assert(IsZero(1e-20, 1e-9));
    assert(!IsZero(1.0, 1e-9));
    assert(!IsZero(1e-6, 1e-9));
}

static void TestFloatOverload() {
    assert(NearlyEqual(1.0f, 1.0f + 1e-6f, 1e-4f));
    assert(!NearlyEqual(1.0f, 1.1f, 1e-4f));
    assert(IsZero(0.0f));
}

// Compile-time sanity: NearlyEqual/IsZero are usable in constexpr context.
static_assert(NearlyEqual(1.0, 1.0));
static_assert(!NearlyEqual(1.0, 2.0, 0.1));
static_assert(IsZero(0.0));
static_assert(!IsZero(1.0, 1e-9));

int main() {
    TestExactEquality();
    TestExplicitToleranceAccepts();
    TestExplicitToleranceRejects();
    TestNearZeroUsesAbsoluteComparison();
    TestRelativeScalingAtLargeMagnitude();
    TestDefaultToleranceIsUsableAtUnitScale();
    TestIsZero();
    TestFloatOverload();

    std::puts("ToleranceTests: all tests passed.");
    return 0;
}
