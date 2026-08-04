# Snap Backlog

Deferred Snap-specific decisions and future work. Each entry: what's
deferred, why, and what would trigger picking it up. Mirrors
`docs/DXF_BACKLOG.md`'s own convention and reasoning (see that file's
intro) -- this is the second module-specific backlog file in the
project, which is itself worth noting: `DXF_BACKLOG.md`'s intro
anticipated exactly this ("a subdirectory implies grouping multiple
files, which isn't justified until a second module has a comparable
backlog to group alongside this one"). That trigger has now happened.
Whether to consolidate both under a `backlog/` subdirectory is a
documentation-organization decision worth making deliberately, on its
own, not folded into a feature Sprint -- same treatment already agreed
for the `ROADMAP_EXECUTION.md`/`CHANGELOG.md` Spiral-numbering
inconsistency found earlier. Left as an open observation here, not
acted on.

For decisions already made and shipped, see `CHANGELOG.md`. This file
is only for things NOT yet implemented.

---

## SNAP-INTERSECTION-001

**Title:** Intersection snap (the 4th `SnapType`, alongside the
`Endpoint`/`Midpoint`/`Center` shipped in SNAP-CORE-001).

**Priority:** Medium

**Status:** Deferred -- separate Sprint by design, needs its own
audit/probe before implementation

**Current behaviour:** `document::FindSnapPoint()` (`Snap.hpp`)
recognizes only `Endpoint`, `Midpoint`, and `Center` -- see
SNAP-CORE-001 in `CHANGELOG.md`. No intersection-point computation
exists anywhere in the codebase: `geometry::BoundingBox2::Intersects()`
only tests whether two bounding boxes overlap (a `bool`), not the
actual `(x, y)` point(s) where two curves cross, which is what an
Intersection snap needs.

**Proposed behaviour:** For each pair of nearby entities, compute their
actual geometric intersection point(s) and offer them as `Intersection`
-typed `SnapCandidate`s, same mechanism as the three kinds already
shipped. Needs up to 6 distinct geometric formulas (`Line-Line`,
`Line-Circle`, `Line-Arc`, `Circle-Circle`, `Circle-Arc`, `Arc-Arc`),
each with its own edge cases (0/1/2 solutions, tangency, coincident
circles, etc.) -- see the Snap audit (message history / any saved copy
of "Audit: Snap (Spiral 6)") for the full breakdown per pair.

**Reason for deferring, not bundling into SNAP-CORE-001:** Endpoint/
Midpoint/Center are all "extract an already-known point from one
shape" -- geometrically trivial, almost entirely already available
from existing primitives (`Line2::Midpoint`, `Arc2::StartPoint`/
`EndPoint`/`Midpoint`, `.center`). Intersection is a categorically
different kind of problem -- computing a NEW point that exists only in
the relationship between TWO shapes -- with meaningfully higher formula
risk. Per `docs/AI-Working-Agreement.md` Principle 2 (independent
verification for error-prone formulas -- the precedent being
`BulgeToArc` in DXF-002, checked against 20+ randomized cases before
integration), each of the up-to-6 intersection formulas here deserves
that same treatment individually, which is a substantially larger and
differently-shaped Sprint than SNAP-CORE-001 was.

**Trigger to revisit:** Whenever CAD-core Snap work resumes after
SNAP-CORE-001 -- this is understood to be next in line for that track,
not waiting on an external consumer the way e.g. `DXF-ROBUST-003b` is.
Before implementation starts, this Sprint should get its own audit/
probe phase (mirroring how SNAP-CORE-001 itself was scoped) to work
out each formula's approach and edge cases before writing code, per
the pattern that has worked for every Sprint so far.
