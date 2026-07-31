#pragma once

#include <cstdint>

namespace openhouse::foundation {

struct Version final {
    static constexpr std::uint32_t Major = 0;
    static constexpr std::uint32_t Minor = 1;
    static constexpr std::uint32_t Patch = 0;
};

} // namespace openhouse::foundation
