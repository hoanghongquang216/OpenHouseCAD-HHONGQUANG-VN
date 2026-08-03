# ADR-0005: Geometry Point and Vector Semantics

- Status: Accepted
- Date: 2026-08-01

## Naming note

This ADR was originally filed as `0001-geometry-point-vector-semantics.md`,
before this project's `ARCHITECTURE_DECISION_RECORDS` directory
standardized on the `ADR-000N-description.md` naming convention used by
every other ADR in this directory. Renumbered to `ADR-0005` (the next
available number at the time of standardization) and renamed to match,
rather than renumbering the existing `ADR-0001` through `ADR-0004` and
every place that cross-references them -- the content and original
2026-08-01 decision date are otherwise unchanged.

## Context

The geometry kernel needs simple coordinate primitives before it can represent
CAD entities. Treating positions and displacements as the same type permits
invalid operations, such as adding two positions, and makes later unit-aware
extensions harder.

## Problem

Choose the initial public model for two- and three-dimensional coordinates
without committing the kernel to a particular scalar precision or unit system.

## Options considered

1. Represent every coordinate as `Vector2` or `Vector3`.
2. Introduce distinct `Point2`/`Point3` and `Vector2`/`Vector3` types.
3. Introduce a fully unit-aware coordinate system immediately.

## Decision

Use distinct templated point and vector types parameterized by one scalar type
`T`. The initial API permits these operations:

- vector addition, subtraction, negation, scalar multiplication and division;
- dot product and squared length for both vector dimensions;
- 2D scalar cross product and 3D vector cross product;
- point plus or minus vector, and point minus point.

The API intentionally does not define point plus point. Unit types and
normalization are deferred until the Math and Geometry Kernel phases establish
their scalar, tolerance, and error-handling policies.

## Consequences

Callers get compile-time separation between positions and displacements while
retaining aggregate initialization and `constexpr` use. Generic `T` avoids a
premature choice of `float` or `double`, but callers are responsible for using
a scalar that supports the requested arithmetic.

## Implementation note (added after this ADR's original filing)

The implemented API additionally constrains `LengthSquared` (and `Dot`/
`Cross`) to integral-or-floating-point scalar types (not just floating
point) since squared length is exact for integers (no division or square
root involved) -- only `Length` (which needs a square root) is restricted
to floating-point `T`. This is a refinement of, not a deviation from, this
ADR's decision; recorded here rather than as a separate ADR since it is an
implementation detail within the scope this ADR already defines.

## Revisit criteria

Revisit this decision when introducing project-wide units, numeric tolerances,
affine transforms, or SIMD-backed storage.
