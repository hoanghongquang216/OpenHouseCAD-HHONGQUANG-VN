#pragma once

#include "ScopeExit.hpp"

namespace openhouse::foundation {

template<typename F>
[[nodiscard]] auto Finally(F&& f) {
    return ScopeExit<std::decay_t<F>>(std::forward<F>(f));
}

}
