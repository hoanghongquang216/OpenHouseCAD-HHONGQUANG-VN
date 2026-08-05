# ADR-0006: Three-Layer Responsibility Split — Geometry Kernel / Document Services / Application

## Context

The `SNAP-INTERSECTION-001` audit (docs/SNAP_BACKLOG.md) found that
intersection computation (Line-Line, Line-Circle, Circle-Circle, plus
Line-Arc/Circle-Arc/Arc-Arc as thin filters over `AngleOnArc`) is not
actually a Snap-specific problem. It is a general geometric capability
that a second, already-planned consumer will also need: Epic 5 —
Editing (Trim, Extend, Offset, Fillet, Chamfer) is fundamentally built
on the same primitives (Trim/Extend resolve to "extend to intersection
with another entity"; Fillet resolves to a tangent-circle construction
in the same problem family as Circle-Circle).

This is the same pattern this project has already hit twice before,
informally, without naming it as a rule:
- `geometry::DistanceToShape()` (`HitTest.hpp`) returns a raw distance;
  `document::HitTest()` is the one that applies a `tolerance` and
  decides what counts as "hit."
- `geometry::AngleOnArc()` (`Bounds.hpp`) is a pure geometric predicate,
  reused unchanged by both `Bounds(Arc2)` and `DistanceToShape(Arc2,
  Point2)` — two different consumers, one primitive.

Until now this pattern existed by convention, not by a written rule —
nothing stopped a future Sprint from putting a geometric algorithm
directly inside `document` (or even `app`) "for now," the same drift
ADR-0004's Matrix4/Angle finding describes for a different kind of
scope creep.

## Problem

Without an explicit, named layering rule:
- It is not obvious, when starting a new Sprint, which module a new
  algorithm belongs in — `GEOM-INTERSECTION-001` could plausibly have
  been implemented directly inside `Snap.hpp`, and probably would have
  been, absent this ADR.
- A future contributor (or a differently-scoped AI session) has no
  single place to check "does this belong in Geometry or Document?"
  before writing code, and no vocabulary to describe the mistake if it
  happens (compare `DXF-LAYER-PROPS-003`'s finding: a `document::Entity`
  change that should have been scoped separately, caught only by
  accident during a different audit).
- Epic 5 (Editing) would be at real risk of either duplicating
  intersection logic that Epic 3 (Snap) already wrote inside `document`,
  or of `Editing` depending on `Snap` just to reach it — an inverted,
  accidental dependency neither Epic actually wants.

## Options Considered

1. **No formal rule; decide per-Sprint.** Rejected: this is exactly the
   status quo that let the question "where does Intersection belong"
   need a dedicated audit conversation to resolve. Doing that audit
   again for `GEOM-PROJECTION-001`, `GEOM-OFFSET-001`, etc., one Sprint
   at a time, is wasted repeated effort for a question with a single,
   reusable answer.
2. **Two layers only (`geometry`+`document` merged into one "core," vs.
   `app`).** Rejected: the existing module boundary between `geometry`
   and `document` is already real and already enforced by CMake
   (`document` -> `geometry`, not the reverse) and by the
   `math`/`document` non-dependency found during the
   `GEOM-INTERSECTION-001` audit. Collapsing it in documentation while
   the build graph keeps it separate would make the ADR describe a
   fiction.
