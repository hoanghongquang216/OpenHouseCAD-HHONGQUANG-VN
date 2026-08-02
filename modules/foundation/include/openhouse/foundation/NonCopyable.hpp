#pragma once

namespace openhouse::foundation {

// Deletes copy construction/assignment while explicitly preserving move
// semantics. Without explicitly defaulting move members, deleting copy
// members suppresses compiler-generated move members as well.
class NonCopyable {
protected:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;

public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

}
