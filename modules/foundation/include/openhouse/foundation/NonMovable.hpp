#pragma once

namespace openhouse::foundation {

// Deletes move construction/assignment. Copy is intentionally left
// unavailable too (not explicitly defaulted), rather than "restored".
//
// This is deliberate, not an oversight: in C++, if a type deletes only its
// move constructor/assignment while leaving copy available, an attempt to
// move such an object does NOT fail to compile -- it silently falls back
// to the copy constructor instead (verified empirically: GCC 13,
// -std=c++23). That is a dangerous, silent behavior change for any caller
// that assumed std::move(x) either moves or fails loudly. Since there is
// no way to reliably express "copyable but not movable" in C++ without
// this footgun, NonMovable disables both, matching NonCopyable's
// convention of "if you can't do the disabled operation, you get a clear
// compile error" rather than a silent, unintended copy.
//
// If you need "not copyable, but movable" (the safe, well-defined
// direction), use NonCopyable instead.
class NonMovable {
protected:
    constexpr NonMovable() = default;
    ~NonMovable() = default;
public:
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

}
