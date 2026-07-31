#pragma once

namespace openhouse::foundation {

[[noreturn]] inline void Unreachable() {
#if defined(__cpp_lib_unreachable)
    std::unreachable();
#elif defined(_MSC_VER)
    __assume(false);
#else
    __builtin_unreachable();
#endif
}

}
