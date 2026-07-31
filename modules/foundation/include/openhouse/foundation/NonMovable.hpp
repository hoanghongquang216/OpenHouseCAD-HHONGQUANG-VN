#pragma once

namespace openhouse::foundation {
class NonMovable {
protected:
    constexpr NonMovable() = default;
    ~NonMovable() = default;
public:
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};
}
