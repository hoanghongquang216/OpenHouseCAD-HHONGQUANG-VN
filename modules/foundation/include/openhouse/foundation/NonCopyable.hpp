#pragma once

namespace openhouse::foundation {

// Deletes copy construction/assignment while explicitly preserving move
// semantics. Without explicitly defaulting the move members, deleting the
// copy members would also suppress the compiler-generated move members
// (per [class.copy.ctor]), silently making the derived type immovable too.
class NonCopyable {
protected:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;
    constexpr NonCopyable(NonCopyable&&) = default;
    constexpr NonCopyable& operator=(NonCopyable&&) = default;
public:
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

}
