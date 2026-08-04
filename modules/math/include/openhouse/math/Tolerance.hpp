#pragma once

#include <openhouse/foundation/Algorithm.hpp>
#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/math/NumericTraits.hpp>

namespace openhouse::math {

namespace detail {

// A hand-rolled absolute value, deliberately NOT delegating to
// std::abs/foundation::abs here. Root-caused via a real Clang+libc++ CI
// failure (see CHANGELOG.md / commit history): libc++ declares a
// double abs(double) overload in <stdlib.h> that is NOT constexpr
// (unlike <cmath>'s std::abs(double), constexpr since C++23), and
// under Clang, unqualified `abs()` inside these constexpr functions
// resolved to that non-constexpr overload -- breaking every
// static_assert-based compile-time use of NearlyEqual/IsZero (this
// project relies on that -- see ToleranceTests.cpp). GCC's libstdc++
// does not have this ambiguity, so the bug was invisible until Clang
// was added to CI. A three-line ternary sidesteps the entire
// cross-standard-library overload question rather than depending on
// any particular <cmath>/<cstdlib> implementation detail.
template<foundation::FloatingPoint T>
[[nodiscard]] constexpr T Abs(T value) noexcept {
    return value < T{0} ? -value : value;
}

} // namespace detail

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
    const T diff = detail::Abs(a - b);
    if (diff <= tolerance) {
        return true;
    }
    const T largest = foundation::max(detail::Abs(a), detail::Abs(b));
    return diff <= largest * tolerance;
}

template<foundation::FloatingPoint T>
[[nodiscard]] constexpr bool IsZero(T value, T tolerance = NumericTraits<T>::DefaultTolerance) noexcept {
    return detail::Abs(value) <= tolerance;
}

}
