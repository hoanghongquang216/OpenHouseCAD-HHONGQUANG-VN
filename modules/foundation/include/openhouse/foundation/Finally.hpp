#pragma once

#include "ScopeExit.hpp"

namespace openhouse::foundation {

template<class F>
auto Finally(F&& f) {
    return ScopeExit<F>(std::forward<F>(f));
}

}
