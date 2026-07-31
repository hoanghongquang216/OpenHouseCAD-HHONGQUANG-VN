#pragma once

#include <limits>

namespace openhouse::foundation {

template<typename T>
inline constexpr T kInvalid = std::numeric_limits<T>::max();

}
