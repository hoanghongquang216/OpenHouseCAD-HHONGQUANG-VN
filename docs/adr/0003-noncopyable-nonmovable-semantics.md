# ADR 0003: NonCopyable and NonMovable Semantics

## Status
Accepted

## Context
OpenHouseCAD uses utility types in the foundation module to express ownership and lifetime constraints across the codebase.

During test implementation, two C++ special member function pitfalls were discovered:

1. A class that deletes copy construction/assignment without explicitly declaring move operations will not receive implicit move operations.
2. A design that allows copy while deleting move can create surprising behavior where `std::move()` silently selects copy operations.

## Findings

### Finding 1: NonCopyable must preserve move semantics

The original NonCopyable implementation deleted copy operations but did not explicitly enable move operations. This unintentionally prevented move construction and move assignment.

## Decision

`NonCopyable` means:

- Copy construction: disabled.
- Copy assignment: disabled.
- Move construction: enabled.
- Move assignment: enabled.

Implementation must explicitly default move operations.

## Finding 2: NonMovable must not allow copy fallback

A type that disables move but allows copy creates a dangerous semantic mismatch. Code using `std::move()` may silently invoke copying instead of failing compilation.

## Decision

`NonMovable` means:

- Copy construction: disabled.
- Copy assignment: disabled.
- Move construction: disabled.
- Move assignment: disabled.

This favors compile-time safety over attempting to support a rarely useful "copyable but not movable" pattern.

## Guidance

Use `NonCopyable` when a type should have unique ownership but may be transferred.

Use `NonMovable` when an object must remain at a fixed identity/location and neither copying nor moving is acceptable.

Avoid designing classes that are copyable but intentionally non-movable unless there is a documented exceptional reason.

## Consequences

Positive:

- Prevents accidental copies.
- Prevents hidden copy operations through `std::move()`.
- Makes ownership intent explicit.

Negative:

- Some uncommon use cases requiring copy-only semantics are not supported by these utilities.

## Revisit Criteria

Revisit this decision only if:

- A real production use case requires copy-only semantics.
- Compiler/library behavior changes materially.
- Ownership model requirements change.
