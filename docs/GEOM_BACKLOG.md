# Geometry Kernel Backlog

Deferred `geometry`-module work: pure geometric primitives and
predicates that belong in the Geometry Kernel layer (see
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0006-geometry-document-application-layering.md`),
regardless of which Document Services feature (Snap, Editing, ...) is
the first or current consumer motivating the item. Mirrors
`docs/DXF_BACKLOG.md`/`docs/SNAP_BACKLOG.md`'s own convention (each
entry: what's deferred, why, and what would trigger picking it up) --
this is the third module-specific backlog file, which `SNAP_BACKLOG.md`
already flagged as the point where a `backlog/` subdirectory becomes
worth considering; still deliberately left as an open observation, not
acted on here either.

For decisions already made and shipped, see `CHANGELOG.md`. This file
is only for things NOT yet implemented.

---

## GEOM-INTERSECTION-001

**Title:** Intersection primitives for `Line2`, `Circle2`, `Arc2` --
`geometry::Intersect(...)` overloads returning the actual `(x, y)`
point(s) where two shapes cross.

**Priority:** Medium (unblocks `SNAP-INTERSECTION-001`, see
`docs/SNAP_BACKLOG.md`; also a direct prerequisite for the planned
Epic 5 Editing operations -- Trim, Extend, Fillet -- per ADR-0006)

**Status:** Shipped -- see `CHANGELOG.md`'s `geom-intersection-001`
entry. `docs/SNAP_BACKLOG.md`'s `SNAP-INTERSECTION-001` entry is
unblocked as of this merge.

**Current behaviour:** No intersection-point computation exists
anywhere in `geometry`. `BoundingBox2::Intersects()` only tests whether
two bounding boxes overlap (a `bool`), not an actual crossing point.

**Proposed behaviour:** Three new formulas, plus three thin filters
reusing the existing `AngleOnArc` (`Bounds.hpp`):

- `Intersect(Line2, Line2)` -- bounded-segment intersection (not
  infinite-line): must check both segments' parametric `t`/`s` in
  `[0, 1]`, not just solve the 2-line-equation system. Degenerate
  cases: parallel (0 solutions), collinear-overlapping (returns
  `std::nullopt` -- not a well-defined single point; see Reason below).
- `Intersect(Line2, Circle2)` -- quadratic in the segment's parametric
  `t`; both roots must be re-checked against `[0, 1]` (the infinite
  line can cross the circle twice while the segment covers zero, one,
  or both crossings). 0/1(tangent)/2 solutions.
- `Intersect(Circle2, Circle2)` -- classic two-circle intersection.
  0/1(tangent, internal or external)/2 solutions; concentric
  (same-radius: coincident, `nullopt`; different-radius: 0) is its own
  explicit branch, not left to fall out of the general formula's
  floating-point behavior.
- `Intersect(Line2, Arc2)` = `Intersect(Line2, Circle2)` against the
  arc's underlying circle, filtered by `AngleOnArc`.
- `Intersect(Circle2, Arc2)` = `Intersect(Circle2, Circle2)`, filtered
  by `AngleOnArc`.
- `Intersect(Arc2, Arc2)` = `Intersect(Circle2, Circle2)`, filtered by
  `AngleOnArc` on *both* arcs.

Return type: not yet decided between the entries -- likely something
like a small fixed-capacity result (0/1/2 points) rather than
`std::vector`, to avoid an allocation for what's always at most 2
points; exact shape is an implementation-time decision, not a
scope-changing one.

**Reason for deferring, not implementing directly inside
`Snap.hpp`:** See ADR-0006. Intersection is a general geometric
capability with (at least) two independent Document Services
consumers -- `Snap` (this backlog's original motivation) and the
planned Editing Epic (Trim/Extend/Fillet). Building it as
`document`-layer code (inside `Snap.hpp`, as `SNAP-INTERSECTION-001`
was originally scoped) would either force Editing to duplicate the
same formulas later, or to depend on `Snap` just to reach them --
neither of which the module graph should allow.

**Numerical note:** classifying "tangent" (discriminant exactly 0) is a
measure-zero case in floating point -- a small, local epsilon lives
inside `geometry` for this (see ADR-0006's Consequences), distinct
from `math::Tolerance`/`NearlyEqual` (which `geometry` does not and
should not depend on -- see `Circle2.hpp`'s own comment) and distinct
from Document Services' UI-facing snap tolerance
(`FindSnapPoint`'s `tolerance` parameter).

**Trigger to revisit:** N/A -- this is the next Sprint, not deferred
pending a trigger. Retained in backlog-entry format only because that
format (what/why/proposed behaviour) is the useful one to carry into
implementation, not because this item is actually being put off.

---
