#pragma once

#include <openhouse/foundation/Algorithm.hpp>
#include <openhouse/foundation/CMath.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/math/NumericTraits.hpp>

namespace openhouse::math {

// Combined absolute + relative epsilon comparison ("essentially equal" /
// Knuth-style), rather than a single fixed absolute tolerance.
//
// A pure absolute tolerance breaks down at scale: a tolerance tight enough
// to be meaningful for values near 1.0 is far too tight for values around
// 1e6 (where that much floating-point error is essentially guaranteed by
// accumulated rounding, not a real difference), and far too loose for
// values near 1e-6 (where it would call clearly-different tiny values
// "equal"). This function checks absolute difference first (which also
// correctly handles the near-zero case, where a relative comparison is
// meaningless since dividing by ~0 is unstable), then falls back to a
// relative comparison scaled by the larger operand's magnitude.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr bool NearlyEqual(
    T a, T b, T tolerance = NumericTraits<T>::DefaultTolerance) noexcept {
    const T diff = foundation::abs(a - b);
    if (diff <= tolerance) {
        return true;
    }
    const T largest = foundation::max(foundation::abs(a), foundation::abs(b));
    return diff <= largest * tolerance;
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr bool IsZero(T value, T tolerance = NumericTraits<T>::DefaultTolerance) noexcept {
    return foundation::abs(value) <= tolerance;
}

}
