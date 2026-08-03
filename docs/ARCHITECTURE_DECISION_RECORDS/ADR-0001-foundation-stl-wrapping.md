# ADR-0001: Wrap Standard Library Facilities in `openhouse::foundation`

## Context

OpenHouseCAD is a C++23 project intended to grow into a full CAD kernel
(geometry, constraint solving, rendering, I/O). Every higher-level module
(`geometry`, and future modules such as `math`, `runtime`, `io`) needs a
common, dependable set of primitives: containers, smart pointers,
concurrency types, numeric helpers, and so on.

## Problem

Using `std::` facilities directly throughout the codebase works, but it
creates two long-term risks:

1. **No seam for future substitution.** If the project later needs to swap
   an implementation detail (e.g. a custom allocator-aware `vector`, a
   different `expected`-like type for platforms without full C++23 library
   support, or a hardened `assert`), every call site across all modules
   would need to change.
2. **Inconsistent vocabulary.** Without a single place that declares "these
   are the types and functions this codebase builds on," different modules
   can drift — some using `std::optional`, others rolling their own,
   with no shared convention.

## Options Considered

1. **Use `std::` directly everywhere.**
   - Pros: zero indirection, familiar to any C++ developer, no maintenance
     of wrapper headers.
   - Cons: no seam for substitution later; harder to enforce a curated,
     intentional subset of the standard library; no single place to document
     "this is what OpenHouseCAD is built on."

2. **Wrap STL facilities via `using` declarations in `openhouse::foundation`,
   one header per STL header (`Memory.hpp`, `Containers.hpp`, `Mutex.hpp`,
   etc.), aggregated by `Foundation.hpp`.**
   - Pros: single seam per facility if substitution is ever needed; each
     header documents and curates exactly which STL names are considered
     part of the project's vocabulary; keeps module boundaries loosely
     coupled per Engineering Principle 6; near-zero runtime cost since these
     are `using` aliases, not wrapper types, in almost all cases.
   - Cons: extra indirection layer to maintain; contributors must remember
     to add new STL facilities to the appropriate foundation header instead
     of including `<...>` directly; barrel header (`Foundation.hpp`) must be
     kept in sync with individual headers.

3. **Full custom implementations of core types (own `Vector`, own smart
   pointers, etc.) instead of wrapping `std::`.**
   - Pros: maximum control.
   - Cons: violates Engineering Principle 5 ("adapt rather than copy" mature
     open-source work); large maintenance burden with no clear benefit at
     this stage; standard library implementations are well-tested and
     optimized.

## Decision

Adopt **Option 2**: `openhouse::foundation` wraps curated STL facilities via
`using` declarations, split into focused headers (one STL header's worth of
concerns per file), aggregated through `Foundation.hpp`. Higher-level
modules (starting with `geometry`) depend on `OpenHouse::Foundation` and use
`openhouse::foundation::` names rather than `std::` directly where a
foundation alias exists.

Custom types are only introduced in `foundation` when the standard library
has no equivalent (e.g. `NonCopyable`, `NonMovable`, `Singleton`,
`ScopeExit`/`Finally`, `kInvalid`).

## Consequences

- Every new STL facility a module needs must first be added (or confirmed
  already present) in the appropriate `openhouse::foundation` header before
  use, keeping the vocabulary curated and centrally documented.
- `Foundation.hpp` must be kept exhaustive; a missing entry is a defect
  (see: initial version of `Foundation.hpp` omitted ~17 existing headers,
  corrected in the same sprint this ADR was written).
- If the project ever needs to substitute an implementation (custom
  allocator, hardened assert, freestanding-friendly alternative), the change
  is localized to a single foundation header rather than scattered across
  the codebase.
- Slight compile-time and cognitive overhead of an extra include layer,
  accepted as a worthwhile tradeoff for long-term flexibility per
  Principle 0 (optimize for long-term health over short-term speed).

## Revisit Criteria

Revisit this decision if:
- The wrapper headers consistently lag behind actual usage (i.e. developers
  routinely bypass `foundation` and include `<...>` directly), indicating
  the seam isn't earning its cost.
- A concrete substitution need never materializes after a significant
  portion of the codebase is built, suggesting Option 1 would have been
  sufficient.
