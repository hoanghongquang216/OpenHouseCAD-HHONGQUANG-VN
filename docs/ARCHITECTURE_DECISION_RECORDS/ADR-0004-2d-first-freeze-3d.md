# ADR-0004: 2D-First Scope for v1.x; Freeze 3D Development

## Context

Early in this project's development, the stated ambition was a full 3D
CAD kernel ("tham vọng kernel CAD 3D đầy đủ, dài hạn"). This shaped
several decisions: `Point3`/`Vector3` were built alongside their 2D
counterparts, `Matrix4` (a 4x4 affine transform matrix) was implemented
in the `math` module, and `Angle` was designed generically enough to
support 3D rotation use cases.

A subsequent repository audit found that
`Matrix4` and `Angle` had **no consumer anywhere in the codebase outside
their own test files** -- no demo, no vertical slice, nothing exercised
them end to end. This was flagged as a direct violation of this
project's own Development Spiral principle (see ADR discussions and
`docs/ROADMAP_EXECUTION.md`): building infrastructure speculatively,
ahead of any demonstrated need, is exactly the failure mode Spiral
development is meant to prevent.

Around the same time, a concrete, realistic roadmap was proposed (A1
through A10: Geometry Engine, CAD Document, File IO via DXF, Rendering,
Editing, Snapping, Dimensioning, Printing, Plugin API, Release) modeled
on real, comparable open-source prior art (LibreCAD, QCAD) rather than
on ambitious-but-unproven 3D kernel territory (OpenCASCADE-class
scope, discussed and set aside in ADR-0003). This roadmap is entirely
2D: DXF is fundamentally a 2D-first interchange format in practice for
this class of application, and every entity type it proposes (Line,
Circle, Arc, Polyline) is 2D.

## Problem

Continuing to treat 3D as an active, parallel target creates two costs:

1. **Effort spent maintaining/extending 3D infrastructure with no
   consumer** is effort not spent on the 2D path that has actual demos,
   actual tests exercising real composition (e.g. `RenderToSvg`,
   `Document::Bounds()`), and a concrete, achievable reference point
   (LibreCAD/QCAD-class software).
2. **Ambiguity about what "done" means for the geometry/math layers**:
   without an explicit scope decision, it's unclear whether e.g. a new
   geometry primitive should get both 2D and 3D variants, doubling
   review/test surface for capabilities nothing currently uses.

## Decision

**v1.x targets 2D CAD only.** Concretely:

- DXF is the primary interchange format (import in an earlier Spiral,
  export later -- see `docs/ROADMAP_EXECUTION.md`).
- The `geometry` module's active development is 2D-only: `Point2`,
  `Vector2`, `Line2`, `Circle2`, `Arc2`, `BoundingBox2`, and future 2D
  primitives (e.g. `Polyline2` when needed).
- 3D is explicitly out of scope for the v1.x roadmap. It is not
  abandoned -- see Freeze policy below -- but it is not an active
  target either.

**3D code (`Point3`, `Vector3`, `Matrix4`) is frozen:**

- Kept in the repository, kept building, kept passing its existing
  tests. Not deleted -- deletion would destroy history and the option to
  resume 3D work later without starting from scratch.
- No new API surface added to these types.
- No new tests added beyond what already exists.
- No new 3D algorithms (e.g. no 3D boolean operations, no 3D transform
  utilities beyond what `Matrix4` already has).
- Any future 3D work is a deliberate, separate decision (a new ADR),
  not an incremental drift back into parallel 2D/3D development.

Module status going forward: **Frozen (reserved for future 3D
support)** for the specific types `Point3`, `Vector3`, `Matrix4`. The
`math` and `geometry` modules themselves remain active (for their 2D
content); only these specific 3D-only types are frozen.

## Consequences

- Every new geometry/math primitive from this point forward should be
  built 2D-only unless a concrete 2D CAD feature genuinely requires a 3D
  counterpart (none currently do).
- `Matrix3` (a 2D affine transform matrix, added as part of this
  decision) is the actively developed transform type for 2D affine
  transforms (translation/rotation/scale of `Point2`/`Vector2`),
  matching what the current Spiral work (Document, Renderer) actually
  needs. `Matrix4` remains available if 3D work resumes, but is not
  extended to keep pace with `Matrix3`.
- Code review for new geometry/math work should treat "does this need a
  3D counterpart" as a question with a default answer of no, not yes --
  reversing the implicit assumption that shaped the original `Point3`/
  `Vector3`/`Matrix4`/`Angle` work.
- This ADR does not change anything about `Angle`: it remains 2D/3D-
  agnostic (an angle has no inherent dimensionality) and is actively
  used by 2D work (`Matrix3::Rotation`), so it is not frozen -- only the
  3D-specific types (`Point3`, `Vector3`, `Matrix4`) are.

## Revisit Criteria

Revisit this decision (a new ADR, not a silent resumption of 3D work)
if:
- A concrete 2D CAD feature is found to genuinely require 3D support
  (e.g. an "extrude to 3D preview" feature, which the original A1-A10
  roadmap does not currently include).
- v1.x (2D CAD, per this ADR's scope) reaches a stable release and the
  project deliberately chooses to pursue 3D as a v2.x direction.
