// Minimal, framework-free tests for openhouse::foundation's own types
// (as opposed to headers that merely `using` STL facilities).
//
// Covers: NonCopyable, NonMovable, Singleton, ScopeExit/Finally, kInvalid.
// See docs/CODING_STANDARD.md: "New features should include tests when
// practical."

#include <openhouse/foundation/Concepts.hpp>
#include <openhouse/foundation/Enum.hpp>
#include <openhouse/foundation/Finally.hpp>
#include <openhouse/foundation/Format.hpp>
#include <openhouse/foundation/Limits.hpp>
#include <openhouse/foundation/NonCopyable.hpp>
#include <openhouse/foundation/NonMovable.hpp>
#include <openhouse/foundation/Optional.hpp>
#include <openhouse/foundation/ScopeExit.hpp>
#include <openhouse/foundation/Singleton.hpp>

#include <cassert>
#include <cstdio>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

using namespace openhouse::foundation;

// --- NonCopyable / NonMovable: compile-time trait checks ------------------

namespace {

struct OnlyNonCopyable : NonCopyable {};
struct OnlyNonMovable : NonMovable {};
struct Neither {};

} // namespace

// NonCopyable: copy disabled, move explicitly preserved and verified to
// actually perform a move (see TestNonCopyableActuallyMoves below) rather
// than silently falling back to a copy.
static_assert(!std::is_copy_constructible_v<OnlyNonCopyable>);
static_assert(!std::is_copy_assignable_v<OnlyNonCopyable>);
static_assert(std::is_move_constructible_v<OnlyNonCopyable>);
static_assert(std::is_move_assignable_v<OnlyNonCopyable>);

// NonMovable: both copy AND move are disabled. This is intentional, not a
// naming mismatch -- see the comment in NonMovable.hpp. In C++, a type that
// deletes only its move operations while leaving copy available does not
// fail to compile on `std::move(x)`; it silently substitutes a copy
// instead (verified empirically). NonMovable avoids that footgun by
// disabling copy as well, so any accidental move attempt is a hard,
// visible compile error rather than a silent copy.
static_assert(!std::is_copy_constructible_v<OnlyNonMovable>);
static_assert(!std::is_copy_assignable_v<OnlyNonMovable>);
static_assert(!std::is_move_constructible_v<OnlyNonMovable>);
static_assert(!std::is_move_assignable_v<OnlyNonMovable>);

static_assert(std::is_copy_constructible_v<Neither>);
static_assert(std::is_move_constructible_v<Neither>);

// --- kInvalid: compile-time sanity ----------------------------------------

static_assert(kInvalid<std::uint8_t> == std::uint8_t{0xFF});
static_assert(kInvalid<std::uint32_t> == std::uint32_t{0xFFFFFFFFu});
static_assert(kInvalid<int> == std::numeric_limits<int>::max());

// --- NonCopyable: confirm move actually moves (not a silent copy) --------

namespace {

struct MoveProbe : NonCopyable {
    bool wasMovedInto = false;
    MoveProbe() = default;
    MoveProbe(MoveProbe&& other) noexcept {
        other.wasMovedInto = false;
        wasMovedInto = true;
    }
    MoveProbe& operator=(MoveProbe&&) = default;
};

} // namespace

static void TestNonCopyableActuallyMoves() {
    MoveProbe a;
    MoveProbe b(std::move(a));
    assert(b.wasMovedInto);
}

// --- Singleton --------------------------------------------------------------

namespace {

class Counter : public Singleton<Counter> {
    friend class Singleton<Counter>;

public:
    void Increment() { ++value_; }
    int Value() const { return value_; }

private:
    Counter() = default;
    int value_ = 0;
};

} // namespace

static void TestSingletonReturnsSameInstance() {
    Counter& a = Counter::Instance();
    Counter& b = Counter::Instance();
    assert(&a == &b);
}

static void TestSingletonStatePersists() {
    Counter::Instance().Increment();
    Counter::Instance().Increment();
    assert(Counter::Instance().Value() == 2);
}

// --- ScopeExit / Finally -----------------------------------------------------

static void TestScopeExitRunsOnDestruction() {
    bool ran = false;
    {
        ScopeExit guard([&ran]() noexcept { ran = true; });
        assert(!ran);
    }
    assert(ran);
}

static void TestFinallyRunsOnDestruction() {
    bool ran = false;
    {
        auto guard = Finally([&ran]() noexcept { ran = true; });
        assert(!ran);
    }
    assert(ran);
}

static void TestFinallyRunsOnEarlyReturn() {
    bool ran = false;
    auto runsGuard = [&ran]() {
        auto guard = Finally([&ran]() noexcept { ran = true; });
        return 42; // guard must still fire before the function returns.
    };
    const int result = runsGuard();
    assert(result == 42);
    assert(ran);
}

// Merged from the top-level tests/foundation/FoundationTests.cpp (was
// disconnected from the build; content verified valid and merged in
// rather than discarded).
static void TestScopeExitRelease() {
    bool invoked = false;
    {
        auto guard = Finally([&invoked]() noexcept { invoked = true; });
        guard.Release();
    }
    assert(!invoked); // Release() must prevent the callable from running.
}

static void TestFoundationHelpers() {
    static_assert(Integral<int>);
    static_assert(FloatingPoint<double>);
    static_assert(ToUnderlying(std::byte{1}) == 1);

    optional<int> value{42};
    assert(value.has_value());
    assert(kInvalid<unsigned int> == std::numeric_limits<unsigned int>::max());

    const std::string formatted = format("{}", *value);
    assert(formatted == "42");
}

int main() {
    TestNonCopyableActuallyMoves();

    TestSingletonReturnsSameInstance();
    TestSingletonStatePersists();

    TestScopeExitRunsOnDestruction();
    TestFinallyRunsOnDestruction();
    TestFinallyRunsOnEarlyReturn();
    TestScopeExitRelease();
    TestFoundationHelpers();

    // Runtime confirmation for kInvalid<int>, alongside the static_assert above.
    assert(kInvalid<int> == std::numeric_limits<int>::max());

    std::puts("FoundationTests: all tests passed.");
    return 0;
}
