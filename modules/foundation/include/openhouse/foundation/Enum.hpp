#pragma once

#include <type_traits>

namespace openhouse::foundation {

template<typename E>
constexpr auto ToUnderlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

}
