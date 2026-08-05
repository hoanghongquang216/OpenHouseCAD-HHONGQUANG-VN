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

**Depends on:** `GEOM-INTERSECTION-001` (see `docs/GEOM_BACKLOG.md`).

**Status:** Blocked by Geometry primitive -- not started until
`GEOM-INTERSECTION-001` merges.

**Current behaviour:** `document::FindSnapPoint()` (`Snap.hpp`)
recognizes only `Endpoint`, `Midpoint`, and `Center` -- see
SNAP-CORE-001 in `CHANGELOG.md`.

**Proposed behaviour:** For each pair of nearby entities, call
`geometry::Intersect(...)` (per `GEOM-INTERSECTION-001`) and offer
qualifying results as `Intersection`-typed `SnapCandidate`s, same
mechanism as the three kinds already shipped. Per
`docs/ARCHITECTURE_DECISION_RECORDS/ADR-0006-geometry-document-application-layering.md`,
this Sprint does **not** implement any geometric formula itself -- the
Line-Line/Line-Circle/Circle-Circle math and the Arc filtering via
`AngleOnArc` all live in `geometry`, per `GEOM-INTERSECTION-001`. This
Sprint's own scope is Document-layer composition only: which entity
pairs to check (nearby, per the existing per-entity iteration
`FindSnapPoint` already does), calling the Geometry Kernel primitive,
and wrapping results as `SnapCandidate`.

**Reason for deferring, not bundling into SNAP-CORE-001:** Endpoint/
Midpoint/Center are all "extract an already-known point from one
shape" -- geometrically trivial, almost entirely already available
from existing primitives (`Line2::Midpoint`, `Arc2::StartPoint`/
`EndPoint`/`Midpoint`, `.center`). Intersection needs a genuinely new
geometric capability (`GEOM-INTERSECTION-001`), which is why it was
split out as its own Geometry Kernel Sprint rather than Document-layer
work -- see ADR-0006 for the full reasoning (Editing's Trim/Extend/
Fillet need the same primitives, so they belong below Snap, not inside
it).

**Trigger to revisit:** `GEOM-INTERSECTION-001` merging. At that point
this Sprint should be small -- most of the formula risk this entry
originally worried about now lives (and is tested) in
`GEOM-INTERSECTION-001` instead.
