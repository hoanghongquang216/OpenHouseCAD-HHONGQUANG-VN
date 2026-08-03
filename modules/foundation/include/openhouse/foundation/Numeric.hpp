#pragma once

#include <type_traits>

namespace openhouse::foundation {

template<typename T>
inline constexpr bool IsIntegralV = std::is_integral_v<T>;

template<typename T>
inline constexpr bool IsFloatingPointV = std::is_floating_point_v<T>;

}
