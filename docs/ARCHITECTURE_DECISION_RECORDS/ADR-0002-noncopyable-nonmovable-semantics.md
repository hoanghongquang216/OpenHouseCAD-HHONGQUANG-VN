# ADR-0002: NonCopyable Preserves Move; NonMovable Disables Copy Too

## Context

`openhouse::foundation` provides `NonCopyable` and `NonMovable` as mixin
base classes for types that should not support one or both of copy/move
semantics (e.g. RAII handles, singletons, mutex-guarding types).

While writing tests for these types (`modules/foundation/tests/
FoundationTests.cpp`), empirical compilation checks against GCC 13.3
(`-std=c++23`) surfaced two subtle, non-obvious C++ special-member-function
interactions that the original implementation did not account for.

## Problem

**Finding 1 — `NonCopyable` silently disabled move too.**

The original `NonCopyable` declared only:
```cpp
NonCopyable(const NonCopyable&) = delete;
NonCopyable& operator=(const NonCopyable&) = delete;
```

Per `[class.copy.ctor]`, user-declaring a copy constructor/assignment
operator (even as `=delete`) suppresses the *implicit generation* of the
move constructor/assignment operator entirely — they are not "deleted",
they simply do not exist. Move-construction then falls back to trying the
(deleted) copy constructor, which fails. Verified with GCC 13.3:

```cpp
struct A : NonCopyable {};
static_assert(std::is_move_constructible_v<A>); // FAILED before the fix
```

This defeats the evident intent of the name `NonCopyable` — a type that
should still be movable (analogous to `std::unique_ptr`) was silently
immovable as well.

**Finding 2 — the "obvious" fix for `NonMovable` is a worse, silent bug.**

The symmetric-looking fix — explicitly default the copy operations on
`NonMovable` so only move is disabled — was tried and reverted after
empirical testing revealed a more dangerous problem: a type that deletes
only its move constructor/assignment while an accessible copy constructor
exists does not fail to compile when moved. `std::move(x)` silently
resolves to the copy constructor instead, with **no compiler diagnostic**.
Verified with GCC 13.3 by instrumenting each special member with a
`printf`:

```cpp
struct NonMovableFixed { /* copy = default, move = delete */ };
struct D : NonMovableFixed {};
D a;
D b(std::move(a)); // compiles; silently invokes the COPY constructor
```

This is strictly worse than a compile error: code that assumes
`std::move(x)` either moves or fails loudly would instead get a silent,
unintended deep copy, with correctness and performance implications that
are easy to miss in review.

## Options Considered

1. **Leave both classes as originally written** (copy-only deletion on
   `NonCopyable`, move-only deletion on `NonMovable`).
   - Pros: no change needed.
   - Cons: `NonCopyable` silently forbids move, contradicting its name and
     its most common real-world use case (RAII handles that must move but
     never copy).

2. **Fix both symmetrically**: explicitly default the "other" operation on
   each class, so `NonCopyable` = copy-disabled/move-enabled and
   `NonMovable` = move-disabled/copy-enabled.
   - Pros: names match behavior exactly; simple mental model.
   - Cons: the `NonMovable` half introduces the silent-copy-on-move footgun
     described above (Finding 2). This is worse than either leaving it
     broken or over-restricting it, because it fails silently rather than
     loudly.

3. **Fix `NonCopyable` to preserve move (as in Option 2); keep `NonMovable`
   disabling both copy and move**, with the tradeoff documented in the
   header.
   - Pros: `NonCopyable` now correctly matches its name and its safe,
     well-defined direction (copy disabled, move preserved — no fallback
     footgun possible, since copy being deleted forecloses the silent
     substitution). `NonMovable` remains maximally restrictive but safe:
     any attempt to copy or move is a hard compile error, never a silent
     behavior change. No caller is ever surprised at runtime.
   - Cons: `NonMovable` does not, in the strict sense, permit copying —
     the name promises less than it technically could in principle, but
     "could in principle" is exactly the unsafe direction per Finding 2.

## Decision

Adopt **Option 3**:

- `NonCopyable`: copy constructor/assignment deleted; move constructor/
  assignment explicitly defaulted. Verified movable via both
  `static_assert(std::is_move_constructible_v<...>)` and a runtime test
  that instruments the move constructor to confirm it actually executes
  (`TestNonCopyableActuallyMoves` in `FoundationTests.cpp`).
- `NonMovable`: move constructor/assignment deleted; copy constructor/
  assignment left non-declared (not explicitly defaulted), so both copy
  and move are disabled for any derived type. The header comment explains
  this is intentional, not an oversight, and points to `NonCopyable` for
  the "disable one, keep the other" use case, since only the
  copy-disabled/move-enabled direction can be expressed safely in C++.

Guidance for future code in this repository: prefer `NonCopyable` whenever
a type needs "not copyable" semantics and could plausibly be moved (RAII
handles, unique-ownership types). Treat "copyable but not movable" as
effectively unsupportable in this codebase; if a genuine need for it
surfaces, revisit this ADR rather than re-attempting Option 2's approach
ad hoc.

## Consequences

- `openhouse::foundation::NonCopyable`-derived types behave as most C++
  developers would expect from the name (copy disabled, move enabled),
  removing a footgun that would otherwise silently break move-only usage
  patterns (e.g. storing such types in a `std::vector` that needs to
  reallocate, or returning them by value under RVO/move).
- `openhouse::foundation::NonMovable`-derived types are stricter than the
  name alone implies (copy also disabled). This is documented at the
  point of use (`NonMovable.hpp`) so it is discoverable without reading
  this ADR, but contributors reaching for `NonMovable` expecting a
  copyable type should read the header comment or this ADR.
- `Singleton<T>` (which privately inherits `NonCopyable`) is unaffected in
  practice, since singletons are accessed by reference and never
  constructed by value; the fix does not change its observable behavior,
  but does correctly reflect the type's real capabilities.
- Test coverage in `FoundationTests.cpp` now asserts on the *actual*
  copy/move-constructibility traits (`std::is_copy_constructible_v`,
  `std::is_move_constructible_v`, etc.) rather than assuming them, and
  includes a runtime check that a "move" genuinely invokes the move path
  rather than silently copying — guarding against regression of Finding 2
  if `NonMovable` is ever touched again.

## Revisit Criteria

Revisit this decision if:
- A concrete, justified need arises for a "copyable but not movable" type
  in this codebase. Re-evaluate whether newer language facilities or a
  different pattern (e.g. a non-inheritable trait check, or explicitly
  deleting/defining all four operations on the derived type itself rather
  than via a shared base) can express that safely.
- Static analysis or a newer compiler version changes the observed
  behavior described in Finding 2 (i.e. if `-Wall -Wextra` or a sanitizer
  starts reliably flagging the silent-copy-on-move pattern at the call
  site), which could make Option 2 viable with a compile-time safety net.