3. **Three layers, named and scoped explicitly (chosen).** Matches the
   module graph that already exists (`foundation` -> `geometry` ->
   `document` -> `app`, per each module's own `CMakeLists.txt`) and
   gives it a name and a responsibility test, rather than leaving the
   boundary implicit.

## Decision

Three layers, in dependency order:

**Geometry Kernel** (`modules/geometry`, `modules/math`)
- Knows: `Point2`/`Vector2`, `Line2`, `Circle2`, `Arc2`, `BoundingBox2`,
  pure geometric predicates and constructions (`Bounds`, `AngleOnArc`,
  `DistanceToShape`, and — per `GEOM-INTERSECTION-001` — `Intersect`).
- Does NOT know: what a "layer" or "entity" is, what a "snap" or a
  "selection" is, tolerance-as-a-UI-concept (a *numerical* epsilon for
  classifying a discriminant near zero is still in scope here — see
  `GEOM-INTERSECTION-001` — but "is this within snapping distance of
  the cursor" is not).
- Returns raw, exact results (0/1/2 points, a distance, a bool) and
  leaves any judgment call about what to do with them to its callers.

**Document Services** (`modules/document`)
- Knows: `Layer`, `Entity`, `Document`, `HitTest`, `Snap`, `Selection`,
  `Command`/`CommandHistory`, and (once built) `Editing` operations
  (Trim/Extend/Offset/Fillet/Chamfer).
- Composes Geometry Kernel primitives against real document state
  (which layer is visible/locked, which entities exist) and applies
  the tolerance/nearest-match judgment calls Geometry Kernel
  deliberately does not make.
- Does NOT know: Qt, a cursor, a mouse event, a rubber-band selection
  rectangle, or any other UI-specific concept.

**Application** (`modules/app`)
- Knows: Qt6, mouse/keyboard input, cursor position, rubber-band
  selection, highlight rendering, dialogs, docking panels.
- Translates UI events into Document Services calls and Document
  Services results into what the user sees. Contains no geometry
  formulas and no document-model logic of its own.

**Test:** when starting a new Sprint, ask "does this algorithm need to
know about a `Document`/`Layer`/`Entity` to do its job?" If no, it
belongs in Geometry Kernel, even if today's only planned consumer is
Document Services (e.g. `Snap`). If it needs a `Document` but not a
mouse/cursor, it belongs in Document Services. If it needs Qt or user
input, it belongs in Application.

## Consequences

- `GEOM-INTERSECTION-001` (see `docs/GEOM_BACKLOG.md`) implements
  `Intersect(Line2, Line2)`, `Intersect(Line2, Circle2)`,
  `Intersect(Circle2, Circle2)` in `geometry`, plus
  `Intersect(Line2, Arc2)` / `Intersect(Circle2, Arc2)` /
  `Intersect(Arc2, Arc2)` as thin filters over the existing
  `AngleOnArc`. No mention of `Snap`, `Document`, or `Entity` anywhere
  in this Sprint's implementation.
- `SNAP-INTERSECTION-001` (see `docs/SNAP_BACKLOG.md`) becomes a
  Document Services consumer: iterate nearby entity pairs, call
  `geometry::Intersect(...)`, wrap qualifying results as
  `SnapCandidate{SnapType::Intersection, point}`. Depends on
  `GEOM-INTERSECTION-001`; cannot start before it merges.
  See `docs/SNAP_BACKLOG.md`'s updated entry for the explicit
  dependency/status note.
- Future Editing Sprints (Trim, Extend, Fillet, Chamfer) are expected
  to reuse the same `geometry::Intersect(...)` family directly, as a
  second, independent Document Services consumer — not by depending on
  `Snap`.
- A small, local numerical epsilon for classifying near-zero
  discriminants (tangency, near-parallel lines, near-concentric
  circles) lives inside `geometry` itself, not `math::Tolerance` —
  `document` and `math` remain non-dependent siblings (per the CMake
  graph found during the `GEOM-INTERSECTION-001` audit), and `geometry`
  already does not depend on `math` (see `Circle2.hpp`'s own comment,
  predating this ADR). This is a distinct concept from Document
  Services' UI-facing snap tolerance (`FindSnapPoint`'s `tolerance`
  parameter) and should not be unified with it.
- `docs/GEOM_BACKLOG.md` (new file, mirroring `DXF_BACKLOG.md`/
  `SNAP_BACKLOG.md`'s existing convention) is the home for future
  Geometry Kernel work items (e.g. `GEOM-PROJECTION-001`,
  `GEOM-OFFSET-001`, `GEOM-DISTANCE-001`) — not `SNAP_BACKLOG.md`,
  even when Snap is the first or only current consumer motivating the
  item.
- This project now has three module-specific backlog files
  (`DXF_BACKLOG.md`, `SNAP_BACKLOG.md`, `GEOM_BACKLOG.md`). Whether to
  consolidate under a `backlog/` subdirectory remains the same
  deliberately-deferred, separate documentation-organization decision
  `SNAP_BACKLOG.md`'s own intro already flagged when the second file
  was created — not resolved by this ADR.

## Revisit Criteria

Revisit this decision (a new ADR, not silent drift) if:
- A capability is found that genuinely does not fit any of the three
  layers cleanly (e.g. something that needs both `Document` state and
  raw Qt access at once, in a way that resists factoring into "compute
  in Document Services, render in Application").
- The project takes on a second Application front end (e.g. a headless
  batch tool, or a web UI) where Document Services needs to serve two
  different Application layers — this ADR's boundary already supports
  that (Document Services knows nothing about Qt specifically), but it
  is worth confirming explicitly if/when it actually happens.
