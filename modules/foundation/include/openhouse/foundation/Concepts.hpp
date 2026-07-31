#pragma once

#include <concepts>

namespace openhouse::foundation {

template<typename T>
concept Integral = std::integral<T>;

template<typename T>
concept FloatingPoint = std::floating_point<T>;

}
