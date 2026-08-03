#pragma once

#include <cstdio>
#include <cstdlib>

// OH_CHECK(condition) -- like assert(), but NEVER compiled away, in any
// build configuration (Debug, Release, or anything else that defines
// NDEBUG).
//
// Root-cause fix for a real CI failure: this project's test files
// originally used the standard library's assert() as their sole
// assertion primitive. assert() is defined to a no-op when NDEBUG is
// set -- which CMake's Release build type does by default. Every local
// variable in a test whose only use was inside an assert() therefore
// became "declared but never used" under Release, and this project
// builds with -Werror, turning every one of those into a hard compile
// failure. This was caught by GitHub Actions' Release matrix leg, not
// locally, since local development defaults to Debug (see
// scripts/configure.sh) -- see docs/ARCHITECTURE_DECISION_RECORDS for
// the incident this header exists to prevent from recurring.
//
// Using OH_CHECK instead of assert() in test code fixes this at the
// root: the condition is always evaluated and always "uses" whatever
// variables it references, in every build configuration, so no
// [[maybe_unused]] annotations are needed and no future test can
// silently reintroduce this failure mode by following the same pattern
// everything else in the file already uses.
//
// Deliberately NOT a replacement for assert() outside test code --
// production code that wants a debug-only sanity check should keep
// using assert() (or OH_ASSERT from foundation/Assert.hpp); the whole
// point of OH_CHECK is that test assertions are not that kind of check.
#define OH_CHECK(condition)                                                          \
    do {                                                                             \
        if (!(condition)) {                                                          \
            std::fprintf(stderr, "OH_CHECK failed: %s\n  at %s:%d\n", #condition,    \
                          __FILE__, __LINE__);                                       \
            std::abort();                                                            \
        }                                                                            \
    } while (false)
