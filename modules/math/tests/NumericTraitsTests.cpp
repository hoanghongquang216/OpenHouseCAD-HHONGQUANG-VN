#include <openhouse/math/NumericTraits.hpp>
#include <openhouse/testing/Check.hpp>

#include <cstdio>
#include <limits>

using namespace openhouse::math;

// All compile-time: NumericTraits values are derived via a single,
// deterministic computation path per type (no divergent floating-point
// paths being compared), so exact-equality static_assert is safe here
// (unlike the Angle sum-vs-direct case in MathTests.cpp).

static_assert(NumericTraits<float>::MachineEpsilon == std::numeric_limits<float>::epsilon());
static_assert(NumericTraits<double>::MachineEpsilon == std::numeric_limits<double>::epsilon());

static_assert(NumericTraits<float>::DefaultTolerance ==
              std::numeric_limits<float>::epsilon() * 100.0f);
static_assert(NumericTraits<double>::DefaultTolerance ==
              std::numeric_limits<double>::epsilon() * 100.0);

// Sanity: double's machine epsilon is much tighter than float's.
static_assert(NumericTraits<double>::MachineEpsilon < NumericTraits<float>::MachineEpsilon);

static_assert(NumericTraits<float>::Zero == 0.0f);
static_assert(NumericTraits<float>::One == 1.0f);

static_assert(NumericTraits<float>::Max == std::numeric_limits<float>::max());
static_assert(NumericTraits<float>::Lowest == std::numeric_limits<float>::lowest());
static_assert(NumericTraits<float>::SmallestPositiveNormal == std::numeric_limits<float>::min());

// DefaultTolerance must be meaningfully larger than machine epsilon (the
// entire point of having a separate constant) but still small enough to
// be a sane geometric tolerance, not a magic number that swallows real
// differences.
static_assert(NumericTraits<double>::DefaultTolerance > NumericTraits<double>::MachineEpsilon);
static_assert(NumericTraits<double>::DefaultTolerance < 1e-6);

int main() {
    std::puts("NumericTraitsTests: all static_asserts passed (compile-time only test).");
    return 0;
}
