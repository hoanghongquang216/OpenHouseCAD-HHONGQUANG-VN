#pragma once

#include <openhouse/foundation/Concepts.hpp>

#include <limits>

namespace openhouse::math {

// Per-type numeric constants for geometry/tolerance work. Wraps
// std::numeric_limits (per ADR-0001's convention of curating standard
// library facilities through a named, documented surface) and adds
// DefaultTolerance: a practical absolute-comparison epsilon, distinct
// from the raw machine epsilon.
//
// Why DefaultTolerance != MachineEpsilon: machine epsilon is the gap
// between 1.0 and the next representable value -- it is almost always
// too tight to use directly as a comparison tolerance once a value has
// gone through even a handful of arithmetic operations (each operation
// can introduce rounding error on the order of a few ULPs). A commonly
// used practical default is machine epsilon scaled by a safety factor;
// this traits type fixes that factor in one place so it's consistent
// across the codebase rather than re-guessed at each call site.
template<foundation::FloatingPoint T>
struct NumericTraits {
    static constexpr T MachineEpsilon = std::numeric_limits<T>::epsilon();

    // Scale factor is a deliberately simple, documented constant rather
    // than a per-type magic number -- easy to audit, easy to revisit if
    // real-world usage shows it's mistuned (see docs/ARCHITECTURE_DECISION_RECORDS
    // if that scale factor is ever formally revisited).
    static constexpr T DefaultTolerance = MachineEpsilon * T{100};

    static constexpr T SmallestPositiveNormal = std::numeric_limits<T>::min();
    static constexpr T Max = std::numeric_limits<T>::max();
    static constexpr T Lowest = std::numeric_limits<T>::lowest();
    static constexpr T Infinity = std::numeric_limits<T>::infinity();

    static constexpr T Zero = T{0};
    static constexpr T One = T{1};
};

}
